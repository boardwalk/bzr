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
