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

package space_veins.modules.statistics.VehicleStatistics;

//
// Submodule used to collect vehicle statistics
//
simple VehicleStatistics
{
    parameters:
        @class(space_veins::VehicleStatistics);
        double minElevationAngleV2S = default(-1);    // minimum elevation angle between a vehicle and a satelite such that communication is possible, NaN completly ignores the minElevationAngle.

        @signal[space_veins_vehicleStatistics_sendSatellitePackets](type=long);
        @signal[space_veins_vehicleStatistics_receivedSatellitePackets](type=long);
        @statistic[sendSatellitePackets](source=space_veins_vehicleStatistics_sendSatellitePackets; record=count, vector);
        @statistic[receivedSatellitePackets](source=space_veins_vehicleStatistics_receivedSatellitePackets; record=count, vector);

        @signal[space_veins_vehicleStatistics_sendWlanPackets](type=long);
        @signal[space_veins_vehicleStatistics_receivedWlanPackets](type=long);
        @statistic[sendWlanPackets](source=space_veins_vehicleStatistics_sendWlanPackets; record=count, vector);
        @statistic[receivedWlanPackets](source=space_veins_vehicleStatistics_receivedWlanPackets; record=count, vector);

        @signal[space_veins_vehicleStatistics_sendWlanSchedules](type=long);
        @signal[space_veins_vehicleStatistics_receivedWlanSchedules](type=long);
        @statistic[sendWlanSchedules](source=space_veins_vehicleStatistics_sendWlanSchedules; record=count, vector);
        @statistic[receivedWlanSchedules](source=space_veins_vehicleStatistics_receivedWlanSchedules; record=count, vector);

        @signal[space_veins_vehicleStatistics_sendSatelliteRegistrationMessages](type=long);
        @signal[space_veins_vehicleStatistics_receivedSatelliteRegistrationMessages](type=long);
        @statistic[sendSatelliteRegistrationMessages](source=space_veins_vehicleStatistics_sendSatelliteRegistrationMessages; record=count, vector);
        @statistic[receivedSatelliteRegistrationMessages](source=space_veins_vehicleStatistics_receivedSatelliteRegistrationMessages; record=count, vector);

        @signal[space_veins_vehicleStatistics_missedSatelliteRegistrationAcks](type=long);
        @signal[space_veins_vehicleStatistics_receivedSatelliteRegistrationAcks](type=long);
        @statistic[missedSatelliteRegistrationAcks](source=space_veins_vehicleStatistics_missedSatelliteRegistrationAcks; record=count, vector);
        @statistic[receivedSatelliteRegistrationAcks](source=space_veins_vehicleStatistics_receivedSatelliteRegistrationAcks; record=count, vector);

        @signal[space_veins_vehicleStatistics_applLayerRoundTripTime](type=simtime_t);
        @statistic[vehAppRtt](source=space_veins_vehicleStatistics_applLayerRoundTripTime; record=min, max, avg);
        @signal[space_veins_vehicleStatistics_applLayerSatelliteLatency](type=simtime_t);
        @statistic[vehAppLat](source=space_veins_vehicleStatistics_applLayerSatelliteLatency; record=min, max, avg);

        @signal[space_veins_vehicleStatistics_registeredVehiclesPerSchedule](type=long);
        @statistic[registeredVehiclesPerSchedule](source=space_veins_vehicleStatistics_registeredVehiclesPerSchedule; record=min, max, avg);

        @signal[space_veins_vehicleStatistics_wlanPacketEndToEndDelay](type=simtime_t);
        @statistic[wlanPacketEndToEndDelay](source=space_veins_vehicleStatistics_wlanPacketEndToEndDelay; record=min, max, avg, vector);
}
