#include "Render/Importers/SceneImporter.h"
#include "Assets/AssetManager.h"
#include "Core/HashMap.h"
#include "IO/FileHandle.h"
#include "Render/Importers/TextureImporter.h"
#include "Utils/DebugTimer.h"
#include <assimp/Importer.hpp>
#include <assimp/pbrmaterial.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace
{

enum class ImportMode
{
    Default,
    OBJ,
    GLTF2,
};

enum class TextureType
{
    BaseColor,
    Specular,
    Emissive,
    Normal,
    Occlusion,
};

struct TextureMapping
{
    aiTextureType aType;
    TextureType textureType;
};

static const Array<TextureMapping> textureMappings[3] = {
    //Default
    {
        { aiTextureType_DIFFUSE, TextureType::BaseColor },
        { aiTextureType_SPECULAR, TextureType::Specular },
        { aiTextureType_EMISSIVE, TextureType::Emissive },
        { aiTextureType_NORMALS, TextureType::Normal },
        { aiTextureType_AMBIENT, TextureType::Occlusion },
    },
    //OBJ
    {
        { aiTextureType_DIFFUSE, TextureType::BaseColor },
        { aiTextureType_SPECULAR, TextureType::Specular },
        { aiTextureType_EMISSIVE, TextureType::Emissive },
        { aiTextureType_AMBIENT, TextureType::Occlusion },
        { aiTextureType_HEIGHT, TextureType::Normal },
        { aiTextureType_DISPLACEMENT, TextureType::Normal },
    },
    //GLTF2
    {
        { aiTextureType_DIFFUSE, TextureType::BaseColor },
        { aiTextureType_EMISSIVE, TextureType::Emissive },
        { aiTextureType_NORMALS, TextureType::Normal },
        { aiTextureType_AMBIENT, TextureType::Occlusion },
        //TODO
        { aiTextureType_SPECULAR, TextureType::Specular },
        //{ AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, TextureType::Specular },
    }
};

bool IsSrgbRequired(TextureType textureType)
{
    switch (textureType)
    {
    case TextureType::BaseColor:
    case TextureType::Emissive:
    case TextureType::Occlusion:
        return true;
    default:
        return false;
    }
}

String ToString(const aiString &aStr)
{
    return String(aStr.C_Str());
}

Vector3 ToVector3(const aiVector3D &aVec)
{
    return Vector3(aVec.x, aVec.y, aVec.z);
}

Vector2 ToVector2(const aiVector3D &aVec)
{
    return Vector2(aVec.x, aVec.y);
}

Quat ToQuat(const aiQuaternion &aQuat)
{
    return Quat(aQuat.x, aQuat.y, aQuat.z, aQuat.w);
}

Color ToColor(const aiColor3D &aCol)
{
    return Color(aCol.r, aCol.g, aCol.b);
}

Matrix4 ToMatrix4(const aiMatrix4x4 &aMat)
{
    return Matrix4(
        aMat.a1, aMat.a2, aMat.a3, aMat.a4,
        aMat.b1, aMat.b2, aMat.b3, aMat.b4,
        aMat.c1, aMat.c2, aMat.c3, aMat.c4,
        aMat.d1, aMat.d2, aMat.d3, aMat.d4);
}

float SpecPowerToRoughness(float specPower)
{
    return Math::Clamp01(Math::Sqrt(2.0f / (specPower + 2.0f)));
}

class ImporterImpl
{
public:
    ImporterImpl(const APtr<Scene> &asset, const SPtr<SceneImportSettings> &settings)
        : asset(asset), settings(settings)
    {
    }

    void Import(const String &path)
    {
        auto fileHandle = IO::FileHandle(path);
        directory = fileHandle.GetParentPath();

        Assimp::Importer aImporter;

        uint32 assimpFlags = aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_FlipUVs;
        assimpFlags &= ~(aiProcess_FindDegenerates);
        assimpFlags &= ~(aiProcess_OptimizeGraph);
        assimpFlags &= ~aiProcess_RemoveRedundantMaterials;

        {
            DebugTimer timer(CT_TEXT("Assimp ReadFile"));
            aScene = aImporter.ReadFile(CT_U8_CSTR(path), assimpFlags);
            if (aScene == nullptr || aScene->mFlags == AI_SCENE_FLAGS_INCOMPLETE)
            {
                CT_LOG(Error, CT_TEXT("Load scene failed, error: {0}."), String(aImporter.GetErrorString()));
                return;
            }
        }

        auto extension = fileHandle.GetExtension();
        if (extension == CT_TEXT(".obj"))
            importMode = ImportMode::OBJ;
        else if (extension == CT_TEXT(".gltf") || extension == CT_TEXT(".glb"))
            importMode = ImportMode::GLTF2;

        {
            DebugTimer timer(CT_TEXT("CreateMaterials"));
            if (CreateMaterials() == false)
            {
                CT_LOG(Error, CT_TEXT("Create scene materials failed."));
                return;
            }
        }

        {
            DebugTimer timer(CT_TEXT("CreateSceneGraph"));
            if (CreateSceneGraph() == false)
            {
                CT_LOG(Error, CT_TEXT("Create scene graph failed."));
                return;
            }
        }

        {
            DebugTimer timer(CT_TEXT("CreateMeshes"));
            if (CreateMeshes() == false)
            {
                CT_LOG(Error, CT_TEXT("Create scene meshes failed."), path);
                return;
            }
        }

        {
            DebugTimer timer(CT_TEXT("CreateAnimations"));
            if (CreateAnimations() == false)
            {
                CT_LOG(Error, CT_TEXT("Create scene animations failed."), path);
                return;
            }
        }

        {
            DebugTimer timer(CT_TEXT("CreateCamera"));
            if (CreateCamera() == false)
            {
                CT_LOG(Error, CT_TEXT("Create scene camera failed."), path);
                return;
            }
        }

        {
            DebugTimer timer(CT_TEXT("CreateLights"));
            if (CreateLights() == false)
            {
                CT_LOG(Error, CT_TEXT("Create scene lights failed."), path);
                return;
            }
        }

        gAssetManager->RunMainthread([asset = this->asset, builder = std::move(this->builder)]() mutable {
            DebugTimer timer(CT_TEXT("SceneBuilder"));
            asset.GetData()->ptr = builder.GetScene();
        });
    }

    bool IsBone(const String &name)
    {
        return boneMatrixMap.Contains(name);
    }

    bool IsBone(const aiString &name)
    {
        return boneMatrixMap.Contains(ToString(name));
    }

    int32 GetNodeID(const aiNode *aNode)
    {
        if (!aNode)
            return -1;
        auto ptr = nodePtrToID.TryGet(aNode);
        if (!ptr)
            return -1;
        return *ptr;
    }

    int32 GetNodeID(const String &name, int32 index)
    {
        auto ptr = nodeNameToPtrs.TryGet(name);
        if (!ptr)
            return -1;
        auto &nodes = *ptr;
        if (index < 0 || index >= nodes.Count())
            return -1;
        return GetNodeID(nodes[index]);
    }

    int32 GetNodeCount(const String &name)
    {
        auto ptr = nodeNameToPtrs.TryGet(name);
        if (!ptr)
            return 0;
        auto &nodes = *ptr;
        return nodes.Count();
    }

    void SetTexture(const SPtr<Material> &mat, const APtr<Texture> &texture, TextureType textureType)
    {
        switch (textureType)
        {
        case TextureType::BaseColor:
            mat->SetBaseTexture(texture);
            break;
        case TextureType::Specular:
            mat->SetSpecularTexture(texture);
            break;
        case TextureType::Emissive:
            mat->SetEmissiveTexture(texture);
            break;
        case TextureType::Normal:
            mat->SetNormalTexture(texture);
            break;
        case TextureType::Occlusion:
            mat->SetOcclusionTexture(texture);
            break;
        }
    }

    void LoadTextures(const SPtr<Material> &mat, const aiMaterial *aMat)
    {
        bool useSrgb = !settings->assumeLinearSpaceTextures;

        for (const auto &e : textureMappings[(int32)importMode])
        {
            if (aMat->GetTextureCount(e.aType) > 0)
            {
                aiString aPath;
                aMat->GetTexture(e.aType, 0, &aPath);

                String path = String(aPath.data);
                if (path.IsEmpty())
                {
                    CT_LOG(Warning, CT_TEXT("Load scene texture with empty file name, ignored."));
                    continue;
                }

                APtr<Texture> texture;
                auto texPtr = textureCache.TryGet(path);
                if (texPtr)
                {
                    texture = *texPtr;
                }
                else
                {
                    auto textureSettings = TextureImportSettings::Create();
                    textureSettings->generateMips = true;
                    textureSettings->srgbFormat = useSrgb && IsSrgbRequired(e.textureType);
                    TextureImporter importer;

                    // Embedded texture
                    if (path.StartsWith(CT_TEXT("*")))
                    {
                        int32 i = StringConvert::ParseInt32(path.Substring(1));
                        const auto aTex = aScene->mTextures[i];
                        if (aTex->mHeight == 0)
                        {
                            Array<uint8> bytes;
                            bytes.AddUninitialized(aTex->mWidth);
                            std::memcpy(bytes.GetData(), aTex->pcData, aTex->mWidth);
                            texture = importer.ImportFromMemory(std::move(bytes), settings);
                        }
                        else
                        {
                            CT_EXCEPTION(Render, "Not implement");
                            //TODO Create texture in main thread directly
                            //ResourceFormat format = ResourceFormat::RGBA8Unorm;
                        }
                    }
                    else
                    {
                        String fullPath = directory + CT_TEXT("/") + path;
                        texture = importer.Import(fullPath, textureSettings);
                    }

                    textureCache.Put(path, texture);
                }

                SetTexture(mat, texture, e.textureType);
            }
        }
    }

    bool CreateMaterial(const aiMaterial *aMat)
    {
        aiString aName;
        aMat->Get(AI_MATKEY_NAME, aName);
        String name = ToString(aName);

        auto mat = Material::Create();
        mat->SetName(name);

        int32 shadingModel = settings->shadingModel;
        if (shadingModel == -1)
        {
            shadingModel = (importMode == ImportMode::OBJ ? CT_SHADING_MODEL_SPEC_GLOSS : CT_SHADING_MODEL_METAL_ROUGH);
        }
        mat->SetShadingModel(shadingModel);

        LoadTextures(mat, aMat);

        float opacity;
        if (aMat->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS)
        {
            auto baseColor = mat->GetBaseColor();
            baseColor.a = opacity;
            mat->SetBaseColor(baseColor);
        }

        float bumpScaling;
        if (aMat->Get(AI_MATKEY_BUMPSCALING, bumpScaling) == AI_SUCCESS)
        {
            //TODO
        }

        float shininess;
        if (aMat->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS)
        {
            if (importMode == ImportMode::OBJ)
            {
                float roughness = SpecPowerToRoughness(shininess);
                shininess = 1.0f - roughness;
            }
            auto specular = mat->GetSpecularColor();
            specular.a = shininess;
            mat->SetSpecularColor(specular);
        }

        float refraction;
        if (aMat->Get(AI_MATKEY_REFRACTI, refraction) == AI_SUCCESS)
        {
            mat->SetIndexOfRefraction(refraction);
        }

        aiColor3D aColor;
        if (aMat->Get(AI_MATKEY_COLOR_DIFFUSE, aColor) == AI_SUCCESS)
        {
            Color color(aColor.r, aColor.g, aColor.b, mat->GetBaseColor().a);
            mat->SetBaseColor(color);
        }

        if (aMat->Get(AI_MATKEY_COLOR_SPECULAR, aColor) == AI_SUCCESS)
        {
            Color color(aColor.r, aColor.g, aColor.b, mat->GetSpecularColor().a);
            mat->SetSpecularColor(color);
        }

        if (aMat->Get(AI_MATKEY_COLOR_EMISSIVE, aColor) == AI_SUCCESS)
        {
            Color color(aColor.r, aColor.g, aColor.b, mat->GetEmissiveColor().a);
            mat->SetEmissiveColor(color);
        }

        int doubleSided;
        if (aMat->Get(AI_MATKEY_TWOSIDED, doubleSided) == AI_SUCCESS)
        {
            mat->SetDoubleSided(doubleSided != 0);
        }

        if (importMode == ImportMode::GLTF2)
        {
            if (aMat->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR, aColor) == AI_SUCCESS)
            {
                Color color(aColor.r, aColor.g, aColor.b, mat->GetBaseColor().a);
                mat->SetBaseColor(color);
            }

            Color specColor = mat->GetSpecularColor();
            float metallic;
            if (aMat->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR, metallic) == AI_SUCCESS)
            {
                specColor.b = metallic;
            }
            float roughness;
            if (aMat->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS)
            {
                specColor.g = roughness;
            }
            mat->SetSpecularColor(specColor);
        }

        //TODO Parse name

        if (opacity < 1.0f)
        {
            mat->SetSpecularTransmission(1.0f - opacity);
            mat->SetDoubleSided(true);
        }

        materials.Add(mat);
        return true;
    }

    bool CreateMaterials()
    {
        for (uint32 i = 0; i < aScene->mNumMaterials; ++i)
        {
            if (!CreateMaterial(aScene->mMaterials[i]))
                return false;
        }
        return true;
    }

    void CreateBoneList()
    {
        for (uint32 i = 0; i < aScene->mNumMeshes; ++i)
        {
            const auto aMesh = aScene->mMeshes[i];
            if (!aMesh->HasBones())
                continue;

            for (uint32 b = 0; b < aMesh->mNumBones; ++b)
            {
                String name = ToString(aMesh->mBones[b]->mName);
                boneMatrixMap.Put(name, ToMatrix4(aMesh->mBones[b]->mOffsetMatrix));
            }
        }
    }

    bool ParseNode(const aiNode *aNode)
    {
        SceneNode node = {};

        String name = ToString(aNode->mName);
        const bool boneNode = IsBone(name);
        CT_CHECK(!boneNode || aNode->mNumMeshes == 0);

        node.name = name;
        node.transform = ToMatrix4(aNode->mTransformation);
        node.parent = GetNodeID(aNode->mParent);
        if (boneNode)
            node.localToBindPose = boneMatrixMap[name];

        int32 nodeID = builder.AddNode(std::move(node));
        nodePtrToID.Put(aNode, nodeID);
        auto nodesPtr = nodeNameToPtrs.TryGet(name);
        if (!nodesPtr)
            nodeNameToPtrs.Put(name, {});
        nodeNameToPtrs[name].Add(aNode);

        bool ret = true;
        for (uint32 i = 0; i < aNode->mNumChildren; ++i)
            ret |= ParseNode(aNode->mChildren[i]);
        return ret;
    }

    bool CreateSceneGraph()
    {
        CreateBoneList();
        auto root = aScene->mRootNode;
        CT_CHECK(!IsBone(root->mName));
        bool ret = ParseNode(root);

        //TODO Dump

        return ret;
    }

    void LoadBones(const aiMesh *aMesh, Mesh &mesh)
    {
        const auto vertexCount = aMesh->mNumVertices;
        mesh.boneIDs.SetCount(vertexCount);
        mesh.boneWeights.SetCount(vertexCount);

        for (uint32 b = 0; b < aMesh->mNumBones; ++b)
        {
            const auto aBone = aMesh->mBones[b];
            String name = ToString(aBone->mName);
            CT_CHECK(GetNodeCount(name) == 1);
            int32 boneID = GetNodeID(name, 0);
            for (uint32 w = 0; w < aBone->mNumWeights; ++w)
            {
                const auto &aWeight = aBone->mWeights[w];
                auto &vertexIDs = mesh.boneIDs[aWeight.mVertexId];
                auto &vertexWeights = mesh.boneWeights[aWeight.mVertexId];

                bool found = false;
                for (uint32 i = 0; i < 4; ++i)
                {
                    if (vertexWeights[i] == 0.0f)
                    {
                        vertexIDs[i] = boneID;
                        vertexWeights[i] = aWeight.mWeight;
                        found = true;
                        break;
                    }
                }
                if (!found)
                    CT_LOG(Error, CT_TEXT("Create scene bones error, one vertex has more than 4 bones."));
            }
        }

        for (auto &e : mesh.boneWeights)
        {
            float sum = e.x + e.y + e.z + e.w;
            e /= sum;
        }
    }

    bool CreateMesh(const aiMesh *aMesh, uint32 index)
    {
        Mesh mesh = {};

        const uint32 perFaceIndexCount = aMesh->mFaces[0].mNumIndices;
        const uint32 indexCount = aMesh->mNumFaces * perFaceIndexCount;
        mesh.indices.Reserve(indexCount);
        for (uint32 i = 0; i < aMesh->mNumFaces; ++i)
        {
            CT_CHECK(aMesh->mFaces[i].mNumIndices == perFaceIndexCount); //If mesh contains more than one primitive, use aiProcess_SortByPType
            for (uint32 j = 0; j < perFaceIndexCount; ++j)
            {
                mesh.indices.Add(aMesh->mFaces[i].mIndices[j]);
            }
        }

        const auto vertexCount = aMesh->mNumVertices;
        mesh.positions.Reserve(vertexCount);
        mesh.normals.Reserve(vertexCount);
        mesh.bitangents.Reserve(vertexCount);
        mesh.uvs.Reserve(vertexCount);
        for (uint32 i = 0; i < aMesh->mNumVertices; ++i)
        {
            mesh.positions.Add(ToVector3(aMesh->mVertices[i]));
            mesh.normals.Add(ToVector3(aMesh->mNormals[i]));
            mesh.bitangents.Add(ToVector3(aMesh->mBitangents[i]));

            if (aMesh->HasTextureCoords(0))
                mesh.uvs.Add(ToVector2(aMesh->mTextureCoords[0][i]));
            else
                mesh.uvs.Add(Vector2());
        }

        if (aMesh->HasBones() && !settings->dontLoadBones)
        {
            LoadBones(aMesh, mesh);
        }

        switch (aMesh->mFaces[0].mNumIndices)
        {
        case 1:
            mesh.topology = Topology::PointList;
            break;
        case 2:
            mesh.topology = Topology::LineList;
            break;
        case 3:
            mesh.topology = Topology::TriangleList;
            break;
        }
        CT_CHECK(mesh.topology != Topology::Undefined);

        mesh.material = materials[aMesh->mMaterialIndex];

        int32 meshID = builder.AddMesh(std::move(mesh));
        meshIndexToID.Put(index, meshID);

        return true;
    }

    void AddMeshes(const aiNode *aNode)
    {
        int32 nodeID = GetNodeID(aNode);
        for (uint32 i = 0; i < aNode->mNumMeshes; ++i)
        {
            int32 meshID = meshIndexToID[aNode->mMeshes[i]];
            builder.AddMeshInstance(nodeID, meshID);
        }

        for (uint32 i = 0; i < aNode->mNumChildren; ++i)
        {
            AddMeshes(aNode->mChildren[i]);
        }
    }

    bool CreateMeshes()
    {
        for (uint32 i = 0; i < aScene->mNumMeshes; ++i)
        {
            if (!CreateMesh(aScene->mMeshes[i], i))
                return false;
        }

        AddMeshes(aScene->mRootNode);

        return true;
    }

    void ResetKeyframeTimes(aiNodeAnim *aNode)
    {
        auto ResetTime = [](auto keys, uint32 count) {
            if (count > 1)
                CT_CHECK(keys[1].mTime >= 0);

            if (keys[0].mTime < 0)
                keys[0].mTime = 0;
        };

        ResetTime(aNode->mPositionKeys, aNode->mNumPositionKeys);
        ResetTime(aNode->mRotationKeys, aNode->mNumRotationKeys);
        ResetTime(aNode->mScalingKeys, aNode->mNumScalingKeys);
    }

    template <typename AiKeyType, typename ValType, typename Cast>
    bool ParseAnimationKey(const AiKeyType *aKeys, uint32 count, double time, uint32 &currentIndex, ValType &val, const Cast &cast)
    {
        if (currentIndex >= count)
            return true;

        if (aKeys[currentIndex].mTime == time)
        {
            val = cast(aKeys[currentIndex].mValue);
            currentIndex++;
        }
        return currentIndex >= count;
    }

    bool CreateAnimation(const aiAnimation *aAnim)
    {
        String name = ToString(aAnim->mName);
        double durationInTicks = aAnim->mDuration;
        double ticksPerSecond = aAnim->mTicksPerSecond ? aAnim->mTicksPerSecond : 25;
        double durationInSeconds = durationInTicks / ticksPerSecond;

        auto animation = Animation::Create(durationInSeconds);
        animation->SetName(name);

        auto ResetTime = [](auto keys, uint32 count) {
            if (count > 1)
            {
                CT_CHECK(keys[1].mTime >= 0);
            }

            if (keys[0].mTime < 0)
                keys[0].mTime = 0;
        };

        // Parse node animation
        for (uint32 i = 0; i < aAnim->mNumChannels; ++i)
        {
            auto aNode = aAnim->mChannels[i];
            String nodeName = ToString(aNode->mNodeName);
            ResetKeyframeTimes(aNode);

            Array<int32> channels;
            for (int32 n = 0; n < GetNodeCount(nodeName); ++n)
                channels.Add(animation->AddChannel(GetNodeID(nodeName, n)));

            Animation::Keyframe keyframe = {};
            bool done = false;
            uint32 pos = 0, rot = 0, scl = 0;
            auto NextKeyTime = [&]() {
                double t = -1.0; //Assume key time always >= 0
                if (pos < aNode->mNumPositionKeys)
                    t = Math::Max(t, aNode->mPositionKeys[pos].mTime);
                if (rot < aNode->mNumRotationKeys)
                    t = Math::Max(t, aNode->mRotationKeys[rot].mTime);
                if (scl < aNode->mNumScalingKeys)
                    t = Math::Max(t, aNode->mScalingKeys[scl].mTime);
                CT_CHECK(t >= 0.0);
                return t;
            };
            while (!done)
            {
                auto time = NextKeyTime();
                CT_CHECK(time == 0 || (time / ticksPerSecond) >= keyframe.time);
                keyframe.time = time / ticksPerSecond;

                done = ParseAnimationKey(aNode->mPositionKeys, aNode->mNumPositionKeys, time, pos, keyframe.translation, ToVector3);
                done = ParseAnimationKey(aNode->mRotationKeys, aNode->mNumRotationKeys, time, rot, keyframe.rotation, ToQuat) && done;
                done = ParseAnimationKey(aNode->mScalingKeys, aNode->mNumScalingKeys, time, scl, keyframe.scaling, ToVector3) && done;

                for (auto c : channels)
                    animation->AddKeyframe(c, keyframe);
            }
        }

        builder.AddAnimation(0, animation);

        return true;
    }

    bool CreateAnimations()
    {
        for (uint32 i = 0; i < aScene->mNumAnimations; ++i)
        {
            if (!CreateAnimation(aScene->mAnimations[i]))
                return false;
        }
        return true;
    }

    bool CreateCamera()
    {
        if (aScene->mNumCameras == 0)
        {
            CT_LOG(Warning, CT_TEXT("Scene has no default camera."));
            return true;
        }
        if (aScene->mNumCameras > 1)
        {
            CT_LOG(Warning, CT_TEXT("Scene has more than one camera, only the first camera will be used."));
        }

        const auto aCamera = aScene->mCameras[0];
        camera = Camera::Create();
        camera->SetPosition(ToVector3(aCamera->mPosition));
        camera->SetUp(ToVector3(aCamera->mUp));
        camera->SetTarget(ToVector3(aCamera->mLookAt) + ToVector3(aCamera->mPosition));

        float aspectRatio = aCamera->mAspect != 0.0f ? aCamera->mAspect : camera->GetAspectRatio();
        float focalLength = importMode == ImportMode::GLTF2 ? Math::FovToFocalLength(aCamera->mHorizontalFOV * 2.0f / aspectRatio, camera->GetFrameHeight()) : 35.0f;
        camera->SetAspectRatio(aspectRatio);
        camera->SetFocalLength(focalLength);

        camera->SetNearZ(aCamera->mClipPlaneNear);
        camera->SetFarZ(aCamera->mClipPlaneFar);

        //TODO add node

        builder.SetCamera(camera);

        return true;
    }

    bool AddLightCommon(const SPtr<Light> &light, const aiLight *aLight)
    {
        CT_CHECK(aLight->mColorDiffuse == aLight->mColorSpecular);
        light->SetIntensity({
            aLight->mColorSpecular.r,
            aLight->mColorSpecular.g,
            aLight->mColorSpecular.b,
        });

        //TODO add node

        builder.AddLight(light);

        return true;
    }

    bool CreateDirLight(const aiLight *aLight)
    {
        auto light = DirectionalLight::Create();
        light->SetDirection(ToVector3(aLight->mDirection));

        return AddLightCommon(light, aLight);
    }

    bool CreatePointLight(const aiLight *aLight)
    {
        auto light = PointLight::Create();
        light->SetPosition(ToVector3(aLight->mPosition));
        light->SetDirection(ToVector3(aLight->mDirection));
        light->SetOpeningAngle(aLight->mAngleOuterCone);
        light->SetPenumbraAngle(aLight->mAngleOuterCone - aLight->mAngleInnerCone);

        return AddLightCommon(light, aLight);
    }

    bool CreateLights()
    {
        for (uint32 i = 0; i < aScene->mNumLights; ++i)
        {
            const auto aLight = aScene->mLights[i];
            switch (aLight->mType)
            {
            case aiLightSource_DIRECTIONAL:
                if (!CreateDirLight(aLight))
                    return false;
                break;
            case aiLightSource_POINT:
            case aiLightSource_SPOT:
                if (!CreatePointLight(aLight))
                    return false;
                break;
            default:
                CT_LOG(Warning, CT_TEXT("Loading scene includes unsupported light type, just ignore it."));
                break;
            }
        }
        return true;
    }

private:
    SceneBuilder builder;
    APtr<Scene> asset;
    SPtr<SceneImportSettings> settings;
    ImportMode importMode = ImportMode::Default;
    String directory;
    const aiScene *aScene = nullptr;
    HashMap<String, APtr<Texture>> textureCache;
    Array<SPtr<Material>> materials;
    SPtr<Camera> camera;
    HashMap<const aiNode *, int32> nodePtrToID;
    HashMap<String, Array<const aiNode *>> nodeNameToPtrs;
    HashMap<String, Matrix4> boneMatrixMap;
    HashMap<uint32, int32> meshIndexToID;
};
}

APtr<Scene> SceneImporter::Import(const String &path, const SPtr<ImportSettings> &settings)
{
    DebugTimer timer(CT_TEXT("SceneImport"));

    APtr<Scene> result;
    result.NewData();

    gAssetManager->RunMultithread([=]() {
        ImporterImpl impl(result, ImportSettings::As<SceneImportSettings>(settings));
        impl.Import(path);
    });

    return result;
}