//
// Copyright (C) OpenSim Ltd.
// Copyright (C) 2022 Mario Franke <research@m-franke.net>
//
// Documentation for these modules is at http://sat.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "space_veins/modules/mobility/SatelliteFacingAntennaMobility/SatelliteFacingAntennaMobility.h"

using namespace space_veins;

Define_Module(space_veins::SatelliteFacingAntennaMobility);


void SatelliteFacingAntennaMobility::initialize(int stage)
{
    FacingMobility::initialize(stage);
    EV_TRACE << "initializing SatelliteFacingMobility stage " << stage << endl;
}

inet::Quaternion SatelliteFacingAntennaMobility::getCurrentAngularPosition()
{
    EV_DEBUG << "SatelliteFacingMobility: Satellite Position: " << targetMobility->getCurrentPosition() << std::endl;
    EV_DEBUG << "SatelliteFacingMobility: Source mobility position: " << sourceMobility->getCurrentPosition() << std::endl;
    inet::Coord direction = targetMobility->getCurrentPosition() - sourceMobility->getCurrentPosition();
    distanceMeter = inet::m(direction.length());
    EV_DEBUG << "SatelliteFacingMobility: distance (m): " << distanceMeter << std::endl;
    direction.normalize();
    auto alpha = inet::rad(atan2(direction.y, direction.x));  // psi
    azimuthDeg = inet::deg(alpha);
    EV_DEBUG << "SatelliteFacingMobility: azimuth (deg): " << azimuthDeg << std::endl;
    auto beta = inet::rad(-asin(direction.z));    // theta
    elevationDeg = inet::deg(-beta);    // -beta because beta is the rotation angle for the antenna
    EV_DEBUG << "SatelliteFacingMobility: elevation (deg): " << elevationDeg << std::endl;
    auto gamma = inet::rad(0.0);  // phi
    lastOrientation = inet::Quaternion(inet::EulerAngles(alpha, beta, gamma));
    return lastOrientation;
}

inet::Coord SatelliteFacingAntennaMobility::getCurrentPosition()
{
    lastPosition = sourceMobility->getCurrentPosition();
    return lastPosition;
}

inet::Coord SatelliteFacingAntennaMobility::getCurrentVelocity()
{
    return sourceMobility->getCurrentVelocity();
}

inet::Coord SatelliteFacingAntennaMobility::getCurrentAcceleration()
{
    return sourceMobility->getCurrentAcceleration();
}
