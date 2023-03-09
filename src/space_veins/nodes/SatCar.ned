//
// Copyright (C) 2008 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
// Copyright (C) 2021 Mario Franke <research@m-franke.net>
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

package space_veins.nodes;

import org.car2x.veins.subprojects.veins_inet.VeinsInetCar;

import inet.linklayer.contract.IWirelessInterface;

import space_veins.modules.statistics.VehicleStatistics.VehicleStatistics;
import space_veins.modules.nic.SamacInterface;

module SatCar extends VeinsInetCar
{
    parameters:
        int numSatelliteNics = default(1);
        bool useSamac = default(false);

    gates:
        input satelliteRadioIn[numSatelliteNics];  // gate for satellite communication

    submodules:
        samacNic: SamacInterface if useSamac!=false {
            parameters:
                @display("p=150,150");
        }
        satNic[numSatelliteNics]: <default("SatelliteInterface")> like IWirelessInterface {
            parameters:
                @display("p=825,616,row,150;q=queue");
        }

        vehicleStatistics: VehicleStatistics;

    connections allowunconnected:
        for i=0..sizeof(satelliteRadioIn)-1 {
            satelliteRadioIn[i] --> { @display("m=s"); } --> satNic[i].radioIn;
        }
        for i=1..sizeof(satelliteRadioIn)-1 {
            satNic[i].upperLayerOut --> nl.in++;
            satNic[i].upperLayerIn <-- nl.out++;
        }
        satNic[0].upperLayerOut --> nl.in++ if !useSamac;
        satNic[0].upperLayerIn <-- nl.out++ if !useSamac;

        satNic[0].upperLayerOut --> samacNic.lowerLayerIn if useSamac;
        satNic[0].upperLayerIn <-- samacNic.lowerLayerOut if useSamac;

        samacNic.upperLayerOut --> nl.in++ if useSamac;
        samacNic.upperLayerIn <-- nl.out++ if useSamac;
}
