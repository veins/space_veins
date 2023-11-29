//
// Copyright (C) 2023 Mario Franke <research@m-franke.net>
//
// Documentation for these modules is at http://sat.car2x.org/
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "space_veins/modules/mac/sidelink/SAMAC/vehicles/SAMAC.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OfdmMode.h"
#include "space_veins/modules/mac/sidelink/SAMAC/messages/SpaceVeinsRegistrationMessage_m.h"
#include "space_veins/modules/mac/sidelink/SAMAC/messages/SpaceVeinsSatelliteAck_m.h"
#include "space_veins/modules/mac/sidelink/SAMAC/messages/SpaceVeinsWlanScheduleMessage_m.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/MulticastTag_m.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/transportlayer/common/L4Tools.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"
#include "inet/transportlayer/udp/Udp.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/ieee80211/llc/Ieee80211EtherTypeHeader_m.h"
#include "inet/linklayer/ieee80211/llc/LlcProtocolTag_m.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211Tag_m.h"

#define SATELLITE_REGISTRATION_ACK_TIMER 0

using namespace inet;

Define_Module(SAMAC);

SAMAC::SAMAC()
{
}

void SAMAC::initialize(int stage)
{
    MacProtocolBase::initialize(stage);
    EV_TRACE << "initialize -- stage: " << stage << std::endl;
    if (stage == INITSTAGE_LOCAL) {
        satelliteRegistrationAckTimer = new cMessage("satelliteRegistrationAckTimer", SATELLITE_REGISTRATION_ACK_TIMER);

        leoInGateId = findGate("leoIn");
        leoOutGateId = findGate("leoOut");
        ieee80211pRadioOutGateId = findGate("radioOut");

        pendingQueue = check_and_cast<inet::queueing::IPacketQueue *>(getSubmodule("pendingQueue"));
        inProgressFrames = check_and_cast<inet::queueing::IPacketQueue *>(getSubmodule("inProgressFrames"));
        mib = check_and_cast<inet::ieee80211::Ieee80211Mib*>(getModuleFromPar<inet::ieee80211::Ieee80211Mib>(par("mibModule"), this));

        wlanBitrate = bps(par("wlanBitrate"));
        wlanBandwidth = Hz(par("wlanBandwidth"));

        maxSatelliteProcDelay_s = par("maxSatelliteProcDelay_s");
        WATCH(maxSatelliteProcDelay_s);
        WATCH(registeredAtSatellite);
        WATCH(receivedInitialWlanSchedule);
        WATCH(qosStation);
        WATCH(leoSatelliteSamacDestPort);
        WATCH(address);
        WATCH(leoSatelliteMacAddress);
        EV_TRACE << "initialize -- INITSTAGE_LOCAL done." << std::endl;
    }
    else if (stage == 1) {
        ieee80211Radio = check_and_cast<inet::physicallayer::Ieee80211Radio*>(getParentModule()->getParentModule()->getSubmodule("wlan", 0)->getSubmodule("radio", 0));
        ieee80211Mac = check_and_cast<inet::ieee80211::Ieee80211Mac*>(getParentModule()->getParentModule()->getSubmodule("wlan", 0)->getSubmodule("mac", 0));
        qosStation = ieee80211Mac->par("qosStation");
        if (qosStation) {
            omds = check_and_cast<inet::ieee80211::IOriginatorMacDataService*>(ieee80211Mac->getSubmodule("hcf", 0)->getSubmodule("originatorMacDataService", 0));
        }
        else {
            omds = check_and_cast<inet::ieee80211::IOriginatorMacDataService*>(ieee80211Mac->getSubmodule("dcf", 0)->getSubmodule("originatorMacDataService", 0));
        }

        vehicleStatistics = VehicleStatisticsAccess().get(getParentModule()->getParentModule());
        EV_TRACE << "initialize -- INITSTAGE 1 done." << std::endl;
    }
}
void SAMAC::finish()
{
    cancelAndDelete(satelliteRegistrationAckTimer);
}

