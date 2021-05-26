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

#include "space_veins/base/satellitesConnectionManager/SatelliteNicEntryDirect.h"
#include "space_veins/base/satellitesConnectionManager/SatelliteChannelAccess.h"

using std::endl;
using namespace space_veins;

void SatelliteNicEntryDirect::connectTo(NicEntry* other)
{
    cModule* otherPtr = other->nicPtr;

    EV_TRACE << "connecting nic #" << nicId << " and #" << other->nicId << endl;

    cGate* radioGate = nullptr;
    if ((radioGate = otherPtr->gate("radioIn")) == nullptr) throw cRuntimeError("Nic has no radioIn gate!");

    outConns[other] = radioGate->getPathStartGate();
}

void SatelliteNicEntryDirect::disconnectFrom(NicEntry* other)
{
    EV_TRACE << "disconnecting nic #" << nicId << " and #" << other->nicId << endl;
    outConns.erase(other);
}
