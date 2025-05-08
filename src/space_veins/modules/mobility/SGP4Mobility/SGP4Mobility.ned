//
// Copyright (C) 2021 Mario Franke <research@m-franke.net>
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
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

package space_veins.modules.mobility.SGP4Mobility;

import inet.mobility.base.MovingMobilityBase;

//
// Mobility module for spacecrafts based on SGP4 model.
//
simple SGP4Mobility extends MovingMobilityBase
{
    parameters:
        @class(space_veins::SGP4Mobility);
        @display("i=block/cogwheel");
        bool updatePositionOnlyAboveHorizon = default(false); // update satellite position only when it is above the horizon of the SOP
        double precalculationInterval_s = default(180s) @unit(s);    // calculate satellite positions every <precalculationInterval_s> seconds for testing if the satellite is above the horizon
        int getCurrentPositionBehavior = default(0); // Changes the behavior of getCurrentPosition(): 0: getCurrentPosition() returns the last calculated position. It does not recalculate it according to the current simulation time. It does not consider the simulation time. If the last update happened at t_0=5.0s and the updateInterval is 1.0s and getCurrentPosition() is called at t_1=5.5s, the position at t_0 is returned.
        //                                                                                            1: if lastPosition is outdated (lastUpdate != simTime()) the satellite's position is calculated according to simTime() using the SGP4 model. Otherwise lastPosition is returned.
        //                                                                                            2: if lastPosition is outdated (lastUpdate != simTime()) the satellite's position is calculated according to simTime() using linear interpolation. Otherwise lastPosition is returned.
        //                                                                                            3: if lastPosition is outdated (lastUpdate != simTime()) the returned satellite position is the nearest position calculated by a periodic update: if simTime() - lastPeriodicUpdate <= updateInterval/2 return satellite position at t = lastUpdate; if simTime() - lastPeriodicUpdate > updateInterval/2 return satellite position at t = lastPeriodicUpdate + updateInterval
        // OMNeT++ event management
        @signal[mobilityStateChanged](type=inet::MobilityBase);
        @signal[space_veins_sgp4mobility_processedEventSignal](type=long);
        @signal[space_veins_sgp4mobility_periodicUpdateSignal](type=long);
        @signal[space_veins_sgp4mobility_periodicUpdateCallSignal](type=long);
        @signal[space_veins_sgp4mobility_getCurrentPositionUpdateSignal](type=long);
        @signal[space_veins_sgp4mobility_getCurrentPositionUpdateCallSignal](type=long);
        @signal[space_veins_sgp4mobility_moveAndUpdateCallSignal](type=long);
        @signal[space_veins_sgp4mobility_moveAndUpdateLastUpdateNowCallSignal](type=long);
        @signal[space_veins_sgp4mobility_moveAndUpdateLastUpdateNotNowCallSignal](type=long);
        // SGP4Mobility events
        @signal[space_veins_sgp4mobility_lastElevationAngleToSopSignal](type=double);
        @signal[space_veins_sgp4mobility_satelliteRisesAboveHorizonSignal](type=simtime_t);
        @signal[space_veins_sgp4mobility_satelliteSetsBelowHorizonSignal](type=simtime_t);
        @signal[space_veins_sgp4mobility_periodAboveHorizonSignal](type=simtime_t);
        // SGP4Mobility positions and coordinates
        @signal[space_veins_sgp4mobility_omnetCoordX](type=double);
        @signal[space_veins_sgp4mobility_omnetCoordY](type=double);
        @signal[space_veins_sgp4mobility_omnetCoordZ](type=double);
        @signal[space_veins_sgp4mobility_wgs84CoordLat](type=double);
        @signal[space_veins_sgp4mobility_wgs84CoordLon](type=double);
        @signal[space_veins_sgp4mobility_wgs84CoordAlt](type=double);
        @signal[space_veins_sgp4mobility_wgs84CartCoordX](type=double);
        @signal[space_veins_sgp4mobility_wgs84CartCoordY](type=double);
        @signal[space_veins_sgp4mobility_wgs84CartCoordZ](type=double);
        @signal[space_veins_sgp4mobility_sopRelativeCoordX](type=double);
        @signal[space_veins_sgp4mobility_sopRelativeCoordY](type=double);
        @signal[space_veins_sgp4mobility_sopRelativeCoordZ](type=double);
        @signal[space_veins_sgp4mobility_linearInterpolationSpeedMeterPerSecondSignal](type=double);
        @signal[space_veins_sgp4mobility_sgp4SpeedMeterPerSecondSignal](type=double);
        // SGP4Mobility performance
        @signal[space_veins_sgp4mobility_linearInterpolationComputationsSignal](type=double);
        @signal[space_veins_sgp4mobility_sgp4ComputationsSignal](type=double);

        // OMNeT++ event management
        @statistic[positionUpdates](source=mobilityStateChanged; record=count);
        @statistic[processedEvents](source=space_veins_sgp4mobility_processedEventSignal; record=count, vector);
        @statistic[periodicUpdates](source=space_veins_sgp4mobility_periodicUpdateSignal; record=count, vector);
        @statistic[periodicUpdateCalls](source=space_veins_sgp4mobility_periodicUpdateCallSignal; record=count, vector);
        @statistic[getCurrentPositionUpdates](source=space_veins_sgp4mobility_getCurrentPositionUpdateSignal; record=count, vector);
        @statistic[getCurrentPositionUpdateCalls](source=space_veins_sgp4mobility_getCurrentPositionUpdateCallSignal; record=count, vector);
        @statistic[moveAndUpdateCalls](source=space_veins_sgp4mobility_moveAndUpdateCallSignal; record=count, vector);
        @statistic[moveAndUpdateLastUpdateNowCalls](source=space_veins_sgp4mobility_moveAndUpdateLastUpdateNowCallSignal; record=count, vector);
        @statistic[moveAndUpdateLastUpdateNotNowCalls](source=space_veins_sgp4mobility_moveAndUpdateLastUpdateNotNowCallSignal; record=count, vector);
        // SGP4Mobility events
        @statistic[lastElevationAngleToSop](source=space_veins_sgp4mobility_lastElevationAngleToSopSignal; record=count, vector);
        @statistic[horizonRises](source=space_veins_sgp4mobility_satelliteRisesAboveHorizonSignal; record=count, vector);
        @statistic[horizonSets](source=space_veins_sgp4mobility_satelliteSetsBelowHorizonSignal; record=count, vector);
        @statistic[periodsAboveHorizon](source=space_veins_sgp4mobility_periodAboveHorizonSignal; record=count, vector);
        @statistic[totalTimeAboveHorizon](source=sum(space_veins_sgp4mobility_periodAboveHorizonSignal); record=last);
        // SGP4Mobility positions and coordinates
        @statistic[omnetCoordX](source=space_veins_sgp4mobility_omnetCoordX; record=vector);
        @statistic[omnetCoordY](source=space_veins_sgp4mobility_omnetCoordY; record=vector);
        @statistic[omnetCoordZ](source=space_veins_sgp4mobility_omnetCoordZ; record=vector);
        @statistic[wgs84CoordLat](source=space_veins_sgp4mobility_wgs84CoordLat; record=vector);
        @statistic[wgs84CoordLon](source=space_veins_sgp4mobility_wgs84CoordLon; record=vector);
        @statistic[wgs84CoordAlt](source=space_veins_sgp4mobility_wgs84CoordAlt; record=vector);
        @statistic[wgs84CartCoordX](source=space_veins_sgp4mobility_wgs84CartCoordX; record=vector);
        @statistic[wgs84CartCoordY](source=space_veins_sgp4mobility_wgs84CartCoordY; record=vector);
        @statistic[wgs84CartCoordZ](source=space_veins_sgp4mobility_wgs84CartCoordZ; record=vector);
        @statistic[sopRelativeCoordX](source=space_veins_sgp4mobility_sopRelativeCoordX; record=vector);
        @statistic[sopRelativeCoordY](source=space_veins_sgp4mobility_sopRelativeCoordY; record=vector);
        @statistic[sopRelativeCoordZ](source=space_veins_sgp4mobility_sopRelativeCoordZ; record=vector);
        @statistic[linearInterpolationSpeedMeterPerSecond](source=space_veins_sgp4mobility_linearInterpolationSpeedMeterPerSecondSignal; record=vector);
        @statistic[sgp4SpeedMeterPerSecond](source=space_veins_sgp4mobility_sgp4SpeedMeterPerSecondSignal; record=vector);
        // SGP4Mobility performance
        @statistic[linearInterpolationComputations](source=space_veins_sgp4mobility_linearInterpolationComputationsSignal; record=count, vector);
        @statistic[sgp4Computations](source=space_veins_sgp4mobility_sgp4ComputationsSignal; record=count, vector);
}