void SAMAC::configureInterfaceEntry()
{
    address = parseMacAddressParameter(par("localMacAddress"));
    EV_DEBUG << "configureInterfaceEntry -- assigned SAMAC address: " << address.str() << std::endl;

    interfaceEntry->setDatarate(wlanBitrate.get());

    // generate a link-layer address to be used as interface token for IPv6
    interfaceEntry->setMacAddress(address);
    interfaceEntry->setInterfaceToken(address.formInterfaceIdentifier());

    // MTU: typical values are 576 (Internet de facto), 1500 (Ethernet-friendly),
    // 4000 (on some point-to-point links), 4470 (Cisco routers default, FDDI compatible)
    interfaceEntry->setMtu(par("mtu"));

    interfaceEntry->setMulticast(true);
    interfaceEntry->setBroadcast(true);
}

void SAMAC::handleMessageWhenUp(cMessage* message)
{
    if (timerManager.handleMessage(message)) return;

    if (message->isSelfMessage()) {
        handleSelfMessage(message);
    }else if (message->isPacket()) {
        if (message->arrivedOn("lowerLayerIn")) {
            EV_TRACE << "handleMessageWhenUp -- received packet from LEO satellite." << std::endl;
            handleLeoPacket(check_and_cast<Packet*>(message));
        }else if (message->arrivedOn("upperLayerIn")) {
            // emit signal for data link visualization
            emit(packetReceivedFromUpperSignal, message);
            handleUpperPacket(check_and_cast<Packet*>(message));
            EV_TRACE << "handleMessageWhenUp -- received packet from upper layer (expected to be a packet for the sidelink channel)." << std::endl;
        }
    }
    else {
        EV_WARN << "handleMessageWhenUp -- received an unknown message: " << message << std::endl;
        throw cRuntimeError("handleMessageWhenUp -- received an unknown message.");
    }
}

void SAMAC::handleSelfMessage(cMessage *msg)
{
    if (msg->getKind() == SATELLITE_REGISTRATION_ACK_TIMER) {
        EV_TRACE << "handelSelfMessage -- satellite registration ack timer expired." << std::endl;
        ASSERT(!satelliteRegistrationAckTimer->isScheduled());
        if (simTime() < lastUsefulRegistrationTime) {
            EV_TRACE << "handelSelfMessage -- retry to register at satellite." << std::endl;
            auto t_spec = veins::TimerSpecification([this]() {registerAtSatellite();});
            t_spec.oneshotAt(uniform(simTime(), lastUsefulRegistrationTime));
        }
        else {
            EV_TRACE << "handelSelfMessage -- no time left for registering at satellite." << std::endl;
        }
    }
    else {
        EV_TRACE << "handelSelfMessage -- received unknown self message:" << msg << std::endl;
        throw cRuntimeError("handleSelfMessage -- received unknown self message.");
    }
}

void SAMAC::handleUpperPacket(Packet *packet)
{
    // Did we hear a LEO satellite?
    if (receivedInitialWlanSchedule) {
      // put appl. layer packet in pending packet queue and wait for schedule
      pendingQueue->pushPacket(packet);
    }
    else {
        EV_WARN << "handleUpperPacket -- no LEO satellite available, deleting packet" << std::endl;
        delete packet;
        // no LEO satellite available forward packet to Ieee80211Interface for CSMA channel access
        //sendDown(packet);
    }
}

void SAMAC::handleLeoPacket(Packet *packet)
{
    processLeoPacket(std::shared_ptr<inet::Packet>(packet));
}

void SAMAC::handleStartOperation(LifecycleOperation* operation)
{
    MacProtocolBase::handleStartOperation(operation);

    // Get satelliteInterfaceEntry
    IInterfaceTable* ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
    std::string satelliteInterface = "satNic0";
    ASSERT(satelliteInterface[0]);
#if INET_VERSION >= 0x0403
    satelliteInterfaceEntry = ift->findInterfaceByName(satelliteInterface.c_str());
#elif INET_VERSION >= 0x0402
    satelliteInterfaceEntry = ift->findInterfaceByName(satelliteInterface.c_str());
#else
    satelliteInterfaceEntry = ift->getInterfaceByName(satelliteInterface.c_str());
#endif
    ASSERT(satelliteInterfaceEntry);
}

