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
        @signal[mobilityStateChanged](type=inet::MobilityBase);
        bool updatePositionOnlyAboveHorizon = default(false); // update satellite position only when it is above the horizon of the SOP
        double precalculationInterval_s = default(180s) @unit(s);    // calculate satellite positions every <precalculationInterval_s> seconds for testing if the satellite is above the horizon
        int getCurrentPositionBehavior = default(0); // Changes the behavior of getCurrentPosition(): 0: getCurrentPosition() returns the last calculated position. It does not recalculate it according to the current simulation time. It does not consider the simulation time. If the last update happened at t_0=5.0s and the updateInterval is 1.0s and getCurrentPosition() is called at t_1=5.5s, the position at t_0 is returned.
        //                                                                                            1: if lastPosition is outdated (lastUpdate != simTime()) the satellite's position is calculated according to simTime() using the SGP4 model. Otherwise lastPosition is returned.
        //                                                                                            2: if lastPosition is outdated (lastUpdate != simTime()) the satellite's position is calculated according to simTime() using linear interpolation. Otherwise lastPosition is returned.
        //                                                                                            3: if lastPosition is outdated (lastUpdate != simTime()) the returned satellite position is the nearest position calculated by a periodic update: if simTime() - lastPeriodicUpdate <= updateInterval/2 return satellite position at t = lastUpdate; if simTime() - lastPeriodicUpdate > updateInterval/2 return satellite position at t = lastPeriodicUpdate + updateInterval
        @statistic[calculatedPositionUpdates](source=mobilityStateChanged; record=count);
        @signal[space_veins_sgp4mobility_processedEventSignal](type=long);
        @signal[space_veins_sgp4mobility_periodicUpdateSignal](type=long);
        @signal[space_veins_sgp4mobility_getCurrentPositionUpdateSignal](type=long);
        @statistic[processedEvents](source=space_veins_sgp4mobility_processedEventSignal; record=count, vector);
        @statistic[periodicUpdates](source=space_veins_sgp4mobility_periodicUpdateSignal; record=count, vector);
        @statistic[getCurrentPositionUpdates](source=space_veins_sgp4mobility_getCurrentPositionUpdateSignal; record=count, vector);
}
