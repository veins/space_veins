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

#pragma once

#include "space_veins/space_veins.h"
#include "space_veins/modules/statistics/VehicleStatistics/VehicleStatistics.h"
#include "space_veins/modules/mac/sidelink/SAMAC/messages/WlanSchedule.h"
#include "space_veins/modules/mac/sidelink/SAMAC/messages/SpaceVeinsWlanScheduleMessage_m.h"

#include "veins_inet/VeinsInetApplicationBase.h"

#if INET_VERSION >= 0x0403
#include "inet/networklayer/common/NetworkInterface.h"
#else
#include "inet/networklayer/common/InterfaceEntry.h"
#endif

using namespace space_veins;

class SPACE_VEINS_API SamacSatelliteAppl : public veins::VeinsInetApplicationBase {
protected:
    bool haveForwarded = false;
    const int portNumber = 9001;

#if INET_VERSION >= 0x0403
    NetworkInterface* ie;
#else
    inet::InterfaceEntry* ie;
#endif

    cMessage* wlanScheduleTimer = nullptr;

    /* Statistics */
    VehicleStatistics* vehicleStatistics;

    /* SAMAC */
    WlanSchedule wlanSchedule;
    double wlanSlot_s;
    simtime_t wlanScheduleInterval_s;    // wlan schedule interval
    simtime_t maxV2SLatency_s;

protected:
    virtual bool startApplication() override;
    virtual bool stopApplication() override;
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage* msg) override;
    virtual void handleSelfMessage(cMessage* msg);
    virtual void processPacket(std::shared_ptr<inet::Packet> pk) override;
    virtual void handleStartOperation(inet::LifecycleOperation* doneCallback) override;
    virtual void sendPacket(std::unique_ptr<inet::Packet> pk) override;
    void sendSampleMessage();
    void sendVehicleRegistrationAck(inet::MacAddress vehicleId);
    void addToWlanSchedule(inet::MacAddress vehicleId);
    void publishWlanSchedule();

public:
    SamacSatelliteAppl();
    ~SamacSatelliteAppl();
};