void SAMAC::handleStopOperation(LifecycleOperation* operation)
{
    MacProtocolBase::handleStopOperation(operation);
}

void SAMAC::handleCrashOperation(LifecycleOperation* operation)
{
    MacProtocolBase::handleCrashOperation(operation);
}

/*
 * This method is based on code snippets from INET 4.4.1. The code snippets are modified
 * such that they work with space_Veins. The original implementation can be found in the
 * following files:
 * <INET_root_folder/src/inet/linklayer/ieee80211/mac/Ieee80211Mac.cc
 * <INET_root_folder/src/inet/linklayer/ieee80211/llc/Ieee80211LlcEpd.cc
 */
void SAMAC::encapsulate(inet::Packet* packet)
{
    const Protocol* protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
    int ethType = ProtocolGroup::ethertype.findProtocolNumber(protocol);
    if (ethType == -1)
        throw cRuntimeError("EtherType not found for protocol %s", protocol ? protocol->getName() : "(nullptr)");
    const auto& llcHeader = makeShared<Ieee80211EtherTypeHeader>();
    llcHeader->setEtherType(ethType);
    packet->insertAtFront(llcHeader);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211EtherType);
    packet->addTagIfAbsent<inet::ieee80211::LlcProtocolTag>()->setProtocol(&Protocol::ieee80211EtherType);

    auto macAddressReq = packet->getTag<MacAddressReq>();
    auto destAddress = macAddressReq->getDestAddress();
    const auto& header = makeShared<inet::ieee80211::Ieee80211DataHeader>();
    header->setTransmitterAddress(mib->address);
    if (mib->mode == inet::ieee80211::Ieee80211Mib::INDEPENDENT)
        header->setReceiverAddress(destAddress);
    else if (mib->mode == inet::ieee80211::Ieee80211Mib::INFRASTRUCTURE) {
        if (mib->bssStationData.stationType == inet::ieee80211::Ieee80211Mib::ACCESS_POINT) {
            header->setFromDS(true);
            header->setAddress3(mib->address);
            header->setReceiverAddress(destAddress);
        }
        else if (mib->bssStationData.stationType == inet::ieee80211::Ieee80211Mib::STATION) {
            header->setToDS(true);
            header->setReceiverAddress(mib->bssData.bssid);
            header->setAddress3(destAddress);
        }
        else
            throw cRuntimeError("Unknown station type");
    }
    else
        throw cRuntimeError("Unknown mode");
    if (auto userPriorityReq = packet->findTag<UserPriorityReq>()) {
        // make it a QoS frame, and set TID
        header->setType(inet::ieee80211::ST_DATA_WITH_QOS);
        header->addChunkLength(inet::ieee80211::QOSCONTROL_PART_LENGTH);
        header->setTid(userPriorityReq->getUserPriority());
    }
    // duration field should be 0 because we are transmitting multicasts,
    // see <INET_root_folder/src/inet/linklayer/ieee80211/mac/protectionmechanism/OriginatorProtectionMechanism.cc:44ff
    header->setDurationField(0);
    packet->insertAtFront(header);
    // sequence number
    inProgressFrames->pushPacket(packet);
    auto frames = omds->extractFramesToTransmit(inProgressFrames);
    ASSERT(frames->size() == 1);
    packet = frames->front();
    take(packet);
    const auto& macTrailer = makeShared<inet::ieee80211::Ieee80211MacTrailer>();
    macTrailer->setFcsMode(FCS_DECLARED_CORRECT);
    packet->insertAtBack(macTrailer);
    auto packetProtocolTag = packet->addTagIfAbsent<PacketProtocolTag>();
    packetProtocolTag->setProtocol(&Protocol::ieee80211Mac);
    // set OfdmMode
    auto ieee80211ModeReq = packet->addTagIfAbsent<inet::physicallayer::Ieee80211ModeReq>();
    // TODO: Try not use the magic number -> figure out how to get the signalRateField OR implement a dictonary mbps -> signalRateField
    // signalRateField == 9 -> corresponds to ofdmMode12MbpsCS10Mhz
    ieee80211ModeReq->setMode(&inet::physicallayer::Ieee80211OfdmCompliantModes::getCompliantMode(9, wlanBandwidth));
}

