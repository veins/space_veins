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

#include <sstream>
#include <iterator>

#include "space_veins/modules/statistics/GlobalStatistics/GlobalStatistics.h"

#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"

#include "inet/common/Simsignals.h"
#include "inet/common/packet/Packet.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/common/packetlevel/Radio.h"
#include "inet/physicallayer/base/packetlevel/TransmissionBase.h"
#include "inet/physicallayer/base/packetlevel/ReceptionBase.h"

Define_Module(space_veins::GlobalStatistics);

using namespace space_veins;

void GlobalStatistics::initialize(int stage)
{
    if (stage == 0) {
        EV_DEBUG << "GlobalStatistics initialized." << std::endl;

        recordReceptions = par("recordReceptions");
        oneLinePerReception = par("oneLinePerReception");
        receptionsMapPath = par("receptionsMapPath").stringValue();

        WATCH(numTotalCars);
        WATCH(numRemovedCars);
        WATCH(numCurrentCars);
        WATCH(lastHash);

        vecCurrentCars.setName("activeTraciModules:vector");
        vecPacketReceptionHash.setName("packetReceptionHash:vector");

        // listen for all packet receptions and calculate a hash
        auto onPacketReception = [this](veins::SignalPayload<cObject*> payload) {
            lastHash = hasher(lastHash + (size_t)(check_and_cast<inet::Packet*>(payload.p)->getId()));
            vecPacketReceptionHash.record(lastHash);
        };
        signalManager.subscribeCallback(getSimulation()->getSystemModule(), inet::packetReceivedSignal, onPacketReception);

        auto onTraciModuleAdded = [this](veins::SignalPayload<cObject*> payload) {
            numTotalCars += 1;
            numCurrentCars += 1;
            vecCurrentCars.record(numCurrentCars);
        };
        signalManager.subscribeCallback(getSimulation()->getSystemModule(), veins::TraCIScenarioManager::traciModuleAddedSignal, onTraciModuleAdded);

        auto onTraciModuleRemoved = [this](veins::SignalPayload<cObject*> payload) {
            numRemovedCars += 1;
            numCurrentCars -= 1;
            vecCurrentCars.record(numCurrentCars);
        };
        signalManager.subscribeCallback(getSimulation()->getSystemModule(), veins::TraCIScenarioManager::traciModuleRemovedSignal, onTraciModuleRemoved);

        auto onTransmissionEndedSignal = [this](veins::SignalPayload<cObject*> payload) {
            simtime_t now = simTime();
            simtime_t warmupPeriod = getSimulation()->getWarmupPeriod();
            if (now >= warmupPeriod && recordReceptions) {
                inet::physicallayer::TransmissionBase* transmission = check_and_cast<inet::physicallayer::TransmissionBase*>(payload.p);
                const inet::Packet* packet = transmission->getPacket();
                // ToDo filter for transmissions from satellite that are higher than minElevationAngle
                const inet::physicallayer::Radio* senderRadioModule = check_and_cast<const inet::physicallayer::Radio*>(transmission->getTransmitter());
                const cModule* senderModule = senderRadioModule->getParentModule()->getParentModule();
                EV_DEBUG << "GlobalStatistics - onTransmissionEndedSignal; senderModule: " << senderModule->getFullPath() << std::endl;

                // get vector index of sender, 0 if sender is not in a vector
                unsigned long senderVectorIndex = 0;
                if (senderModule->isVector()) senderVectorIndex = senderModule->getIndex();

                // get packet counter out of the packet name
                std::string packetName(packet->getName());
                std::size_t dash_pos = packetName.find('-');
                std::string digits = packetName.substr(dash_pos + 1);
                unsigned int packetId = std::stoul(digits);

                std::pair<unsigned short, unsigned int> pn(senderVectorIndex, packetId);
                auto it = receptionsMap.find(pn);
                if (it == receptionsMap.end()) {
                    // we have not seen this packet before
                    // identify number of possible recipients
                    cModule* recipient = getSimulation()->getSystemModule()->getSubmodule("node", 0);
                    unsigned long nodeModuleVectorSize = 1;
                    if (recipient != nullptr && recipient->isVector()) nodeModuleVectorSize = recipient->getVectorSize();

                    EV_DEBUG << "GlobalStatistics - onTransmissionEndedSignal; insert ReceptionCounter - senderId: " << senderVectorIndex
                             << ", sendersPacketId: " << packetId
                             << ", simTime: " << now.dbl()
                             << ", nodeModuleVectorSize: " << nodeModuleVectorSize
                             << std::endl;
                    receptionsMap.insert({std::pair<unsigned short, unsigned int>(senderVectorIndex, packetId),
                                          ReceptionCounter_t(senderVectorIndex, packetId, now, std::max(numCurrentCars, nodeModuleVectorSize))});
                }
            }
        };
        signalManager.subscribeCallback(getSimulation()->getSystemModule(), inet::physicallayer::IRadio::transmissionEndedSignal, onTransmissionEndedSignal);

        auto onReceptionEndedSignal = [this](veins::SignalPayload<cObject*> payload) {
            simtime_t now = simTime();
            simtime_t warmupPeriod = getSimulation()->getWarmupPeriod();
            if (now >= warmupPeriod && recordReceptions) {
                const inet::physicallayer::ReceptionBase* reception = check_and_cast<const inet::physicallayer::ReceptionBase*>(payload.p);
                const inet::physicallayer::TransmissionBase* transmission = check_and_cast<const inet::physicallayer::TransmissionBase*>(reception->getTransmission());
                const inet::Packet* packet = transmission->getPacket();
                if (!packet->hasBitError()) { // only continue if the packet has no bit errors
                    const inet::physicallayer::Radio* senderRadioModule = check_and_cast<const inet::physicallayer::Radio*>(transmission->getTransmitter());
                    const cModule* senderModule = senderRadioModule->getParentModule()->getParentModule();
                    EV_DEBUG << "GlobalStatistics - onReceptionEndedSignal; senderModule: " << senderModule->getFullPath() << std::endl;

                    // get vector index of sender, 0 if sender is not in a vector
                    int senderVectorIndex = 0;
                    if (senderModule->isVector()) senderVectorIndex = senderModule->getIndex();

                    // get packet counter out of the packet name
                    std::string packetName(packet->getName());
                    std::size_t dash_pos = packetName.find('-');
                    std::string digits = packetName.substr(dash_pos + 1);
                    unsigned int packetId = std::stoul(digits);

                    std::pair<unsigned short, unsigned int> pn(senderVectorIndex, packetId);
                    auto it = receptionsMap.find(pn);
                    if (it != receptionsMap.end()) {
                        // we found the transmission of the satellite in our map
                        const inet::physicallayer::Radio* receiverRadioModule = check_and_cast<const inet::physicallayer::Radio*>(reception->getReceiver());
                        cModule* receiverModule = receiverRadioModule->getParentModule()->getParentModule();
                        EV_DEBUG << "GlobalStatistics - onReceptionEndedSignal; receiverModule: " << receiverModule->getFullPath() << std::endl;
                        // get vector index of receiver, 0 if receiver is not in a vector
                        long unsigned int receiverVectorIndex = 0;
                        // if receiver has TraCIMobility use externalId, otherwise vector index
                        veins::TraCIMobility* tm = veins::TraCIMobilityAccess().get(receiverModule);
                        if (tm == nullptr) {
                            // receiver module has no traci mobility, use vector index
                            if (receiverModule->isVector()) receiverVectorIndex = receiverModule->getIndex();
                        } else {
                            // use externalId, assuming that it is a string containing an integer number
                            // externalId should be equal to routeId in this setup
                            std::string externalId = tm->getExternalId();
                            // cast string to long
                            receiverVectorIndex = std::stoul(externalId);
                        }
                        if (it->second.receptionArray.size() <= receiverVectorIndex) {
                            throw cRuntimeError("GlobalStatistics receiverArray is too small.");
                        }
                        if (it->second.receptionArray[receiverVectorIndex] == 1) {
                            throw cRuntimeError("GlobalStatistics receiverArray entry is already equals to 1.");
                        } else {
                            EV_DEBUG << "GlobalStatistics - onReceptionEndedSignal; update ReceptionCounter - senderId: " << senderVectorIndex
                                     << ", sendersPacketId: " << packetId
                                     << ", simTime: " << now.dbl()
                                     << ", receiverVectorIndex: " << receiverVectorIndex
                                     << std::endl;
                            it->second.receptionArray[receiverVectorIndex] = 1;
                        }
                    } else {
                        // we did not find the transmission of the satellite in our map
                        throw cRuntimeError("GlobalStatistics listened to a packet reception with unknown message id.");
                    }
                } else {
                    throw cRuntimeError("GlobalStatistics received a packet with bit errors. This is not supposed to happen");
                }
                // if (std::string(arrivalModule->getFullName()) != "udp" ) {
                // } // ignore signals from the udp module
            }
        };
        signalManager.subscribeCallback(getSimulation()->getSystemModule(), inet::physicallayer::IRadio::receptionEndedSignal, onReceptionEndedSignal);
    }
}

