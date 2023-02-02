
#pragma once

#include <DirectXMath.h>
#include <initializer_list>
#include <array>
#include <cmath>
#include <cassert>
#include <limits>

// floatは誤差が発生するのでこの関数を定義しています
//template<typename Ty>
//_NODISCARD static inline constexpr bool AdjEqual(const Ty epsilon_num, const Ty num)
//{
//    template<class Ty = double> constexpr Ty _0{ static_cast<Ty>(0.0) };
//
//    static_assert(std::is_floating_point<Ty>(), "This type isn't floating point");
//
//    // cmathのfabsはconstexprに対応していないので手動で書いています。(少なくともVS2019では...)
//    constexpr auto Fabs{ [](const Ty num) constexpr {
//        if (num > _0<Ty>) return num; else return -num;
//    } };
//
//    static constexpr auto Epsilon{ std::numeric_limits<Ty>::epsilon() };
//    auto dis{ Fabs(epsilon_num - num) };
//
//    return (dis <= Epsilon);
//}

//-------------------------------------------------------------------------------------------------------------
//. XMFLOAT4系
//-------------------------------------------------------------------------------------------------------------

static inline void operator+= (DirectX::XMFLOAT4& v1, const DirectX::XMFLOAT4& v2)
{
    v1.x += v2.x;
    v1.y += v2.y;
    v1.z += v2.z;
    v1.w += v2.w;
}

static inline void operator-= (DirectX::XMFLOAT4& v1, const DirectX::XMFLOAT4& v2)
{
    v1.x -= v2.x;
    v1.y -= v2.y;
    v1.z -= v2.z;
    v1.w -= v2.w;
}

static inline void operator/= (DirectX::XMFLOAT4& v1, const DirectX::XMFLOAT4& v2)
{
    v1.x /= v2.x;
    v1.y /= v2.y;
    v1.z /= v2.z;
    v1.w /= v2.w;
}

static inline void operator*= (DirectX::XMFLOAT4& v1, const DirectX::XMFLOAT4& v2)
{
    v1.x *= v2.x;
    v1.y *= v2.y;
    v1.z *= v2.z;
    v1.w *= v2.w;
}

static inline void operator%= (DirectX::XMFLOAT4& v1, const DirectX::XMFLOAT4& v2)
{
    v1.x = ::fmodf(v1.x, v2.x);
    v1.y = ::fmodf(v1.y, v2.y);
    v1.z = ::fmodf(v1.z, v2.z);
    v1.w = ::fmodf(v1.w, v2.w);
}

_NODISCARD static inline constexpr auto operator+ (const DirectX::XMFLOAT4& v1, const DirectX::XMFLOAT4& v2)
{
    return DirectX::XMFLOAT4{ v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w };
}

_NODISCARD static inline constexpr auto operator- (const DirectX::XMFLOAT4& v1, const DirectX::XMFLOAT4& v2)
{
    return DirectX::XMFLOAT4{ v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w };
}

_NODISCARD static inline constexpr auto operator* (const DirectX::XMFLOAT4& v1, const DirectX::XMFLOAT4& v2)
{
    return DirectX::XMFLOAT4{ v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w };
}

_NODISCARD static inline constexpr auto operator/ (const DirectX::XMFLOAT4& v1, const DirectX::XMFLOAT4& v2)
{
    return DirectX::XMFLOAT4{ v1.x / v2.x, v1.y / v2.y, v1.z / v2.z, v1.w / v2.w };
}

_NODISCARD static inline constexpr auto operator% (const DirectX::XMFLOAT4& v1, const DirectX::XMFLOAT4& v2)
{
    return DirectX::XMFLOAT4{ ::fmodf(v1.x, v2.x), ::fmodf(v1.y, v2.y), ::fmodf(v1.z, v2.z), ::fmodf(v1.w, v2.w) };
}

static inline void operator+= (DirectX::XMFLOAT4& v1, const std::initializer_list<float>& v2)
{
    assert(v2.size() == 4u && "不正なイニシャライザーリスト");

    auto itr{ v2.begin() };
    v1.x += *itr;
    itr++;
    v1.y += *itr;
    itr++;
    v1.z += *itr;
    itr++;
    v1.w += *itr;
}

static inline void operator-= (DirectX::XMFLOAT4& v1, const std::initializer_list<float>& v2)
{
    assert(v2.size() == 4u && "不正なイニシャライザーリスト");

    auto itr{ v2.begin() };
    v1.x -= *itr;
    itr++;
    v1.y -= *itr;
    itr++;
    v1.z -= *itr;
    itr++;
    v1.w -= *itr;
}

static inline void operator/= (DirectX::XMFLOAT4& v1, const std::initializer_list<float>& v2)
{
    assert(v2.size() == 4u && "不正なイニシャライザーリスト");

    auto itr{ v2.begin() };
    v1.x /= *itr;
    itr++;
    v1.y /= *itr;
    itr++;
    v1.z /= *itr;
    itr++;
    v1.w /= *itr;
}

static inline void operator*= (DirectX::XMFLOAT4& v1, const std::initializer_list<float>& v2)
{
    assert(v2.size() == 4u && "不正なイニシャライザーリスト");

    auto itr{ v2.begin() };
    v1.x *= *itr;
    itr++;
    v1.y *= *itr;
    itr++;
    v1.z *= *itr;
    itr++;
    v1.w *= *itr;
}

static inline void operator%= (DirectX::XMFLOAT4& v1, const std::initializer_list<float>& v2)
{
    assert(v2.size() == 4u && "不正なイニシャライザーリスト");

    auto itr{ v2.begin() };
    v1.x = ::fmodf(v1.x, *itr);
    itr++;
    v1.y = ::fmodf(v1.y, *itr);
    itr++;
    v1.z = ::fmodf(v1.z, *itr);
    itr++;
    v1.w = ::fmodf(v1.w, *itr);
}