std::unique_ptr<inet::Packet> SAMAC::createPacket(std::string name)
{
    return std::unique_ptr<inet::Packet>(new inet::Packet(name.c_str()));
}

void SAMAC::timestampPayload(inet::Ptr<inet::Chunk> payload)
{
    payload->removeTagIfPresent<CreationTimeTag>(b(0), b(-1));
    auto creationTimeTag = payload->addTag<CreationTimeTag>();
    creationTimeTag->setCreationTime(simTime());
}

void SAMAC::registerAtSatellite()
{
    expectedAck_Timestamp = simTime() + 2 * currentSatelliteLatency + maxSatelliteProcDelay_s;

    auto payload = makeShared<SpaceVeinsRegistrationMessage>();
    payload->setChunkLength(B(sizeof(int)));
    EV_TRACE << "registerAtSatellite -- address used: " << address.str() << std::endl;
    payload->setSrcMacAddress(address);
    timestampPayload(payload);

    auto packet = createPacket("registerAtSatellite");
    packet->insertAtBack(payload);

    // Add network interface tag
    auto interfaceReq = packet.get()->addTagIfAbsent<inet::InterfaceReq>();
    interfaceReq->setInterfaceId(satelliteInterfaceEntry->getInterfaceId());
    // Add UDP header
    addUdpHeader(packet.get());
    // Add IPv4 header
    addIpv4Header(packet.get());

    EV_TRACE << "registerAtSatellite -- transmitting SatelliteRegistrationMessage." << std::endl;
    sendSatellitePacket(std::move(packet)); 
    // statistics
    vehicleStatistics->recordSendSatelliteRegistrationMessages();
    vehicleStatistics->recordSendSatellitePackets();

    // Wait expectedAck_Timestamp
    EV_TRACE << "registerAtSatellite -- expectedAck_Timestamp: " << expectedAck_Timestamp << "s" << std::endl;
    scheduleAt(expectedAck_Timestamp, satelliteRegistrationAckTimer);
}

/*
 * This method is based on code snippets from INET 4.4.1. The code snippets are modified
 * such that they work with space_Veins. The original implementation can be found in the
 * following files:
 * <INET_root_folder/src/inet/transportlayer/udp/Udp.cc
 */
