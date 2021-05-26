//
// Copyright (C) 2016 David Eckhoff <eckhoff@cs.fau.de>
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

#pragma once

#include <map>

#include "space_veins/space_veins.h"
#include "space_veins/modules/mac/SatelliteApplLayerToSatelliteMacInterface.h"
#include "space_veins/modules/messages/BaseSatelliteFrame_m.h"

#include "veins/base/modules/BaseApplLayer.h"
#include "veins/base/utils/FindModule.h"
// #include "veins/modules/utility/Consts80211p.h"
// #include "veins/modules/messages/BaseFrame1609_4_m.h"
// #include "veins/modules/messages/DemoServiceAdvertisement_m.h"
// #include "veins/modules/messages/DemoSafetyMessage_m.h"
// #include "veins/base/connectionManager/ChannelAccess.h"
// #include "veins/modules/mac/ieee80211p/DemoBaseApplLayerToMac1609_4Interface.h"
// #include "veins/modules/mobility/traci/TraCIMobility.h"
// #include "veins/modules/mobility/traci/TraCICommandInterface.h"

namespace space_veins {

/**
 * @brief
 * Demo application layer base class.
 *
 * @author David Eckhoff
 *
 * @ingroup applLayer
 *
 * @see DemoBaseApplLayer
 * @see Mac1609_4
 * @see PhyLayer80211p
 * @see Decider80211p
 */
class VEINS_API SatelliteApplLayer : public veins::BaseApplLayer {

public:
    ~SatelliteApplLayer() override;
    void initialize(int stage) override;
    void finish() override;

    void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details) override;

protected:
    /** @brief handle messages from below and calls the onWSM, onBSM, and onWSA functions accordingly */
    void handleLowerMsg(cMessage* msg) override;

    /** @brief handle self messages */
    void handleSelfMsg(cMessage* msg) override;

    /** @brief this function is called every time the vehicle receives a position update signal */
    //virtual void handlePositionUpdate(cObject* obj);

    void sendPing();

    void sendPong();

    /**
     * @brief overloaded for error handling and stats recording purposes
     *
     * @param msg the message to be sent.
     */
    virtual void sendDown(cMessage* msg);

    /**
     * @brief overloaded for error handling and stats recording purposes
     *
     * @param msg the message to be sent.
     * @param delay the delay for the message
     */
    virtual void sendDelayedDown(cMessage* msg, simtime_t delay);

protected:
    SatelliteApplLayerToSatelliteMacInterface* mac;

    uint32_t satPingLengthBits;
    uint32_t satPongLengthBits;

    uint32_t dataLengthBits;
    veins::LAddress::L2Type myId = 0;
};

} // namespace space_veins
