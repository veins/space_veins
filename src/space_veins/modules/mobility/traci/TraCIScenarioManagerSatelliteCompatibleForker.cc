//
// Copyright (C) 2006-2016 Christoph Sommer <christoph.sommer@uibk.ac.at>
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

#include "space_veins/modules/mobility/traci/TraCIScenarioManagerSatelliteCompatibleForker.h"

using namespace space_veins;

Define_Module(space_veins::TraCIScenarioManagerSatelliteCompatibleForker);

TraCIScenarioManagerSatelliteCompatibleForker::~TraCIScenarioManagerSatelliteCompatibleForker() {}

void TraCIScenarioManagerSatelliteCompatibleForker::initialize(int stage)
{
    EV_DEBUG << "TraCIScenarioManagerSatelliteCompatibleForker::initialize" << std::endl;
    veins::TraCIScenarioManagerForker::initialize(stage);
}

void TraCIScenarioManagerSatelliteCompatibleForker::finish()
{
    EV_DEBUG << "TraCIScenarioManagerSatelliteCompatibleForker::finish" << std::endl;
    veins::TraCIScenarioManagerForker::finish();
}

void TraCIScenarioManagerSatelliteCompatibleForker::deleteManagedModule(std::string nodeId)
{
    EV_DEBUG << "TraCIScenarioManagerSatelliteCompatibleForker::deleteManagedModule nodeId: " << nodeId << std::endl;
    cModule* mod = getManagedModule(nodeId);
    if (!mod) throw cRuntimeError("no vehicle with Id \"%s\" found", nodeId.c_str());

    emit(traciModuleRemovedSignal, mod);

    auto cas = veins::getSubmodulesOfType<veins::ChannelAccess>(mod, true);
    for (auto ca : cas) {
        cModule* nic = ca->getParentModule();
        auto connectionManager = veins::ChannelAccess::getConnectionManager(nic);
        connectionManager->unregisterNic(nic);
    }
    if (vehicleObstacleControl) {
        for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++) {
            cModule* submod = *iter;
            veins::TraCIMobility* mm = dynamic_cast<veins::TraCIMobility*>(submod);
            if (!mm) continue;
            auto vo = vehicleObstacles.find(mm);
            ASSERT(vo != vehicleObstacles.end());
            vehicleObstacleControl->erase(vo->second);
        }
    }

    // unregister all satellite nics
    auto scas = veins::getSubmodulesOfType<SatelliteChannelAccess>(mod, true);
    for (auto sca : scas) {
        cModule* nic = sca->getParentModule();
        auto satelliteConnectionManager = SatelliteChannelAccess::getConnectionManager(nic);
        satelliteConnectionManager->unregisterNic(nic);
    }

    hosts.erase(nodeId);
    mod->callFinish();
    mod->deleteModule();
}

