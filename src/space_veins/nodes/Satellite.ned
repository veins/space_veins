//
// Copyright (C) 2008 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
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

package space_veins.nodes;

import org.car2x.veins.base.modules.*;
import space_veins.modules.nic.SatelliteNic;
import space_veins.base.modules.BaseSatelliteApplLayer;

module Satellite
{
    parameters:
        string applType = default("space_veins.base.modules.BaseSatelliteApplLayer"); //type of the application layer
        string nicType = default("space_veins.modules.nic.SatelliteNic"); // type of network interface card
        //string spaceveinsmobilityType = default("space_veins.mobility.SkyfieldMobility"); //type of the mobility module
        string satelliteName = default("Sat");
    gates:
        input satelliteradioIn; // gate for sendDirect
    submodules:
        appl: <applType> like space_veins.base.modules.IBaseSatelliteApplLayer {
            parameters:
                @display("p=60,50");
        }

        nic: <nicType> like space_veins.modules.nic.ISatelliteNic {
            parameters:
                @display("p=60,166");
        }

        spaceveinsmobility: space_veins.modules.mobility.SkyfieldMobility {
            parameters:
                @display("p=130,172;i=block/cogwheel");
        }
    connections:
        nic.upperLayerOut --> appl.lowerLayerIn;
        nic.upperLayerIn <-- appl.lowerLayerOut;
        nic.upperControlOut --> appl.lowerControlIn;
        nic.upperControlIn <-- appl.lowerControlOut;

        satelliteradioIn --> nic.radioIn;

}
