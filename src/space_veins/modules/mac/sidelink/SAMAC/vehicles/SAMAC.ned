//
// Copyright (C) 2023 Mario Franke <research@m-franke.net>
//
// Documentation for these modules is at http://sat.car2x.org/
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

package space_veins.modules.mac.sidelink.SAMAC.vehicles;

import inet.queueing.contract.IPacketQueue;
import inet.linklayer.contract.IMacProtocol;
import inet.linklayer.base.MacProtocolBase;

module SAMAC extends MacProtocolBase like IMacProtocol
{
    parameters:
        @class(SAMAC);
        double maxSatelliteProcDelay_s = default(0.01s) @unit(s);    // estimated processing delay of the satellite processing a registration
        string localMacAddress = default("auto");   // MAC address as hex string (12 hex digits), or
                                                    // "auto". "auto" values will be replaced by
                                                    // a generated MAC address in init stage 0.

        int leoSatelliteSamacDestPort = default(9001);
        string leoSatelliteSamacMulticastAddress = default("224.0.0.1");
        double wlanBitrate @unit(bps) = default(12Mbps);
        double wlanBandwidth @unit(Hz) = default(10MHz);
        int mtu @unit(B) = default(4470B);
        string mibModule = default("^.^.wlan[0].mib");

        string pendingQueueType = default("DropTailQueue");
        string inProgressFramesType = default("DropTailQueue");
    submodules:
        pendingQueue: <pendingQueueType> like IPacketQueue {
            parameters:
                @display("p=150,150;q=queue");
        }
        inProgressFrames: <inProgressFramesType> like IPacketQueue {
            parameters:
                @display("p=350,150;q=queue");
        }
}
