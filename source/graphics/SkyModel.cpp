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
#include "graphics/SkyModel.h"
#include <algorithm>

// A Practical Analytic Model for Daylight
// A. J. Preetham et al
// http://www.cs.utah.edu/~shirley/papers/sunsky/sunsky.pdf

static fp_t F(fp_t coeff[5], fp_t theta, fp_t gamma)
{
    return (fp_t(1.0) + coeff[0] * glm::exp(coeff[1] / glm::cos(theta))) *
        (fp_t(1.0) + coeff[2] * glm::exp(coeff[3] * gamma) + coeff[4] * glm::pow(glm::cos(gamma), fp_t(2.0)));
}

static fp_t tonemap(fp_t luminance)
{
    luminance *= 16.0;
    fp_t x = max(fp_t(0.0), luminance - fp_t(0.004));
    return (x * (fp_t(6.2) * x + fp_t(0.5))) / (x * (fp_t(6.2) * x + fp_t(1.7)) + fp_t(0.06));
}

void SkyModel::prepare(const Params& p)
{
    // calculate solar time
    auto t_s = p.tm * fp_t(24.0); // standard time in decimal hours
    auto J = p.dt; // Julian day
    auto SM = int(p.lng / fp_t(15.0)) * fp_t(15.0); // standard meridian for time zone
    auto L = p.lng; // longitude in radians

    auto t = t_s + fp_t(0.170) * glm::sin(fp_t(4.0) * pi() * (J - fp_t(80.0)) / fp_t(373.0)) - fp_t(0.129) * glm::sin(fp_t(2.0) * pi() * (J - fp_t(8.0)) / fp_t(355.0)) + fp_t(12.0) * (SM - L) / pi();

    // calculate solar declination
    auto delta = fp_t(0.4093) * glm::sin(fp_t(2.0) * pi() * (J - fp_t(81.0)) / fp_t(368.0));

    // calculate solar position
    auto l = p.lat; // latitude in radians

    _theta_s = pi() / fp_t(2.0) - glm::asin(sin(l) * sin(delta) - cos(l) * cos(delta) * cos(pi() * t / fp_t(12.0)));
    _phi_s = (fp_t)atan2(-glm::cos(delta) * glm::sin(pi() * t / fp_t(12.0)), (glm::cos(l) * glm::sin(delta) - glm::sin(l) * glm::cos(delta) * glm::cos(pi() * t / fp_t(12.0))));

    // calculate A B, C, D and E coefficients
    _coeffs_Y[0] = fp_t(0.1787) * p.tu - fp_t(1.4630);
    _coeffs_Y[1] = fp_t(-0.3554) * p.tu + fp_t(0.4275);
    _coeffs_Y[2] = fp_t(-0.0227) * p.tu + fp_t(5.3251);
    _coeffs_Y[3] = fp_t(0.1206) * p.tu - fp_t(2.5771);
    _coeffs_Y[4] = fp_t(-0.0670) * p.tu + fp_t(0.3703);

    _coeffs_x[0] = fp_t(-0.0193) * p.tu - fp_t(0.2592);
    _coeffs_x[1] = fp_t(-0.0665) * p.tu + fp_t(0.0008);
    _coeffs_x[2] = fp_t(-0.0004) * p.tu + fp_t(0.2125);
    _coeffs_x[3] = fp_t(-0.0641) * p.tu - fp_t(0.8989);
    _coeffs_x[4] = fp_t(-0.0033) * p.tu + fp_t(0.0452);

    _coeffs_y[0] = fp_t(-0.0167) * p.tu - fp_t(0.2608);
    _coeffs_y[1] = fp_t(-0.0950) * p.tu + fp_t(0.0092);
    _coeffs_y[2] = fp_t(-0.0079) * p.tu + fp_t(0.2102);
    _coeffs_y[3] = fp_t(-0.0441) * p.tu - fp_t(1.6537);
    _coeffs_y[4] = fp_t(-0.0109) * p.tu + fp_t(0.0529);

    // calculate Y sub z
    auto chi = (fp_t(4.0 / 9.0) - p.tu / fp_t(120.0)) * (pi() - fp_t(2.0) * _theta_s);
    _Y_z = (fp_t(4.0453) * p.tu - fp_t(4.9710)) * tan(chi) - fp_t(0.2155) * p.tu + fp_t(2.4192);
    _Y_z *= fp_t(1000.0); // convert kcd/m^2 to cd/m^2

    auto tu_sq = p.tu * p.tu;
    auto theta_s_sq = _theta_s * _theta_s;
    auto theta_s_cu = theta_s_sq * _theta_s;

    // calculate x sub z
    auto c1 = tu_sq * fp_t(0.00166) + p.tu * fp_t(-0.02903) + fp_t(0.11693);
    auto c2 = tu_sq * fp_t(-0.00375) + p.tu * fp_t(0.06377) + fp_t(-0.21196);
    auto c3 = tu_sq * fp_t(0.00209) + p.tu * fp_t(-0.03202) + fp_t(0.06052);
    auto c4 = p.tu * fp_t(0.00394) + fp_t(0.25886);

    _x_z = c1 * theta_s_cu + c2 * theta_s_sq + c3 * _theta_s + c4;

    // calculate y sub z
    c1 = tu_sq * fp_t(0.00275) + p.tu * fp_t(-0.04214) + fp_t(0.15346);
    c2 = tu_sq * fp_t(-0.00610) + p.tu * fp_t(0.08970) + fp_t(-0.26756);
    c3 = tu_sq * fp_t(0.00317) + p.tu * fp_t(-0.04153) + fp_t(0.06670);
    c4 = p.tu * fp_t(0.00516) + fp_t(0.26688);

    _y_z = c1 * theta_s_cu + c2 * theta_s_sq + c3 * _theta_s + c4;
}

