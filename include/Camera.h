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
#ifndef BZR_CAMERA_H
#define BZR_CAMERA_H

#include "math/Mat4.h"
#include "math/Quat.h"
#include "math/Vec3.h"

class Camera
{
public:
    Camera();

    // -dx strafe left
    // +dx strafe right
    // -dy move back
    // +dy move forward
    void move(double dx, double dy);

    // -dx look left
    // +dx look right
    // -dy look down
    // +dy look up
    void look(double dx, double dy);

    void step(double dt);

    void setSpeed(double newSpeed);
    void setPosition(const Vec3& newPosition);
    void setHeadPosition(const Vec3& newHeadPosition);
    void setHeadOrientation(const Quat& newHeadOrientation);

    const Vec3& position() const;
    const Quat& rotationQuat() const;
    const Mat4& viewMatrix() const;

private:
    void updateRotationQuat();
    void updateViewMatrix();
    
    double _speed;
    Vec3 _position;
    double _yaw;
    double _pitch;
    double _roll;
    
    Vec3 _headPosition;
    Quat _headOrientation;

    Quat _rotationQuat;
    Mat4 _viewMatrix;
};

#endif
