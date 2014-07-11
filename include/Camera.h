#ifndef BZR_CAMERA_H
#define BZR_CAMERA_H

#include "math/Vec3.h"
#include "math/Mat4.h"

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

	const Mat4& viewMatrix() const;

private:
	Mat4 _viewMatrix;

	Vec3 _position;
	//double _yaw;
	//double _pitch;
	//double _roll;
};

#endif
