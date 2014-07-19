#ifndef BZR_CAMERA_H
#define BZR_CAMERA_H

#include "math/Mat4.h"
#include "math/Vec3.h"
#include "math/Quat.h"

class Camera
{
public:
    Camera();

    // -dx look left
    // +dx look right
    // -dy look down
    // +dy look up
    void look(double dx, double dy);

    // -dx strafe left
    // +dx strafe right
    // -dy move back
    // +dy move forward
    void move(double dx, double dy);

    void step(double dt);

    const Quat& rotationQuat() const;
    const Mat4& viewMatrix() const;
    void setPosition(const Vec3& position);
    const Vec3& position() const;

private:
    Quat _rotationQuat;
    Mat4 _viewMatrix;

    Vec3 _position;

    double _yaw;
    double _pitch;
    double _roll;
};

#endif
