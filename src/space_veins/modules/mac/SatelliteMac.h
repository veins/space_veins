//
// Copyright (C) 2012 David Eckhoff <eckhoff@cs.fau.de>
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

#include <queue>
#include <memory>
#include <stdint.h>

#include "space_veins/space_veins.h"
#include "space_veins/modules/utility/ConstsSatellites.h"
#include "space_veins/modules/mac/SatelliteApplLayerToSatelliteMacInterface.h"
#include "space_veins/modules/mac/SatelliteMacToSatellitePhyInterface.h"
#include "space_veins/modules/messages/BaseSatelliteFrame_m.h"
#include "space_veins/modules/utility/MacToPhyControlInfoSatellite.h"
#include "space_veins/modules/phy/SatellitePhy.h"

// #include "veins/base/modules/BaseLayer.h"
// #include "veins/modules/phy/PhyLayer80211p.h"
// #include "veins/modules/utility/Consts80211p.h"
#include "veins/base/utils/FindModule.h"
// #include "veins/modules/messages/Mac80211Pkt_m.h"
#include "veins/base/messages/MacPkt_m.h"
// #include "veins/modules/messages/BaseFrame1609_4_m.h"
// #include "veins/modules/messages/AckTimeOutMessage_m.h"
// #include "veins/modules/messages/Mac80211Ack_m.h"
#include "veins/modules/messages/PhyControlMessage_m.h"
#include "veins/base/modules/BaseMacLayer.h"
// #include "veins/modules/utility/HasLogProxy.h"

namespace space_veins {

/**
 * @brief
 * Manages timeslots for CCH and SCH listening and sending.
 *
 * @author David Eckhoff : rewrote complete model
 * @author Christoph Sommer : features and bug fixes
 * @author Michele Segata : features and bug fixes
 * @author Stefan Joerer : features and bug fixes
 * @author Gurjashan Pannu: features (unicast model)
 * @author Mario Franke: space veins modificaitons
 * @author Christopher Saloman: initial version
 *
 * @ingroup macLayer
 *
 * @see DemoBaseApplLayer
 * @see Mac1609_4
 * @see PhyLayer80211p
 * @see Decider80211p
 */

class DeciderSatelliteResult;

class SPACE_VEINS_API SatelliteMac : public veins::BaseMacLayer, public SatelliteApplLayerToSatelliteMacInterface {

public:
    // tell to anybody which is interested when the channel turns busy or idle
    static const simsignal_t sigChannelBusy;
    // tell to anybody which is interested when a collision occurred
    static const simsignal_t sigCollision;
    // tell to anybody which is interested when a packet was sent
    static const simsignal_t sigSentPacket;
    // tell to anybody which is interested when a acknowledgement was sent
    static const simsignal_t sigSentAck;
    // tell to anybody which is interested when a failed unicast transmission occurred
    static const simsignal_t sigRetriesExceeded;


public:
    SatelliteMac()
        : nextChannelSwitch(nullptr)
        , nextMacEvent(nullptr)
    {
    }
    ~SatelliteMac() override;

    /**
     * @brief Change the default tx power the NIC card is using
     *
     * @param txPower_mW the tx power to be set in mW
     */
    void setTxPower(double txPower_mW);

    /**
     * @brief Change the default MCS the NIC card is using
     *
     * @param mcs the default modulation and coding scheme
     * to use
     */
    void setMCS(veins::MCS mcs);

    /**
     * @brief Change the phy layer carrier sense threshold.
     *
     * @param ccaThreshold_dBm the cca threshold in dBm
     */
    // void setCCAThreshold(double ccaThreshold_dBm);

protected:
    /** @brief Initialization of the module and some variables.*/
    void initialize(int) override;

    /** @brief Delete all dynamically allocated objects of the module.*/
    void finish() override;

    /** @brief Handle messages from lower layer.*/
    void handleLowerMsg(cMessage*) override;

    /** @brief Handle messages from upper layer.*/
    void handleUpperMsg(cMessage*) override;

    /** @brief Handle control messages from upper layer.*/
    void handleUpperControl(cMessage* msg) override;

    /** @brief Handle self messages such as timers.*/
    void handleSelfMsg(cMessage*) override;

    /** @brief Handle control messages from lower layer.*/
    void handleLowerControl(cMessage* msg) override;

    /** @brief Set a state for the channel selecting operation.*/
    // void setActiveChannel(ChannelType state);

    void sendFrame(veins::MacPkt* frame, omnetpp::simtime_t delay, SatelliteChannel channelNr, veins::MCS mcs, double txPower_mW);

    void attachControlInfo(veins::MacPkt* mac, SatelliteChannel channelNr, veins::MCS mcs, double txPower_mW);


    void channelBusy();
    void channelIdle(bool afterSwitch = false);

    void setParametersForBitrate(uint64_t bitrate);

    const veins::LAddress::L2Type& getMACAddress() override
    {
        ASSERT(myMacAddr != veins::LAddress::L2NULL());
        return BaseMacLayer::getMACAddress();
    }

protected:
    /** @brief Self message to indicate that the current channel shall be switched.*/
    cMessage* nextChannelSwitch;

    /** @brief Self message to wake up at next MacEvent */
    cMessage* nextMacEvent;

    /** @brief Last time the channel went idle */
    simtime_t lastIdle;
    simtime_t lastBusy;

    int headerLength;
    double macDelay;

    bool idleChannel;

    /** @brief The power (in mW) to transmit with.*/
    double txPower;

    veins::MCS mcs; ///< Modulation and coding scheme to use unless explicitly specified.

    /** @brief Id for debug messages */
    std::string myId;

    SatelliteMacToSatellitePhyInterface* phySat;
};

} // namespace space_veins