void TraCIScenarioManagerSatelliteCompatibleForker::processSimSubscription(std::string objectId, veins::TraCIBuffer& buf)
{
    EV_DEBUG << "TraCIScenarioManagerSatelliteCompatibleForker::processSimSubscription" << std::endl;
    uint8_t variableNumber_resp;
    buf >> variableNumber_resp;
    for (uint8_t j = 0; j < variableNumber_resp; ++j) {
        uint8_t variable1_resp;
        buf >> variable1_resp;
        uint8_t isokay;
        buf >> isokay;
        if (isokay != veins::TraCIConstants::RTYPE_OK) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == veins::TraCIConstants::TYPE_STRING);
            std::string description;
            buf >> description;
            if (isokay == veins::TraCIConstants::RTYPE_NOTIMPLEMENTED) throw cRuntimeError("TraCI server reported subscribing to variable 0x%2x not implemented (\"%s\"). Might need newer version.", variable1_resp, description.c_str());
            throw cRuntimeError("TraCI server reported error subscribing to variable 0x%2x (\"%s\").", variable1_resp, description.c_str());
        }

        if (variable1_resp == veins::TraCIConstants::VAR_DEPARTED_VEHICLES_IDS) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == veins::TraCIConstants::TYPE_STRINGLIST);
            uint32_t count;
            buf >> count;
            EV_DEBUG << "TraCI reports " << count << " departed vehicles." << endl;
            for (uint32_t i = 0; i < count; ++i) {
                std::string idstring;
                buf >> idstring;
                // adding modules is handled on the fly when entering/leaving the ROI
            }

            activeVehicleCount += count;
            drivingVehicleCount += count;
        }
        else if (variable1_resp == veins::TraCIConstants::VAR_ARRIVED_VEHICLES_IDS) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == veins::TraCIConstants::TYPE_STRINGLIST);
            uint32_t count;
            buf >> count;
            EV_DEBUG << "TraCI reports " << count << " arrived vehicles." << endl;
            for (uint32_t i = 0; i < count; ++i) {
                std::string idstring;
                buf >> idstring;

                if (subscribedVehicles.find(idstring) != subscribedVehicles.end()) {
                    subscribedVehicles.erase(idstring);
                    // no unsubscription via TraCI possible/necessary as of SUMO 1.0.0 (the vehicle has arrived)
                }

                // check if this object has been deleted already (e.g. because it was outside the ROI)
                cModule* mod = getManagedModule(idstring);
                if (mod) deleteManagedModule(idstring);

                if (unEquippedHosts.find(idstring) != unEquippedHosts.end()) {
                    unEquippedHosts.erase(idstring);
                }
            }

            if ((count > 0) && (count >= activeVehicleCount) && autoShutdown) autoShutdownTriggered = true;
            activeVehicleCount -= count;
            drivingVehicleCount -= count;
        }
        else if (variable1_resp == veins::TraCIConstants::VAR_TELEPORT_STARTING_VEHICLES_IDS) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == veins::TraCIConstants::TYPE_STRINGLIST);
            uint32_t count;
            buf >> count;
            EV_DEBUG << "TraCI reports " << count << " vehicles starting to teleport." << endl;
            for (uint32_t i = 0; i < count; ++i) {
                std::string idstring;
                buf >> idstring;

                // check if this object has been deleted already (e.g. because it was outside the ROI)
                cModule* mod = getManagedModule(idstring);
                if (mod) deleteManagedModule(idstring);

                if (unEquippedHosts.find(idstring) != unEquippedHosts.end()) {
                    unEquippedHosts.erase(idstring);
                }
            }

            activeVehicleCount -= count;
            drivingVehicleCount -= count;
        }
        else if (variable1_resp == veins::TraCIConstants::VAR_TELEPORT_ENDING_VEHICLES_IDS) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == veins::TraCIConstants::TYPE_STRINGLIST);
            uint32_t count;
            buf >> count;
            EV_DEBUG << "TraCI reports " << count << " vehicles ending teleport." << endl;
            for (uint32_t i = 0; i < count; ++i) {
                std::string idstring;
                buf >> idstring;
                // adding modules is handled on the fly when entering/leaving the ROI
            }

            activeVehicleCount += count;
            drivingVehicleCount += count;
        }
        else if (variable1_resp == veins::TraCIConstants::VAR_PARKING_STARTING_VEHICLES_IDS) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == veins::TraCIConstants::TYPE_STRINGLIST);
            uint32_t count;
            buf >> count;
            EV_DEBUG << "TraCI reports " << count << " vehicles starting to park." << endl;
            for (uint32_t i = 0; i < count; ++i) {
                std::string idstring;
                buf >> idstring;

                cModule* mod = getManagedModule(idstring);
                auto mobilityModules = veins::getSubmodulesOfType<veins::TraCIMobility>(mod);
                for (auto mm : mobilityModules) {
                    mm->changeParkingState(true);
                }
            }

            parkingVehicleCount += count;
            drivingVehicleCount -= count;
        }
        else if (variable1_resp == veins::TraCIConstants::VAR_PARKING_ENDING_VEHICLES_IDS) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == veins::TraCIConstants::TYPE_STRINGLIST);
            uint32_t count;
            buf >> count;
            EV_DEBUG << "TraCI reports " << count << " vehicles ending to park." << endl;
            for (uint32_t i = 0; i < count; ++i) {
                std::string idstring;
                buf >> idstring;

                cModule* mod = getManagedModule(idstring);
                auto mobilityModules = veins::getSubmodulesOfType<veins::TraCIMobility>(mod);
                for (auto mm : mobilityModules) {
                    mm->changeParkingState(false);
                }
            }
            parkingVehicleCount -= count;
            drivingVehicleCount += count;
        }
        else if (variable1_resp == getCommandInterface()->getTimeStepCmd()) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == getCommandInterface()->getTimeType());
            simtime_t serverTimestep;
            buf >> serverTimestep;
            EV_DEBUG << "TraCI reports current time step as " << serverTimestep << " s." << endl;
            simtime_t omnetTimestep = simTime();
            ASSERT(omnetTimestep == serverTimestep);
        }
        else {
            throw cRuntimeError("Received unhandled sim subscription result");
        }
    }
}

void TraCIScenarioManagerSatelliteCompatibleForker::processSubcriptionResult(veins::TraCIBuffer& buf)
{
    uint8_t cmdLength_resp;
    buf >> cmdLength_resp;
    uint32_t cmdLengthExt_resp;
    buf >> cmdLengthExt_resp;
    uint8_t commandId_resp;
    buf >> commandId_resp;
    std::string objectId_resp;
    buf >> objectId_resp;

    if (commandId_resp == veins::TraCIConstants::RESPONSE_SUBSCRIBE_VEHICLE_VARIABLE)
        processVehicleSubscription(objectId_resp, buf);
    else if (commandId_resp == veins::TraCIConstants::RESPONSE_SUBSCRIBE_SIM_VARIABLE)
        processSimSubscription(objectId_resp, buf);
    else if (commandId_resp == veins::TraCIConstants::RESPONSE_SUBSCRIBE_TL_VARIABLE)
        processTrafficLightSubscription(objectId_resp, buf);
    else {
        throw cRuntimeError("Received unhandled subscription result");
    }
}

void TraCIScenarioManagerSatelliteCompatibleForker::executeOneTimestep()
{

    EV_DEBUG << "TraCIScenarioManagerSatelliteCompatibleForker: Triggering TraCI server simulation advance to t=" << simTime() << endl;

    simtime_t targetTime = simTime();

    emit(traciTimestepBeginSignal, targetTime);

    if (isConnected()) {
        veins::TraCIBuffer buf = connection->query(veins::TraCIConstants::CMD_SIMSTEP2, veins::TraCIBuffer() << targetTime);

        uint32_t count;
        buf >> count;
        EV_DEBUG << "Getting " << count << " subscription results" << endl;
        for (uint32_t i = 0; i < count; ++i) {
            processSubcriptionResult(buf);
        }
    }

    emit(traciTimestepEndSignal, targetTime);

    if (!autoShutdownTriggered) scheduleAt(simTime() + updateInterval, executeOneTimestepTrigger);
}


void TraCIScenarioManagerSatelliteCompatibleForker::handleSelfMsg(cMessage* msg)
{
    if (msg == executeOneTimestepTrigger) {
        executeOneTimestep();
        return;
    }else{
        TraCIScenarioManagerForker::handleSelfMsg(msg);
    }
}
