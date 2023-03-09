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

package space_veins.modules.nic;

import space_veins.modules.mac.sidelink.SAMAC.vehicles.SAMAC;

import inet.linklayer.contract.IWirelessInterface;
import inet.networklayer.common.InterfaceEntry;
import inet.physicallayer.contract.packetlevel.IRadio;

module SamacInterface extends InterfaceEntry
{
    parameters:
        @class(inet::InterfaceEntry);
        string interfaceTableModule;
        string energySourceModule = default("");
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        *.energySourceModule = default(absPath(energySourceModule));
        @display("i=block/ifcard");
    gates:
        output upperLayerOut; // to higher layers
        input upperLayerIn; // from higher layers
        output lowerLayerOut; // to LEO nic
        input lowerLayerIn; // from LEO nic
    submodules:
        samac: SAMAC {
            parameters:
                @display("p=150,200");
        }
    connections:
        samac.upperLayerOut --> upperLayerOut;
        samac.upperLayerIn <-- upperLayerIn;
        samac.lowerLayerOut --> lowerLayerOut;
        samac.lowerLayerIn <-- lowerLayerIn;

        ctsRadio.upperLayerOut --> samac.ctsRadioIn if exists(ctsRadio);
        ctsRadio.upperLayerIn <-- samac.ctsRadioOut if exists(ctsRadio);
}
