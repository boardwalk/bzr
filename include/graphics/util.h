#ifndef BZR_GRAPHICS_UTIL_H
#define BZR_GRAPHICS_UTIL_H

struct Mat3;
struct Mat4;

void loadMat3ToUniform(const Mat3& mat, GLint location);
void loadMat4ToUniform(const Mat4& mat, GLint location);

#endif
