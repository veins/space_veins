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

package space_veins.modules.application.cars;
import space_veins.base.modules.IBaseSatelliteCarApplLayer;

//
// Demo application layer for cars with satellite nic
//
// @author David Eckhoff
// @author Mario Franke
//
// @see DemoBaseApplLayer
// @see Nic80211p
// @see Mac1609_4
// @see PhyLayer80211p
//
simple TraCIDemo11p_Sat like IBaseSatelliteCarApplLayer
{
    parameters:
        @class(space_veins::TraCIDemo11p_Sat);
        int headerLength = default(88bit) @unit(bit); //header length of the application

        bool sendBeacons = default(true); //tell the applayer to periodically send beacons
        int beaconLengthBits = default(256bit) @unit(bit); //the length of a beacon packet
        int beaconUserPriority = default(7); //the user priority (UP) of the beacon messages
        double beaconInterval = default(1s) @unit(s); //the intervall between 2 beacon messages

        int dataLengthBits = default(1024bit) @unit(bit); //the length of a data packet
        bool dataOnSch = default(false); //tells the applayer whether to use a service channel for datapackets or the control channel
        int dataUserPriority = default(7); //the default user priority (UP) for data packets

        bool avoidBeaconSynchronization = default(true); //don't start beaconing directly after node was created but delay to avoid artifical synchronization

        bool sendWSA = default(false);
        int wsaLengthBits = default(250bit) @unit(bit);
        double wsaInterval =  default(1s) @unit(s);

        // Satellite parameters
        int satPingLengthBits = default(32bit) @unit(bit); // transmitting 4 chars "ping": 4 * 8bit = 32bit
        int satPongLengthBits = default(32bit) @unit(bit); // transmitting 4 chars "pong": 4 * 8bit = 32bit

    gates:
        input lowerLayerIn; // from c2x network layer
        output lowerLayerOut; // to c2x network layer
        input lowerControlIn; // control from c2x network layer
        output lowerControlOut; // control to c2x network layer

        input satLowerLayerIn; // from satellite network layer
        output satLowerLayerOut; // to satellite network layer
        input satLowerControlIn; // control from satellite network layer
        output satLowerControlOut; // control to satellite network layer
}
