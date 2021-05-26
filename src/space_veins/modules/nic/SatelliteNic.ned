//
// Copyright (C) 2011 David Eckhoff <eckhoff@cs.fau.de>
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

package space_veins.modules.nic;

import space_veins.modules.phy.SatellitePhy;
import space_veins.modules.mac.SatelliteMac;

//
// This NIC implements a satellite network interface card.
//
// @author David Eckhoff
// @author Mario Franke
//
// @see DemoBaseApplLayer
// @see Mac1609_4
// @see PhyLayer80211p
//
module SatelliteNic like ISatelliteNic
{
    parameters:
        string connectionManagerName = default("satellitesConnectionManager");
    gates:
        input upperLayerIn; // to upper layers
        output upperLayerOut; // from upper layers
        output upperControlOut; // control information
        input upperControlIn; // control information
        input radioIn; // radioIn gate for sendDirect

    submodules:
        satellitePhy: SatellitePhy {
            @display("p=69,218;i=block/process_s");
        }

        satelliteMac: SatelliteMac {
            @display("p=69,82");
        }

    connections:
        radioIn --> satellitePhy.radioIn;

        satelliteMac.lowerControlOut --> satellitePhy.upperControlIn;
        satelliteMac.lowerLayerOut --> satellitePhy.upperLayerIn;
        satellitePhy.upperLayerOut --> satelliteMac.lowerLayerIn;
        satellitePhy.upperControlOut --> satelliteMac.lowerControlIn;

        satelliteMac.upperControlIn <-- upperControlIn;
        satelliteMac.upperLayerIn <-- upperLayerIn;

        satelliteMac.upperLayerOut --> upperLayerOut;
        satelliteMac.upperControlOut --> upperControlOut;
}