std::string GlobalStatistics::receptionCounterToString(ReceptionCounter_t rc, bool oneLinePerReception) {
    std::string line;
    if (oneLinePerReception) { // write one line per reception
        unsigned long i = 0;
        for (const auto& value : rc.receptionArray) {
            line.append(std::to_string(rc.packetNumber.first));
            line.append(",");
            line.append(std::to_string(rc.packetNumber.second));
            line.append(",");
            line.append(std::to_string(rc.transmissionEndTime.dbl()));
            line.append(",");
            line.append(std::to_string(i));
            line.append(",");
            line.append(std::to_string(value));
            line.append("\n");
            i++;
        }
    } else { // write one line per transmission
        line.append(std::to_string(rc.packetNumber.first));
        line.append(",");
        line.append(std::to_string(rc.packetNumber.second));
        line.append(",");
        line.append(std::to_string(rc.transmissionEndTime.dbl()));
        line.append(",");
        std::ostringstream oss;
        std::copy(rc.receptionArray.begin(), rc.receptionArray.end() - 1,
                  std::ostream_iterator<short>(oss, ","));
        oss << rc.receptionArray.back();  // Add last element without comma
        line.append(oss.str());
        line.append("\n");
    }
    return line;
}

std::string GlobalStatistics::receptionsMapToString() {
    std::string output;
    // output.reserve(1000) // reserves memory for 1000 characters
    if (oneLinePerReception) {
        output.append("senderVectorIndex,senderPacketId,simTime,receiverVectorIndex,packetReceived\n");
    } else { // write one line per transmission
        output.append("senderVectorIndex,senderPacketId,simTime,packetReceptionArray\n");
    }
    for (const auto& [key, value] : receptionsMap) {
        output.append(receptionCounterToString(value, oneLinePerReception));
    }
    return output;
}

void GlobalStatistics::finish() {
    // do not use signals because of warmup period
    recordScalar("totalAddedTraciModules:count", numTotalCars);
    recordScalar("totalRemovedTraciModules:count", numRemovedCars);
    recordScalar("activeTraciModules:last", numCurrentCars);

    recordScalar("packetReceptionHash:last", lastHash);

    if (recordReceptions) {
        // write repectionsMap
        std::string receptionsMapString = receptionsMapToString();
        const char* configName = getEnvir()->getConfigEx()->getActiveConfigName();
        int repetition = getEnvir()->getConfigEx()->getActiveRunNumber();
        std::string fileName;
        fileName.append(receptionsMapPath);
        fileName.append("/ReceptionsMap_");
        fileName.append(std::string(configName));
        fileName.append("_");
        fileName.append(std::to_string(repetition));
        fileName.append(".csv");
        FILE* file = fopen(fileName.c_str(), "w");
        if (file) {
            fwrite(receptionsMapString.data(), 1, receptionsMapString.size(), file);
            fclose(file);
        }
    }
}

void GlobalStatistics::handleMessage(cMessage *msg)
{
    delete msg; // This module should not get any messages
}

GlobalStatistics::~GlobalStatistics() {
}