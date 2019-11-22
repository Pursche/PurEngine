#pragma once
#include <string>
#include "Math.h"

class Vector3;

// Right handed coordinate system, positive X points north and positive Y points west
class Vector2
{
public:
    // Constructors
    Vector2() {};
    Vector2(float inX, float inY)
    {
        x = inX;
        y = inY;
    }
    Vector2(float in)
    {
        x = in;
        y = in;
    }
    Vector2(unsigned char inX, unsigned char inY)
    {
        x = static_cast<float>(inX);
        y = static_cast<float>(inY);
    }
    Vector2(unsigned char in)
    {
        x = static_cast<float>(in);
        y = static_cast<float>(in);
    }
    Vector2(unsigned short inX, unsigned short inY)
    {
        x = static_cast<float>(inX);
        y = static_cast<float>(inY);
    }
    Vector2(unsigned short in)
    {
        x = static_cast<float>(in);
        y = static_cast<float>(in);
    }
    Vector2(unsigned int inX, unsigned int inY)
    {
        x = static_cast<float>(inX);
        y = static_cast<float>(inY);
    }
    Vector2(unsigned int in)
    {
        x = static_cast<float>(in);
        y = static_cast<float>(in);
    }
    Vector2(signed char inX, signed char inY)
    {
        x = static_cast<float>(inX);
        y = static_cast<float>(inY);
    }
    Vector2(signed char in)
    {
        x = static_cast<float>(in);
        y = static_cast<float>(in);
    }
    Vector2(signed short inX, signed short inY)
    {
        x = static_cast<float>(inX);
        y = static_cast<float>(inY);
    }
    Vector2(signed short in)
    {
        x = static_cast<float>(in);
        y = static_cast<float>(in);
    }
    Vector2(signed int inX, signed int inY)
    {
        x = static_cast<float>(inX);
        y = static_cast<float>(inY);
    }
    Vector2(signed int in)
    {
        x = static_cast<float>(in);
        y = static_cast<float>(in);
    }

    //Vector2(Vector2& in) { x = in.x; y = in.y; }
    Vector2(const Vector2& in)
    {
        x = in.x;
        y = in.y;
    }
    Vector2(const Vector3& in);

    // Returns the length of the vector
    inline float Length()
    {
        return Math::Sqrt(SqrLength());
    }

    // Returns the squared length of the vector
    inline float SqrLength()
    {
        return x * x + y * y;
    }

    // Returns the dot product of the vector and another vector
    inline float Dot(const Vector2& other)
    {
        return (x * other.x) + (y * other.y);
    }

    // Returns the vector with a length of 1, does not modify the original
    inline Vector2 Normalize()
    {
        float length = Length();
        return Vector2(x / length, y / length);
    }

    // Returns the lerp result of the vector over T
    static Vector2 Lerp(Vector2 start, Vector2 end, float t);

    // Returns a nicely formatted string of the vector
    inline std::string ToString()
    {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
    }

    // Overloaded operators
    inline Vector2 operator+(const Vector2& other)
    {
        return Vector2(x + other.x, y + other.y);
    }

    inline Vector2 operator+(const float other)
    {
        return Vector2(x + other, y + other);
    }

    inline Vector2 operator+(const unsigned char other)
    {
        return Vector2(x + other, y + other);
    }

    inline Vector2 operator+(const unsigned short other)
    {
        return Vector2(x + other, y + other);
    }

    inline Vector2 operator+(const unsigned int other)
    {
        return Vector2(x + other, y + other);
    }

    inline Vector2 operator+(const signed char other)
    {
        return Vector2(x + other, y + other);
    }

    inline Vector2 operator+(const signed short other)
    {
        return Vector2(x + other, y + other);
    }

    inline Vector2 operator+(const signed int other)
    {
        return Vector2(x + other, y + other);
    }

    inline Vector2 operator-(const Vector2& other)
    {
        return Vector2(x - other.x, y - other.y);
    }

