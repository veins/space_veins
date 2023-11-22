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

package space_veins.modules.SatelliteObservationPoint;

import inet.mobility.static.StationaryMobility;

//
// Module representing the satellite observation point used for the SGP4Mobility
//
module SatelliteObservationPoint
{
    parameters:
        @class(space_veins::SatelliteObservationPoint);
        @display("i=block/cogwheel");
        int margin = default(25); // margin added by the TraCIScenarioManager. Both value must be identical.
        xml sumoNetXmlFile = default(xmldoc("erlangen.net.xml"));  // path to the simulated SUMO scenario, required to read in the proj string

    submodules:
        mobility: StationaryMobility {
            parameters:
                @display("p=200,200");
        }
}
