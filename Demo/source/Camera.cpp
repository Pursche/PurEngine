#include "Camera.h"
#include <windows.h>

Camera::Camera(const Vector3& pos)
    : _viewMatrix()
{
    _position = pos;
    _rotation = Vector3(0, 0, 0);
}


void Camera::Update(f32 deltaTime)
{
    // Movement
    if (GetAsyncKeyState('W'))
    {
        _position += _viewMatrix.at * -_movementSpeed * deltaTime;
    }
    if (GetAsyncKeyState('S'))
    {
        _position += _viewMatrix.at * _movementSpeed * deltaTime;
    }
    if (GetAsyncKeyState('A'))
    {
        _position += _viewMatrix.right * _movementSpeed * deltaTime;
    }
    if (GetAsyncKeyState('D'))
    {
        _position += _viewMatrix.right * -_movementSpeed * deltaTime;
    }
    if (GetAsyncKeyState(VK_SPACE))
    {
        _position += _viewMatrix.up * -_movementSpeed * deltaTime;
    }
    if (GetAsyncKeyState(VK_SHIFT))
    {
        _position += _viewMatrix.up * _movementSpeed * deltaTime;
    }
    if (GetAsyncKeyState(VK_TAB))
    {
        _mouseControlMode = !_mouseControlMode;
    }

    // Rotation
    Vector2 rotation;
    if (!_mouseControlMode)
    {
        if (GetAsyncKeyState(VK_UP))
        {
            rotation.x = -_rotationSpeed * deltaTime;
        }
        if (GetAsyncKeyState(VK_DOWN))
        {
            rotation.x = _rotationSpeed * deltaTime;
        }
        if (GetAsyncKeyState(VK_LEFT))
        {
            rotation.y = -_rotationSpeed * deltaTime;
        }
        if (GetAsyncKeyState(VK_RIGHT))
        {
            rotation.y = _rotationSpeed * deltaTime;
        }
    }
    _rotation += rotation;
    if (_rotation.x > 360)
        _rotation.x = 0;
    if (_rotation.x < 0)
        _rotation.x = 360;

    if (_rotation.y > 360)
        _rotation.y = 0;
    if (_rotation.y < 0)
        _rotation.y = 360;

    // Calculate view matrix
    _viewMatrix = Matrix(true);
    _viewMatrix.pos = _position;
    _viewMatrix.RotateX(_rotation.x, Matrix::MultiplicationType::PRE);
    _viewMatrix.RotateY(_rotation.y, Matrix::MultiplicationType::PRE);
    _viewMatrix.RotateZ(_rotation.z, Matrix::MultiplicationType::PRE);
}