_NODISCARD static inline auto operator+ (const DirectX::XMFLOAT4& v1, const std::initializer_list<float>& v2)
{
    assert(v2.size() == 4u && "不正なイニシャライザーリスト");

    DirectX::XMFLOAT4 temp{ v1 };
    auto itr{ v2.begin() };

    temp.x += *itr;
    itr++;
    temp.y += *itr;
    itr++;
    temp.z += *itr;
    itr++;
    temp.w += *itr;

    return temp;
}

_NODISCARD static inline auto operator- (const DirectX::XMFLOAT4& v1, const std::initializer_list<float>& v2)
{
    assert(v2.size() == 4u && "不正なイニシャライザーリスト");

    DirectX::XMFLOAT4 temp{ v1 };
    auto itr{ v2.begin() };

    temp.x -= *itr;
    itr++;
    temp.y -= *itr;
    itr++;
    temp.z -= *itr;
    itr++;
    temp.w -= *itr;

    return temp;
}

_NODISCARD static inline auto operator* (const DirectX::XMFLOAT4& v1, const std::initializer_list<float>& v2)
{
    assert(v2.size() == 4u && "不正なイニシャライザーリスト");

    DirectX::XMFLOAT4 temp{ v1 };
    auto itr{ v2.begin() };

    temp.x *= *itr;
    itr++;
    temp.y *= *itr;
    itr++;
    temp.z *= *itr;
    itr++;
    temp.w *= *itr;

    return temp;
}

_NODISCARD static inline auto operator/ (const DirectX::XMFLOAT4& v1, const std::initializer_list<float>& v2)
{
    assert(v2.size() == 4u && "不正なイニシャライザーリスト");

    DirectX::XMFLOAT4 temp{ v1 };
    auto itr{ v2.begin() };

    temp.x /= *itr;
    itr++;
    temp.y /= *itr;
    itr++;
    temp.z /= *itr;
    itr++;
    temp.w /= *itr;

    return temp;
}

_NODISCARD static inline auto operator% (const DirectX::XMFLOAT4& v1, const std::initializer_list<float>& v2)
{
    assert(v2.size() == 4u && "不正なイニシャライザーリスト");

    DirectX::XMFLOAT4 temp{ v1 };
    auto itr{ v2.begin() };

    temp.x = ::fmodf(v1.x, *itr);
    itr++;
    temp.y = ::fmodf(v1.y, *itr);
    itr++;
    temp.z = ::fmodf(v1.z, *itr);
    itr++;
    temp.w = ::fmodf(v1.w, *itr);

    return temp;
}

static inline void operator+= (DirectX::XMFLOAT4& v1, const float num)
{
    v1.x += num;
    v1.y += num;
    v1.z += num;
    v1.w += num;
}

static inline void operator-= (DirectX::XMFLOAT4& v1, const float num)
{
    v1.x -= num;
    v1.y -= num;
    v1.z -= num;
    v1.w -= num;
}

static inline void operator/= (DirectX::XMFLOAT4& v1, const float num)
{
    v1.x /= num;
    v1.y /= num;
    v1.z /= num;
    v1.w /= num;
}

static inline void operator*= (DirectX::XMFLOAT4& v1, const float num)
{
    v1.x *= num;
    v1.y *= num;
    v1.z *= num;
    v1.w *= num;
}

static inline void operator%= (DirectX::XMFLOAT4& v1, const float num)
{
    v1.x = ::fmodf(v1.x, num);
    v1.y = ::fmodf(v1.y, num);
    v1.z = ::fmodf(v1.z, num);
    v1.w = ::fmodf(v1.w, num);
}

_NODISCARD static inline constexpr auto operator+ (const DirectX::XMFLOAT4& v1, const float num)
{
    return DirectX::XMFLOAT4{ v1.x + num, v1.y + num, v1.z + num, v1.w + num };
}

_NODISCARD static inline constexpr auto operator- (const DirectX::XMFLOAT4& v1, const float num)
{
    return DirectX::XMFLOAT4{ v1.x - num, v1.y - num, v1.z - num, v1.w - num };
}

_NODISCARD static inline constexpr auto operator* (const DirectX::XMFLOAT4& v1, const float num)
{
    return DirectX::XMFLOAT4{ v1.x * num, v1.y * num, v1.z * num, v1.w * num };
}

_NODISCARD static inline constexpr auto operator/ (const DirectX::XMFLOAT4& v1, const float num)
{
    return DirectX::XMFLOAT4{ v1.x / num, v1.y / num, v1.z / num, v1.w / num };
}

_NODISCARD static inline constexpr auto operator% (const DirectX::XMFLOAT4& v1, const float num)
{
    return DirectX::XMFLOAT4{ ::fmodf(v1.x, num), ::fmodf(v1.y, num), ::fmodf(v1.z, num), ::fmodf(v1.w, num) };
}

_NODISCARD static inline constexpr auto operator+ (const float num, DirectX::XMFLOAT4& v1)
{
    return DirectX::XMFLOAT4{ num + v1.x, num + v1.y, num + v1.z, num + v1.w };
}

_NODISCARD static inline constexpr auto operator- (const float num, DirectX::XMFLOAT4& v1)
{
    return DirectX::XMFLOAT4{ num - v1.x, num - v1.y, num - v1.z, num - v1.w };
}

_NODISCARD static inline constexpr auto operator* (const float num, DirectX::XMFLOAT4& v1)
{
    return DirectX::XMFLOAT4{ num * v1.x, num * v1.y, num * v1.z, num * v1.w };
}

_NODISCARD static inline constexpr auto operator/ (const float num, DirectX::XMFLOAT4& v1)
{
    return DirectX::XMFLOAT4{ num / v1.x, num / v1.y, num / v1.z, num / v1.w };
}

_NODISCARD static inline constexpr auto operator% (const float num, DirectX::XMFLOAT4& v1)
{
    return DirectX::XMFLOAT4{ fmodf(num, v1.x), fmodf(num, v1.y), fmodf(num, v1.z), fmodf(num, v1.w) };
}