    inline Vector2 operator-(const float other)
    {
        return Vector2(x - other, y - other);
    }

    inline Vector2 operator-(const unsigned char other)
    {
        return Vector2(x - other, y - other);
    }

    inline Vector2 operator-(const unsigned short other)
    {
        return Vector2(x - other, y - other);
    }

    inline Vector2 operator-(const unsigned int other)
    {
        return Vector2(x - other, y - other);
    }

    inline Vector2 operator-(const signed char other)
    {
        return Vector2(x - other, y - other);
    }

    inline Vector2 operator-(const signed short other)
    {
        return Vector2(x - other, y - other);
    }

    inline Vector2 operator-(const signed int other)
    {
        return Vector2(x - other, y - other);
    }

    inline Vector2 operator*(const Vector2& other)
    {
        return Vector2(x * other.x, y * other.y);
    }

    inline Vector2 operator*(const float other)
    {
        return Vector2(x * other, y * other);
    }

    inline Vector2 operator*(const unsigned char other)
    {
        return Vector2(x * other, y * other);
    }

    inline Vector2 operator*(const unsigned short other)
    {
        return Vector2(x * other, y * other);
    }

    inline Vector2 operator*(const unsigned int other)
    {
        return Vector2(x * other, y * other);
    }

    inline Vector2 operator*(const signed char other)
    {
        return Vector2(x * other, y * other);
    }

    inline Vector2 operator*(const signed short other)
    {
        return Vector2(x * other, y * other);
    }

    inline Vector2 operator*(const signed int other)
    {
        return Vector2(x * other, y * other);
    }

    inline Vector2 operator/(const Vector2& other)
    {
        return Vector2(x / other.x, y / other.y);
    }

    inline Vector2 operator/(const float other)
    {
        return Vector2(x / other, y / other);
    }

    inline Vector2 operator/(const unsigned char other)
    {
        return Vector2(x / other, y / other);
    }

    inline Vector2 operator/(const unsigned short other)
    {
        return Vector2(x / other, y / other);
    }

    inline Vector2 operator/(const unsigned int other)
    {
        return Vector2(x / other, y / other);
    }

    inline Vector2 operator/(const signed char other)
    {
        return Vector2(x / other, y / other);
    }

    inline Vector2 operator/(const signed short other)
    {
        return Vector2(x / other, y / other);
    }

    inline Vector2 operator/(const signed int other)
    {
        return Vector2(x / other, y / other);
    }

    inline Vector2 operator%(const Vector2& other)
    {
        return Vector2(Math::Modulus(x, other.x), Math::Modulus(y, other.y));
    }

    inline Vector2 operator%(const float other)
    {
        return Vector2(Math::Modulus(x, other), Math::Modulus(y, other));
    }

    inline Vector2 operator%(const unsigned char other)
    {
        return Vector2(Math::Modulus(x, static_cast<float>(other)), Math::Modulus(y, static_cast<float>(other)));
    }

    inline Vector2 operator%(const unsigned short other)
    {
        return Vector2(Math::Modulus(x, static_cast<float>(other)), Math::Modulus(y, static_cast<float>(other)));
    }

    inline Vector2 operator%(const unsigned int other)
    {
        return Vector2(Math::Modulus(x, static_cast<float>(other)), Math::Modulus(y, static_cast<float>(other)));
    }

    inline Vector2 operator%(const signed char other)
    {
        return Vector2(Math::Modulus(x, static_cast<float>(other)), Math::Modulus(y, static_cast<float>(other)));
    }

    inline Vector2 operator%(const signed short other)
    {
        return Vector2(Math::Modulus(x, static_cast<float>(other)), Math::Modulus(y, static_cast<float>(other)));
    }

    inline Vector2 operator%(const signed int other)
    {
        return Vector2(Math::Modulus(x, static_cast<float>(other)), Math::Modulus(y, static_cast<float>(other)));
    }

