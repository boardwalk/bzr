#ifndef BZR_MATH_MAT3_H
#define BZR_MATH_MAT3_H

struct Mat4;

struct Mat3
{
    Mat3();
    Mat3(const Mat4& other);

    Mat3 inverse() const;
    Mat3 transpose() const;

    double m[9];
};

#endif
