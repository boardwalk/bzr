#ifndef BZR_GRAPHICS_SKYMODEL_H
#define BZR_GRAPHICS_SKYMODEL_H

#include "math/Vec3.h"

class SkyModel
{
public:
    struct Params
    {
        // dt is the day of the year, between 0 and 365
        double dt;

        // tm is a normalized value, 0 indicating midnight, 0.5 indicating noon
        double tm;

        // lng is in radians, -pi to pi
        double lng;

        // lat is in radians, -pi to pi
        double lat;

        // tu is the turbidity, or haziness, of the sky
        // rough scale of values:
        //   1 pure air
        // 1.5 very clear
        // 2.5 clear
        //   8 light haze
        //  24 haze
        //  48 thin fog
        double tu;
    };

    // prepare for getColor to be called
    void prepare(const Params& p);

    // theta is the zenith angle (angle from directly above)
    // phi is the azimuth angle (ccw angle from directly south)
    // returns a normalized sRGB color value
    Vec3 getColor(double theta, double phi);

    double thetaSun() const;
    double phiSun() const;

private:
    // zenith angle of the sun
    double _theta_s;

    // azimuth angle of the sun
    double _phi_s;

    // A, B, C, D and E coefficients for F()
    double _coeffs_x[5];
    double _coeffs_y[5];
    double _coeffs_Y[5];

    // color at zenith
    double _x_z;
    double _y_z;
    double _Y_z;
};

#endif