//_NODISCARD static inline constexpr bool operator== (const DirectX::XMFLOAT4& v1, const DirectX::XMFLOAT4& v2)
//{
//    return (AdjEqual(v1.x, v2.x) && AdjEqual(v1.y, v2.y) && AdjEqual(v1.z, v2.z) && AdjEqual(v1.w, v2.w));
//}
//
//_NODISCARD static inline constexpr bool operator!= (const DirectX::XMFLOAT4& v1, const DirectX::XMFLOAT4& v2)
//{
//    return !(AdjEqual(v1.x, v2.x) && AdjEqual(v1.y, v2.y) && AdjEqual(v1.z, v2.z) && AdjEqual(v1.w, v2.w));
//}
//
//_NODISCARD static inline constexpr bool operator== (const DirectX::XMFLOAT4& v1, const float num)
//{
//    return (AdjEqual(v1.x, num) && AdjEqual(v1.y, num) && AdjEqual(v1.z, num) && AdjEqual(v1.w, num));
//}
//
//_NODISCARD static inline constexpr bool operator!= (const DirectX::XMFLOAT4& v1, const float num)
//{
//    return !(AdjEqual(v1.x, num) && AdjEqual(v1.y, num) && AdjEqual(v1.z, num) && AdjEqual(v1.w, num));
//}

_NODISCARD static inline constexpr bool operator< (const DirectX::XMFLOAT4& v1, const DirectX::XMFLOAT4& v2)
{
    return ((v1.x < v2.x) && (v1.y < v2.y) && (v1.z < v2.z) && (v1.w < v2.w));
}

_NODISCARD static inline constexpr bool operator> (const DirectX::XMFLOAT4& v1, const DirectX::XMFLOAT4& v2)
{
    return ((v1.x > v2.x) && (v1.y > v2.y) && (v1.z > v2.z) && (v1.w > v2.w));
}

//_NODISCARD static inline constexpr bool operator<= (const DirectX::XMFLOAT4& v1, const DirectX::XMFLOAT4& v2)
//{
//    return ((v1.x < v2.x) && (v1.y < v2.y) && (v1.z < v2.z) && (v1.w < v2.w)) || (AdjEqual(v1.x, v2.x) && AdjEqual(v1.y, v2.y) && AdjEqual(v1.z, v2.z) && AdjEqual(v1.w, v2.w));
//}
//
//_NODISCARD static inline constexpr bool operator>= (const DirectX::XMFLOAT4& v1, const DirectX::XMFLOAT4& v2)
//{
//    return ((v1.x > v2.x) && (v1.y > v2.y) && (v1.z > v2.z) && (v1.w > v2.w)) || (AdjEqual(v1.x, v2.x) && AdjEqual(v1.y, v2.y) && AdjEqual(v1.z, v2.z) && AdjEqual(v1.w, v2.w));
//}

_NODISCARD static inline constexpr auto operator- (const DirectX::XMFLOAT4& v1)
{
    return DirectX::XMFLOAT4{ -v1.x, -v1.y, -v1.z, -v1.w };
}

//-------------------------------------------------------------------------------------------------------------
//. XMFLOAT3系
//-------------------------------------------------------------------------------------------------------------

static inline void operator+= (DirectX::XMFLOAT3& v1, const std::initializer_list<float>& v2)
{
    assert(v2.size() == 3u && "不正なイニシャライザーリスト");

    auto itr{ v2.begin() };
    v1.x += *itr;
    itr++;
    v1.y += *itr;
    itr++;
    v1.z += *itr;
}

static inline void operator-= (DirectX::XMFLOAT3& v1, const std::initializer_list<float>& v2)
{
    assert(v2.size() == 3u && "不正なイニシャライザーリスト");

    auto itr{ v2.begin() };
    v1.x -= *itr;
    itr++;
    v1.y -= *itr;
    itr++;
    v1.z -= *itr;
}

static inline void operator/= (DirectX::XMFLOAT3& v1, const std::initializer_list<float>& v2)
{
    assert(v2.size() == 3u && "不正なイニシャライザーリスト");

    auto itr{ v2.begin() };
    v1.x /= *itr;
    itr++;
    v1.y /= *itr;
    itr++;
    v1.z /= *itr;
}

static inline void operator*= (DirectX::XMFLOAT3& v1, const std::initializer_list<float>& v2)
{
    assert(v2.size() == 3u && "不正なイニシャライザーリスト");

    auto itr{ v2.begin() };
    v1.x *= *itr;
    itr++;
    v1.y *= *itr;
    itr++;
    v1.z *= *itr;
}

static inline void operator%= (DirectX::XMFLOAT3& v1, const std::initializer_list<float>& v2)
{
    assert(v2.size() == 3u && "不正なイニシャライザーリスト");

    auto itr{ v2.begin() };
    v1.x = ::fmodf(v1.x, *itr);
    itr++;
    v1.y = ::fmodf(v1.y, *itr);
    itr++;
    v1.z = ::fmodf(v1.z, *itr);
}

_NODISCARD static inline auto operator+ (const DirectX::XMFLOAT3& v1, const std::initializer_list<float>& v2)
{
    assert(v2.size() == 3u && "不正なイニシャライザーリスト");

    DirectX::XMFLOAT3 temp{ v1 };
    auto itr{ v2.begin() };

    temp.x += *itr;
    itr++;
    temp.y += *itr;
    itr++;
    temp.z += *itr;

    return temp;
}

_NODISCARD static inline auto operator- (const DirectX::XMFLOAT3& v1, const std::initializer_list<float>& v2)
{
    assert(v2.size() == 3u && "不正なイニシャライザーリスト");

    DirectX::XMFLOAT3 temp{ v1 };
    auto itr{ v2.begin() };

    temp.x -= *itr;
    itr++;
    temp.y -= *itr;
    itr++;
    temp.z -= *itr;

    return temp;
}

