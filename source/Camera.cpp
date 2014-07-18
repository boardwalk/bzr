#include "Camera.h"
#include "math/Quat.h"

Camera::Camera()
{
    _position = Vec3(88.5, 22.5, 125);
    _yaw = 0.0;
    _pitch = -0.26666;
    _roll = 0.0;
}

void Camera::look(double dx, double dy)
{
    // TODO
    _pitch -= dy;
    _yaw += dx;
    
    //_position.z -= dy * 30.0;

    //printf("position=%f %f %f, pitch=%f\n", _position.x, _position.y, _position.z, _pitch);
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
    Quat q;
    q.makeFromYawPitchRoll(_yaw, _pitch, _roll);

    _rotationMatrix.makeRotation(q);

    _viewMatrix.makeTranslation(-_position);
    _viewMatrix = _rotationMatrix * _viewMatrix;
}

const Mat4& Camera::rotationMatrix() const
{
    return _rotationMatrix;
}

const Mat4& Camera::viewMatrix() const
{
    return _viewMatrix;
}

const Vec3& Camera::position() const
{
    return _position;
}
