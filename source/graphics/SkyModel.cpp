#include "graphics/SkyModel.h"
#include <algorithm>

// A Practical Analytic Model for Daylight
// A. J. Preetham et al
// http://www.cs.utah.edu/~shirley/papers/sunsky/sunsky.pdf

static double F(double coeff[5], double theta, double gamma)
{
    return (1.0 + coeff[0] * exp(coeff[1] / cos(theta))) * (1.0 + coeff[2] * exp(coeff[3] * gamma) + coeff[4] * pow(cos(gamma), 2.0));
}

static double tonemap(double luminance)
{
    luminance *= 16.0;
    auto x = max(0.0, luminance - 0.004);
    return (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);
    //const double whiteLuminance = 8000.0;
    //return luminance * (1.0 + luminance / (whiteLuminance * whiteLuminance)) / (1.0 + luminance);
}

void SkyModel::prepare(const Params& p)
{
    // calculate solar time
    auto t_s = p.tm * 24.0; // standard time in decimal hours
    auto J = p.dt; // Julian day
    auto SM = int(p.lng / 15.0) * 15.0; // standard meridian for time zone
    auto L = p.lng; // longitude in radians

    auto t = t_s + 0.170 * sin(4 * PI * (J - 80.0) / 373.0) - 0.129 * sin(2 * PI * (J - 8.0) / 355.0) + 12.0 * (SM - L) / PI;

    // calculate solar declination
    auto delta = 0.4093 * sin(2 * PI * (J - 81.0) / 368.0);

    // calculate solar position
    auto l = p.lat; // latitude in radians

    _theta_s = PI / 2.0 - asin(sin(l) * sin(delta) - cos(l) * cos(delta) * cos(PI * t / 12.0));
    _phi_s = atan2(-cos(delta) * sin(PI * t / 12.0), (cos(l) * sin(delta) - sin(l) * cos(delta) * cos(PI * t / 12.0)));

    // calculate A B, C, D and E coefficients
    _coeffs_Y[0] = 0.1787 * p.tu - 1.4630;
    _coeffs_Y[1] = -0.3554 * p.tu + 0.4275;
    _coeffs_Y[2] = -0.0227 * p.tu + 5.3251;
    _coeffs_Y[3] = 0.1206 * p.tu - 2.5771;
    _coeffs_Y[4] = -0.0670 * p.tu + 0.3703;

    _coeffs_x[0] = -0.0193 * p.tu - 0.2592;
    _coeffs_x[1] = -0.0665 * p.tu + 0.0008;
    _coeffs_x[2] = -0.0004 * p.tu + 0.2125;
    _coeffs_x[3] = -0.0641 * p.tu - 0.8989;
    _coeffs_x[4] = -0.0033 * p.tu + 0.0452;

    _coeffs_y[0] = -0.0167 * p.tu - 0.2608;
    _coeffs_y[1] = -0.0950 * p.tu + 0.0092;
    _coeffs_y[2] = -0.0079 * p.tu + 0.2102;
    _coeffs_y[3] = -0.0441 * p.tu - 1.6537;
    _coeffs_y[4] = -0.0109 * p.tu + 0.0529;

    // calculate Y sub z
    auto chi = (4.0 / 9.0 - p.tu / 120.0) * (PI - 2.0 * _theta_s);
    _Y_z = (4.0453 * p.tu - 4.9710) * tan(chi) - 0.2155 * p.tu + 2.4192;
    _Y_z *= 1000.0; // convert kcd/m^2 to cd/m^2

    auto tu_sq = p.tu * p.tu;
    auto theta_s_sq = _theta_s * _theta_s;
    auto theta_s_cu = theta_s_sq * _theta_s;

    // calculate x sub z
    auto c1 = tu_sq * 0.00166 + p.tu * -0.02903 + 0.11693;
    auto c2 = tu_sq * -0.00375 + p.tu * 0.06377 + -0.21196;
    auto c3 = tu_sq * 0.00209 + p.tu * -0.03202 + 0.06052;
    auto c4 = p.tu * 0.00394 + 0.25886;

    _x_z = c1 * theta_s_cu + c2 * theta_s_sq + c3 * _theta_s + c4;

    // calculate y sub z
    c1 = tu_sq * 0.00275 + p.tu * -0.04214 + 0.15346;
    c2 = tu_sq * -0.00610 + p.tu * 0.08970 + -0.26756;
    c3 = tu_sq * 0.00317 + p.tu * -0.04153 + 0.06670;
    c4 = p.tu * 0.00516 + 0.26688;

    _y_z = c1 * theta_s_cu + c2 * theta_s_sq + c3 * _theta_s + c4;
}

Vec3 toCartesian(double theta, double phi)
{
    return Vec3(
        sin(theta)*cos(phi),
        sin(theta)*sin(phi),
        cos(theta));
}

Vec3 SkyModel::getColor(double theta, double phi)
{
    // this is the angle between theta_s, phi_s and theta, phi
    auto gamma = acos(toCartesian(theta, phi).dot(toCartesian(_theta_s, _phi_s)));

    auto Y = F(_coeffs_Y, theta, gamma) / F(_coeffs_Y, 0.0, _theta_s) * _Y_z;
    auto x = F(_coeffs_x, theta, gamma) / F(_coeffs_x, 0.0, _theta_s) * _x_z;
    auto y = F(_coeffs_y, theta, gamma) / F(_coeffs_y, 0.0, _theta_s) * _y_z;

    // Y is luminance in candela/m^2 (aka nit)
    Y = min(tonemap(abs(Y)), 1.0);

    // convert xyY to XYZ
    // http://www.brucelindbloom.com/index.html?Equations.html
    Vec3 XYZ;

    if(y > 0)
    {
        XYZ.x = x * Y / y;
        XYZ.y = Y;
        XYZ.z = (1.0 - x - y) * Y / y;
    }

    auto RGB = Vec3(
        XYZ.x *  3.2406 + XYZ.y * -1.5372 + XYZ.z * -0.4986,
        XYZ.x * -0.9689 + XYZ.y *  1.8758 + XYZ.z *  0.0415,
        XYZ.x *  0.0557 + XYZ.y * -0.2040 + XYZ.z *  1.0570);

    RGB.x = min(RGB.x, 1.0);
    RGB.y = min(RGB.y, 1.0);
    RGB.z = min(RGB.z, 1.0);

    return RGB;
}

double SkyModel::thetaSun() const
{
    return _theta_s;
}

double SkyModel::phiSun() const
{
    return _phi_s;
}
