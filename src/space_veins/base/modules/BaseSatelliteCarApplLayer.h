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
#include "veins/base/modules/BaseApplLayer.h"

namespace space_veins {

/**
 * @brief
 * Extension class for the application layer for cars with satellite interface.
 *
 * @author David Eckhoff
 * @author Mario Franke
 *
 * @ingroup applLayer
 *
 * @see DemoBaseApplLayer
 * @see Mac1609_4
 * @see PhyLayer80211p
 * @see Decider80211p
 */
class SPACE_VEINS_API BaseSatelliteCarApplLayer : public veins::BaseApplLayer {

public:
    ~BaseSatelliteCarApplLayer();
    void initialize(int stage);
    void finish();

    // void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details) override;

protected:
    /** @brief handle messages from below and calls the onWSM, onBSM, and onWSA functions accordingly */
    void handleLowerMsg(cMessage* msg);

    /** @brief handle self messages */
    void handleSelfMsg(cMessage* msg);

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
     * @brief overloaded for error handling and stats recording purposes
     *
     * @param msg the message to be sent.
     * @param delay the delay for the message
     */
    virtual void sendDelayedViaC2X(cMessage* msg, simtime_t delay);

    /**
     * @brief overloaded for error handling and stats recording purposes
     *
     * @param msg the message to be sent.
     * @param delay the delay for the message
     */
    virtual void sendDelayedViaSatellite(cMessage* msg, simtime_t delay);

protected:

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
