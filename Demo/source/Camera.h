#pragma once
#include <Core.h>

class Camera
{
public:
    Camera(const Vector3& pos);

    void Update(f32 deltaTime);

    Matrix& GetViewMatrix() { return _viewMatrix; }
private:

private:
    bool _mouseControlMode = false;
    Vector3 _position;
    Vector3 _rotation;
    Matrix _viewMatrix;
    f32 _movementSpeed = 3.0f;
    f32 _rotationSpeed = 90.0f;
};