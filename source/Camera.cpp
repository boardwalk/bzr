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
    speed_ = fp_t(0.0);
    position_ = glm::vec3(88.5, 22.5, 125);
    yaw_ = fp_t(0.0);
    pitch_ = fp_t(0.26666);
    roll_ = fp_t(0.0);
    updateRotationQuat();
}

void Camera::move(fp_t dx, fp_t dy)
{
    glm::vec3 dp(dx * speed_, 0.0, -dy * speed_);
    position_ = position_ + glm::conjugate(rotationQuat_) * dp;
    updateViewMatrix();
}

void Camera::look(fp_t dx, fp_t dy)
{
    pitch_ = glm::mod(pitch_ - fp_t(2.0) * dy, fp_t(2.0) * pi());
    yaw_ = glm::mod(yaw_ + fp_t(2.0) * dx, fp_t(2.0) * pi());
    updateRotationQuat();
}

void Camera::step(fp_t dt)
{
    (void)dt;
}

void Camera::setSpeed(fp_t newSpeed)
{
    speed_ = newSpeed;
}

void Camera::setPosition(const glm::vec3& newPosition)
{
    position_ = newPosition;
    updateViewMatrix();
}

void Camera::setHeadPosition(const glm::vec3& newHeadPosition)
{
    headPosition_ = newHeadPosition;
    updateViewMatrix();
}

void Camera::setHeadOrientation(const glm::quat& newHeadOrientation)
{
    headOrientation_ = newHeadOrientation;
    updateRotationQuat();
}

const glm::vec3& Camera::position() const
{
    return position_;
}

const glm::quat& Camera::rotationQuat() const
{
    return rotationQuat_;
}

const glm::mat4& Camera::viewMatrix() const
{
    return viewMatrix_;
}

void Camera::updateRotationQuat()
{
    auto initialPitchQuat = glm::angleAxis(-pi() / fp_t(2.0), glm::vec3(1.0, 0.0, 0.0));
    auto yawQuat = glm::angleAxis(yaw_, glm::vec3(0.0, 1.0, 0.0));
    auto pitchQuat = glm::angleAxis(pitch_, glm::vec3(1.0, 0.0, 0.0));

    rotationQuat_ = headOrientation_ * pitchQuat * yawQuat * initialPitchQuat;

    updateViewMatrix();
}

void Camera::updateViewMatrix()
{
    viewMatrix_ = glm::mat4_cast(rotationQuat_) * glm::translate(glm::mat4(), -(position_ + headPosition_));
}
