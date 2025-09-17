
//
// Copyright (C) 2025 Mario Franke <research@m-franke.net>
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
simple SGP4MobilityBase extends MovingMobilityBase
{
    parameters:
        @class(space_veins::SGP4MobilityBase);
        @display("i=block/cogwheel");
        bool updatePositionOnlyAboveHorizon = default(false); // update satellite position only when it is above the horizon of the SOP
        double precalculationInterval_s = default(180s) @unit(s);    // calculate satellite positions every <precalculationInterval_s> seconds for testing if the satellite is above the horizon
        double precalculationUpperTimeLimit_s = default (-1s) @unit(s);   // the upper time limit for which precalculation intervals are calculated. -1 means that the whole simulation time is considered. If precalculationUpperTimeLimit > simTimeLimit, normal event scheduling is done for simTime > precalculationUpperTimeLimit.
        // OMNeT++ event management
        @signal[mobilityStateChanged](type=inet::MobilityBase);
        @signal[space_veins_sgp4mobilitybase_processedEventSignal](type=long);
        @signal[space_veins_sgp4mobilitybase_periodicUpdateSignal](type=long);
        @signal[space_veins_sgp4mobilitybase_periodicUpdateCallSignal](type=long);
        @signal[space_veins_sgp4mobilitybase_getCurrentPositionUpdateSignal](type=long);
        @signal[space_veins_sgp4mobilitybase_getCurrentPositionUpdateCallSignal](type=long);
        @signal[space_veins_sgp4mobilitybase_moveAndUpdateCallSignal](type=long);
        @signal[space_veins_sgp4mobilitybase_moveAndUpdateLastUpdateNowCallSignal](type=long);
        @signal[space_veins_sgp4mobilitybase_moveAndUpdateLastUpdateNotNowCallSignal](type=long);
        // SGP4Mobility events
        @signal[space_veins_sgp4mobilitybase_lastElevationAngleToSopSignal](type=double);
        @signal[space_veins_sgp4mobilitybase_satelliteRisesAboveHorizonSignal](type=simtime_t);
        @signal[space_veins_sgp4mobilitybase_satelliteSetsBelowHorizonSignal](type=simtime_t);
        @signal[space_veins_sgp4mobilitybase_periodAboveHorizonSignal](type=simtime_t);
        // SGP4Mobility positions and coordinates
        @signal[space_veins_sgp4mobilitybase_omnetCoordX](type=double);
        @signal[space_veins_sgp4mobilitybase_omnetCoordY](type=double);
        @signal[space_veins_sgp4mobilitybase_omnetCoordZ](type=double);
        @signal[space_veins_sgp4mobilitybase_wgs84CoordLat](type=double);
        @signal[space_veins_sgp4mobilitybase_wgs84CoordLon](type=double);
        @signal[space_veins_sgp4mobilitybase_wgs84CoordAlt](type=double);
        @signal[space_veins_sgp4mobilitybase_wgs84CartCoordX](type=double);
        @signal[space_veins_sgp4mobilitybase_wgs84CartCoordY](type=double);
        @signal[space_veins_sgp4mobilitybase_wgs84CartCoordZ](type=double);
        @signal[space_veins_sgp4mobilitybase_sopRelativeCoordX](type=double);
        @signal[space_veins_sgp4mobilitybase_sopRelativeCoordY](type=double);
        @signal[space_veins_sgp4mobilitybase_sopRelativeCoordZ](type=double);
        @signal[space_veins_sgp4mobilitybase_sgp4SpeedMeterPerSecondSignal](type=double);
        // SGP4Mobility performance
        @signal[space_veins_sgp4mobilitybase_sgp4ComputationsSignal](type=double);

        // OMNeT++ event management
        @statistic[positionUpdates](source=mobilityStateChanged; record=count);
        @statistic[processedEvents](source=space_veins_sgp4mobilitybase_processedEventSignal; record=count, vector);
        @statistic[periodicUpdates](source=space_veins_sgp4mobilitybase_periodicUpdateSignal; record=count, vector);
        @statistic[periodicUpdateCalls](source=space_veins_sgp4mobilitybase_periodicUpdateCallSignal; record=count, vector);
        @statistic[getCurrentPositionUpdates](source=space_veins_sgp4mobilitybase_getCurrentPositionUpdateSignal; record=count, vector);
        @statistic[getCurrentPositionUpdateCalls](source=space_veins_sgp4mobilitybase_getCurrentPositionUpdateCallSignal; record=count, vector);
        @statistic[moveAndUpdateCalls](source=space_veins_sgp4mobilitybase_moveAndUpdateCallSignal; record=count, vector);
        @statistic[moveAndUpdateLastUpdateNowCalls](source=space_veins_sgp4mobilitybase_moveAndUpdateLastUpdateNowCallSignal; record=count, vector);
        @statistic[moveAndUpdateLastUpdateNotNowCalls](source=space_veins_sgp4mobilitybase_moveAndUpdateLastUpdateNotNowCallSignal; record=count, vector);
        // SGP4Mobility events
        @statistic[lastElevationAngleToSop](source=space_veins_sgp4mobilitybase_lastElevationAngleToSopSignal; record=count, vector);
        @statistic[horizonRises](source=space_veins_sgp4mobilitybase_satelliteRisesAboveHorizonSignal; record=count, vector);
        @statistic[horizonSets](source=space_veins_sgp4mobilitybase_satelliteSetsBelowHorizonSignal; record=count, vector);
        @statistic[periodsAboveHorizon](source=space_veins_sgp4mobilitybase_periodAboveHorizonSignal; record=count, vector);
        @statistic[totalTimeAboveHorizon](source=sum(space_veins_sgp4mobilitybase_periodAboveHorizonSignal); record=last);
        // SGP4Mobility positions and coordinates
        @statistic[omnetCoordX](source=space_veins_sgp4mobilitybase_omnetCoordX; record=vector);
        @statistic[omnetCoordY](source=space_veins_sgp4mobilitybase_omnetCoordY; record=vector);
        @statistic[omnetCoordZ](source=space_veins_sgp4mobilitybase_omnetCoordZ; record=vector);
        @statistic[wgs84CoordLat](source=space_veins_sgp4mobilitybase_wgs84CoordLat; record=vector);
        @statistic[wgs84CoordLon](source=space_veins_sgp4mobilitybase_wgs84CoordLon; record=vector);
        @statistic[wgs84CoordAlt](source=space_veins_sgp4mobilitybase_wgs84CoordAlt; record=vector);
        @statistic[wgs84CartCoordX](source=space_veins_sgp4mobilitybase_wgs84CartCoordX; record=vector);
        @statistic[wgs84CartCoordY](source=space_veins_sgp4mobilitybase_wgs84CartCoordY; record=vector);
        @statistic[wgs84CartCoordZ](source=space_veins_sgp4mobilitybase_wgs84CartCoordZ; record=vector);
        @statistic[sopRelativeCoordX](source=space_veins_sgp4mobilitybase_sopRelativeCoordX; record=vector);
        @statistic[sopRelativeCoordY](source=space_veins_sgp4mobilitybase_sopRelativeCoordY; record=vector);
        @statistic[sopRelativeCoordZ](source=space_veins_sgp4mobilitybase_sopRelativeCoordZ; record=vector);
        @statistic[sgp4SpeedMeterPerSecond](source=space_veins_sgp4mobilitybase_sgp4SpeedMeterPerSecondSignal; record=vector);
        // SGP4Mobility performance
        @statistic[sgp4Computations](source=space_veins_sgp4mobilitybase_sgp4ComputationsSignal; record=count, vector);
}
