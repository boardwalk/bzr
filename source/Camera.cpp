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
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

Camera::Camera()
{
    _speed = fp_t(0.0);
    _position = glm::vec3(88.5, 22.5, 125);
    _yaw = fp_t(0.0);
    _pitch = fp_t(0.26666);
    _roll = fp_t(0.0);
    updateRotationQuat();
}

void Camera::move(fp_t dx, fp_t dy)
{
    glm::vec3 dp(dx * _speed, 0.0, -dy * _speed);
    _position = _position + glm::conjugate(_rotationQuat) * dp;
    updateViewMatrix();
}

void Camera::look(fp_t dx, fp_t dy)
{
    _pitch = glm::mod(_pitch - fp_t(2.0) * dy, fp_t(2.0) * pi());
    _yaw = glm::mod(_yaw + fp_t(2.0) * dx, fp_t(2.0) * pi());
    updateRotationQuat();
}

void Camera::step(fp_t dt)
{
    (void)dt;
}

void Camera::setSpeed(fp_t newSpeed)
{
    _speed = newSpeed;
}

void Camera::setPosition(const glm::vec3& newPosition)
{
    _position = newPosition;
    updateViewMatrix();
}

void Camera::setHeadPosition(const glm::vec3& newHeadPosition)
{
    _headPosition = newHeadPosition;
    updateViewMatrix();
}

void Camera::setHeadOrientation(const glm::quat& newHeadOrientation)
{
    _headOrientation = newHeadOrientation;
    updateRotationQuat();
}

const glm::vec3& Camera::position() const
{
    return _position;
}

const glm::quat& Camera::rotationQuat() const
{
    return _rotationQuat;
}

const glm::mat4& Camera::viewMatrix() const
{
    return _viewMatrix;
}

void Camera::updateRotationQuat()
{
    auto initialPitchQuat = glm::angleAxis(-pi() / fp_t(2.0), glm::vec3(1.0, 0.0, 0.0));
    auto yawQuat = glm::angleAxis(_yaw, glm::vec3(0.0, 1.0, 0.0));
    auto pitchQuat = glm::angleAxis(_pitch, glm::vec3(1.0, 0.0, 0.0));

    _rotationQuat = _headOrientation * pitchQuat * yawQuat * initialPitchQuat;

    updateViewMatrix();
}

void Camera::updateViewMatrix()
{
    _viewMatrix = glm::mat4_cast(_rotationQuat) * glm::translate(glm::mat4(), -(_position + _headPosition));
}
