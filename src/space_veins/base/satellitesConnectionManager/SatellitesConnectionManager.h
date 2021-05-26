//
// Copyright (C) 2007 Technische Universitaet Berlin (TUB), Germany, Telecommunication Networks Group
// Copyright (C) 2007 Technische Universiteit Delft (TUD), Netherlands
// Copyright (C) 2007 Universitaet Paderborn (UPB), Germany
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

#include <proj.h>
#include <stdlib.h> /* abs */
#include <math.h>   /* asin */

#include "space_veins/space_veins.h"
#include "space_veins/base/satellitesConnectionManager/SkyfieldMobilityClient.h"
#include "space_veins/base/satellitesConnectionManager/SatelliteChannelAccess.h"
#include "space_veins/base/utils/RelativeSatellitePosition.h"
#include "space_veins/base/satellitesConnectionManager/SatelliteNicEntry.h"
#include "space_veins/base/satellitesConnectionManager/SatelliteNicEntryDebug.h"
#include "space_veins/base/satellitesConnectionManager/SatelliteNicEntryDirect.h"

// Hack that enables calling updateNicConnections which is declared private
// in veins::BaseConnectionManager.h
// #define private protected
// 
#include "veins/base/connectionManager/BaseConnectionManager.h"
// 
// #undef private

#include "veins/modules/mobility/traci/TraCICoordinateTransformation.h"
#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/mobility/traci/TraCIConnection.h"
#include "veins/modules/mobility/traci/TraCICoord.h"
#include "veins/base/utils/Coord.h"
#include "veins/base/utils/AntennaPosition.h"
#include "veins/base/connectionManager/NicEntry.h"
#include "veins/base/connectionManager/NicEntryDebug.h"
#include "veins/base/connectionManager/NicEntryDirect.h"
#include "veins/base/utils/Heading.h"


namespace space_veins {

class SatelliteChannelAccess;

/**
 * @brief SatellitesConnectionManager implementation
 *
 * @ingroup connectionManager
 */
class SPACE_VEINS_API SatellitesConnectionManager : public veins::BaseConnectionManager {
//class SPACE_VEINS_API SatellitesConnectionManager : public veins::BaseModule {
public:
    using Latitude = double;
    using Longitude = double;
    using WGS84Coordinate = std::pair<Latitude, Longitude>;

protected:
    cOutVector dist_m_sender_receiver_vec;
    cOutVector height_diff_m_sender_receiver_vec;
    cOutVector altitude_deg_sender_receiver_vec;

private:
    std::unique_ptr<SkyfieldMobilityClient> smc;
    std::unique_ptr<veins::TraCIConnection> traciConnection;
    double minAltitudeAngle_deg;
    bool traciConnectionEstablished = false;

    /** @brief Type for map from nic-module id to nic-module pointer.*/
    typedef std::map<int, SatelliteNicEntry*> NicEntries;

    /** @brief Map from nic-module ids to nic-module pointers.*/
    NicEntries nics;

    /** @brief Does the ConnectionManager use sendDirect or not?*/
    bool sendDirect;

    // proj context
    PJ_CONTEXT* pj_ctx;
    PJ* projection;

protected:
    /**
     * @brief Calculate interference distance
     *
     * You may want to overwrite this function in order to do your own
     * interference calculation
     */
    double calcInterfDist() override;

    void initialize(int stage) override;

    std::string getProjectionString(const cXMLElement* sumoNetXmlFile) const;

    bool isInRange(NicEntries::mapped_type pFromNic, NicEntries::mapped_type pToNic);

    void updateNicConnections(NicEntries::mapped_type nic);

public:
    WGS84Coordinate omnetCoord2GeoCoord(const veins::Coord omnetCoord);

    RelativeSatellitePosition getRelativeSatellitePosition(const WGS84Coordinate geo, const std::string satelliteName);

    bool isTraCIConnectionEstablished() const;

    /** @brief Returns the ingates of all nics in range*/
    const veins::NicEntry::GateList& getGateList(int nicID) const;

    /** @brief Updates the position information of a registered nic.*/
    void updateNicPos(int nicID, veins::Coord newPos, veins::Heading heading);

    /**
     * @brief Registers a nic to have its connections managed by ConnectionManager.
     *
     * If you want to do your own stuff at the registration of a nic see
     * "registerNicExt()".
     */
    bool registerNic(cModule* nic, SatelliteChannelAccess* chAccess, veins::Coord nicPos, veins::Heading heading);

    /**
     * @brief Unregisters a NIC such that its connections aren't managed by the CM
     * anymore.
     *
     * NOTE: This method asserts that the passed NIC module was previously registered
     * with this ConnectionManager!
     *
     * This method should be used for dynamic networks were hosts can actually disappear.
     *
     * @param nic the NIC module to be unregistered
     * @return returns true if the NIC was unregistered successfully
     */
    bool unregisterNic(cModule* nic);

};

} // namespace space_veins
