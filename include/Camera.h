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

class Camera
{
public:
    Camera();

    // -dx strafe left
    // +dx strafe right
    // -dy move back
    // +dy move forward
    void move(fp_t dx, fp_t dy);

    // -dx look left
    // +dx look right
    // -dy look down
    // +dy look up
    void look(fp_t dx, fp_t dy);

    void step(fp_t dt);

    void setSpeed(fp_t newSpeed);
    void setPosition(const glm::vec3& newPosition);
    void setHeadPosition(const glm::vec3& newHeadPosition);
    void setHeadOrientation(const glm::quat& newHeadOrientation);

    const glm::vec3& position() const;
    const glm::quat& rotationQuat() const;
    const glm::mat4& viewMatrix() const;

private:
    void updateRotationQuat();
    void updateViewMatrix();
    
    fp_t speed_;
    glm::vec3 position_;
    fp_t yaw_;
    fp_t pitch_;
    fp_t roll_;
    
    glm::vec3 headPosition_;
    glm::quat headOrientation_;

    glm::quat rotationQuat_;
    glm::mat4 viewMatrix_;
};

#endif
