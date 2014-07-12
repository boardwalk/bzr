#include "Camera.h"

Camera::Camera()
{
    _viewMatrix.makeIdentity();
    _position = Vec3(-86, -78, -120);
}

void Camera::look(double dx, double dy)
{
    // TODO
    _position.z += dx * 30.0;

    //printf("%f %f %f\n", _position.x, _position.y, _position.z);
}

void Camera::move(double dx, double dy)
{
    // TODO
    _position.x += dx * 30.0;
    _position.y += dy * 30.0;

    //printf("%f %f %f\n", _position.x, _position.y, _position.z);
}

void Camera::step(double dt)
{
    _viewMatrix.makeTranslation(_position);
}

const Mat4& Camera::viewMatrix() const
{
    return _viewMatrix;
}