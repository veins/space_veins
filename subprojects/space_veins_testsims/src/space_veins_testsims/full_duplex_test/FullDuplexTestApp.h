//
// Copyright (C) 2006-2012 Christoph Sommer <christoph.sommer@uibk.ac.at>
// Copyright (C) 2025 Mario Franke <research@m-franke.net>
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
#include <list>

#include "space_veins/space_veins.h"
#include "inet/applications/udpapp/UdpSink.h"
#include "veins/base/modules/BaseModule.h"


namespace space_veins {

class FullDuplexTestApp : public veins::BaseModule {
public:
    int numInitStages() const
    {
        return std::max(veins::BaseModule::numInitStages(), 1);
    }
    inline void initialize(int stage);
    void finish();

protected:
    // module parameters
    int testNumber;
    int packetReceivedCount = 0;
    int packetSentCount = 0;
    std::set<int> oldPacketIds = {};

protected:
    void handleSelfMsg(cMessage*);
    void handleLowerMsg(cMessage*);

    void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details);

    /**
     * Perform test for time step t (special value t=0: initialize, t=999: finish)
     */
    void performTest(const simtime_t t);
};

} // namespace veins
