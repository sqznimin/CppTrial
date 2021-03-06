#pragma once

#include "Core/.Package.h"
#include "Core/Array.h"

enum class VariantType
{
    Empty = 0,
    Bool,
    Int8,
    Int16,
    Int32,
    Int64,
    UInt8,
    UInt16,
    UInt32,
    UInt64,
    Char,
    WChar,
    Float,
    Double,
    String,
};

namespace VariantInternal
{

template <typename T>
struct VariantTypeTraits
{
};

template <>
struct VariantTypeTraits<bool>
{
    static VariantType GetType()
    {
        return VariantType::Bool;
    }
};

template <>
struct VariantTypeTraits<int8>
{
    static VariantType GetType()
    {
        return VariantType::Int8;
    }
};

template <>
struct VariantTypeTraits<int16>
{
    static VariantType GetType()
    {
        return VariantType::Int16;
    }
};

template <>
struct VariantTypeTraits<int32>
{
    static VariantType GetType()
    {
        return VariantType::Int32;
    }
};

template <>
struct VariantTypeTraits<int64>
{
    static VariantType GetType()
    {
        return VariantType::Int64;
    }
};

template <>
struct VariantTypeTraits<uint8>
{
    static VariantType GetType()
    {
        return VariantType::UInt8;
    }
};

template <>
struct VariantTypeTraits<uint16>
{
    static VariantType GetType()
    {
        return VariantType::UInt16;
    }
};

template <>
struct VariantTypeTraits<uint32>
{
    static VariantType GetType()
    {
        return VariantType::UInt32;
    }
};

template <>
struct VariantTypeTraits<uint64>
{
    static VariantType GetType()
    {
        return VariantType::UInt64;
    }
};

template <>
struct VariantTypeTraits<char8>
{
    static VariantType GetType()
    {
        return VariantType::Char;
    }
};

template <>
struct VariantTypeTraits<wchar>
{
    static VariantType GetType()
    {
        return VariantType::WChar;
    }
};

template <>
struct VariantTypeTraits<float>
{
    static VariantType GetType()
    {
        return VariantType::Float;
    }
};

template <>
struct VariantTypeTraits<double>
{
    static VariantType GetType()
    {
        return VariantType::Double;
    }
};

template <>
struct VariantTypeTraits<String>
{
    static VariantType GetType()
    {
        return VariantType::String;
    }
};

template <typename T>
CT_INLINE void WriteBytes(Array<uint8> &bytes, const T &value)
{
    bytes.AddUninitialized(sizeof(T));
    std::memcpy(bytes.GetData(), &value, sizeof(T));
}

CT_INLINE void WriteBytes(Array<uint8> &bytes, const String &value)
{
    const auto &strArr = value.GetCharArray();
    int32 byteNum = sizeof(CharType) * strArr.Count();
    bytes.AddUninitialized(byteNum);
    std::memcpy(bytes.GetData(), strArr.GetData(), byteNum);
}

template <typename T>
CT_INLINE void ReadBytes(const Array<uint8> &bytes, T &value)
{
    std::memcpy(&value, bytes.GetData(), sizeof(T));
}

CT_INLINE void ReadBytes(const Array<uint8> &bytes, String &value)
{
    int32 charNum = bytes.Count() / sizeof(CharType);
    Array<CharType> temp;
    temp.AddUninitialized(charNum);
    std::memcpy(temp.GetData(), bytes.GetData(), bytes.Count());
    value = temp.GetData();
}

} // namespace VariantInternal

template <typename T>
concept ConvertibleToVariant = requires
{
    VariantInternal::VariantTypeTraits<std::decay_t<T>>::GetType();
};

class Variant
{
public:
    Variant() = default;
    Variant(const Variant &) = default;
    Variant &operator=(const Variant &) = default;
    ~Variant() = default;

    Variant(Variant &&other)
        : type(other.type), data(std::move(other.data))
    {
        other.type = VariantType::Empty;
    }

    Variant &operator=(Variant &&other)
    {
        if (this != &other)
        {
            type = other.type;
            data = std::move(other.data);
            other.type = VariantType::Empty;
        }
        return *this;
    }

    template <ConvertibleToVariant T>
    Variant(T &&value)
    {
        VariantInternal::WriteBytes(data, std::forward<T>(value));
        type = VariantInternal::VariantTypeTraits<std::decay_t<T>>::GetType();
    }

    template <ConvertibleToVariant T>
    Variant &operator=(T &&value)
    {
        Variant temp(std::forward<T>(value));
        Swap(temp);
        return *this;
    }

    void Swap(Variant &other)
    {
        std::swap(type, other.type);
        std::swap(data, other.data);
    }

    void Clear()
    {
        data.Clear();
        type = VariantType::Empty;
    }

    bool IsEmpty() const
    {
        return type == VariantType::Empty;
    }

    VariantType GetType() const
    {
        return type;
    }

    template <ConvertibleToVariant T>
    T GetValue() const
    {
        CT_CHECK(VariantInternal::VariantTypeTraits<T>::GetType() == type);
        T ret;
        VariantInternal::ReadBytes(data, ret);
        return ret;
    }

    bool operator==(const Variant &other) const
    {
        return (type == other.type) && (data == other.data);
    }

    bool operator!=(const Variant &other) const
    {
        return !(*this == other);
    }

private:
    VariantType type = VariantType::Empty;
    Array<uint8> data;
};

namespace std
{
inline void swap(Variant &lhs, Variant &rhs)
{
    lhs.Swap(rhs);
}
} // namespace std