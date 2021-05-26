//
// Copyright (C) 2006-2012 Christoph Sommer <christoph.sommer@uibk.ac.at>
// Copyright (C) 2021 Mario Franke <research@m-franke.net>
//
// Documentation for these modules is at http://veins.car2x.org/
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

#include "space_veins/modules/mobility/SkyfieldMobility.h"

using namespace space_veins;

//using space_veins::SkyfieldMobility;
using WGS84Coordinate = SatellitesConnectionManager::WGS84Coordinate;

Define_Module(space_veins::SkyfieldMobility);

SatellitesConnectionManager* SkyfieldMobility::getSatellitesConnectionManager()
{
    return veins::FindModule<SatellitesConnectionManager*>::findGlobalModule();
}

void SkyfieldMobility::initialize(int stage)
{
    BaseMobility::initialize(stage);
    if (stage == 0) {

        updateInterval_ms = par("updateInterval").doubleValue() * 1000;
        satelliteName = this->getParentModule()->par("satelliteName").stringValue();

        observerPosition = veins::Coord(0,0,0);

        EV_DEBUG << "Initializing SkyfieldMobility module." << std::endl;
        EV_DEBUG << "SkyfieldMobility updateInterval_ms: " << updateInterval_ms << std::endl;

        auto t_spec = veins::TimerSpecification([this]() {updateSatellitePosition();});
        t_spec.absoluteStart(SimTime(updateInterval_ms, SIMTIME_MS));
        t_spec.interval(SimTime(updateInterval_ms, SIMTIME_MS));
        timerManager.create(t_spec, "updateSatellitePosition");

        /* Statistics */
        distance_km_to_observationPosition_vec.setName("distance_km_to_observationPosition");
        altitude_deg_to_observationPosition_vec.setName("altitude_deg_to_observationPosition");
        azimuth_deg_to_observationPosition_vec.setName("azimuth_deg_to_observationPosition");

        currentPosXVec.setName("satellitePosX");
        currentPosYVec.setName("satellitePosY");
        currentPosZVec.setName("satellitePosZ");

    }else if (stage == 1) {
        scc = getSatellitesConnectionManager();
        ASSERT(scc);
    }
}

void SkyfieldMobility::updateSatellitePosition()
{
    EV_DEBUG << "SkyfieldMobility update RelativeSatellitePosition at " << simTime() << std::endl;
    EV_DEBUG << "SkyfieldMobility current position: " << this->getPositionAt(simTime()) << std::endl;
    // WGS84Coordinate geo = scc->omnetCoord2GeoCoord(this->getPositionAt(simTime()));
    WGS84Coordinate geo = scc->omnetCoord2GeoCoord(observerPosition);
    EV_DEBUG << "SkyfieldMobility current geo position: latitude: " << geo.first << ", longitude: " << geo.second << std::endl;
    rsp = scc->getRelativeSatellitePosition(geo, satelliteName);
    EV_DEBUG << "SkyfieldMobility rsp: " << rsp << std::endl;
    heading = skyfieldAzimuth2VeinsHeading(rsp.azimuth_deg);
    veins::Coord unitDirVecAltitude = getUnitDirectionVectorAltitude(rsp.altitude_deg);
    EV_DEBUG << "SkyfieldMobility unitDirVecAltitude: " << unitDirVecAltitude << std::endl;
    veins::Coord rspUnitDirVec = getUnitDirectionVector(heading, unitDirVecAltitude);
    EV_DEBUG << "SkyfieldMobility rspUnitDirVec: " << rspUnitDirVec << std::endl;
    veins::Coord satellitePosition = rspUnitDirVec * rsp.distance_km * 1000;
    EV_DEBUG << "SkyfieldMobility satellitePosition: " << satellitePosition << std::endl;
    // Update move object which stores the position, direction, orientation, and speed
    move.setStart(satellitePosition);
    // Update direction and orientation
    // TODO: Currently there are no values for the direction of the satellite
    // viewing angle of the reference position is used instead.
    move.setDirectionByVector(rspUnitDirVec);
    move.setOrientationByVector(rspUnitDirVec);
    // Set speed
    // TODO: No speed information available, set it to 0
    move.setSpeed(0);
    // publish the the new move
    emit(mobilityStateChangedSignal, this);

    /* Statistics */
    distance_km_to_observationPosition_vec.record(rsp.distance_km);
    altitude_deg_to_observationPosition_vec.record(rsp.altitude_deg);
    azimuth_deg_to_observationPosition_vec.record(rsp.azimuth_deg);

    currentPosXVec.record(satellitePosition.x);
    currentPosYVec.record(satellitePosition.y);
    currentPosZVec.record(satellitePosition.z);
}

veins::Heading SkyfieldMobility::skyfieldAzimuth2VeinsHeading(double azimuth_deg)
{
    EV_DEBUG << "Skyfield azimuth angle in degree: " << azimuth_deg << std::endl;
    // transformation from skyfield azimuth orientation to veins heading
    double veins_heading_deg = 360 - azimuth_deg + 90;
    // normalize angle
    while (veins_heading_deg >= 360) {
        veins_heading_deg -= 360;
    }
    while (veins_heading_deg < 0) {
        veins_heading_deg += 360;
    }
    EV_DEBUG << "Veins heading angle in degree: " << veins_heading_deg << std::endl;
    // Heading expects an angle in radians
    return veins::Heading(veins_heading_deg * (M_PI / 180));
}

veins::Coord SkyfieldMobility::getUnitDirectionVectorAltitude(double altitude_deg)
{
    // azimuth_deg == 0 -> returns vector in xz-plane
    return veins::Coord(cos(altitude_deg * (M_PI / 180)),   // x
                        0,                                  // y
                        sin(altitude_deg * (M_PI / 180))    // z
                        );
}

veins::Coord SkyfieldMobility::getUnitDirectionVector(veins::Heading azimuth, veins::Coord unitDirectionVectorAltitude)
{
    return veins::Coord(cos(azimuth.getRad()) * unitDirectionVectorAltitude.x,        // x
                        -1 * sin(azimuth.getRad()) * unitDirectionVectorAltitude.x,   // y
                        unitDirectionVectorAltitude.z                                 // z
                        );
}

void SkyfieldMobility::finish()
{
    BaseMobility::finish();
}

void SkyfieldMobility::handleSelfMsg(cMessage* msg)
{
    EV_DEBUG << "SkyfieldMobility self msg: " << msg->info() << std::endl;
    if (timerManager.handleMessage(msg)) {
        return;
    }
}
