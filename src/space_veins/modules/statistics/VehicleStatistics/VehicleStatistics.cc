//
// Copyright (C) 2021 Mario Franke <research@m-franke.net>
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "space_veins/modules/statistics/VehicleStatistics/VehicleStatistics.h"

Define_Module(space_veins::VehicleStatistics);

using namespace space_veins;

const simsignal_t VehicleStatistics::omnetCoordX = registerSignal("space_veins_vehicleStatistics_omnetCoordX");
const simsignal_t VehicleStatistics::omnetCoordY = registerSignal("space_veins_vehicleStatistics_omnetCoordY");
const simsignal_t VehicleStatistics::omnetCoordZ = registerSignal("space_veins_vehicleStatistics_omnetCoordZ");

const simsignal_t VehicleStatistics::wgs84CoordLat = registerSignal("space_veins_vehicleStatistics_wgs84CoordLat");
const simsignal_t VehicleStatistics::wgs84CoordLon = registerSignal("space_veins_vehicleStatistics_wgs84CoordLon");
const simsignal_t VehicleStatistics::wgs84CoordAlt = registerSignal("space_veins_vehicleStatistics_wgs84CoordAlt");

const simsignal_t VehicleStatistics::wgs84CartCoordX = registerSignal("space_veins_vehicleStatistics_wgs84CartCoordX");
const simsignal_t VehicleStatistics::wgs84CartCoordY = registerSignal("space_veins_vehicleStatistics_wgs84CartCoordY");
const simsignal_t VehicleStatistics::wgs84CartCoordZ = registerSignal("space_veins_vehicleStatistics_wgs84CartCoordZ");

const simsignal_t VehicleStatistics::sopRelativeCoordX = registerSignal("space_veins_vehicleStatistics_sopRelativeCoordX");
const simsignal_t VehicleStatistics::sopRelativeCoordY = registerSignal("space_veins_vehicleStatistics_sopRelativeCoordY");
const simsignal_t VehicleStatistics::sopRelativeCoordZ = registerSignal("space_veins_vehicleStatistics_sopRelativeCoordZ");

const simsignal_t VehicleStatistics::sendSatellitePackets = registerSignal("space_veins_vehicleStatistics_sendSatellitePackets");
const simsignal_t VehicleStatistics::receivedSatellitePackets = registerSignal("space_veins_vehicleStatistics_receivedSatellitePackets");

const simsignal_t VehicleStatistics::sendWlanPackets = registerSignal("space_veins_vehicleStatistics_sendWlanPackets");
const simsignal_t VehicleStatistics::receivedWlanPackets = registerSignal("space_veins_vehicleStatistics_receivedWlanPackets");

const simsignal_t VehicleStatistics::sendWlanSchedules = registerSignal("space_veins_vehicleStatistics_sendWlanSchedules");
const simsignal_t VehicleStatistics::receivedWlanSchedules = registerSignal("space_veins_vehicleStatistics_receivedWlanSchedules");

const simsignal_t VehicleStatistics::sendSatelliteRegistrationMessages = registerSignal("space_veins_vehicleStatistics_sendSatelliteRegistrationMessages");
const simsignal_t VehicleStatistics::receivedSatelliteRegistrationMessages = registerSignal("space_veins_vehicleStatistics_receivedSatelliteRegistrationMessages");

const simsignal_t VehicleStatistics::missedSatelliteRegistrationAcks = registerSignal("space_veins_vehicleStatistics_missedSatelliteRegistrationAcks");
const simsignal_t VehicleStatistics::receivedSatelliteRegistrationAcks = registerSignal("space_veins_vehicleStatistics_receivedSatelliteRegistrationAcks");

const simsignal_t VehicleStatistics::applLayerRoundTripTime = registerSignal("space_veins_vehicleStatistics_applLayerRoundTripTime");
const simsignal_t VehicleStatistics::applLayerSatelliteLatency = registerSignal("space_veins_vehicleStatistics_applLayerSatelliteLatency");

const simsignal_t VehicleStatistics::registeredVehiclesPerSchedule = registerSignal("space_veins_vehicleStatistics_registeredVehiclesPerSchedule");

const simsignal_t VehicleStatistics::wlanPacketEndToEndDelay = registerSignal("space_veins_vehicleStatistics_wlanPacketEndToEndDelay");

void VehicleStatistics::initialize(int stage)
{
    if (stage == 0) {
        EV_DEBUG << "VehicleStatistics initialized." << std::endl;
    }
}

void VehicleStatistics::finish() {}

void VehicleStatistics::handleMessage(cMessage *msg)
{
    delete msg; // This module should not get any messages
}

VehicleStatistics::~VehicleStatistics() {
}

void VehicleStatistics::recordOmnetCoord(const veins::Coord c)
{
    emit(omnetCoordX, c.x);
    emit(omnetCoordY, c.y);
    emit(omnetCoordZ, c.z);
}

void VehicleStatistics::recordWGS84Coord(const WGS84Coord c)
{
    emit(wgs84CoordLat, c.lat);
    emit(wgs84CoordLon, c.lon);
    emit(wgs84CoordAlt, c.alt);
}

void VehicleStatistics::recordWGS84CartCoord(const PJ_COORD c)
{
    emit(wgs84CartCoordX, c.xyz.x);
    emit(wgs84CartCoordY, c.xyz.y);
    emit(wgs84CartCoordZ, c.xyz.z);
}

void VehicleStatistics::recordSopRelativeCoord(const veins::Coord c)
{
    emit(sopRelativeCoordX, c.x);
    emit(sopRelativeCoordY, c.y);
    emit(sopRelativeCoordZ, c.z);
}

void VehicleStatistics::recordSendSatellitePackets()
{
    emit(sendSatellitePackets, 1);
}

void VehicleStatistics::recordReceivedSatellitePackets()
{
    emit(receivedSatellitePackets, 1);
}

void VehicleStatistics::recordSendWlanPackets()
{
    emit(sendWlanPackets, 1);
}

void VehicleStatistics::recordReceivedWlanPackets()
{
    emit(receivedWlanPackets, 1);
}

void VehicleStatistics::recordSendSatelliteRegistrationMessages()
{
    emit(sendSatelliteRegistrationMessages, 1);
}

void VehicleStatistics::recordReceivedSatelliteRegistrationMessages()
{
    emit(receivedSatelliteRegistrationMessages, 1);
}

void VehicleStatistics::recordMissedSatelliteRegistrationAcks()
{
    emit(missedSatelliteRegistrationAcks, 1);
}

void VehicleStatistics::recordReceivedSatelliteRegistrationAcks()
{
    emit(receivedSatelliteRegistrationAcks, 1);
}

void VehicleStatistics::recordSendWlanSchedules()
{
    emit(sendWlanSchedules, 1);
}

void VehicleStatistics::recordReceivedWlanSchedules()
{
    emit(receivedWlanSchedules, 1);
}

void VehicleStatistics::recordApplLayerRoundTripTime(simtime_t rtt)
{
    emit(applLayerRoundTripTime, rtt);
}

void VehicleStatistics::recordApplLayerSatelliteLatency(simtime_t lat)
{
    emit(applLayerSatelliteLatency, lat);
}

void VehicleStatistics::recordRegisteredVehiclesPerSchedule(const long count)
{
    emit(registeredVehiclesPerSchedule, count);
}

void VehicleStatistics::recordWlanPacketEndToEndDelay(const simtime_t delay)
{
    emit(wlanPacketEndToEndDelay, delay);
}
