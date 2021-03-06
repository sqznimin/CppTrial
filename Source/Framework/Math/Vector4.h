#pragma once

#include "Math/.Package.h"
#include "Math/Vector3.h"

class Vector4
{
public:
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;

    static const Vector4 ZERO;
    static const Vector4 ONE;
    static const Vector4 X;
    static const Vector4 Y;
    static const Vector4 Z;
    static const Vector4 W;

public:
    Vector4() = default;
    Vector4(const Vector4 &) = default;
    Vector4 &operator=(const Vector4 &) = default;

    Vector4(float x, float y, float z, float w)
        : x(x), y(y), z(z), w(w)
    {
    }

    explicit operator Vector3() const
    {
        return Vector3(x, y, z);
    }

    float operator[](int32 i) const
    {
        CT_CHECK(i >= 0 && i < 4);
        return *(&x + i);
    }

    float &operator[](int32 i)
    {
        CT_CHECK(i >= 0 && i < 4);
        return *(&x + i);
    }

    Vector4 &operator+=(const Vector4 &rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        w += rhs.w;
        return *this;
    }

    Vector4 &operator+=(float rhs)
    {
        x += rhs;
        y += rhs;
        z += rhs;
        w += rhs;
        return *this;
    }

    Vector4 &operator-=(const Vector4 &rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        w -= rhs.w;
        return *this;
    }

    Vector4 &operator-=(float rhs)
    {
        x -= rhs;
        y -= rhs;
        z -= rhs;
        w -= rhs;
        return *this;
    }

    Vector4 &operator*=(const Vector4 &rhs)
    {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        w *= rhs.w;
        return *this;
    }

    Vector4 &operator*=(float rhs)
    {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        w *= rhs;
        return *this;
    }

    Vector4 &operator/=(const Vector4 &rhs)
    {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
        w /= rhs.w;
        return *this;
    }

    Vector4 &operator/=(float rhs)
    {
        x /= rhs;
        y /= rhs;
        z /= rhs;
        w /= rhs;
        return *this;
    }

    bool operator==(const Vector4 &other) const
    {
        return (x == other.x && y == other.y && z == other.z && w == other.w);
    }

    bool operator!=(const Vector4 &other) const
    {
        return (x != other.y || y != other.y || z != other.z || w != other.w);
    }

    Vector4 operator+() const
    {
        return *this;
    }

    Vector4 operator-() const
    {
        return Vector4(-x, -y, -z, -w);
    }

    friend Vector4 operator+(const Vector4 &lhs, const Vector4 &rhs)
    {
        return Vector4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w);
    }

    friend Vector4 operator-(const Vector4 &lhs, const Vector4 &rhs)
    {
        return Vector4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w);
    }

    friend Vector4 operator*(const Vector4 &lhs, const Vector4 &rhs)
    {
        return Vector4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w);
    }

    friend Vector4 operator*(float lhs, const Vector4 &rhs)
    {
        return Vector4(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w);
    }

    friend Vector4 operator*(const Vector4 &lhs, float rhs)
    {
        return Vector4(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs);
    }

    friend Vector4 operator/(const Vector4 &lhs, const Vector4 &rhs)
    {
        return Vector4(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w);
    }

    friend Vector4 operator/(float lhs, const Vector4 &rhs)
    {
        return Vector4(lhs / rhs.x, lhs / rhs.y, lhs / rhs.z, lhs / rhs.w);
    }

    friend Vector4 operator/(const Vector4 &lhs, float rhs)
    {
        return Vector4(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs);
    }

    float Length2() const
    {
        return (x * x + y * y + z * z + w * w);
    }

    float Length() const
    {
        return Math::Sqrt(Length2());
    }

    float Distance2(const Vector4 &other) const
    {
        return (*this - other).Length2();
    }

    float Distance(const Vector4 &other) const
    {
        return (*this - other).Length();
    }

    float Dot(const Vector4 &other) const
    {
        return x * other.x + y * other.y + z * other.z;
    }

    Vector4 &Normalize()
    {
        float len = Length();
        if (len < Math::EPSILON)
        {
            x = 0.0f;
            y = 0.0f;
            z = 0.0f;
            w = 0.0f;
        }
        else
        {
            x /= len;
            y /= len;
            z /= len;
            w /= len;
        }
        return *this;
    }

    bool IsZero() const
    {
        float len = Length();
        return len < Math::EPSILON;
    }

    bool IsPerpendicular(const Vector4 &other) const
    {
        return Dot(other) < Math::EPSILON;
    }

    static Vector4 Lerp(const Vector4 &a, const Vector4 &b, float t)
    {
        return Vector4(Math::Lerp(a.x, b.x, t), Math::Lerp(a.y, b.y, t), Math::Lerp(a.z, b.z, t), Math::Lerp(a.w, b.w, t));
    }

    static Vector4 Max(const Vector4 &a, const Vector4 &b)
    {
        return Vector4(Math::Max(a.x, b.x), Math::Max(a.y, b.y), Math::Max(a.z, b.z), Math::Max(a.w, b.w));
    }

    static Vector4 Min(const Vector4 &a, const Vector4 &b)
    {
        return Vector4(Math::Min(a.x, b.x), Math::Min(a.y, b.y), Math::Min(a.z, b.z), Math::Min(a.w, b.w));
    }

    String ToString() const
    {
        return String::Format(CT_TEXT("[x={0},y={1},z={2},w={3}]"), x, y, z, w);
    }

    HashType HashCode() const
    {
        HashType hash = Hash::HashValue(x);
        Hash::HashCombine(hash, Hash::HashValue(y));
        Hash::HashCombine(hash, Hash::HashValue(z));
        Hash::HashCombine(hash, Hash::HashValue(w));
        return hash;
    }
};

inline const Vector4 Vector4::ZERO(0.0f, 0.0f, 0.0f, 0.0f);
inline const Vector4 Vector4::ONE(1.0f, 1.0f, 1.0f, 1.0f);
inline const Vector4 Vector4::X(1.0f, 0.0f, 0.0f, 0.0f);
inline const Vector4 Vector4::Y(0.0f, 1.0f, 0.0f, 0.0f);
inline const Vector4 Vector4::Z(0.0f, 0.0f, 1.0f, 0.0f);
inline const Vector4 Vector4::W(0.0f, 0.0f, 0.0f, 1.0f);