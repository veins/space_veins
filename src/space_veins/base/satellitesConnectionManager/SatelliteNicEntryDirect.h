//
// Copyright (C) 2005 Telecommunication Networks Group (TKN) at Technische Universitaet Berlin, Germany.
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

// author:      Daniel Willkomm
// part of:     framework implementation developed by tkn
// description: Class to store information about a nic for the
//              ConnectionManager module

// author:      Mario Franke
// part of:     Space Veins
// description: Class to store information about a satellite nic for the
//              SatellitesConnectionManager module

#pragma once

#include "space_veins/base/satellitesConnectionManager/SatelliteNicEntry.h"

namespace space_veins {

/**
 * @brief SatelliteNicEntry is used by SatellitesConnectionManager to store the necessary
 * information for each satellite nic
 *
 * @ingroup connectionManager, satellitesConnectionManager
 * @author Daniel Willkomm, Mario Franke
 * @sa ConnectionManager, NicEntry, SatellitesConnectionManager, SatelliteNicEntry
 */
class SPACE_VEINS_API SatelliteNicEntryDirect : public SatelliteNicEntry {
public:
    /** @brief Constructor, initializes all members
     */
    SatelliteNicEntryDirect(cComponent* owner)
        : SatelliteNicEntry(owner){};

    /**
     * @brief Destructor -- needs to be there...
     */
    ~SatelliteNicEntryDirect() override
    {
    }

    /** @brief Connect two nics
     *
     * Establish unidirectional connection with other nic
     *
     * @param other reference to remote nic (other NicEntry)
     *
     * This function acquires an in gate at the remote nic and an out
     * gate at this nic, connects the two and updates the freeInGate,
     * freeOutGate and outConns data sets.
     */
    void connectTo(NicEntry*) override;

    /** @brief Disconnect two nics
     *
     * Release unidirectional connection with other nic
     *
     * @param other reference to remote nic (other NicEntry)
     */
    void disconnectFrom(NicEntry*) override;
};

} // namespace space_veins
