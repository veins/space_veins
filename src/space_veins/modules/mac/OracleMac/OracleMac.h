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

#include <set>
#include <map>

#include "inet/linklayer/acking/AckingMac.h"

#include "space_veins/space_veins.h"
#include "space_veins/modules/mac/OracleMac/GlobalOracleMac.h"
#include "space_veins/modules/mobility/SGP4Mobility/SGP4MobilityBase.h"

namespace space_veins {
class SPACE_VEINS_API OracleMac : public inet::AckingMac {

    protected:
        struct satelliteContact_t {
            int satelliteId;
            simtime_t lastContact;
            int numberOfContacts;
        };
        std::map<int, satelliteContact_t> satelliteContacts;
        simtime_t satelliteContactTimeout;

        bool ignoreGlobalOracle;

        GlobalOracleMac* globalOracleMac = nullptr;
        SGP4MobilityBase* sgp4m = nullptr;
        bool isSatellite;
        cOutVector vecSendDown;

        unsigned long uniquePacketsReceivedBySatellites = 0;
        unsigned long totalPacketsReceivedBySatellites = 0;
        unsigned int acksPerPacket = 0;
        std::set<int> receivingSatelliteIds;
        cOutVector vecReceivingSatelliteIds;
        cOutVector vecAcksPerPacket;
        cOutVector vecSatelliteContacts;
        int nodeId;
        cMessage* sendTimer = nullptr;
        inet::Packet* currentTxFrameDuplicate = nullptr;
        enum SelfMessageIds {
            SEND_TIMER = 0
        };

    public:
        virtual ~OracleMac();
        bool getUseAck() { return useAck; }


    protected:
        virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;
        virtual void startTransmitting() override;
        virtual void acked(inet::Packet *frame, int senderId);
        virtual void handleSelfMessage(cMessage *message) override;
        virtual void handleLowerPacket(inet::Packet *packet) override;
        virtual void encapsulate(inet::Packet *packet) override;
        virtual void finish() override;
};
} // end of namespace space_veins
