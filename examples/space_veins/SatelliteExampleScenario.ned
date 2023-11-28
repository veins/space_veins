//
// Copyright (C) 2017 Christoph Sommer <sommer@ccs-labs.org>
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

// Veins
import org.car2x.veins.visualizer.roads.RoadsCanvasVisualizer;
import org.car2x.veins.modules.world.annotations.AnnotationManager;

// Veins_INET
import org.car2x.veins.subprojects.veins_inet.IVeinsInetManager;
import org.car2x.veins.subprojects.veins_inet.VeinsInetTraCIVehicleInserter;

// space_Veins
import space_veins.nodes.SatCar;
import space_veins.modules.SatelliteObservationPoint.SatelliteObservationPoint;
import space_veins.modules.statistics.GlobalStatistics.GlobalStatistics;
import space_veins.modules.mobility.SatelliteInserter.SatelliteInserter;

// INET
import inet.node.inet.INetworkNode;
import inet.physicallayer.contract.packetlevel.IRadioMedium;
import inet.visualizer.integrated.IntegratedMultiVisualizer;
import inet.environment.common.PhysicalEnvironment;

network SatelliteExampleScenario
{
    parameters:
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)
        string satelliteType = default("Satellite");

        @display("bgb=$playgroundSizeX,$playgroundSizeY");
        //@figure[title](type=label; pos=0,-1; anchor=sw; color=darkblue);

    submodules:
        // Veins
        annotations: AnnotationManager {
            @display("p=750,744.98");
        }
        roadsCanvasVisualizer: RoadsCanvasVisualizer {
            @display("p=750,750");
        }

        // Veins INET
        manager: <default("VeinsInetManager")> like IVeinsInetManager {
            @display("p=192,320");
        }
        vehicleInserter : VeinsInetTraCIVehicleInserter {
            @display("p=64,612");
        }

        // INET
        physicalEnvironment: PhysicalEnvironment {
            @display("p=192,224");
        }
        wlanRadioMedium: <default("Ieee80211ScalarRadioMedium")> like IRadioMedium {
            @display("p=64,224");
        }
        satelliteRadioMedium: <default("UnitDiskRadioMedium")> like IRadioMedium {
            @display("p=580,275");
        }
        visualizer: IntegratedMultiVisualizer {
            @display("p=580,125");
        }

        // space_Veins
        globalStatistics: GlobalStatistics {
            @display("p=580, 425");
        }
        satelliteInserter : SatelliteInserter {
            @display("p=128, 612");
        }
        sop: SatelliteObservationPoint {
            @display("p=0,0");
        }
        leoSatellite[0]: <default(satelliteType)> like INetworkNode {
            @display("p=250, 612");
        }
        leoSTARLINK[0]: <default(satelliteType)> like INetworkNode {
            @display("p=300, 612");
        }
        leoIRIDIUM[0]: <default(satelliteType)> like INetworkNode {
            @display("p=350, 612");
        }
        leoONEWEB[0]: <default(satelliteType)> like INetworkNode {
            @display("p=400, 612");
        }
        leoORBCOMM[0]: <default(satelliteType)> like INetworkNode {
            @display("p=450, 612");
        }
        leoGLOBALSTAR[0]: <default(satelliteType)> like INetworkNode {
            @display("p=500, 612");
        }
        leoSPACEBEE[0]: <default(satelliteType)> like INetworkNode {
            @display("p=550, 612");
        }
        leoSPACEBEENZ[0]: <default(satelliteType)> like INetworkNode {
            @display("p=600, 612");
        }
        node[0]: SatCar {
        }

    connections allowunconnected:
}