void SAMAC::addUdpHeader(inet::Packet* packet)
{
    L3Address srcAddr, destAddr;
    int srcPort = -1, destPort = -1;

    auto addressReq = packet->addTagIfAbsent<L3AddressReq>();
    srcAddr = addressReq->getSrcAddress();
    destAddr = addressReq->getDestAddress();

    L3AddressResolver().tryResolve(satelliteInterfaceEntry->getProtocolData<Ipv4InterfaceData>()->getIPAddress().str().c_str(), srcAddr);
    addressReq->setSrcAddress(srcAddr);

    // multicast address
    L3AddressResolver().tryResolve(par("leoSatelliteSamacMulticastAddress"), destAddr);
    addressReq->setDestAddress(destAddr);

    auto portsReq = packet->removeTagIfPresent<L4PortReq>();
    delete portsReq;

    srcPort = 12345;    // can be a dummy port because packets from the LEO satellite
                        // are not forwarded to any application

    destPort = par("leoSatelliteSamacDestPort");
    EV_TRACE << "addUdpHeader -- destPort: " << destPort << std::endl;

    if (addressReq->getDestAddress().isUnspecified())
        throw cRuntimeError("send: unspecified destination address");

    if (destPort <= 0 || destPort > 65535)
        throw cRuntimeError("send: invalid remote port number %d", destPort);

    if (packet->findTag<MulticastReq>() == nullptr)
        packet->addTag<MulticastReq>()->setMulticastLoop(false);    // do not receive own multicasts

    const Protocol *l3Protocol = nullptr;
    // TODO: apps use ModuleIdAddress if the network interface doesn't have an IP address configured, and UDP uses NextHopForwarding which results in a weird error in MessageDispatcher
    if (destAddr.getType() == L3Address::IPv4)
        l3Protocol = &Protocol::ipv4;
    else if (destAddr.getType() == L3Address::IPv6)
        l3Protocol = &Protocol::ipv6;
    else
        l3Protocol = &Protocol::nextHopForwarding;

    auto udpHeader = makeShared<UdpHeader>();
    // set source and destination port
    udpHeader->setSourcePort(srcPort);
    udpHeader->setDestinationPort(destPort);

    B totalLength = udpHeader->getChunkLength() + packet->getTotalLength();
    if(totalLength.get() > UDP_MAX_MESSAGE_SIZE)
        throw cRuntimeError("send: total UDP message size exceeds %u", UDP_MAX_MESSAGE_SIZE);

    udpHeader->setTotalLengthField(totalLength);
    // always use crcMode == CRC_DECLARED_CORRECT
    udpHeader->setCrcMode(CRC_DECLARED_CORRECT);
    udpHeader->setCrc(0x0000);

    insertTransportProtocolHeader(packet, Protocol::udp, udpHeader);
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(l3Protocol);
    packet->addTagIfAbsent<MacAddressReq>()->setDestAddress(leoSatelliteMacAddress);
    packet->setKind(0);
}

/*
 * This method is based on code snippets from INET 4.4.1. The code snippets are modified
 * such that they work with space_Veins. The original implementation can be found in the
 * following files:
 * <INET_root_folder/src/inet/networklayer/ipv4/Ipv4.cc
 */
void SAMAC::addIpv4Header(inet::Packet* packet)
{
    const auto& ipv4Header = makeShared<Ipv4Header>();

    auto l3AddressReq = packet->removeTag<L3AddressReq>();
    Ipv4Address src = l3AddressReq->getSrcAddress().toIpv4();
    Ipv4Address dest = l3AddressReq->getDestAddress().toIpv4();
    delete l3AddressReq;

    ipv4Header->setProtocolId((IpProtocolId)ProtocolGroup::ipprotocol.getProtocolNumber(packet->getTag<PacketProtocolTag>()->getProtocol()));

    // set source and destination address
    ipv4Header->setDestAddress(dest);
    ipv4Header->setSrcAddress(src);

    ipv4Header->setIdentification(0); // we do not fragment packets -> size of MTU must be enough, typically 1500 Byte
    ipv4Header->setMoreFragments(false);
    ipv4Header->setDontFragment(true);
    ipv4Header->setFragmentOffset(0);

    short ttl = -1;
    if (ttl != -1) {
        ASSERT(ttl > 0);
    }
    else if (ipv4Header->getDestAddress().isLinkLocalMulticast())
        ttl = 1;
    else if (ipv4Header->getDestAddress().isMulticast())
        ttl = 32;   // TODO: add Ned parameters
    else
        ttl = 32;
    ipv4Header->setTimeToLive(ttl);

    ASSERT(ipv4Header->getChunkLength() <= IPv4_MAX_HEADER_LENGTH);
    ipv4Header->setHeaderLength(ipv4Header->getChunkLength());
    ipv4Header->setTotalLengthField(ipv4Header->getChunkLength() + packet->getDataLength());
    ipv4Header->setCrcMode(CRC_DECLARED_CORRECT);
    ipv4Header->setCrc(0);

    insertNetworkProtocolHeader(packet, Protocol::ipv4, ipv4Header);
}