_NODISCARD static inline auto operator* (const DirectX::XMFLOAT3& v1, const std::initializer_list<float>& v2)
{
    assert(v2.size() == 3u && "不正なイニシャライザーリスト");

    DirectX::XMFLOAT3 temp{ v1 };
    auto itr{ v2.begin() };

    temp.x *= *itr;
    itr++;
    temp.y *= *itr;
    itr++;
    temp.z *= *itr;

    return temp;
}

_NODISCARD static inline auto operator/ (const DirectX::XMFLOAT3& v1, const std::initializer_list<float>& v2)
{
    assert(v2.size() == 3u && "不正なイニシャライザーリスト");

    DirectX::XMFLOAT3 temp{ v1 };
    auto itr{ v2.begin() };

    temp.x /= *itr;
    itr++;
    temp.y /= *itr;
    itr++;
    temp.z /= *itr;

    return temp;
}

_NODISCARD static inline auto operator% (const DirectX::XMFLOAT3& v1, const std::initializer_list<float>& v2)
{
    assert(v2.size() == 3u && "不正なイニシャライザーリスト");

    DirectX::XMFLOAT3 temp{ v1 };
    auto itr{ v2.begin() };

    temp.x = ::fmodf(v1.x, *itr);
    itr++;
    temp.y = ::fmodf(v1.y, *itr);
    itr++;
    temp.z = ::fmodf(v1.z, *itr);

    return temp;
}

static inline void operator+= (DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2)
{
    v1.x += v2.x;
    v1.y += v2.y;
    v1.z += v2.z;
}

static inline void operator-= (DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2)
{
    v1.x -= v2.x;
    v1.y -= v2.y;
    v1.z -= v2.z;
}

static inline void operator*= (DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2)
{
    v1.x *= v2.x;
    v1.y *= v2.y;
    v1.z *= v2.z;
}

static inline void operator/= (DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2)
{
    v1.x /= v2.x;
    v1.y /= v2.y;
    v1.z /= v2.z;
}

static inline void operator%= (DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2)
{
    v1.x = ::fmodf(v1.x, v2.x);
    v1.y = ::fmodf(v1.y, v2.y);
    v1.z = ::fmodf(v1.z, v2.z);
}

