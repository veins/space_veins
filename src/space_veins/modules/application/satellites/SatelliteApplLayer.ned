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

package space_veins.modules.application.satellites;
import space_veins.base.modules.IBaseSatelliteApplLayer;

//
// Demo application layer base class.
//
// @author David Eckhoff
//
// @see DemoBaseApplLayer
// @see Nic80211p
// @see Mac1609_4
// @see PhyLayer80211p
//
simple SatelliteApplLayer like IBaseSatelliteApplLayer
{
    parameters:
        @class(space_veins::SatelliteApplLayer);
        int headerLength = default(88bit) @unit(bit); //header length of the application
        int dataLengthBits = default(1024bit) @unit(bit); //the length of a data packet

        int satPingLengthBits = default(32bit) @unit(bit); // transmitting 4 chars "ping": 4 * 8bit = 32bit
        int satPongLengthBits = default(32bit) @unit(bit); // transmitting 4 chars "pong": 4 * 8bit = 32bit

    gates:
        input lowerLayerIn; // from mac layer
        output lowerLayerOut; // to mac layer
        input lowerControlIn;
        output lowerControlOut;
}
