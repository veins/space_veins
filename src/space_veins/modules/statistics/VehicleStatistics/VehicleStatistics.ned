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

        @signal[space_veins_vehicleStatistics_omnetCoordX](type=double);
        @signal[space_veins_vehicleStatistics_omnetCoordY](type=double);
        @signal[space_veins_vehicleStatistics_omnetCoordZ](type=double);
        @statistic[omnetCoordX](source=space_veins_vehicleStatistics_omnetCoordX; record=vector?);
        @statistic[omnetCoordY](source=space_veins_vehicleStatistics_omnetCoordY; record=vector?);
        @statistic[omnetCoordZ](source=space_veins_vehicleStatistics_omnetCoordZ; record=vector?);

        @signal[space_veins_vehicleStatistics_wgs84CoordLat](type=double);
        @signal[space_veins_vehicleStatistics_wgs84CoordLon](type=double);
        @signal[space_veins_vehicleStatistics_wgs84CoordAlt](type=double);
        @statistic[wgs84CoordLat](source=space_veins_vehicleStatistics_wgs84CoordLat; record=vector?);
        @statistic[wgs84CoordLon](source=space_veins_vehicleStatistics_wgs84CoordLon; record=vector?);
        @statistic[wgs84CoordAlt](source=space_veins_vehicleStatistics_wgs84CoordAlt; record=vector?);

        @signal[space_veins_vehicleStatistics_wgs84CartCoordX](type=double);
        @signal[space_veins_vehicleStatistics_wgs84CartCoordY](type=double);
        @signal[space_veins_vehicleStatistics_wgs84CartCoordZ](type=double);
        @statistic[wgs84CartCoordX](source=space_veins_vehicleStatistics_wgs84CartCoordX; record=vector?);
        @statistic[wgs84CartCoordY](source=space_veins_vehicleStatistics_wgs84CartCoordY; record=vector?);
        @statistic[wgs84CartCoordZ](source=space_veins_vehicleStatistics_wgs84CartCoordZ; record=vector?);

        @signal[space_veins_vehicleStatistics_sopRelativeCoordX](type=double);
        @signal[space_veins_vehicleStatistics_sopRelativeCoordY](type=double);
        @signal[space_veins_vehicleStatistics_sopRelativeCoordZ](type=double);
        @statistic[sopRelativeCoordX](source=space_veins_vehicleStatistics_sopRelativeCoordX; record=vector?);
        @statistic[sopRelativeCoordY](source=space_veins_vehicleStatistics_sopRelativeCoordY; record=vector?);
        @statistic[sopRelativeCoordZ](source=space_veins_vehicleStatistics_sopRelativeCoordZ; record=vector?);

        @signal[space_veins_vehicleStatistics_sendSatellitePackets](type=long);
        @signal[space_veins_vehicleStatistics_receivedSatellitePackets](type=long);
        @statistic[sendSatellitePackets](source=space_veins_vehicleStatistics_sendSatellitePackets; record=count, vector?);
        @statistic[receivedSatellitePackets](source=space_veins_vehicleStatistics_receivedSatellitePackets; record=count, vector?);

        @signal[space_veins_vehicleStatistics_sendWlanPackets](type=long);
        @signal[space_veins_vehicleStatistics_receivedWlanPackets](type=long);
        @statistic[sendWlanPackets](source=space_veins_vehicleStatistics_sendWlanPackets; record=count, vector?);
        @statistic[receivedWlanPackets](source=space_veins_vehicleStatistics_receivedWlanPackets; record=count, vector?);

        @signal[space_veins_vehicleStatistics_sendWlanSchedules](type=long);
        @signal[space_veins_vehicleStatistics_receivedWlanSchedules](type=long);
        @statistic[sendWlanSchedules](source=space_veins_vehicleStatistics_sendWlanSchedules; record=count, vector?);
        @statistic[receivedWlanSchedules](source=space_veins_vehicleStatistics_receivedWlanSchedules; record=count, vector?);

        @signal[space_veins_vehicleStatistics_sendSatelliteRegistrationMessages](type=long);
        @signal[space_veins_vehicleStatistics_receivedSatelliteRegistrationMessages](type=long);
        @statistic[sendSatelliteRegistrationMessages](source=space_veins_vehicleStatistics_sendSatelliteRegistrationMessages; record=count, vector?);
        @statistic[receivedSatelliteRegistrationMessages](source=space_veins_vehicleStatistics_receivedSatelliteRegistrationMessages; record=count, vector?);

        @signal[space_veins_vehicleStatistics_missedSatelliteRegistrationAcks](type=long);
        @signal[space_veins_vehicleStatistics_receivedSatelliteRegistrationAcks](type=long);
        @statistic[missedSatelliteRegistrationAcks](source=space_veins_vehicleStatistics_missedSatelliteRegistrationAcks; record=count, vector?);
        @statistic[receivedSatelliteRegistrationAcks](source=space_veins_vehicleStatistics_receivedSatelliteRegistrationAcks; record=count, vector?);

        @signal[space_veins_vehicleStatistics_applLayerRoundTripTime](type=simtime_t);
        @statistic[vehAppRtt](source=space_veins_vehicleStatistics_applLayerRoundTripTime; record=min, max, avg);
        @signal[space_veins_vehicleStatistics_applLayerSatelliteLatency](type=simtime_t);
        @statistic[vehAppLat](source=space_veins_vehicleStatistics_applLayerSatelliteLatency; record=min, max, avg);

        @signal[space_veins_vehicleStatistics_registeredVehiclesPerSchedule](type=long);
        @statistic[registeredVehiclesPerSchedule](source=space_veins_vehicleStatistics_registeredVehiclesPerSchedule; record=min, max, avg);

        @signal[space_veins_vehicleStatistics_wlanPacketEndToEndDelay](type=simtime_t);
        @statistic[wlanPacketEndToEndDelay](source=space_veins_vehicleStatistics_wlanPacketEndToEndDelay; record=min, max, avg, vector);
}