void SAMAC::doPacketInjection()
{
    EV_TRACE << "doPacketInjection -- " << std::endl;
    // get packet from queue
    inet::Packet* packet = pendingQueue->popPacket();
    // get Ownership of the packet
    take(packet);
    // encapsulate packet, do the same as Ieee80211Mac does
    encapsulate(packet);
    // Send direct to mac output gate. radio input gate does not work because it is connected with the mac output gate.
    // Connected gates cannot receive packets from sendDirect
    ieee80211MacOutputGate = getParentModule()->getParentModule()->getSubmodule("wlan", 0)->getSubmodule("mac", 0)->gate("lowerLayerOut", -1);
    ASSERT(ieee80211MacOutputGate != nullptr);
    ieee80211Radio->setRadioMode(inet::physicallayer::IRadio::RADIO_MODE_TRANSMITTER);
    sendDirect(packet, ieee80211MacOutputGate);
}

void SAMAC::sendSatellitePacket(std::unique_ptr<inet::Packet> pk)
{
    sendDown(pk.release());
}

SAMAC::~SAMAC()
{
}


void SAMAC::processLeoPacket(std::shared_ptr<inet::Packet> pk)
{
    PacketFilter satelliteAckFilter;
    satelliteAckFilter.setPattern("*", "SpaceVeinsSatelliteAck");

    PacketFilter wlanScheduleFilter;
    wlanScheduleFilter.setPattern("*", "space_veins::SpaceVeinsWlanScheduleMessage");

    if (satelliteAckFilter.matches(pk.get())) {
        EV_TRACE << "processLeoPacket -- received Leo Ack." << std::endl;
        processLeoAck(pk);
    }
    else if (wlanScheduleFilter.matches(pk.get())) {
        EV_TRACE << "processLeoPacket -- received wlanSchedule." << std::endl;
        processWlanSchedule(pk);
    }else{
        EV_WARN << "processLeoPacket -- received packet of unknown type: " << pk << std::endl;
    }
}

void SAMAC::processLeoAck(std::shared_ptr<inet::Packet> pk)
{
    // Remove IPv4 and UDP headers
    pk->popAtFront();
    pk->popAtFront();
    auto ack = pk->peekAtFront<SpaceVeinsSatelliteAck>();
    EV_TRACE << "processLeoAck -- received Ack with address: " << ack->getAcknowledgedVehicleId().str() << std::endl;
    if (ack->getAcknowledgedVehicleId() == address) {
        registeredAtSatellite = true;
        EV_TRACE << "processLeoAck -- registered with satellite." << std::endl;

        // statistics
        vehicleStatistics->recordReceivedSatelliteRegistrationAcks();

    }
    else{
        EV_TRACE << "processLeoAck -- received SpaceVeinsSatelliteAck with different vehicleId, discarding" << std::endl;
    }
    // statistics
    vehicleStatistics->recordReceivedSatellitePackets();
}

void SAMAC::processWlanSchedule(std::shared_ptr<inet::Packet> pk)
{
    auto* macAddresses = pk->getTag<inet::MacAddressInd>();
    leoSatelliteMacAddress = macAddresses->getDestAddress();
    EV_TRACE << "processWlanSchedule -- received LEO MAC address: " << leoSatelliteMacAddress.str() << std::endl;

    // Remove IPv4 and UDP headers
    pk->popAtFront();
    pk->popAtFront();
    auto ws = pk->peekAtFront<SpaceVeinsWlanScheduleMessage>();

    handleWlanSchedule(ws.get());
    if (!receivedInitialWlanSchedule) {
        receivedInitialWlanSchedule = true;
    }

    // statistics
    vehicleStatistics->recordReceivedWlanSchedules();
    vehicleStatistics->recordReceivedSatellitePackets();
}

