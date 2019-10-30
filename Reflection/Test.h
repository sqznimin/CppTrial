#pragma once

#include "Reflection/QualifiedType.h"
#include "Reflection/Constructor.h"
#include "Reflection/Property.h"
#include "Reflection/Method.h"
#include "Reflection/Type.h"
#include "Reflection/Registry.h"

CT_SCOPE_BEGIN

class TestClass1
{
    CT_TYPE_DECLARE(TestClass1, std::nullptr_t);

public:
    String name = L"None";
    int32 num = 0;

private:
    int32 *numPtr = nullptr;

public:
    TestClass1() = default;

    TestClass1(const String &name) : name(name)
    {
    }

    TestClass1(const String &name, int32 num) : name(name), num(num)
    {
    }

    TestClass1(const String &name, int32 num, int32 *numPtr) : name(name), num(num), numPtr(numPtr)
    {
    }

    void Print() const;

    void IncNum(int32 inc)
    {
        num += inc;
    }

    const String &GetName() const
    {
        return name;
    }
};

namespace Reflection
{

void TestBuiltinType();

void TestTypeMacro();

void Test();

} // namespace Reflection
CT_SCOPE_END