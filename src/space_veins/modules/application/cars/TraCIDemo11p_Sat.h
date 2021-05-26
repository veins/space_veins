//
// Copyright (C) 2006-2011 Christoph Sommer <christoph.sommer@uibk.ac.at>
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

#include "space_veins/space_veins.h"
#include "space_veins/modules/messages/BaseSatelliteFrame_m.h"

#include "veins/modules/utility/TimerManager.h"
#include "veins/modules/application/traci/TraCIDemo11p.h"

namespace space_veins {

/**
 * @brief
 * A tutorial demo for TraCI. When the car is stopped for longer than 10 seconds
 * it will send a message out to other cars containing the blocked road id.
 * Receiving cars will then trigger a reroute via TraCI.
 * When channel switching between SCH and CCH is enabled on the MAC, the message is
 * instead send out on a service channel following a Service Advertisement
 * on the CCH.
 *
 * @author Christoph Sommer : initial DemoApp
 * @author David Eckhoff : rewriting, moving functionality to DemoBaseApplLayer, adding WSA
 * @author Mario Franke : Space Veins modifications
 *
 */

class SPACE_VEINS_API TraCIDemo11p_Sat : public veins::TraCIDemo11p {
public:
    void initialize(int stage) override;

protected:
    void handleMessage(cMessage* msg) override;
    void handleSelfMsg(cMessage* msg) override;
    void handlePositionUpdate(cObject* obj) override;

    void sendPingViaSatellite();
    void sendPongViaSatellite();

    /** @brief Sends a message via c2xNic
     *
     * Short hand for send(msg, c2xLowerLayerOut);
     *
     * @param msg the message to be sent.
     */
    void sendViaC2X(cMessage* msg);

    /** @brief Sends a message via satelliteNic
     *
     * Short hand for send(msg, satLowerLayerOut);
     *
     * @param msg the message to be sent.
     */
    void sendViaSatellite(cMessage* msg);

    /**
     * @brief Sends a message via c2xNic with delay.
     *
     * @param msg the message to be sent.
     * @param delay the delay for the message
     */
    void sendDelayedViaC2X(cMessage* msg, simtime_t delay);

    /**
     * @brief Sends a message via satelliteNic with delay.
     *
     * @param msg the message to be sent.
     * @param delay the delay for the message
     */
    void sendDelayedViaSatellite(cMessage* msg, simtime_t delay);

    void handleLowerSatMessage(cMessage* msg);

    void handleLowerSatControlMessage(cMessage* msg);

protected:

    /** Statistics */
    cOutVector ping_s_vec;
    simtime_t sentPing;

    veins::TimerManager timerManager{this};

    uint32_t satPingLengthBits;
    uint32_t satPongLengthBits;

    /** Gates */
    int satLowerLayerIn;
    int satLowerLayerOut;
    int satLowerControlIn;
    int satLowerControlOut;

    int c2xLowerLayerIn;
    int c2xLowerLayerOut;
    int c2xLowerControlIn;
    int c2xLowerControlOut;
};

} // namespace space_veins
