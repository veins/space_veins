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

import org.car2x.veins.base.modules.BaseMobility;

// Mobility module for spacecrafts based on SGP4 model.

simple SGP4Mobility extends BaseMobility
{
    parameters:
        @class(space_veins::SGP4Mobility);
        double updateInterval @unit("s") = default(0.1s); // time interval to update the hosts position (seconds)

        double sop_omnet_x_coord = default(0);  // X coordinate of the satellite observer position in the OMNeT++ coordinate system
        double sop_omnet_y_coord = default(0);  // Y coordinate of the satellite observer position in the OMNeT++ coordinate system
        double sop_omnet_z_coord = default(0);  // Z coordinate of the satellite observer position in the OMNeT++ coordinate system

        string wall_clock_sim_start_time_utc = "2021-08-16-09-00-41"; // yyyy-mm-dd-hh-mm-ss
        string satelliteName; // satellite name according to the TLE data
        string tle_line_one;  // TLE data line 1
        string tle_line_two;  // TLE data line 2

        xml sumoNetXmlFile = default(xmldoc("erlangen.net.xml"));  // path to the simulated SUMO scenario, required to read in the proj string
}
