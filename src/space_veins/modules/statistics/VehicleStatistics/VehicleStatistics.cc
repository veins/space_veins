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
#include "space_veins/modules/mobility/SGP4Mobility/SGP4MobilityBase.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"

Define_Module(space_veins::VehicleStatistics);

const simsignal_t space_veins::VehicleStatistics::omnetCoordX = registerSignal("space_veins_vehicleStatistics_omnetCoordX");
const simsignal_t space_veins::VehicleStatistics::omnetCoordY = registerSignal("space_veins_vehicleStatistics_omnetCoordY");
const simsignal_t space_veins::VehicleStatistics::omnetCoordZ = registerSignal("space_veins_vehicleStatistics_omnetCoordZ");

const simsignal_t space_veins::VehicleStatistics::wgs84CoordLat = registerSignal("space_veins_vehicleStatistics_wgs84CoordLat");
const simsignal_t space_veins::VehicleStatistics::wgs84CoordLon = registerSignal("space_veins_vehicleStatistics_wgs84CoordLon");
const simsignal_t space_veins::VehicleStatistics::wgs84CoordAlt = registerSignal("space_veins_vehicleStatistics_wgs84CoordAlt");

const simsignal_t space_veins::VehicleStatistics::wgs84CartCoordX = registerSignal("space_veins_vehicleStatistics_wgs84CartCoordX");
const simsignal_t space_veins::VehicleStatistics::wgs84CartCoordY = registerSignal("space_veins_vehicleStatistics_wgs84CartCoordY");
const simsignal_t space_veins::VehicleStatistics::wgs84CartCoordZ = registerSignal("space_veins_vehicleStatistics_wgs84CartCoordZ");

const simsignal_t space_veins::VehicleStatistics::sopRelativeCoordX = registerSignal("space_veins_vehicleStatistics_sopRelativeCoordX");
const simsignal_t space_veins::VehicleStatistics::sopRelativeCoordY = registerSignal("space_veins_vehicleStatistics_sopRelativeCoordY");
const simsignal_t space_veins::VehicleStatistics::sopRelativeCoordZ = registerSignal("space_veins_vehicleStatistics_sopRelativeCoordZ");

const simsignal_t space_veins::VehicleStatistics::sendSatellitePacketsOverMinElevationAngle = registerSignal("space_veins_vehicleStatistics_sendSatellitePacketsOverMinElevationAngle");

const simsignal_t space_veins::VehicleStatistics::sendSatellitePackets = registerSignal("space_veins_vehicleStatistics_sendSatellitePackets");
const simsignal_t space_veins::VehicleStatistics::receivedSatellitePackets = registerSignal("space_veins_vehicleStatistics_receivedSatellitePackets");

const simsignal_t space_veins::VehicleStatistics::sendWlanPackets = registerSignal("space_veins_vehicleStatistics_sendWlanPackets");
const simsignal_t space_veins::VehicleStatistics::receivedWlanPackets = registerSignal("space_veins_vehicleStatistics_receivedWlanPackets");

const simsignal_t space_veins::VehicleStatistics::sendWlanSchedules = registerSignal("space_veins_vehicleStatistics_sendWlanSchedules");
const simsignal_t space_veins::VehicleStatistics::receivedWlanSchedules = registerSignal("space_veins_vehicleStatistics_receivedWlanSchedules");

const simsignal_t space_veins::VehicleStatistics::sendSatelliteRegistrationMessages = registerSignal("space_veins_vehicleStatistics_sendSatelliteRegistrationMessages");
const simsignal_t space_veins::VehicleStatistics::receivedSatelliteRegistrationMessages = registerSignal("space_veins_vehicleStatistics_receivedSatelliteRegistrationMessages");

const simsignal_t space_veins::VehicleStatistics::missedSatelliteRegistrationAcks = registerSignal("space_veins_vehicleStatistics_missedSatelliteRegistrationAcks");
const simsignal_t space_veins::VehicleStatistics::receivedSatelliteRegistrationAcks = registerSignal("space_veins_vehicleStatistics_receivedSatelliteRegistrationAcks");

const simsignal_t space_veins::VehicleStatistics::applLayerRoundTripTime = registerSignal("space_veins_vehicleStatistics_applLayerRoundTripTime");
const simsignal_t space_veins::VehicleStatistics::applLayerSatelliteLatency = registerSignal("space_veins_vehicleStatistics_applLayerSatelliteLatency");

const simsignal_t space_veins::VehicleStatistics::registeredVehiclesPerSchedule = registerSignal("space_veins_vehicleStatistics_registeredVehiclesPerSchedule");

