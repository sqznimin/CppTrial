#pragma once

#include "Render/.Package.h"

struct CameraData
{
    Matrix4 viewMat;
    Matrix4 projMat;
    Matrix4 viewProjMat;
    Matrix4 invViewProj;

    Vector3 pos; // world space position
    float focalLength = 21.0f;
    Vector3 up{0.0f, 1.0f, 0.0f};
    float aspectRatio = 1.7777f; // 16:9
    Vector3 target{0.0f, 0.0f, -1.0f};
    float nearZ = 0.1f;
    Vector3 cameraU{0.0f, 0.0f, 1.0f};
    float farZ = 1000.0f;
    Vector3 cameraV{0.0f, 1.0f, 0.0f};
    float jitterX = 0.0f;
    Vector3 cameraW{1.0f, 0.0f, 0.0f};
    float jitterY = 0.0f;

    float frameHeight = 24.0f;
    float focalDistance = 10000.0f;
    float apertureRadius = 0.0f;
    float shutterSpeed = 0.004f;
};

class Camera
{
public:
    virtual ~Camera() = default;

    const Matrix4 &GetView() const;
    const Matrix4 &GetProjection() const;
    const Matrix4 &GetViewProjection() const;
    const Matrix4 &GetInvViewProjection() const;

    void SetPosition(const Vector3 &pos)
    {
        data.pos = pos;
        dirty = true;
    }

    Vector3 GetPosition() const
    {
        return data.pos;
    }

    void SetUp(const Vector3 &up)
    {
        data.up = up;
        dirty = true;
    }

    Vector3 GetUp() const
    {
        return data.up;
    }

    void SetTarget(const Vector3 &target)
    {
        data.target = target;
        dirty = true;
    }

    Vector3 GetTarget() const
    {
        return data.target;
    }

    void SetFocalLength(float value)
    {
        data.focalLength = value;
        dirty = true;
    }

    float GetFocalLength() const
    {
        return data.focalLength;
    }

    void SetAspectRatio(float value)
    {
        data.aspectRatio = value;
        dirty = true;
    }

    float GetAspectRatio() const
    {
        return data.aspectRatio;
    }

    void SetNearZ(float value)
    {
        data.nearZ = value;
        dirty = true;
    }

    float GetNearZ() const
    {
        return data.nearZ;
    }

    void SetFarZ(float value)
    {
        data.farZ = value;
        dirty = true;
    }

    float GetFarZ() const
    {
        return data.farZ;
    }

    void SetFrameHeight(float value)
    {
        data.frameHeight = value;
        dirty = true;
    }

    float GetFrameHeight() const
    {
        return data.frameHeight;
    }

    void SetFocalDistance(float value)
    {
        data.focalDistance = value;
        dirty = true;
    }

    float GetFocalDistance() const
    {
        return data.focalDistance;
    }

    void SetApertureRadius(float value)
    {
        data.apertureRadius = value;
        dirty = true;
    }

    float GetApertureRadius() const
    {
        return data.apertureRadius;
    }

    void SetShutterSpeed(float value)
    {
        data.shutterSpeed = value;
        dirty = true;
    }

    float GetShutterSpeed() const
    {
        return data.shutterSpeed;
    }

    const CameraData &GetData() const
    {
        return data;
    }

    const String &GetName() const
    {
        return name;
    }

    void SetName(const String &newName)
    {
        name = newName;
    }

    static SPtr<Camera> Create();

private:
    void CalculateParameters() const;

private:
    String name;
    mutable CameraData data{};
    mutable bool dirty = true;
};