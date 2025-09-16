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

#include "space_veins/modules/mobility/SGP4Mobility/SGP4MobilityWithNearestPeriodicPositionUpdates.h"

using namespace space_veins;

Define_Module(space_veins::SGP4MobilityWithNearestPeriodicPositionUpdates);

void SGP4MobilityWithNearestPeriodicPositionUpdates::initialize(int stage)
{
    EV_DEBUG << "SGP4MobilityWithNearestPeriodicPositionUpdates stage: " << stage << std::endl;
    SGP4MobilityBase::initialize(stage);
}

bool SGP4MobilityWithNearestPeriodicPositionUpdates::nextGetCurrentPositionUpdate() {
    EV_TRACE << "SGP4MobilityWithNearestPeriodicPositionUpdates::nextGetCurrentPositionUpdate() called" << std::endl;
    bool updated = false;
    simtime_t now = simTime();
    simtime_t timeSinceLastPeriodicUpdate = now - lastPeriodicUpdate;
    if (timeSinceLastPeriodicUpdate <= updateInterval/2) {
        // no need to update lastPosition, getCurrentPosition is supposed to simply return the current lastPosition, thus this function returns false as there is no update of lastPosition.
        updated = false;
    } else if (lastPosition == linearInterpolationReference.omnetppCoord) {
        // no need to update lastPosition, as it already stores the correct position
        updated = false;
    } else { // timeSinceLastPeriodicUpdate > updateInterval/2 --> use the SGP4-based position from lastPeriodicUpdate + updateInterval, even if timeSinceLastPeriodicUpdate > updateInterval due to updatePositionOnlyAboveHorizon == true
        // if linearInterpolationReference is NOT outdated we can use it
        // if linearInterpolationReference.simulationTime == lastPeriodicUpdate + updateInterval && now - linearInterpolationReference.simulationTime <= 0 (linearInterpolationReference is in the future or up-to-date)
        if (linearInterpolationReference.simulationTime == lastPeriodicUpdate + updateInterval && now - linearInterpolationReference.simulationTime <= 0) {
            // linearInterpolationReference is NOT outdated, we can use it.
            lastPosition = linearInterpolationReference.omnetppCoord;
            lastElevationAngleToSop = calculateElevationAngleRelativeToSop(lastPosition);
        } else { // simTime() - linearInterpolationReference.simulationTime <= 0 --> linearInterpolationReference stores the position for lastPeriodicUpdate + updateInterval
            // linearInterpolationReference IS outdated
            // update the satellite's position using the SGP4Model, calculate it at lastPeriodicUpdate + updateInterval, as this is the time of the next periodic position update
            SGP4PositionUpdate(lastPeriodicUpdate + updateInterval);
        }
        // calculate the next linear interpolation reference at lastPeriodicUpdate + updateInterval
        linearInterpolationReference = calculateSgp4SatellitePosition(lastPeriodicUpdate + updateInterval);
        // update management data
        updated = true;
        updateManagementDataAndStatistics(getCurrentPositionUpdate, updated);
    }
    return updated;
}

bool SGP4MobilityWithNearestPeriodicPositionUpdates::nextPeriodicPositionUpdate() {
    EV_TRACE << "SGP4MobilityWithNearestPeriodicPositionUpdates::nextPeriodicPositionUpdate() called" << std::endl;
    bool updated = false;
    simtime_t now = simTime();
    if (now == linearInterpolationReference.simulationTime) {
        EV_TRACE << "SGP4MobilityWithNearestPeriodicPositionUpdates::nextPeriodicPositionUpdate(): now == linearInterpolationReference.simulationTime -> we can simply copy the reference." << std::endl;
        // we can update the lastCalculatedSgp4SatellitePosition by overwriting it with linearInterpolationReference.
        // lastCalculatedSgp4SatellitePosition is the satellite's position at t=tau and linearInterpolationReference is always the satellite's position at t=tau+updateInterval
        lastCalculatedSgp4SatellitePosition = linearInterpolationReference;
        lastPosition = lastCalculatedSgp4SatellitePosition.omnetppCoord;
        recordSgp4SpeedMeterPerSecond(lastCalculatedSgp4SatellitePosition.sgp4SpeedMeterPerSecond);
        lastElevationAngleToSop = calculateElevationAngleRelativeToSop(lastPosition);
    } else {
        EV_TRACE << "SGP4MobilityWithNearestPeriodicPositionUpdates::nextPeriodicPositionUpdate(): now != linearInterpolationReference.simulationTime -> reference is outdated!" << std::endl;
        SGP4PositionUpdate(now);
    }
    // calculate the next linear interpolation reference at simTime() + updateInterval
    linearInterpolationReference = calculateSgp4SatellitePosition(now + updateInterval);
    // Management
    updated = true;
    updateManagementDataAndStatistics(periodicUpdate, updated);
    return updated;
}

void SGP4MobilityWithNearestPeriodicPositionUpdates::finish()
{
    SGP4MobilityBase::finish();
}