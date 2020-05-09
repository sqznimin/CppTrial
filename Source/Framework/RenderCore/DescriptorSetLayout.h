#pragma once

#include "RenderCore/.Package.h"

namespace RenderCore
{
class DescriptorSetLayout
{
public:
    struct Element
    {
        String name;
        DescriptorType descriptorType;

        Element(const String &name, DescriptorType type)
            : name(name), descriptorType(type)
        {
        }
    };

    DescriptorSetLayout(std::initializer_list<Element> initList, ShaderVisibilityFlags visibility)
        : elements(initList), visibility(visibility)
    {
    }

    const Array<Element> &GetElements() const
    {
        return elements;
    }

    ShaderVisibilityFlags GetVisibility() const
    {
        return visibility;
    }

    static SPtr<DescriptorSetLayout> Create(std::initializer_list<Element> initList, ShaderVisibilityFlags visibility = ShaderVisibility::All)
    {
        return Memory::MakeShared<DescriptorSetLayout>(initList, visibility);
    }

private: 
    Array<Element> elements;
    ShaderVisibilityFlags visibility = ShaderVisibility::All;
};

}