const simsignal_t space_veins::VehicleStatistics::wlanPacketEndToEndDelay = registerSignal("space_veins_vehicleStatistics_wlanPacketEndToEndDelay");

void space_veins::VehicleStatistics::initialize(int stage)
{
    if (stage == 0) {
        EV_DEBUG << "space_veins::VehicleStatistics initialized." << std::endl;

    }
    if (stage == 5) {
        // after initialization of the SGP4Mobility
        sgp4m = SGP4MobilityBaseAccess().get(getParentModule());
        // TODO: get minElevationAngle from MediumLimitCache
        if (sgp4m->getLastElevationAngleToSop().get() >= inet::deg(8).get()) {
            emit(sendSatellitePacketsOverMinElevationAngle, 1);
        }

        // listen for all packets that have been transmitted
        auto onTransmissionEndedSignal = [this](veins::SignalPayload<cObject*> payload) {
        };
        signalManager.subscribeCallback(getParentModule(), inet::physicallayer::IRadio::transmissionEndedSignal, onTransmissionEndedSignal);
    }
}

void space_veins::VehicleStatistics::finish() {}

void space_veins::VehicleStatistics::handleMessage(cMessage *msg)
{
    delete msg; // This module should not get any messages
}

space_veins::VehicleStatistics::~VehicleStatistics() {
}

void space_veins::VehicleStatistics::recordOmnetCoord(const veins::Coord c)
{
    emit(omnetCoordX, c.x);
    emit(omnetCoordY, c.y);
    emit(omnetCoordZ, c.z);
}

void space_veins::VehicleStatistics::recordWGS84Coord(const WGS84Coord c)
{
    emit(wgs84CoordLat, c.lat);
    emit(wgs84CoordLon, c.lon);
    emit(wgs84CoordAlt, c.alt);
}

void space_veins::VehicleStatistics::recordWGS84CartCoord(const PJ_COORD c)
{
    emit(wgs84CartCoordX, c.xyz.x);
    emit(wgs84CartCoordY, c.xyz.y);
    emit(wgs84CartCoordZ, c.xyz.z);
}

void space_veins::VehicleStatistics::recordSopRelativeCoord(const veins::Coord c)
{
    emit(sopRelativeCoordX, c.x);
    emit(sopRelativeCoordY, c.y);
    emit(sopRelativeCoordZ, c.z);
}

void space_veins::VehicleStatistics::recordSendSatellitePackets()
{
    emit(sendSatellitePackets, 1);
}

void space_veins::VehicleStatistics::recordReceivedSatellitePackets()
{
    emit(receivedSatellitePackets, 1);
}

void space_veins::VehicleStatistics::recordSendWlanPackets()
{
    emit(sendWlanPackets, 1);
}

void space_veins::VehicleStatistics::recordReceivedWlanPackets()
{
    emit(receivedWlanPackets, 1);
}

void space_veins::VehicleStatistics::recordSendSatelliteRegistrationMessages()
{
    emit(sendSatelliteRegistrationMessages, 1);
}

void space_veins::VehicleStatistics::recordReceivedSatelliteRegistrationMessages()
{
    emit(receivedSatelliteRegistrationMessages, 1);
}

void space_veins::VehicleStatistics::recordMissedSatelliteRegistrationAcks()
{
    emit(missedSatelliteRegistrationAcks, 1);
}

void space_veins::VehicleStatistics::recordReceivedSatelliteRegistrationAcks()
{
    emit(receivedSatelliteRegistrationAcks, 1);
}

void space_veins::VehicleStatistics::recordSendWlanSchedules()
{
    emit(sendWlanSchedules, 1);
}

void space_veins::VehicleStatistics::recordReceivedWlanSchedules()
{
    emit(receivedWlanSchedules, 1);
}

void space_veins::VehicleStatistics::recordApplLayerRoundTripTime(simtime_t rtt)
{
    emit(applLayerRoundTripTime, rtt);
}

void space_veins::VehicleStatistics::recordApplLayerSatelliteLatency(simtime_t lat)
{
    emit(applLayerSatelliteLatency, lat);
}

void space_veins::VehicleStatistics::recordRegisteredVehiclesPerSchedule(const long count)
{
    emit(registeredVehiclesPerSchedule, count);
}

void space_veins::VehicleStatistics::recordWlanPacketEndToEndDelay(const simtime_t delay)
{
    emit(wlanPacketEndToEndDelay, delay);
}