glm::vec3 toCartesian(fp_t theta, fp_t phi)
{
    return glm::vec3(
        glm::sin(theta)*glm::cos(phi),
        glm::sin(theta)*glm::sin(phi),
        glm::cos(theta));
}

glm::vec3 SkyModel::getColor(fp_t theta, fp_t phi)
{
    // this is the angle between theta_s, phi_s and theta, phi
    auto gamma = glm::acos(glm::dot(toCartesian(theta, phi), toCartesian(_theta_s, _phi_s)));

    auto Y = F(_coeffs_Y, theta, gamma) / F(_coeffs_Y, 0.0, _theta_s) * _Y_z;
    auto x = F(_coeffs_x, theta, gamma) / F(_coeffs_x, 0.0, _theta_s) * _x_z;
    auto y = F(_coeffs_y, theta, gamma) / F(_coeffs_y, 0.0, _theta_s) * _y_z;

    // Y is luminance in candela/m^2 (aka nit)
    Y = min(tonemap(abs(Y)), fp_t(1.0));

    // convert xyY to XYZ
    // http://www.brucelindbloom.com/index.html?Equations.html
    glm::vec3 XYZ;

    if(y > 0)
    {
        XYZ.x = x * Y / y;
        XYZ.y = Y;
        XYZ.z = (fp_t(1.0) - x - y) * Y / y;
    }

    auto RGB = glm::vec3(
        XYZ.x * fp_t(3.2406)  + XYZ.y * fp_t(-1.5372) + XYZ.z * fp_t(-0.4986),
        XYZ.x * fp_t(-0.9689) + XYZ.y * fp_t( 1.8758) + XYZ.z * fp_t( 0.0415),
        XYZ.x * fp_t(0.0557)  + XYZ.y * fp_t(-0.2040) + XYZ.z * fp_t( 1.0570));

    RGB.x = min(RGB.x, fp_t(1.0));
    RGB.y = min(RGB.y, fp_t(1.0));
    RGB.z = min(RGB.z, fp_t(1.0));

    return RGB;
}

fp_t SkyModel::thetaSun() const
{
    return _theta_s;
}

fp_t SkyModel::phiSun() const
{
    return _phi_s;
}