_NODISCARD static inline constexpr auto operator+ (const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2)
{
    return DirectX::XMFLOAT3{ v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}

_NODISCARD static inline constexpr auto operator- (const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2)
{
    return DirectX::XMFLOAT3{ v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
}

_NODISCARD static inline constexpr auto operator* (const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2)
{
    return DirectX::XMFLOAT3{ v1.x * v2.x, v1.y * v2.y, v1.z * v2.z };
}

_NODISCARD static inline constexpr auto operator/ (const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2)
{
    return DirectX::XMFLOAT3{ v1.x / v2.x, v1.y / v2.y, v1.z / v2.z };
}

_NODISCARD static inline constexpr auto operator% (const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2)
{
    return DirectX::XMFLOAT3{ ::fmodf(v1.x, v2.x), ::fmodf(v1.y, v2.y), ::fmodf(v1.z, v2.z) };
}

static inline void operator+= (DirectX::XMFLOAT3& v1, const float num)
{
    v1.x += num;
    v1.y += num;
    v1.z += num;
}

static inline void operator-= (DirectX::XMFLOAT3& v1, const float num)
{
    v1.x -= num;
    v1.y -= num;
    v1.z -= num;
}

static inline void operator/= (DirectX::XMFLOAT3& v1, const float num)
{
    v1.x /= num;
    v1.y /= num;
    v1.z /= num;
}

static inline void operator*= (DirectX::XMFLOAT3& v1, const float num)
{
    v1.x *= num;
    v1.y *= num;
    v1.z *= num;
}

static inline void operator%= (DirectX::XMFLOAT3& v1, const float num)
{
    v1.x = ::fmodf(v1.x, num);
    v1.y = ::fmodf(v1.y, num);
    v1.z = ::fmodf(v1.z, num);
}

_NODISCARD static inline constexpr auto operator+ (const DirectX::XMFLOAT3& v1, const float num)
{
    return DirectX::XMFLOAT3{ v1.x + num, v1.y + num, v1.z + num };
}

_NODISCARD static inline constexpr auto operator- (const DirectX::XMFLOAT3& v1, const float num)
{
    return DirectX::XMFLOAT3{ v1.x - num, v1.y - num, v1.z - num };
}

_NODISCARD static inline constexpr auto operator* (const DirectX::XMFLOAT3& v1, const float num)
{
    return DirectX::XMFLOAT3{ v1.x * num, v1.y * num, v1.z * num };
}

_NODISCARD static inline constexpr auto operator% (const DirectX::XMFLOAT3& v1, const float num)
{
    return DirectX::XMFLOAT3{ ::fmodf(v1.x, num), ::fmodf(v1.y, num), ::fmodf(v1.z, num) };
}

_NODISCARD static inline constexpr auto operator/ (const DirectX::XMFLOAT3& v1, const float num)
{
    return DirectX::XMFLOAT3{ v1.x / num, v1.y / num, v1.z / num };
}

_NODISCARD static inline constexpr auto operator+ (const float num, DirectX::XMFLOAT3& v1)
{
    return DirectX::XMFLOAT3{ num + v1.x, num - v1.y, num - v1.z };
}

_NODISCARD static inline constexpr auto operator- (const float num, DirectX::XMFLOAT3& v1)
{
    return DirectX::XMFLOAT3{ num - v1.x, num - v1.y, num - v1.z };
}

_NODISCARD static inline constexpr auto operator* (const float num, DirectX::XMFLOAT3& v1)
{
    return DirectX::XMFLOAT3{ num * v1.x, num - v1.y, num - v1.z };
}

_NODISCARD static inline constexpr auto operator/ (const float num, DirectX::XMFLOAT3& v1)
{
    return DirectX::XMFLOAT3{ num / v1.x, num / v1.y, num / v1.z };
}

_NODISCARD static inline constexpr auto operator% (const float num, DirectX::XMFLOAT3& v1)
{
    return DirectX::XMFLOAT3{ fmodf(num, v1.x), fmodf(num, v1.y), fmodf(num, v1.z) };
}

//_NODISCARD static inline constexpr bool operator== (const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2)
//{
//    return (AdjEqual(v1.x, v2.x) && AdjEqual(v1.y, v2.y) && AdjEqual(v1.z, v2.z));
//}
//
//_NODISCARD static inline constexpr bool operator== (const DirectX::XMFLOAT3& v1, const float num)
//{
//    return (AdjEqual(v1.x, num) && AdjEqual(v1.y, num) && AdjEqual(v1.z, num));
//}
//
//_NODISCARD static inline constexpr bool operator!= (const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2)
//{
//    return !(AdjEqual(v1.x, v2.x) && AdjEqual(v1.y, v2.y) && AdjEqual(v1.z, v2.z));
//}
//
//_NODISCARD static inline constexpr bool operator!= (const DirectX::XMFLOAT3& v1, const float num)
//{
//    return !(AdjEqual(v1.x, num) && AdjEqual(v1.y, num) && AdjEqual(v1.z, num));
//}

_NODISCARD static inline constexpr bool operator< (const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2)
{
    return ((v1.x < v2.x) && (v1.y < v2.y) && (v1.z < v2.z));
}

_NODISCARD static inline constexpr bool operator> (const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2)
{
    return ((v1.x > v2.x) && (v1.y > v2.y) && (v1.z > v2.z));
}

//_NODISCARD static inline constexpr bool operator<= (const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2)
//{
//    return ((v1.x < v2.x) && (v1.y < v2.y) && (v1.z < v2.z)) || (AdjEqual(v1.x, v2.x) && AdjEqual(v1.y, v2.y) && AdjEqual(v1.z, v2.z));
//}
//
//_NODISCARD static inline constexpr bool operator>= (const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2)
//{
//    return ((v1.x > v2.x) && (v1.y > v2.y) && (v1.z > v2.z)) || (AdjEqual(v1.x, v2.x) && AdjEqual(v1.y, v2.y) && AdjEqual(v1.z, v2.z));
//}

_NODISCARD static inline constexpr auto operator- (const DirectX::XMFLOAT3& v1)
{
    return DirectX::XMFLOAT3{ -v1.x, -v1.y, -v1.z };
}

//-------------------------------------------------------------------------------------------------------------
//. XMFLOAT2系
//-------------------------------------------------------------------------------------------------------------

static inline void operator+= (DirectX::XMFLOAT2& v1, const std::initializer_list<float>& v2)
{
    assert(v2.size() == 2u && "不正なイニシャライザーリスト");

    auto itr{ v2.begin() };
    v1.x += *itr;
    itr++;
    v1.y += *itr;
}

static inline void operator-= (DirectX::XMFLOAT2& v1, const std::initializer_list<float>& v2)
{
    assert(v2.size() == 2u && "不正なイニシャライザーリスト");

    auto itr{ v2.begin() };
    v1.x -= *itr;
    itr++;
    v1.y -= *itr;
}

static inline void operator/= (DirectX::XMFLOAT2& v1, const std::initializer_list<float>& v2)
{
    assert(v2.size() == 2u && "不正なイニシャライザーリスト");

    auto itr{ v2.begin() };
    v1.x /= *itr;
    itr++;
    v1.y /= *itr;
}

static inline void operator*= (DirectX::XMFLOAT2& v1, const std::initializer_list<float>& v2)
{
    assert(v2.size() == 2u && "不正なイニシャライザーリスト");

    auto itr{ v2.begin() };
    v1.x *= *itr;
    itr++;
    v1.y *= *itr;
}

static inline void operator%= (DirectX::XMFLOAT2& v1, const std::initializer_list<float>& v2)
{
    assert(v2.size() == 2u && "不正なイニシャライザーリスト");

    auto itr{ v2.begin() };
    v1.x = ::fmodf(v1.x, *itr);
    itr++;
    v1.y = ::fmodf(v1.y, *itr);
}

_NODISCARD static inline auto operator+ (const DirectX::XMFLOAT2& v1, const std::initializer_list<float>& v2)
{
    assert(v2.size() == 2u && "不正なイニシャライザーリスト");

    DirectX::XMFLOAT2 temp{ v1 };
    auto itr{ v2.begin() };

    temp.x += *itr;
    itr++;
    temp.y += *itr;

    return temp;
}

_NODISCARD static inline auto operator- (const DirectX::XMFLOAT2& v1, const std::initializer_list<float>& v2)
{
    assert(v2.size() == 2u && "不正なイニシャライザーリスト");

    DirectX::XMFLOAT2 temp{ v1 };
    auto itr{ v2.begin() };

    temp.x -= *itr;
    itr++;
    temp.y -= *itr;

    return temp;
}

_NODISCARD static inline auto operator* (const DirectX::XMFLOAT2& v1, const std::initializer_list<float>& v2)
{
    assert(v2.size() == 2u && "不正なイニシャライザーリスト");

    DirectX::XMFLOAT2 temp{ v1 };
    auto itr{ v2.begin() };

    temp.x *= *itr;
    itr++;
    temp.y *= *itr;

    return temp;
}

_NODISCARD static inline auto operator/ (const DirectX::XMFLOAT2& v1, const std::initializer_list<float>& v2)
{
    assert(v2.size() == 2u && "不正なイニシャライザーリスト");

    DirectX::XMFLOAT2 temp{ v1 };
    auto itr{ v2.begin() };

    temp.x /= *itr;
    itr++;
    temp.y /= *itr;

    return temp;
}

_NODISCARD static inline auto operator% (const DirectX::XMFLOAT2& v1, const std::initializer_list<float>& v2)
{
    assert(v2.size() == 2u && "不正なイニシャライザーリスト");

    DirectX::XMFLOAT2 temp{ v1 };
    auto itr{ v2.begin() };

    temp.x = ::fmodf(v1.x, *itr);
    itr++;
    temp.y = ::fmodf(v1.y, *itr);

    return temp;
}

static inline void operator+= (DirectX::XMFLOAT2& v1, const DirectX::XMFLOAT2& v2)
{
    v1.x += v2.x;
    v1.y += v2.y;
}

static inline void operator-= (DirectX::XMFLOAT2& v1, const DirectX::XMFLOAT2& v2)
{
    v1.x -= v2.x;
    v1.y -= v2.y;
}

static inline void operator/= (DirectX::XMFLOAT2& v1, const DirectX::XMFLOAT2& v2)
{
    v1.x /= v2.x;
    v1.y /= v2.y;
}

static inline void operator*= (DirectX::XMFLOAT2& v1, const DirectX::XMFLOAT2& v2)
{
    v1.x *= v2.x;
    v1.y *= v2.y;
}

static inline void operator%= (DirectX::XMFLOAT2& v1, const DirectX::XMFLOAT2& v2)
{
    v1.x = ::fmodf(v1.x, v2.x);
    v1.y = ::fmodf(v1.y, v2.y);
}

_NODISCARD static inline constexpr auto operator+ (const DirectX::XMFLOAT2& v1, const DirectX::XMFLOAT2& v2)
{
    return DirectX::XMFLOAT2{ v1.x + v2.x, v1.y + v2.y };
}

_NODISCARD static inline constexpr auto operator- (const DirectX::XMFLOAT2& v1, const DirectX::XMFLOAT2& v2)
{
    return DirectX::XMFLOAT2{ v1.x - v2.x, v1.y - v2.y };
}

_NODISCARD static inline constexpr auto operator* (const DirectX::XMFLOAT2& v1, const DirectX::XMFLOAT2& v2)
{
    return DirectX::XMFLOAT2{ v1.x * v2.x, v1.y * v2.y };
}

_NODISCARD static inline constexpr auto operator/ (const DirectX::XMFLOAT2& v1, const DirectX::XMFLOAT2& v2)
{
    return DirectX::XMFLOAT2{ v1.x / v2.x, v1.y / v2.y };
}

_NODISCARD static inline constexpr auto operator% (const DirectX::XMFLOAT2& v1, const DirectX::XMFLOAT2& v2)
{
    return DirectX::XMFLOAT2{ ::fmodf(v1.x, v2.x), ::fmodf(v1.y, v2.y) };
}

static inline void operator+= (DirectX::XMFLOAT2& v1, const float num)
{
    v1.x += num;
    v1.y += num;
}

static inline void operator-= (DirectX::XMFLOAT2& v1, const float num)
{
    v1.x -= num;
    v1.y -= num;
}

static inline void operator/= (DirectX::XMFLOAT2& v1, const float num)
{
    v1.x /= num;
    v1.y /= num;
}

static inline void operator*= (DirectX::XMFLOAT2& v1, const float num)
{
    v1.x *= num;
    v1.y *= num;
}

static inline void operator%= (DirectX::XMFLOAT2& v1, const float num)
{
    v1.x = ::fmodf(v1.x, num);
    v1.y = ::fmodf(v1.y, num);
}

_NODISCARD static inline constexpr auto operator+ (const DirectX::XMFLOAT2& v1, const float num)
{
    return DirectX::XMFLOAT2{ v1.x + num, v1.y + num };
}

_NODISCARD static inline constexpr auto operator- (const DirectX::XMFLOAT2& v1, const float num)
{
    return DirectX::XMFLOAT2{ v1.x - num, v1.y - num };
}

_NODISCARD static inline constexpr auto operator* (const DirectX::XMFLOAT2& v1, const float num)
{
    return DirectX::XMFLOAT2{ v1.x * num, v1.y * num };
}

_NODISCARD static inline constexpr auto operator/ (const DirectX::XMFLOAT2& v1, const float num)
{
    return DirectX::XMFLOAT2{ v1.x / num, v1.y / num };
}

_NODISCARD static inline constexpr auto operator% (const DirectX::XMFLOAT2& v1, const float num)
{
    return DirectX::XMFLOAT2{ ::fmodf(v1.x, num), ::fmodf(v1.y, num) };
}

_NODISCARD static inline constexpr auto operator+ (const float num, DirectX::XMFLOAT2& v1)
{
    return DirectX::XMFLOAT2{ num + v1.x, num + v1.y };
}

_NODISCARD static inline constexpr auto operator- (const float num, DirectX::XMFLOAT2& v1)
{
    return DirectX::XMFLOAT2{ num - v1.x, num - v1.y };
}

_NODISCARD static inline constexpr auto operator* (const float num, DirectX::XMFLOAT2& v1)
{
    return DirectX::XMFLOAT2{ num * v1.x, num * v1.y };
}

_NODISCARD static inline constexpr auto operator/ (const float num, DirectX::XMFLOAT2& v1)
{
    return DirectX::XMFLOAT2{ num / v1.x, num / v1.y };
}

_NODISCARD static inline constexpr auto operator% (const float num, DirectX::XMFLOAT2& v1)
{
    return DirectX::XMFLOAT2{ fmodf(num, v1.x), fmodf(num, v1.y) };
}

//_NODISCARD static inline constexpr bool operator== (const DirectX::XMFLOAT2& v1, const float num)
//{
//    return (AdjEqual(v1.x, num) && AdjEqual(v1.y, num));
//}
//
//_NODISCARD static inline constexpr bool operator!= (const DirectX::XMFLOAT2& v1, const float num)
//{
//    return !(AdjEqual(v1.x, num) && AdjEqual(v1.y, num));
//}
//
//_NODISCARD static inline constexpr bool operator== (const DirectX::XMFLOAT2& v1, const DirectX::XMFLOAT2& v2)
//{
//    return (AdjEqual(v1.x, v2.x) && AdjEqual(v1.y, v2.y));
//}
//
//_NODISCARD static inline constexpr bool operator!= (const DirectX::XMFLOAT2& v1, const DirectX::XMFLOAT2& v2)
//{
//    return !(AdjEqual(v1.x, v2.x) && AdjEqual(v1.y, v2.y));
//}

_NODISCARD static inline constexpr bool operator< (const DirectX::XMFLOAT2& v1, const DirectX::XMFLOAT2& v2)
{
    return ((v1.x < v2.x) && (v1.y < v2.y));
}

_NODISCARD static inline constexpr bool operator> (const DirectX::XMFLOAT2& v1, const DirectX::XMFLOAT2& v2)
{
    return ((v1.x > v2.x) && (v1.y > v2.y));
}

//_NODISCARD static inline constexpr bool operator<= (const DirectX::XMFLOAT2& v1, const DirectX::XMFLOAT2& v2)
//{
//    return ((v1.x < v2.x) && (v1.y < v2.y)) || (AdjEqual(v1.x, v2.x) && AdjEqual(v1.y, v2.y));
//}
//
//_NODISCARD static inline constexpr bool operator>= (const DirectX::XMFLOAT2& v1, const DirectX::XMFLOAT2& v2)
//{
//    return ((v1.x > v2.x) && (v1.y > v2.y)) || (AdjEqual(v1.x, v2.x) && AdjEqual(v1.y, v2.y));
//}

_NODISCARD static inline constexpr auto operator- (const DirectX::XMFLOAT2& v1)
{
    return DirectX::XMFLOAT2{ -v1.x, -v1.y };
}

//-------------------------------------------------------------------------------------------------------------
// 変換関数系
//-------------------------------------------------------------------------------------------------------------

//. XMVECTOR変換-----------------------------------------------------------------------------------------------

_NODISCARD static inline auto ToXMVECTOR(const DirectX::XMFLOAT4& vec)
{
    return DirectX::XMLoadFloat4(&vec);
}

_NODISCARD static inline auto ToXMVECTOR(const DirectX::XMFLOAT3& vec)
{
    return DirectX::XMLoadFloat3(&vec);
}

_NODISCARD static inline auto ToXMVECTOR(const DirectX::XMFLOAT2& vec)
{
    return DirectX::XMLoadFloat2(&vec);
}

_NODISCARD static inline auto ToXMVECTOR(const float vec)
{
    return DirectX::XMLoadFloat(&vec);
}

//. XMVECTOR変換(正規化)---------------------------------------------------------------------------------------

_NODISCARD static inline auto ToNormalizeXMVECTOR(const DirectX::XMFLOAT4& vec)
{
    return DirectX::XMVector4Normalize(DirectX::XMLoadFloat4(&vec));
}

_NODISCARD static inline auto ToNormalizeXMVECTOR(const DirectX::XMFLOAT3& vec)
{
    return DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&vec));
}

