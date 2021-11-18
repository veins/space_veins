// SPDX-FileCopyrightText: 2021 Mario Franke <research@m-franke.net>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// Various constants defined and required by Skyfield.

// Definitions.
double AU_M = 149597870700;             // per IAU 2012 Resolution B2
double AU_KM = 149597870.700;
double ASEC360 = 1296000.0;
double DAY_S = 86400.0;

// Angles.
double ASEC2RAD = 4.848136811095359935899141e-6;
double DEG2RAD = 0.017453292519943296;
double RAD2DEG = 57.295779513082321;
double pi = 3.141592653589793;
double tau = 6.283185307179586476925287;  // lower case, for symmetry with math.pi

// Physics.
double C = 299792458.0;                            // m/s
double GM_SUN_Pitjeva_2005_km3_s2 = 132712440042;  // Elena Pitjeva, 2015JPCRD..44c1210P

// Earth and its orbit.
double ANGVEL = 7.2921150e-5;           // radians/s
double ERAD = 6378136.6;                // meters
double IERS_2010_INVERSE_EARTH_FLATTENING = 298.25642;

// Heliocentric gravitational constant in meters^3 / second^2, from DE-405.
double GS = 1.32712440017987e+20;

// Time.
double T0 = 2451545.0;
double B1950 = 2433282.4235;

double C_AUDAY = C * DAY_S / AU_M;

// vectors
std::vector<double> _cross120 = {1.0, 2.0, 0.0};
std::vector<double> _cross201 = {2.0, 0.0, 1.0};
