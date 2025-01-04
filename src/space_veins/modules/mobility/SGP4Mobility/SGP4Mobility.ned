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
}
