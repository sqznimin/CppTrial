#pragma once

#include "Core/General.h"

CT_SCOPE_BEGIN

template <typename T>
struct Plus
{
    T operator()(const T &a, const T &b) const
    {
        return a + b;
    }
};

template <typename T>
struct Minus
{
    T operator()(const T &a, const T &b) const
    {
        return a - b;
    }
};

template <typename T>
struct Multiplies
{
    T operator()(const T &a, const T &b) const
    {
        return a * b;
    }
};

template <typename T>
struct Divides
{
    T operator()(const T &a, const T &b) const
    {
        return a / b;
    }
};

template <typename T>
struct Modulus
{
    T operator()(const T &a, const T &b) const
    {
        return a % b;
    }
};

template <typename T>
struct EqualTo
{
    bool operator()(const T &a, const T &b) const
    {
        return a == b;
    }
};

template <typename T>
struct NotEqualTo
{
    bool operator()(const T &a, const T &b) const
    {
        return a != b;
    }
};

template <typename T>
struct Greater
{
    bool operator()(const T &a, const T &b) const
    {
        return a > b;
    }
};

template <typename T>
struct Less
{
    bool operator()(const T &a, const T &b) const
    {
        return a < b;
    }
};

template <typename T>
struct GreaterEqual
{
    bool operator()(const T &a, const T &b) const
    {
        return a >= b;
    }
};

template <typename T>
struct LessEqual
{
    bool operator()(const T &a, const T &b) const
    {
        return a <= b;
    }
};

CT_SCOPE_END