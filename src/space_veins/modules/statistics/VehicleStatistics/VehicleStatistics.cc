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
#include "space_veins/modules/mobility/SGP4Mobility/SGP4MobilityNoGetCurrentPositionUpdates.h"
#include "space_veins/modules/mobility/SGP4Mobility/SGP4MobilityWithGetCurrentPositionUpdates.h"
#include "space_veins/modules/mobility/SGP4Mobility/SGP4MobilityWithLinearInterpolationUpdates.h"
#include "space_veins/modules/mobility/SGP4Mobility/SGP4MobilityWithNearestPeriodicPositionUpdates.h"

#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/common/Simsignals.h"

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
const simsignal_t space_veins::VehicleStatistics::ageOfInformation = registerSignal("space_veins_vehicleStatistics_ageOfInformation");

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
        moduleType = getParentModule()->getNedTypeName();
    }
    if (stage == 5) {
        if (moduleType == "space_veins.nodes.Satellite") {
            // after initialization of the SGP4Mobility
            // search for SGP4Mobility
            sgp4m = SGP4MobilityNoGetCurrentPositionUpdatesAccess().get(getParentModule());
            if (sgp4m == nullptr) {
                sgp4m = SGP4MobilityWithGetCurrentPositionUpdatesAccess().get(getParentModule());
            } else if (sgp4m == nullptr) {
                sgp4m = SGP4MobilityWithLinearInterpolationUpdatesAccess().get(getParentModule());
            } else if (sgp4m == nullptr) {
                sgp4m = SGP4MobilityWithNearestPeriodicPositionUpdatesAccess().get(getParentModule());
            }
            if (sgp4m == nullptr) {
                throw cRuntimeError("VehicleStatistics: Satellite has no mobility");
            }
            ASSERT(sgp4m);
            // SGP4Mobility found, if not: SatCars do not have a SGP4Mobility
            // listen for all packets that have been transmitted
            auto onTransmissionEndedSignal = [this](veins::SignalPayload<cObject*> payload) {
                // TODO: get minElevationAngle from MediumLimitCache
                if (sgp4m->getLastElevationAngleToSop().get() >= inet::deg(8).get()) {
                    emit(sendSatellitePacketsOverMinElevationAngle, 1);
                }
            };
            signalManager.subscribeCallback(getParentModule(), inet::physicallayer::IRadio::transmissionEndedSignal, onTransmissionEndedSignal);
        } else if (moduleType == "space_veins.nodes.SatCar") {
            auto onPacketReceivedSignal = [this](veins::SignalPayload<cObject*> payload) {
                simtime_t now = simTime();
                simtime_t warmupPeriod = getSimulation()->getWarmupPeriod();

                cMessage* payloadMsg = check_and_cast<omnetpp::cMessage*>(payload.p);
                long newId = payloadMsg->getId();
                std::set<long>::iterator it = uniqueIds.find(newId);
                if (it == uniqueIds.end()) {
                    // newId is not in the set, we have not seen this packet before
                    uniqueIds.insert(newId);
                    if (now >= warmupPeriod) {
                        if (beginAverageTimeDuration > omnetpp::SimTime::ZERO) {
                            // if beginAverageTimeDuration > SimTime::ZERO means that we have
                            // received a packet before outside of the warumup period. Such we
                            // can calculate the time difference between the new and the old
                            // packet.
                            simtime_t timeSinceLastPacketReception = now - lastPacketReception;
                            // In order to calculate the area under the curve we can simply
                            // multiply timeSinceLastPacketReception * timeSinceLastPacketReception
                            // with 0.5 as we want to calculate the area of a triangle
                            // with sides equal to timeSinceLastPacketReception
                            double aoiAreaUnderTheCurve = 0.5 * timeSinceLastPacketReception.dbl() * timeSinceLastPacketReception.dbl();
                            // aoiAreaUnderCurveSum stores the total sum of the areas of all
                            // triangles.
                            aoiAreaUnderTheCurveSum += aoiAreaUnderTheCurve;
                            // averageTimeDuration stores the time duration since we started to
                            // measure the age of information
                            simtime_t averageTimeDuration = now - beginAverageTimeDuration;
                            double averageAoI = aoiAreaUnderTheCurveSum / averageTimeDuration.dbl();
                            emit(ageOfInformation, averageAoI);
                        }
                        lastPacketReception = now;
                        if (beginAverageTimeDuration == omnetpp::SimTime::ZERO) {
                            beginAverageTimeDuration = now;
                        }
                    }
                }   // else { do nothing because we already have seen this packet }
            };
            signalManager.subscribeCallback(getParentModule(), inet::packetReceivedSignal, onPacketReceivedSignal);
        }
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
