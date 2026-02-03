
//
// Copyright (C) 2013 OpenSim Ltd.
// Copyright (C) 2023 Mario Franke <research@m-franke.net>
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

#include "space_veins/modules/mac/OracleMac/OracleMac.h"
#include "inet/linklayer/acking/AckingMacHeader_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"

Define_Module(space_veins::OracleMac);

using namespace space_veins;

OracleMac::~OracleMac()
{
    cancelAndDelete(sendTimer);
}

void OracleMac::initialize(int stage)
{
    if (stage == inet::INITSTAGE_LOCAL) {
        vecReceivingSatelliteIds.setName("receivingSatelliteIds:vector");
        vecAcksPerPacket.setName("acksPerPacket:vector");
        vecSatelliteContacts.setName("satelliteContacts:vector");

        nodeId = getParentModule()->getParentModule()->getIndex();

        satelliteContactTimeout = par("satelliteContactTimeout");
        ignoreGlobalOracle = par("ignoreGlobalOracle");

        sendTimer = new cMessage("sendTimer", SEND_TIMER);
        WATCH(nodeId);
        WATCH(ignoreGlobalOracle);
        WATCH(acksPerPacket);
        WATCH(uniquePacketsReceivedBySatellites);
        WATCH(totalPacketsReceivedBySatellites);
    }
    if (stage == inet::INITSTAGE_LINK_LAYER) {
        // initialize globalOracleMac
        globalOracleMac = GlobalOracleMacAccess().get();
        ASSERT(globalOracleMac);
    }
    AckingMac::initialize(stage);
}

void OracleMac::startTransmitting()
{
    EV_DEBUG << "OracleMac::startTransmitting() - currentTxFrame: " << currentTxFrame->getFullName() << std::endl;
    // if there's any control info, remove it; then encapsulate the packet
    inet::MacAddress dest = currentTxFrame->getTag<inet::MacAddressReq>()->getDestAddress();
    // if (useAck && !dest.isBroadcast() && !dest.isMulticast() && !dest.isUnspecified()) {    // unicast
    if (useAck) {    // wait for ACKS for multicast and broadcast, too
        currentTxFrameDuplicate = currentTxFrame->dup();
        scheduleAt(simTime() + ackTimeout, ackTimeoutMsg);
    }
    else
        currentTxFrame = nullptr;

    encapsulate(currentTxFrameDuplicate);

    // send
    simtime_t sendAt;
    if (!ignoreGlobalOracle) {
        sendAt = globalOracleMac->askOracle();
    } else {
        sendAt = simTime();
    }
    EV << "Schedule transmission of " << currentTxFrame << " at: " << sendAt << endl;
    scheduleAt(sendAt, sendTimer);
}

void OracleMac::acked(inet::Packet *frame, int receiverId)
{
    Enter_Method_Silent();
    ASSERT(useAck);

    if (currentTxFrame == nullptr)
        throw cRuntimeError("Unexpected ACK received");

    EV_DEBUG << "OracleMac::acked(" << frame->getFullName() << ") is accepted\n";
    // todo: check if the ACK is for the currentTxFrame
    if (simTime() >= getSimulation()->getWarmupPeriod()) {
        totalPacketsReceivedBySatellites++;
        acksPerPacket++;
        receivingSatelliteIds.insert(receiverId);
        if (satelliteContacts.find(receiverId) == satelliteContacts.end()) {
            satelliteContact_t contact;
            contact.satelliteId = receiverId;
            contact.lastContact = simTime();
            contact.numberOfContacts = 1;
            satelliteContacts[receiverId] = contact;
        } else if (simTime() - satelliteContacts[receiverId].lastContact > satelliteContactTimeout) {
            satelliteContacts[receiverId].lastContact = simTime();
            satelliteContacts[receiverId].numberOfContacts++;
        }
    }
}

