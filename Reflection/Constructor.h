#pragma once

#include "Reflection/.Package.h"
#include "Reflection/ParamInfo.h"

CT_SCOPE_BEGIN

namespace Reflection
{

class Constructor : public MetaBase
{
protected:
    Constructor(Type *ownerType, std::initializer_list<QualifiedType> paramTypes)
        : MetaBase(), ownerType(ownerType)
    {
        uint32 index = 0;
        for (const auto &paramType : paramTypes)
        {
            paramInfos.Add(ParamInfo(paramType, index++));
        }
    }

public:
    Type *GetOwnerType() const
    {
        return ownerType;
    }

    SizeType GetParamCount() const
    {
        return paramInfos.Size();
    }

    const Array<ParamInfo> &GetParamInfos() const
    {
        return paramInfos;
    }

    virtual Any Invoke() const
    {
        CT_THROW("Not implement");
        return Any();
    }

    virtual Any Invoke(Any arg1) const
    {
        CT_THROW("Not implement");
        return Any();
    }

    virtual Any Invoke(Any arg1, Any arg2) const
    {
        CT_THROW("Not implement");
        return Any();
    }

    virtual Any Invoke(Any arg1, Any arg2, Any arg3) const
    {
        CT_THROW("Not implement");
        return Any();
    }

    virtual Any Invoke(Any arg1, Any arg2, Any arg3, Any arg4) const
    {
        CT_THROW("Not implement");
        return Any();
    }

    virtual Any Invoke(Any arg1, Any arg2, Any arg3, Any arg4, Any arg5) const
    {
        CT_THROW("Not implement");
        return Any();
    }

    virtual Any Invoke(Any arg1, Any arg2, Any arg3, Any arg4, Any arg5, Any arg6) const
    {
        CT_THROW("Not implement");
        return Any();
    }

protected:
    Type *ownerType;
    Array<ParamInfo> paramInfos;
};

template <typename T, typename... Args>
class ConstructorImpl : public Constructor
{
public:
    ConstructorImpl() : Constructor(TypeOf<T>(), {GetQualifiedType<Args>()...})
    {
        static_assert(TIsConstructible<T, Args...>::value, "Attempt to define an undeclared constructor.");
    }

    virtual Any Invoke(typename TAsType<Args, Any>::value... args) const override
    {
        return (Any)Memory::New<T>(std::forward<Args>((Args)args)...);
    }
};

} // namespace Reflection

CT_SCOPE_END