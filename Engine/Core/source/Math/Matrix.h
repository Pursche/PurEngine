#pragma once
#include "Math.h"
#include "Vector3.h"
#include "Vector4.h"

// This is a 4x4 Matrix, if we ever need other kinds we can refactor
class Matrix
{
public:
    enum MultiplicationType
    {
        PRE,	/**< Multiply the new matrix before the current matrix */
        POST,	/**< Multiply the new matrix after the current matrix */
        REPLACE	/**< Replace the current matrix with the new one */
    };

public:
    Matrix(bool identity = true);
    Matrix(const Matrix& other);

#pragma warning(push)
#pragma warning(disable : 4201) // warning C4201:  nonstandard extension used: nameless struct/union
    union
    {
        struct
        {
            Vector4 row[4];
        };
        struct
        {
            Vector3 right;
            float pad1;
            Vector3 up;
            float pad2;
            Vector3 at;
            float pad3;
            Vector3 pos;
            float pad4;
        };
        struct
        {
            float element[16];
        };
    };
#pragma warning(pop)

    Matrix& operator=(const Matrix& other);

    Matrix operator*(const Matrix& other) const;
    Matrix operator*(const float scalar) const;
    Matrix operator+(const Matrix& other) const;
    Matrix operator/(const float scalar) const;

    Matrix& operator*=(const Matrix& other);
    Matrix& operator*=(const float scalar);
    Matrix& operator+=(const Matrix& other);

    Matrix& Transpose();
    Matrix Transposed() const;

    Matrix& Invert();
    Matrix Inverted() const;

    Matrix& RotateX(float angle, MultiplicationType mulType);
    Matrix& RotateY(float angle, MultiplicationType mulType);
    Matrix& RotateZ(float angle, MultiplicationType mulType);
    Matrix& RotateXYZ(Vector3& angle, MultiplicationType mulType);

    Matrix& Scale(Vector3 scale, MultiplicationType mulType);
    Matrix& Scale(float scale, MultiplicationType mulType);
    Matrix& Transform(const Matrix& transformation, MultiplicationType mulType);
    void TransformVector(Vector3& vector, bool translation) const;
};