//
// Copyright (C) 2018 Christoph Sommer <sommer@ccs-labs.org>
// Copyright (C) 2022 Mario Franke <research@m-franke.net>
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

#include "space_veins/modules/mac/sidelink/SAMAC/satelliteAppl/SamacSatelliteAppl.h"
#include "space_veins/modules/mac/sidelink/SAMAC/messages/SpaceVeinsSatelliteAck_m.h"
#include "space_veins/modules/mac/sidelink/SAMAC/messages/SpaceVeinsRegistrationMessage_m.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/PacketFilter.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"

#define WLAN_SCHEDULE_TIMER 0

using namespace inet;

Define_Module(SamacSatelliteAppl);

SamacSatelliteAppl::SamacSatelliteAppl()
{
}

void SamacSatelliteAppl::initialize(int stage)
{
    EV_TRACE << "initialize -- stage: " << stage << std::endl;
    VeinsInetApplicationBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        wlanScheduleTimer = new cMessage("wlanScheduleTimer", WLAN_SCHEDULE_TIMER);
        wlanSlot_s = par("wlanSlot_s").doubleValue();
        maxV2SLatency_s = par("maxV2SLatency_s").doubleValue();
        wlanScheduleInterval_s = par("wlanScheduleInterval_s");
        WATCH(portNumber);
        EV_TRACE << "initialize -- INITSTAGE_LOCAL done: " << stage << std::endl;
    }
    else if (stage == 1) {
        // Statistics
        vehicleStatistics = VehicleStatisticsAccess().get(getParentModule());
        EV_TRACE << "initialize -- (stage == 1) done: " << stage << std::endl;
    }
}

void SamacSatelliteAppl::handleMessageWhenUp(cMessage* msg)
{
    if (msg->isSelfMessage()) {
        handleSelfMessage(msg);
    }
    else {
        veins::VeinsInetApplicationBase::handleMessageWhenUp(msg);
    }
}

void SamacSatelliteAppl::handleSelfMessage(cMessage* msg)
{
    if (msg->getKind() == WLAN_SCHEDULE_TIMER) {
        publishWlanSchedule();
        scheduleAt(simTime() + wlanScheduleInterval_s, wlanScheduleTimer);
    }
}

void SamacSatelliteAppl::handleStartOperation(LifecycleOperation* doneCallback)
{
    L3AddressResolver().tryResolve("224.0.0.1", destAddress);
    ASSERT(!destAddress.isUnspecified());

    socket.setOutputGate(gate("socketOut"));
    socket.bind(L3Address(), portNumber);

    const char* interface = par("interface");
    ASSERT(interface[0]);
    IInterfaceTable* ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
#if INET_VERSION >= 0x0403
    ie = ift->findInterfaceByName(interface);
#elif INET_VERSION >= 0x0402
    ie = ift->findInterfaceByName(interface);
#else
    ie = ift->getInterfaceByName(interface);
#endif
    ASSERT(ie);
    socket.setMulticastOutputInterface(ie->getInterfaceId());
    socket.setMulticastLoop(false); // Do not receive own multicast messages

    MulticastGroupList mgl = ift->collectMulticastGroups();
    socket.joinLocalMulticastGroups(mgl);

    socket.setCallback(this);

    bool ok = startApplication();
    ASSERT(ok);
}

bool SamacSatelliteAppl::startApplication()
{
    scheduleAt(simTime() + wlanScheduleInterval_s, wlanScheduleTimer);
    EV_DEBUG << "startApplication -- Start publishing the wlanSchedule." << std::endl;
    return true;
}

