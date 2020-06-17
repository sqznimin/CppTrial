#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "Utility.h"

layout(binding = 0) uniform UB
{
    vec4 tint;
};

layout(binding = 1) uniform sampler mainSampler;
layout(binding = 2) uniform texture2D mainTexture;

layout(location = 0) in vec2 UV;

layout(location = 0) out vec4 FragmentColor;

void main() 
{
    vec4 col = texture(sampler2D(mainTexture, mainSampler), UV);
    col *= tint;
    FragmentColor = col;
}