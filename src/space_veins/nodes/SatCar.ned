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

import space_veins.base.modules.*;
import org.car2x.veins.modules.nic.Nic80211p;
import space_veins.modules.nic.SatelliteNic;

module SatCar
{
    parameters:
        string applType; //type of the application layer
        string c2xNicType = default("Nic80211p"); // type of network interface card for c2x
        string satelliteNicType = default("SatelliteNic"); // type of network interface card for satellite communication
        string veinsmobilityType = default("org.car2x.veins.modules.mobility.traci.TraCIMobility"); //type of the mobility module
    gates:
        input veinsradioIn; // gate for sendDirect
        input satelliteradioIn;
    submodules:
        appl: <applType> like space_veins.base.modules.IBaseSatelliteCarApplLayer {
            parameters:
                @display("p=60,50");
        }

        c2xNic: <c2xNicType> like org.car2x.veins.modules.nic.INic80211p {
            parameters:
                @display("p=60,166");
        }

        satelliteNic: <satelliteNicType> like space_veins.modules.nic.ISatelliteNic {
            parameters:
                @display("p=60,176");
        }

        veinsmobility: <veinsmobilityType> like org.car2x.veins.base.modules.IMobility {
            parameters:
                @display("p=130,172;i=block/cogwheel");
        }

    connections:
        c2xNic.upperLayerOut --> appl.lowerLayerIn;
        c2xNic.upperLayerIn <-- appl.lowerLayerOut;
        c2xNic.upperControlOut --> appl.lowerControlIn;
        c2xNic.upperControlIn <-- appl.lowerControlOut;

        satelliteNic.upperLayerOut --> appl.satLowerLayerIn;
        satelliteNic.upperLayerIn <-- appl.satLowerLayerOut;
        satelliteNic.upperControlOut --> appl.satLowerControlIn;
        satelliteNic.upperControlIn <-- appl.satLowerControlOut;

        veinsradioIn --> c2xNic.radioIn;
        satelliteradioIn --> satelliteNic.radioIn;

}
