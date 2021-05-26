//
// Copyright (C) 2004 Telecommunication Networks Group (TKN) at Technische Universitaet Berlin, Germany.
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

// author:      Marc Loebbers
// part of:     framework implementation developed by tkn
// description: - Base class for physical layers
//              - if you create your own physical layer, please subclass
//                from this class and use the sendToChannel() function!!
// author:      Mario Franke
// part of:     Space Veins
// description: - Implemenation of modifications for Space Veins

#pragma once

#include <vector>
#include <map>
#include <proj.h>
#include <sstream>

#include "space_veins/space_veins.h"

#include "veins/base/connectionManager/BaseConnectionManager.h"
#include "veins/base/connectionManager/ChannelAccess.h"

#include "space_veins/base/utils/RelativeSatellitePosition.h"
#include "space_veins/base/satellitesConnectionManager/SatellitesConnectionManager.h"
#include "space_veins/base/satellitesConnectionManager/SatelliteNicEntry.h"

namespace space_veins {

using ChannelMobilityAccessType = veins::AccessModuleWrap<veins::BaseMobility>;
using ChannelMobilityPtrType = ChannelMobilityAccessType::wrapType*;
class SatellitesConnectionManager;

/**
 * @brief Basic class for all physical layers, please don't touch!!
 *
 * This class is not supposed to work on its own, but it contains
 * functions and lists that cooperate with ConnectionManager to handle
 * the dynamically created gates. This means EVERY physical layer (the lowest
 * layer in a host) has to be derived from this class!!!!
 *
 * Please don't touch this class.
 *
 * @author Marc Loebbers, Mario Franke
 * @ingroup connectionManager
 * @ingroup phyLayer
 * @ingroup baseModules
 **/
//class SPACE_VEINS_API SatelliteChannelAccess : public veins::ChannelAccess {

// SatelliteChannelAccess must not inherit from veins::ChannelAccess because otherwise
// the TraCIScenarioManager will try to unregister the satellite nics from
// the vehicle ConnectionManager.
class SPACE_VEINS_API SatelliteChannelAccess : public veins::BatteryAccess, protected veins::ChannelMobilityAccessType {
protected:
    /** @brief use sendDirect or not?*/
    bool useSendDirect;

    /** @brief Pointer to the PropagationModel module*/
    veins::BaseConnectionManager* cc;

    /** @brief Defines if the physical layer should simulate propagation delay.*/
    bool usePropagationDelay;

    /** @brief Is this module already registered with ConnectionManager? */
    bool isRegistered;

    /** @brief Pointer to the World Utility, to obtain some global information*/
    veins::BaseWorldUtility* world;

    /** @brief Current antenna position */
    veins::AntennaPosition antennaPosition;

    /** @brief Current antenna heading (angle) */
    veins::Heading antennaHeading;

    /** @brief Offset of antenna position (in m) with respect to what a BaseMobility module will tell us */
    veins::Coord antennaOffset = veins::Coord(0, 0, 0);

    /** @brief Offset of antenna orientation (yaw, in rad) with respect to what a BaseMobility module will tell us */
    double antennaOffsetYaw = 0;

    /** @brief Pointer to the SatellitesConnectionManager module*/
    SatellitesConnectionManager* scc;

    /** @brief Vector that stores all propagation delays */
    cOutVector propagation_delay_s_vec;
    cOutVector phyLayer_dist_m_sender_receiver_vec;

protected:
    /**
     * @brief Calculates the propagation delay to the passed receiving nic.
     */
    simtime_t calculatePropagationDelay(const veins::NicEntry* nic);

    /** @brief Sends a message to all nics connected to this one.
     *
     * This function has to be called whenever a packet is supposed to be
     * sent to the channel. Don't try to figure out what gates you have
     * and which ones are connected, this function does this for you!
     *
     * depending on which ConnectionManager module is used, the messages are
     * send via sendDirect() or to the respective gates.
     **/
    void sendToChannel(cPacket* msg);

public:
    /**
     * @brief Returns a pointer to the SatellitesConnectionManager responsible for the
     * passed NIC module.
     *
     * @param nic a pointer to a NIC module
     * @return a pointer to a connection manager module or NULL if an error
     * occurred
     */
    static SatellitesConnectionManager* getConnectionManager(cModule* nic);

    /**
     * @brief Called by the signalling mechanism to inform of changes.
     *
     * Base class ChannelAccess is subscribed to position changes and informs the
     * ConnectionManager. The SatelliteChannelAccess updates the relative satellite
     * position.
     */
    void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details) override;

    /** @brief Register with ConnectionManager.
     *
     * Upon initialization ChannelAccess registers the nic parent module
     * to have all its connections handeled by ConnectionManager
     **/
    void initialize(int stage) override;

    /**
     * @brief Returns the host's mobility module.
     */
    virtual veins::ChannelMobilityPtrType getMobilityModule()
    {
        return veins::ChannelMobilityAccessType::get(this);
    }

    virtual veins::AntennaPosition getAntennaPosition() const
    {
        return antennaPosition;
    }
};

} // namespace space_veins
