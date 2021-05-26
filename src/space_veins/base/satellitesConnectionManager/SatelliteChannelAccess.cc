//
// Copyright (C) 2004 Telecommunication Networks Group (TKN) at Technische Universitaet Berlin, Germany.
// Copyright (C) 2021 Mario Franke <research@m-franke.net>
//
// Documentation for these modules is at http://veins.car2x.org/
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

// author:      Marc Loebbers
// part of:     framework implementation developed by tkn
// description: - Base class for physical layers
//              - if you create your own physical layer, please subclass
//                from this class and use the sendToChannel() function!!

// author:      Mario Franke
// part of:     Space Veins
// description: - Implemenation of modifications for Space Veins

#include "space_veins/base/satellitesConnectionManager/SatelliteChannelAccess.h"

#include "veins/base/utils/FindModule.h"
#include "veins/base/modules/BaseWorldUtility.h"
#include "veins/base/connectionManager/BaseConnectionManager.h"

using namespace space_veins;

using Latitude = SatellitesConnectionManager::Latitude;
using Longitude = SatellitesConnectionManager::Longitude;
using WGS84Coordinate = SatellitesConnectionManager::WGS84Coordinate;

SatellitesConnectionManager* SatelliteChannelAccess::getConnectionManager(cModule* nic)
{
    std::string cmName = nic->hasPar("connectionManagerName") ? nic->par("connectionManagerName").stringValue() : "";
    if (cmName != "") {
        cModule* ccModule = veins::findModuleByPath(cmName.c_str());

        return dynamic_cast<SatellitesConnectionManager*>(ccModule);
    }
    else {
        throw cRuntimeError("Variable connectionManagerName must be specified");
    }
}

void SatelliteChannelAccess::initialize(int stage)
{
    BatteryAccess::initialize(stage);

    if (stage == 0) {
        if (hasPar("antennaOffsetX")) {
            antennaOffset.x = par("antennaOffsetX").doubleValue();
        }

        if (hasPar("antennaOffsetY")) {
            antennaOffset.y = par("antennaOffsetY").doubleValue();
        }

        if (hasPar("antennaOffsetZ")) {
            antennaOffset.z = par("antennaOffsetZ").doubleValue();
        }

        if (hasPar("antennaOffsetYaw")) {
            antennaOffsetYaw = par("antennaOffsetYaw").doubleValue();
        }

        findHost()->subscribe(veins::BaseMobility::mobilityStateChangedSignal, this);

        cModule* nic = getParentModule();
        // initialize cc for BaseConnectionManager, BasePhyLayer needs cc to be initialized.
        // cc = veins::ChannelAccess::getConnectionManager(nic);
        // if (cc == nullptr) throw cRuntimeError("Could not find connectionmanager module");
        // initialized scc for SatellitesConnectionManager
        scc = getConnectionManager(nic);
        if (scc == nullptr) throw cRuntimeError("Could not find satellitesconnectionmanager module");
        isRegistered = false;
        usePropagationDelay = par("usePropagationDelay");

        /* Statisitcs */
        propagation_delay_s_vec.setName("propagation_delay_s");
        phyLayer_dist_m_sender_receiver_vec.setName("phyLayer_dist_m_sender_receiver");

        EV_DEBUG << "SatelliteChannelAccess is initialized." << std::endl;
    }
}

simtime_t SatelliteChannelAccess::calculatePropagationDelay(const veins::NicEntry* nic)
{
    if (!usePropagationDelay) return 0;

    auto satNic = (SatelliteNicEntry*) nic;

    SatelliteChannelAccess* const senderModule = this;
    SatelliteChannelAccess* const receiverModule = satNic->satChAccess;
    // const simtime_t_cref sStart         = simTime();

    ASSERT(senderModule);
    ASSERT(receiverModule);

    /** claim the Move pattern of the sender from the Signal */
    veins::Coord senderPos = senderModule->antennaPosition.getPositionAt();
    veins::Coord receiverPos = receiverModule->antennaPosition.getPositionAt();

    // this time-point is used to calculate the distance between sending and receiving host
    auto distance_m = receiverPos.distance(senderPos);
    auto delay_s = distance_m / veins::BaseWorldUtility::speedOfLight();
    EV_DEBUG << "SatelliteChannelAccess: Propagation delay in seconds: " << delay_s << std::endl;

    /* Statistics */
    propagation_delay_s_vec.record(delay_s);
    phyLayer_dist_m_sender_receiver_vec.record(distance_m);

    return delay_s;
}

void SatelliteChannelAccess::sendToChannel(cPacket* msg)
{
    EV_TRACE << "SatelliteChannelAccess sendToChannel: sending to gates." << std::endl;;

    const auto& gateList = scc->getGateList(getParentModule()->getId());

    for (auto&& entry : gateList) {
        const auto gate = entry.second;
        const auto propagationDelay = calculatePropagationDelay(entry.first);

        if (useSendDirect) {
            for (int gateIndex = gate->getBaseId(); gateIndex < gate->getBaseId() + gate->size(); gateIndex++) {
                sendDirect(msg->dup(), propagationDelay, msg->getDuration(), gate->getOwnerModule(), gateIndex);
            }
        }
        else {
            sendDelayed(msg->dup(), propagationDelay, gate);
        }
        EV_DEBUG << "Packet transmitted at " << simTime() << " with delay in seconds: " << propagationDelay << std::endl;
    }
    // Original message no longer needed, copies have been sent to all possible receivers.
    delete msg;
}

void SatelliteChannelAccess::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details)
{
    if (signalID == veins::BaseMobility::mobilityStateChangedSignal) {
        ChannelMobilityPtrType const mobility = check_and_cast<ChannelMobilityPtrType>(obj);

        auto heading = veins::Heading::fromCoord(mobility->getCurrentOrientation());
        antennaPosition = veins::AntennaPosition(getId(), mobility->getPositionAt(simTime()) + antennaOffset.rotatedYaw(-heading.getRad()), mobility->getCurrentSpeed(), simTime());
        antennaHeading = veins::Heading(heading.getRad() + antennaOffsetYaw);

        if (isRegistered) {
            scc->updateNicPos(getParentModule()->getId(), antennaPosition.getPositionAt(), antennaHeading);
        }
        else {
            // register the nic with SatellitesConnectionManager
            // returns true, if sendDirect is used
            useSendDirect = scc->registerNic(getParentModule(), this, antennaPosition.getPositionAt(), antennaHeading);
            isRegistered = true;
        }
    }
}
