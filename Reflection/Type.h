#pragma once

#include "Reflection/.Package.h"
#include "Reflection/ParamInfo.h"

CT_SCOPE_BEGIN

namespace Reflection
{

class Constructor;
class Property;
class Method;

class Type : public MetaBase
{
public:
    bool IsTemplate() const; //TODO

    bool IsEnum() const
    {
        return isEnum;
    }

    bool IsBaseOf(const Type &other) const
    {
        Type *base = other.baseType;
        while (base)
        {
            if (this == base)
            {
                return true;
            }
            else
            {
                base = base->baseType;
            }
        }
        return false;
    }

    template <typename T>
    bool IsBaseOf() const
    {
        return IsBaseOf(TypeOf<T>());
    }

    bool IsDerivedFrom(const Type &other) const
    {
        return other.IsBaseOf(*this);
    }

    template <typename T>
    bool IsDerivedFrom() const
    {
        return IsDerivedFrom(TypeOf<T>());
    }

    SizeType GetSize() const
    {
        return size;
    }

    Type *GetBaseType() const
    {
        return baseType;
    }

    Type *GetUnderlyingType() const
    {
        return underlyingType;
    }

    Array<Type *> GetDerivedTypes() const
    {
        return derivedTypes;
    }

    //virtual const Array<QualifiedType>& GetTemplates() const; //TODO

    Array<Constructor *> GetConstructors() const;
    Array<Property *> GetProperties() const;
    Array<Method *> GetMethods() const;

    Constructor *GetConstructor(const Array<QualifiedType> &typeList = {}) const;
    Property *GetProperty(const Name &name) const;
    Method *GetMethod(const Name &name) const;
    Method *GetMethod(const Name &name, const Array<QualifiedType> &typeList) const;

    static Type *GetType(const Name &name); //TODO

protected:
    Type *SetConstructors(const Array<Constructor *> value);
    Type *SetProperties(const Array<Property *> value);
    Type *SetMethods(const Array<Method *> value);

    static bool MatchParams(const Array<ParamInfo> &params, const Array<QualifiedType> &types);

    Type *baseType = nullptr;
    Type *underlyingType = nullptr;
    Array<Type *> derivedTypes;
    Array<Constructor *> constructors;
    Array<Property *> properties;
    Array<Method *> methods;

    SizeType size;
    bool isEnum = false;
};

template <>
struct TypeTraits<int32>
{
    static Type *GetType()
    {
        static Type type;
        return &type;
    }
};

template <>
struct TypeTraits<String>
{
    static Type *GetType()
    {
        static Type type;
        return &type;
    }
};

} // namespace Reflection

CT_SCOPE_END