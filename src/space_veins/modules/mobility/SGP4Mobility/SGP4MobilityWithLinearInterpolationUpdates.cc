//
// Copyright (C) 2006-2012 Christoph Sommer <christoph.sommer@uibk.ac.at>
// Copyright (C) 2025 Mario Franke <research@m-franke.net>
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

#include "space_veins/modules/mobility/SGP4Mobility/SGP4MobilityWithLinearInterpolationUpdates.h"

using namespace space_veins;

Define_Module(space_veins::SGP4MobilityWithLinearInterpolationUpdates);

const simsignal_t SGP4MobilityWithLinearInterpolationUpdates::linearInterpolationSpeedMeterPerSecondSignal = registerSignal("space_veins_sgp4MobilityWithLinearInterpolationUpdates_linearInterpolationSpeedMeterPerSecondSignal");
const simsignal_t SGP4MobilityWithLinearInterpolationUpdates::linearInterpolationComputationsSignal = registerSignal("space_veins_sgp4MobilityWithLinearInterpolationUpdates_linearInterpolationComputationsSignal");

void SGP4MobilityWithLinearInterpolationUpdates::initialize(int stage)
{
    // EV_DEBUG << "SGP4MobilityWithLinearInterpolationUpdates stage: " << stage << std::endl;
    SGP4MobilityBase::initialize(stage);
    if (stage == 0) {
        WATCH(linearInterpolationVector);
    }
}

inet::Coord SGP4MobilityWithLinearInterpolationUpdates::linearInterpolation(simtime_t t) {
    // EV_TRACE << "SGP4MobilityWithLinearInterpolationUpdates::linearInterpolation(simtime_t t) called" << std::endl;
    // check that that linearInterpolationVector uses the correct positions
    // EV_TRACE << "SimTime: " << t << ", lastUpdate: " << lastUpdate << std::endl;
    // EV_TRACE << "lastCalculatedSgp4SatellitePosition: " << sgp4SatellitePositionToString(lastCalculatedSgp4SatellitePosition) << std::endl;
    // EV_TRACE << "linearInterpolationReference: " << sgp4SatellitePositionToString(linearInterpolationReference) << std::endl;
    if (linearInterpolationReference.simulationTime - lastCalculatedSgp4SatellitePosition.simulationTime != updateInterval) {
        EV_ERROR << "linearInterpolationReference.simulationTime - lastCalculatedSgp4SatellitePosition.simulationTime != updateInterval" << std::endl;
        throw cRuntimeError("linearInterpolationReference.simulationTime - lastCalculatedSgp4SatellitePosition.simulationTime != updateInterval");
    }
    // lastPosition + direction vector of length 1m * velocity in m/s * time span in s
    return lastPosition + linearInterpolationVector * linearInterpolationSpeedMeterPerSecond * (t - lastUpdate).dbl();
}

bool SGP4MobilityWithLinearInterpolationUpdates::nextGetCurrentPositionUpdate() {
    // EV_TRACE << "SGP4MobilityWithLinearInterpolationUpdates::nextGetCurrentPositionUpdate() called" << std::endl;
    simtime_t now = simTime();
    lastPosition = linearInterpolation(now);
    lastElevationAngleToSop = calculateElevationAngleRelativeToSop(lastPosition);
    // Management
    bool updated = true;
    updateManagementDataAndStatistics(getCurrentPositionUpdate, updated);
    return updated;
}

bool SGP4MobilityWithLinearInterpolationUpdates::nextPeriodicPositionUpdate() {
    // EV_TRACE << "SGP4MobilityWithLinearInterpolationUpdates::nextPeriodicPositionUpdate() called" << std::endl;
    bool updated = false;
    simtime_t now = simTime();
    if (now == linearInterpolationReference.simulationTime) {
        // EV_TRACE << "SGP4MobilityWithLinearInterpolationUpdates::nextPeriodicPositionUpdate(): now == linearInterpolationReference.simulationTime -> we can simply copy the reference." << std::endl;
        // we can update the lastCalculatedSgp4SatellitePosition by overwriting it with linearInterpolationReference.
        // lastCalculatedSgp4SatellitePosition is the satellite's position at t=tau and linearInterpolationReference is always the satellite's position at t=tau+updateInterval
        lastCalculatedSgp4SatellitePosition = linearInterpolationReference;
        lastPosition = lastCalculatedSgp4SatellitePosition.omnetppCoord;
        recordSgp4SpeedMeterPerSecond(lastCalculatedSgp4SatellitePosition.sgp4SpeedMeterPerSecond);
        lastElevationAngleToSop = calculateElevationAngleRelativeToSop(lastPosition);
    } else {
        // EV_TRACE << "SGP4MobilityWithLinearInterpolationUpdates::nextPeriodicPositionUpdate(): now != linearInterpolationReference.simulationTime -> reference is outdated!" << std::endl;
        SGP4PositionUpdate(now);
    }
    // calculate the next linear interpolation reference at simTime() + updateInterval
    linearInterpolationReference = calculateSgp4SatellitePosition(now + updateInterval);
    // update the vector pointing from lastCalculatedSgp4SatellitePosition to linearInterpolationReference
    linearInterpolationVector = (linearInterpolationReference.omnetppCoord - lastCalculatedSgp4SatellitePosition.omnetppCoord);
    linearInterpolationSpeedMeterPerSecond = linearInterpolationVector.length() / updateInterval.dbl();
    linearInterpolationVector = linearInterpolationVector.normalize();
    // Management
    updated = true;
    updateManagementDataAndStatistics(periodicUpdate, updated);
    return updated;
}

void SGP4MobilityWithLinearInterpolationUpdates::updateManagementDataAndStatistics(updateType_t type, bool updated) {
    // EV_TRACE << "SGP4MobilityWithLinearInterpolationUpdates::updateManagementDataAndStatistics: updateType_t: " << type << ", updated: " << updated << std::endl;
    SGP4MobilityBase::updateManagementDataAndStatistics(type, updated);
    if (updated) {
        if (type == periodicUpdate) {
            recordLinearInterpolationSpeedMeterPerSecond(linearInterpolationSpeedMeterPerSecond);
        } else if (type == getCurrentPositionUpdate) {
            recordLinearInterpolationComputation(lastElevationAngleToSop);
        } else {
            EV_ERROR << "SGP4MobilityWithLinearInterpolationUpdates::updateManagementDataAndStatistics: undefined updateType_t: " << type << std::endl;
            throw cRuntimeError("SGP4MobilityWithLinearInterpolationUpdates::updateManagementDataAndStatistics: undefined updateType_t");
        }
    }
}
void SGP4MobilityWithLinearInterpolationUpdates::recordLinearInterpolationSpeedMeterPerSecond(const double speed)
{
    emit(linearInterpolationSpeedMeterPerSecondSignal, speed);
}

void SGP4MobilityWithLinearInterpolationUpdates::recordLinearInterpolationComputation(inet::deg elevationAngleToSop)
{
    emit(linearInterpolationComputationsSignal, elevationAngleToSop.get());
}

void SGP4MobilityWithLinearInterpolationUpdates::finish()
{
    SGP4MobilityBase::finish();
}