/*
 * Bael'Zharon's Respite
 * Copyright (C) 2014 Daniel Skorupski
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include "Camera.h"
#include <algorithm>

Camera::Camera()
{
    _speed = 0.0;
    _position = Vec3(88.5, 22.5, 125);
    _yaw = 0.0;
    _pitch = 0.26666;
    _roll = 0.0;
    updateRotationQuat();
}

void Camera::move(double dx, double dy)
{
    Vec3 dp(dx * _speed, 0.0, -dy * _speed);
    _position = _position +_rotationQuat.conjugate() * dp;
    updateViewMatrix();
}

void Camera::look(double dx, double dy)
{
    _pitch = fmod(_pitch - dy, 2.0 * PI);
    _yaw = fmod(_yaw + dx, 2.0 * PI);
    updateRotationQuat();
}

void Camera::step(double dt)
{
    (void)dt;
}

void Camera::setSpeed(double newSpeed)
{
    _speed = newSpeed;
}

void Camera::setPosition(const Vec3& newPosition)
{
    _position = newPosition;
    updateViewMatrix();
}

void Camera::setHeadPosition(const Vec3& newHeadPosition)
{
    _headPosition = newHeadPosition;
    updateViewMatrix();
}

void Camera::setHeadOrientation(const Quat& newHeadOrientation)
{
    _headOrientation = newHeadOrientation;
    updateRotationQuat();
}

const Vec3& Camera::position() const
{
    return _position;
}

const Quat& Camera::rotationQuat() const
{
    return _rotationQuat;
}

const Mat4& Camera::viewMatrix() const
{
    return _viewMatrix;
}

void Camera::updateRotationQuat()
{
    Quat initialPitchQuat;
    initialPitchQuat.makeFromYawPitchRoll(0.0, -PI / 4.0, 0.0);

    Quat yawQuat;
    yawQuat.makeFromYawPitchRoll(_yaw, 0.0, 0.0);

    Quat pitchQuat;
    pitchQuat.makeFromYawPitchRoll(0.0, _pitch, 0.0);

    _rotationQuat = _headOrientation * pitchQuat * yawQuat * initialPitchQuat;

    updateViewMatrix();
}

void Camera::updateViewMatrix()
{
    Mat4 rotationMatrix;
    rotationMatrix.makeRotation(_rotationQuat);

    _viewMatrix.makeTranslation(-(_position + _headPosition));
    _viewMatrix = rotationMatrix * _viewMatrix;
}
