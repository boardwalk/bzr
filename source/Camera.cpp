#include "Camera.h"
#include "math/Quat.h"
#include <algorithm>

static const auto FT_PER_SECOND = 100.0;

Camera::Camera()
{
    _position = Vec3(88.5, 22.5, 125);
    _yaw = 0.0;
    _pitch = 0.26666;
    _roll = 0.0;
}

void Camera::look(double dx, double dy)
{
    _pitch = fmod(_pitch - dy, 2.0 * PI);
    _yaw = fmod(_yaw + dx, 2.0 * PI);
}

void Camera::move(double dx, double dy)
{
    Vec3 dp(dx * FT_PER_SECOND, 0.0, -dy * FT_PER_SECOND);
    _position = _position +_rotationQuat.conjugate() * dp;
}

void Camera::step(double dt)
{
    (void)dt;
    
    Quat initialPitchQuat;
    initialPitchQuat.makeFromYawPitchRoll(0.0, -PI / 4.0, 0.0);

    Quat yawQuat;
    yawQuat.makeFromYawPitchRoll(_yaw, 0.0, 0.0);

    Quat pitchQuat;
    pitchQuat.makeFromYawPitchRoll(0.0, _pitch, 0.0);

    _rotationQuat = pitchQuat * yawQuat * initialPitchQuat;

    Mat4 rotationMatrix;
    rotationMatrix.makeRotation(_rotationQuat);

    _viewMatrix.makeTranslation(-_position);
    _viewMatrix = rotationMatrix * _viewMatrix;
}

const Quat& Camera::rotationQuat() const
{
    return _rotationQuat;
}

const Mat4& Camera::viewMatrix() const
{
    return _viewMatrix;
}

void Camera::setPosition(const Vec3& position)
{
    _position = position;
}

const Vec3& Camera::position() const
{
    return _position;
}