_NODISCARD static inline auto ToNormalizeXMVECTOR(const DirectX::XMFLOAT2& vec)
{
    return DirectX::XMVector2Normalize(DirectX::XMLoadFloat2(&vec));
}

//. XMFLOATなどに変換-----------------------------------------------------------------------------------------------

_NODISCARD static inline auto ToXMFLOAT4(const DirectX::XMVECTOR& vec)
{
    DirectX::XMFLOAT4 rv;

    DirectX::XMStoreFloat4(&rv, vec);

    return rv;
}

_NODISCARD static inline auto ToXMFLOAT3(const DirectX::XMVECTOR& vec)
{
    DirectX::XMFLOAT3 rv;

    DirectX::XMStoreFloat3(&rv, vec);

    return rv;
}

_NODISCARD static inline auto ToXMFLOAT2(const DirectX::XMVECTOR& vec)
{
    DirectX::XMFLOAT2 rv;

    DirectX::XMStoreFloat2(&rv, vec);

    return rv;
}

_NODISCARD static inline auto ToXMFLOAT1(const DirectX::XMVECTOR& vec)
{
    float rv;

    DirectX::XMStoreFloat(&rv, vec);

    return rv;
}

_NODISCARD static inline auto ToXMFLOAT(const DirectX::XMINT4& vec)
{
    DirectX::XMFLOAT4 rv{ (float)vec.x, (float)vec.y, (float)vec.z, (float)vec.w };

    return rv;
}

