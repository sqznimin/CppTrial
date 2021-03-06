#type vertex

#include "Scene/RasterVS.glsl"

#type pixel

#include "Scene/RasterPS.glsl"

layout(location = 0) out vec4 outColor;

bool constColor = false;

void main() 
{
    if (constColor)
    {
        outColor = vec4(0.0, 1.0, 0.0, 1.0);
        return;
    }

    vec3 viewDir = normalize(GetCameraPosition(camera) - vOut.posW);
    ShadingData sd = PrepareShadingData(gl_PrimitiveID, viewDir);

    vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
    for (int i = 0; i < GetLightCount(); ++i)
    {
        finalColor.rgb += EvalMaterial(sd, GetLight(i), 1.0).color.rgb;
    }
    
    outColor = finalColor;
}