void OracleMac::handleSelfMessage(cMessage *message)
{
    if (message == ackTimeoutMsg) {
        EV_DEBUG << "OracleMac::handleSelfMessage(" << message->getFullName() << ") is accepted\n";
        if (acksPerPacket > 0) {
            // todo: take care of warm-up period
            // record each satellite Id that received the packet
            if (simTime() >= getSimulation()->getWarmupPeriod()) {
                for (auto id : receivingSatelliteIds)
                {
                    vecReceivingSatelliteIds.record(id);
                }
                receivingSatelliteIds.clear();
                uniquePacketsReceivedBySatellites++;
            }
            EV_WARN << "OracleMac: delete currentTxFrame" << std::endl;
            deleteCurrentTxFrame();
        } else {
            // no ack received
            AckingMac::handleSelfMessage(message);
        }
        vecAcksPerPacket.record(acksPerPacket);
        acksPerPacket = 0;
    } else if (message->getKind() == SEND_TIMER) {
        radio->setRadioMode(fullDuplex ? inet::physicallayer::IRadio::RADIO_MODE_TRANSCEIVER : inet::physicallayer::IRadio::RADIO_MODE_TRANSMITTER);
        sendDown(currentTxFrameDuplicate);
        EV << "Starting transmission of " << currentTxFrameDuplicate << std::endl;
    } else {
        AckingMac::handleSelfMessage(message);
    }
}

void OracleMac::handleLowerPacket(inet::Packet *packet)
{
    auto macHeader = packet->peekAtFront<inet::AckingMacHeader>();
    if (packet->hasBitError()) {
        EV << "Received frame '" << packet->getName() << "' contains bit errors or collision, dropping it\n";
        inet::PacketDropDetails details;
        details.setReason(inet::INCORRECTLY_RECEIVED);
        emit(inet::packetDroppedSignal, packet, &details);
        delete packet;
        return;
    }

    if (!dropFrameNotForUs(packet)) {
        // send Ack if needed
        auto dest = macHeader->getDest();
        // bool needsAck = !(dest.isBroadcast() || dest.isMulticast() || dest.isUnspecified()); // same condition as in sender
        bool needsAck = true;    // ack all packets
        if (needsAck) {
            int senderModuleId = macHeader->getSrcModuleId();
            OracleMac *senderMac = check_and_cast<OracleMac *>(getSimulation()->getModule(senderModuleId));
            if (senderMac->getUseAck())
                senderMac->acked(packet, nodeId);
        }

        // decapsulate and attach control info
        decapsulate(packet);
        EV << "Passing up contained packet '" << packet->getName() << "' to higher layer\n";
        sendUp(packet);
    }
}

void OracleMac::encapsulate(inet::Packet *packet)
{
    auto macHeader = inet::makeShared<inet::AckingMacHeader>();
    macHeader->setChunkLength(inet::B(headerLength));
    auto macAddressReq = packet->getTag<inet::MacAddressReq>();
    macHeader->setSrc(macAddressReq->getSrcAddress());
    macHeader->setDest(macAddressReq->getDestAddress());
    inet::MacAddress dest = macAddressReq->getDestAddress();
    // always set SrcModuleId
    macHeader->setSrcModuleId(getId());
    macHeader->setNetworkProtocol(inet::ProtocolGroup::ethertype.getProtocolNumber(packet->getTag<inet::PacketProtocolTag>()->getProtocol()));
    packet->insertAtFront(macHeader);
    auto macAddressInd = packet->addTagIfAbsent<inet::MacAddressInd>();
    macAddressInd->setSrcAddress(macHeader->getSrc());
    macAddressInd->setDestAddress(macHeader->getDest());
    packet->getTag<inet::PacketProtocolTag>()->setProtocol(&inet::Protocol::ackingMac);
}

void OracleMac::finish()
{
    recordScalar("uniquePacketsReceivedBySatellites:count", uniquePacketsReceivedBySatellites);
    recordScalar("totalPacketsReceivedBySatellites:count", totalPacketsReceivedBySatellites);
    for (auto contact : satelliteContacts) {
        vecSatelliteContacts.record(contact.second.satelliteId);
        vecSatelliteContacts.record(contact.second.numberOfContacts);
    }
}
