#pragma once

#include "Demos/ShaderToy/Page.h"

class Page2 : public TPage<Page2>
{
public:
    float progress = 0.0f;

    virtual String GetName() override
    {
        return CT_TEXT("Slider");
    }

    virtual void OnImGuiDraw() override
    {
        ImGui::SliderFloat("Progress", &progress, 0.0f, 1.0f);
    }

    virtual void OnShaderUpdate(SPtr<Shader> &shader) override
    {
        shader->SetFloat(CT_TEXT("Progress"), progress);
    }
};