void SamacSatelliteAppl::sendVehicleRegistrationAck(MacAddress vehicleId)
{
    auto payload = makeShared<SpaceVeinsSatelliteAck>();
    payload->setChunkLength(B(vehicleId.getAddressSize()));
    payload->setAcknowledgedVehicleId(vehicleId);
    timestampPayload(payload);

    auto packet = createPacket("ack");
    packet->insertAtBack(payload);

    // Add network interface tag
    auto interfaceReq = packet.get()->addTagIfAbsent<inet::InterfaceReq>();
    interfaceReq->setInterfaceId(ie->getInterfaceId());

    EV_TRACE << "sendVehicleRegistrationAck -- transmitting SpaceVeinsSatelliteAck for vehicleId: " << vehicleId << std::endl;
    vehicleStatistics->recordSendSatellitePackets();
    sendPacket(std::move(packet));
}

void SamacSatelliteAppl::addToWlanSchedule(MacAddress vehicleId)
{
    size_t wlanScheduleSize = wlanSchedule.size();
    wlanSchedule.insert({vehicleId.getInt(), wlanScheduleSize * wlanSlot_s});
    EV_TRACE << "addToWlanSchedule -- added vehicleId: " << vehicleId << " with simTimeDelta: " << wlanScheduleSize * wlanSlot_s << std::endl;
}

void SamacSatelliteAppl::publishWlanSchedule()
{
    auto payload = makeShared<SpaceVeinsWlanScheduleMessage>();
    if (wlanSchedule.size() > 0 ) {
        // set packet size for the
        payload->setChunkLength(B(
            (wlanSchedule.size() * (MacAddress().getAddressSize() + sizeof(simTimeDelta_t))) +    // number of lines in the schedule times number of bytes used for a single line in the schedule
            3 * sizeof(simtime_t)   // plus 3 times the size of timestamps
        ));
    }else{
        payload->setChunkLength(B(3 * sizeof(simtime_t)));  // 3 times the size of timestamps
    }
    payload->setWlanSchedule(WlanSchedule(wlanSchedule));   // Use copy constructor
    payload->setWlanScheduleInterval(wlanScheduleInterval_s);
    payload->setTransmitTimestamp(simTime());
    payload->setWlanScheduleBeginTimestamp(simTime() + maxV2SLatency_s);
    vehicleStatistics->recordRegisteredVehiclesPerSchedule(wlanSchedule.size());
    wlanSchedule.clear();       // Clear wlanSchedule
    timestampPayload(payload);

    auto packet = createPacket("wlanSchedule");
    packet->insertAtBack(payload);

    // Add network interface tag
    auto interfaceReq = packet.get()->addTagIfAbsent<inet::InterfaceReq>();
    interfaceReq->setInterfaceId(ie->getInterfaceId());

    EV_TRACE << "publishWlanSchedule -- broadcasting WlanSchedule" << std::endl;
    sendPacket(std::move(packet));
    vehicleStatistics->recordSendWlanSchedules();
    vehicleStatistics->recordSendSatellitePackets();
}

void SamacSatelliteAppl::sendPacket(std::unique_ptr<inet::Packet> pk)
{
    emit(packetSentSignal, pk.get());
    socket.sendTo(pk.release(), destAddress, portNumber);
}

bool SamacSatelliteAppl::stopApplication()
{
    return true;
}

SamacSatelliteAppl::~SamacSatelliteAppl()
{
}

void SamacSatelliteAppl::processPacket(std::shared_ptr<inet::Packet> pk)
{
    PacketFilter vehRegFilter;
    vehRegFilter.setPattern("*", "SpaceVeinsRegistrationMessage");

    if (vehRegFilter.matches(pk.get())) {
        auto reg = pk->peekAtFront<SpaceVeinsRegistrationMessage>();
        EV_TRACE << "processPacket -- received SpaceVeinsRegistrationMessage from: " << reg->getSrcNode() << " with vehicleId: " << reg->getSrcMacAddress() << std::endl;
        addToWlanSchedule(reg->getSrcMacAddress());
        sendVehicleRegistrationAck(reg->getSrcMacAddress());

        // statistics
        vehicleStatistics->recordReceivedSatelliteRegistrationMessages();
    }else{
        EV_WARN << "processPacket -- received packet of unknown type: " << pk << std::endl;
    }
    vehicleStatistics->recordReceivedSatellitePackets();
}
