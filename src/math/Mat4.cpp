#include "math/Mat4.h"
#include "math/Quat.h"

void Mat4::makeIdentity()
{
	memset(m, 0, sizeof(m));

	m[0] = 1.0;
	m[5] = 1.0;
	m[10] = 1.0;
	m[15] = 1.0;
}

void Mat4::makeTranslation(const Vec3& v)
{
	makeIdentity();

	m[12] = v.x;
	m[13] = v.y;
	m[14] = v.z;
}

void Mat4::makeRotation(const Quat& q)
{
   m[0] = 1.0 - 2.0 * (q.y * q.y + q.z * q.z);
   m[1] = 2.0 * (q.x * q.y + q.w * q.z);
   m[2] = 2.0 * (q.x * q.z - q.w * q.y);
   m[3] = 0.0;

   m[4] = 2.0 * (q.x * q.y - q.w * q.z);
   m[5] = 1.0 - 2.0 * (q.x * q.x + q.z * q.z);
   m[6] = 2.0 * (q.y * q.z + q.w * q.x);
   m[7] = 0.0;

   m[8] = 2.0 * (q.x * q.z + q.w * q.y);
   m[9] = 2.0 * (q.y * q.z - q.w * q.x);
   m[10] = 1.0 - 2.0 * (q.x * q.x + q.y * q.y);
   m[11] = 0.0;

   m[12] = 0.0;
   m[13] = 0.0;
   m[14] = 0.0;
   m[15] = 1.0;
}