    // opAssign
    inline Vector2& operator+=(const Vector2& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    inline Vector2& operator+=(const float other)
    {
        x += other;
        y += other;
        return *this;
    }

    inline Vector2& operator+=(const unsigned char other)
    {
        x += other;
        y += other;
        return *this;
    }

    inline Vector2& operator+=(const unsigned short other)
    {
        x += other;
        y += other;
        return *this;
    }

    inline Vector2& operator+=(const unsigned int other)
    {
        x += other;
        y += other;
        return *this;
    }

    inline Vector2& operator+=(const signed char other)
    {
        x += other;
        y += other;
        return *this;
    }

    inline Vector2& operator+=(const signed short other)
    {
        x += other;
        y += other;
        return *this;
    }

    inline Vector2& operator+=(const signed int other)
    {
        x += other;
        y += other;
        return *this;
    }

    inline Vector2& operator-=(const Vector2& other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    inline Vector2& operator-=(const float other)
    {
        x -= other;
        y -= other;
        return *this;
    }

    inline Vector2& operator-=(const unsigned char other)
    {
        x -= other;
        y -= other;
        return *this;
    }

    inline Vector2& operator-=(const unsigned short other)
    {
        x -= other;
        y -= other;
        return *this;
    }

    inline Vector2& operator-=(const unsigned int other)
    {
        x -= other;
        y -= other;
        return *this;
    }

    inline Vector2& operator-=(const signed char other)
    {
        x -= other;
        y -= other;
        return *this;
    }

    inline Vector2& operator-=(const signed short other)
    {
        x -= other;
        y -= other;
        return *this;
    }

    inline Vector2& operator-=(const signed int other)
    {
        x -= other;
        y -= other;
        return *this;
    }

    inline Vector2& operator*=(const Vector2& other)
    {
        x *= other.x;
        y *= other.y;
        return *this;
    }

    inline Vector2& operator*=(const float other)
    {
        x *= other;
        y *= other;
        return *this;
    }

    inline Vector2& operator*=(const unsigned char other)
    {
        x *= other;
        y *= other;
        return *this;
    }

    inline Vector2& operator*=(const unsigned short other)
    {
        x *= other;
        y *= other;
        return *this;
    }

    inline Vector2& operator*=(const unsigned int other)
    {
        x *= other;
        y *= other;
        return *this;
    }

    inline Vector2& operator*=(const signed char other)
    {
        x *= other;
        y *= other;
        return *this;
    }

    inline Vector2& operator*=(const signed short other)
    {
        x *= other;
        y *= other;
        return *this;
    }

    inline Vector2& operator*=(const signed int other)
    {
        x *= other;
        y *= other;
        return *this;
    }

    inline Vector2& operator/=(const Vector2& other)
    {
        x /= other.x;
        y /= other.y;
        return *this;
    }

    inline Vector2& operator/=(const float other)
    {
        x /= other;
        y /= other;
        return *this;
    }

    inline Vector2& operator/=(const unsigned char other)
    {
        x /= other;
        y /= other;
        return *this;
    }

    inline Vector2& operator/=(const unsigned short other)
    {
        x /= other;
        y /= other;
        return *this;
    }

    inline Vector2& operator/=(const unsigned int other)
    {
        x /= other;
        y /= other;
        return *this;
    }

    inline Vector2& operator/=(const signed char other)
    {
        x /= other;
        y /= other;
        return *this;
    }

    inline Vector2& operator/=(const signed short other)
    {
        x /= other;
        y /= other;
        return *this;
    }

    inline Vector2& operator/=(const signed int other)
    {
        x /= other;
        y /= other;
        return *this;
    }

    // Comp
    bool operator==(const Vector2& other)
    {
        return (x == other.x) && (y == other.y);
    }

    bool operator!=(const Vector2& other)
    {
        return (x != other.x) || (y != other.y);
    }

public:
    float x = 0.0f;
    float y = 0.0f;

public:
    static Vector2 One;
    static Vector2 Zero;
    static Vector2 Up;
    static Vector2 Down;
    static Vector2 Left;
    static Vector2 Right;
};