_NODISCARD static inline auto ToXMFLOAT(const DirectX::XMINT3& vec)
{
    DirectX::XMFLOAT3 rv{ (float)vec.x, (float)vec.y, (float)vec.z };

    return rv;
}

_NODISCARD static inline auto ToXMFLOAT(const DirectX::XMINT2& vec)
{
    DirectX::XMFLOAT2 rv{ (float)vec.x, (float)vec.y };

    return rv;
}

_NODISCARD static inline auto ToXMINT(const DirectX::XMFLOAT4& vec)
{
    DirectX::XMINT4 rv{ (int)vec.x, (int)vec.y, (int)vec.z, (int)vec.w };

    return rv;
}

_NODISCARD static inline auto ToXMINT(const DirectX::XMFLOAT3& vec)
{
    DirectX::XMINT3 rv{ (int)vec.x, (int)vec.y, (int)vec.z };

    return rv;
}

_NODISCARD static inline auto ToXMINT(const DirectX::XMFLOAT2& vec)
{
    DirectX::XMINT2 rv{ (int)vec.x, (int)vec.y };

    return rv;
}

_NODISCARD static inline auto ToXMFLOAT(const DirectX::XMUINT4& vec)
{
    DirectX::XMFLOAT4 rv{ (float)vec.x, (float)vec.y, (float)vec.z, (float)vec.w };

    return rv;
}