void SAMAC::handleWlanSchedule(const SpaceVeinsWlanScheduleMessage* ws) {
    // schedule wlan transmission
    scheduleWlanTransmission(&(ws->getWlanSchedule()), ws->getWlanScheduleBeginTimestamp());

    // store wlan schedule interval
    wlanScheduleInterval_s = ws->getWlanScheduleInterval();
    EV_TRACE << "handleWlanSchedule -- wlanScheduleInterval_s: " << wlanScheduleInterval_s << "s" << std::endl;
    EV_TRACE << "handleWlanSchedule -- wlanScheduleTransmitTimestamp: " << ws->getTransmitTimestamp() << "s" << std::endl;
    endOfCurrentWlanSchedule = ws->getTransmitTimestamp() + wlanScheduleInterval_s;
    EV_TRACE << "handleWlanSchedule -- endOfCurrentWlanSchedule: " << endOfCurrentWlanSchedule << "s" << std::endl;

    // Calculate satellite->vehicle latency
    currentSatelliteLatency = simTime() - ws->getTransmitTimestamp();
    EV_TRACE << "handleWlanSchedule -- currentSatelliteLatency: " << currentSatelliteLatency << "s" << std::endl;

    // Last useful point in time to transmit a registration request such that it get acked before the new wlan schedule is published by the satellite
    lastUsefulRegistrationTime = endOfCurrentWlanSchedule - ((2 * currentSatelliteLatency) + maxSatelliteProcDelay_s);
    EV_TRACE << "handleWlanSchedule -- lastUsefulRegistrationTime: " << lastUsefulRegistrationTime << "s" << std::endl;
    vehicleStatistics->recordApplLayerRoundTripTime((2 * currentSatelliteLatency) + maxSatelliteProcDelay_s);
    vehicleStatistics->recordApplLayerSatelliteLatency(currentSatelliteLatency);

    // register at satellite for next wlan schedule
    if (!registeredAtSatellite && receivedInitialWlanSchedule) {
        EV_TRACE << "handleWlanSchedule -- no successful registration for current schedule!" << std::endl;
        vehicleStatistics->recordMissedSatelliteRegistrationAcks();
    }
    cancelEvent(satelliteRegistrationAckTimer); // cancel old registration process, try again
    registeredAtSatellite = false;

    // use ALOHA for satellite registration, satNic has no reasonable MAC layer yet
    EV_TRACE << "handleWlanSchedule -- start registration process for next schedule." << std::endl;
    auto satelliteRegistrationCallback = [this]() {
        startRegistrationTimestamp = simTime();
        EV_TRACE << "satelliteRegistrationCallback -- call registerAtSatellite()." << std::endl;
        registerAtSatellite();
    };
    timerManager.create(veins::TimerSpecification(satelliteRegistrationCallback).oneshotAt(uniform(simTime(), lastUsefulRegistrationTime)), "satelliteRegistrationCallback");
}

void SAMAC::scheduleWlanTransmission(const WlanSchedule* ws, const simtime_t begin)
{
    simtime_t delta;
    auto myWlanSchedule = ws->find(address.getInt());
    if (myWlanSchedule != ws->end()) {
        delta = SimTime((int64)(myWlanSchedule->second * 1000), SIMTIME_MS);
        EV_TRACE << "scheduleWlanTransmission -- coordinated transmission scheduled, delta: " << delta << "ms" << std::endl;
    }else{  // TODO: remove magic numbers
        if (ws->size() < 100) {  // Not all slots are occupied
            delta = SimTime(intuniform(ws->size(), 99), SIMTIME_MS);
            EV_TRACE << "scheduleWlanTransmission -- random transmission scheduled, delta: " << delta << "ms" << std::endl;
            // TODO: forward to vanilla ieee 80211p mac interface
        }else{
            if (ws->size() > 100) throw omnetpp::cRuntimeError("myWlanSchedule size is too large, size: %lu", ws->size());
        }
    }
    EV_TRACE << "scheduleWlanTransmission -- scheduling wlanTransmission at: " << begin + delta << std::endl;
    auto scheduleWlanTransmissionCallback = [this]() {
        if (!pendingQueue->isEmpty()) {
          doPacketInjection();
        }
        else{
            EV_WARN << "scheduleWlanTransmissionCallback -- no packet to send, queue empty" << std::endl;
        }
    };
    timerManager.create(veins::TimerSpecification(scheduleWlanTransmissionCallback).oneshotAt(begin + delta), "scheduleWlanTransmissionCallback");
}
