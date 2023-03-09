//
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

#pragma once

#include "veins/modules/utility/TimerManager.h"

#include "space_veins/modules/mac/sidelink/SAMAC/messages/SpaceVeinsWlanScheduleMessage_m.h"
#include "space_veins/modules/mac/sidelink/SAMAC/messages/WlanSchedule.h"
#include "space_veins/modules/statistics/VehicleStatistics/VehicleStatistics.h"

#include "inet/common/packet/PacketFilter.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211Radio.h"
#include "inet/linklayer/base/MacProtocolBase.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/ieee80211/mib/Ieee80211Mib.h"
#include "inet/linklayer/ieee80211/llc/IIeee80211Llc.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211SubtypeTag_m.h"

using namespace space_veins;

class SPACE_VEINS_API SAMAC : public inet::MacProtocolBase {
protected:
    veins::TimerManager timerManager{this};
    cMessage* satelliteRegistrationAckTimer;

    int leoInGateId = -1;
    int leoOutGateId = -1;
    int ieee80211pRadioOutGateId = -1;

    inet::ieee80211::Ieee80211Mac* ieee80211Mac = nullptr;
    inet::ieee80211::IOriginatorMacDataService* omds = nullptr;
    inet::physicallayer::Ieee80211Radio* ieee80211Radio = nullptr;
    inet::ieee80211::Ieee80211Mib* mib = nullptr;
    inet::queueing::IPacketQueue *pendingQueue = nullptr;
    inet::queueing::IPacketQueue *inProgressFrames = nullptr;
    cGate* ieee80211MacOutputGate = nullptr;

    bool registeredAtSatellite = false;
    bool receivedInitialWlanSchedule = false;
    bool qosStation = false;

    VehicleStatistics* vehicleStatistics;

    const int leoSatelliteSamacDestPort = -1;

    inet::MacAddress address;
    inet::MacAddress leoSatelliteMacAddress;


    simtime_t wlanScheduleInterval_s;    // wlan schedule interval
    simtime_t endOfCurrentWlanSchedule;
    simtime_t currentSatelliteLatency;
    simtime_t startRegistrationTimestamp;
    simtime_t lastUsefulRegistrationTime;
    simtime_t maxSatelliteProcDelay_s;
    simtime_t expectedAck_Timestamp;

#if INET_VERSION >= 0x0403
    NetworkInterface* satelliteInterfaceEntry;
#else
    inet::InterfaceEntry* satelliteInterfaceEntry;
#endif

protected:
    virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void finish() override;

    /** implements MacBase functions */
    //@{
    virtual void configureInterfaceEntry() override;
    //@}

    virtual void handleMessageWhenUp(cMessage *message) override;
    virtual void handleSelfMessage(cMessage *msg) override;
    /** @brief Handle messages from upper layer */
    virtual void handleUpperPacket(inet::Packet *packet) override;
    /** @brief Handle messages from Leo satellite */
    virtual void handleLeoPacket(inet::Packet *packet);

    virtual void handleStartOperation(inet::LifecycleOperation* doneCallback) override;
    virtual void handleStopOperation(inet::LifecycleOperation* doneCallback) override;
    virtual void handleCrashOperation(inet::LifecycleOperation* doneCallback) override;

    virtual void encapsulate(inet::Packet* packet);
    virtual std::unique_ptr<inet::Packet> createPacket(std::string name);
    virtual void timestampPayload(inet::Ptr<inet::Chunk> payload);
    void registerAtSatellite();
    void addUdpHeader(inet::Packet* packet);
    void addIpv4Header(inet::Packet* packet);
    void doPacketInjection();
    virtual void processLeoPacket(std::shared_ptr<inet::Packet> pk);
    virtual void processLeoAck(std::shared_ptr<inet::Packet> pk);
    virtual void processWlanSchedule(std::shared_ptr<inet::Packet> pk);
    virtual void handleWlanSchedule(const SpaceVeinsWlanScheduleMessage* ws);
    virtual void sendSatellitePacket(std::unique_ptr<inet::Packet> pk);
    void scheduleWlanTransmission(const WlanSchedule* ws, const simtime_t begin);

public:
    SAMAC();
    ~SAMAC();
};