_NODISCARD static inline auto ToXMFLOAT(const DirectX::XMUINT3& vec)
{
    DirectX::XMFLOAT3 rv{ (float)vec.x, (float)vec.y, (float)vec.z };

    return rv;
}

_NODISCARD static inline auto ToXMFLOAT(const DirectX::XMUINT2& vec)
{
    DirectX::XMFLOAT2 rv{ (float)vec.x, (float)vec.y };

    return rv;
}

_NODISCARD static inline auto ToXMUINT(const DirectX::XMFLOAT4& vec)
{
    DirectX::XMUINT4 rv{ (uint32_t)vec.x, (uint32_t)vec.y, (uint32_t)vec.z, (uint32_t)vec.w };

    return rv;
}

_NODISCARD static inline auto ToXMUINT(const DirectX::XMFLOAT3& vec)
{
    DirectX::XMUINT3 rv{ (uint32_t)vec.x, (uint32_t)vec.y, (uint32_t)vec.z };

    return rv;
}

_NODISCARD static inline auto ToXMUINT(const DirectX::XMFLOAT2& vec)
{
    DirectX::XMUINT2 rv{ (uint32_t)vec.x, (uint32_t)vec.y };

    return rv;
}

_NODISCARD static inline auto ToXMFLOAT(const std::array<float, 4>& vec)
{
    DirectX::XMFLOAT4 rv{ vec.front(), vec.at(1), vec.at(2), vec.back() };

    return rv;
}

_NODISCARD static inline auto ToXMFLOAT(const std::array<float, 3>& vec)
{
    DirectX::XMFLOAT3 rv{ vec.front(), vec.at(1), vec.back() };

    return rv;
}

_NODISCARD static inline auto ToXMFLOAT(const std::array<float, 2>& vec)
{
    DirectX::XMFLOAT2 rv{ vec.front(), vec.back() };

    return rv;
}

_NODISCARD static inline auto ToArray(const DirectX::XMFLOAT4& vec)
{
    std::array<float, 4> rv{ vec.x, vec.y, vec.z, vec.w };

    return rv;
}

_NODISCARD static inline auto ToArray(const DirectX::XMFLOAT3& vec)
{
    std::array<float, 3> rv{ vec.x, vec.y, vec.z };

    return rv;
}

_NODISCARD static inline auto ToArray(const DirectX::XMFLOAT2& vec)
{
    std::array<float, 2> rv{ vec.x, vec.y };

    return rv;
}

//. XMMATRIX変換-----------------------------------------------------------------------------------------

_NODISCARD static inline auto ToXMMATRIX(const DirectX::XMFLOAT4X4& vec)
{
    return DirectX::XMLoadFloat4x4(&vec);
}

_NODISCARD static inline auto ToXMMATRIX(const DirectX::XMFLOAT3X3& vec)
{
    return DirectX::XMLoadFloat3x3(&vec);
}

_NODISCARD static inline auto ToXMMATRIX(const DirectX::XMFLOAT3X4& vec)
{
    return DirectX::XMLoadFloat3x4(&vec);
}

_NODISCARD static inline auto ToXMMATRIX(const DirectX::XMFLOAT4X3& vec)
{
    return DirectX::XMLoadFloat4x3(&vec);
}

//. XMFLOAT変換-----------------------------------------------------------------------------------------------

_NODISCARD static inline auto ToXMFLOAT4X4(const DirectX::XMMATRIX& mat)
{
    DirectX::XMFLOAT4X4 rv;

    DirectX::XMStoreFloat4x4(&rv, mat);

    return rv;
}

_NODISCARD static inline auto ToXMFLOAT3X3(const DirectX::XMMATRIX& mat)
{
    DirectX::XMFLOAT3X3 rv;

    DirectX::XMStoreFloat3x3(&rv, mat);

    return rv;
}

_NODISCARD static inline auto ToXMFLOAT3X4(const DirectX::XMMATRIX& mat)
{
    DirectX::XMFLOAT3X4 rv;

    DirectX::XMStoreFloat3x4(&rv, mat);

    return rv;
}

_NODISCARD static inline auto ToXMFLOAT4X3(const DirectX::XMMATRIX& mat)
{
    DirectX::XMFLOAT4X3 rv;

    DirectX::XMStoreFloat4x3(&rv, mat);

    return rv;
}

//-------------------------------------------------------------------------------------------------------------
// 特殊変換関数
//-------------------------------------------------------------------------------------------------------------

// 型上げ-----------------------------------------------------------------------------------------------

_NODISCARD static inline auto RaiseToXMFLOAT4(const DirectX::XMFLOAT3& vec, const float w_component = 0.f)
{
    return DirectX::XMFLOAT4{ vec.x, vec.y, vec.z, w_component };
}

_NODISCARD static inline auto RaiseToXMFLOAT4(const DirectX::XMFLOAT2& vec, const float z_component = 0.f, const float w_component = 0.f)
{
    return DirectX::XMFLOAT4{ vec.x, vec.y, z_component, w_component };
}

_NODISCARD static inline auto RaiseToXMFLOAT3(const DirectX::XMFLOAT2& vec, const float z_component = 0.f)
{
    return DirectX::XMFLOAT3{ vec.x, vec.y, z_component };
}

// 型下げ-----------------------------------------------------------------------------------------------

// W, Z成分が切り捨てられる
_NODISCARD static inline auto LowerToXMFLOAT2(const DirectX::XMFLOAT4& vec)
{
    return DirectX::XMFLOAT2{ vec.x, vec.y };
}

// Z成分が切り捨てられる
_NODISCARD static inline auto LowerToXMFLOAT2(const DirectX::XMFLOAT3& vec)
{
    return DirectX::XMFLOAT2{ vec.x, vec.y };
}

// W成分が切り捨てられる
_NODISCARD static inline auto LowerToXMFLOAT3(const DirectX::XMFLOAT4& vec)
{
    return DirectX::XMFLOAT3{ vec.x, vec.y, vec.z };
}
