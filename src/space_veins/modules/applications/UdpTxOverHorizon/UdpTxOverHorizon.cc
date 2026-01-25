// Copyright (C) 2026 Mario Franke <research@m-franke.net>
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

#include "space_veins/modules/applications/UdpTxOverHorizon/UdpTxOverHorizon.h"

using namespace space_veins;

Define_Module(space_veins::UdpTxOverHorizon);

void UdpTxOverHorizon::initialize(int stage) {
    inet::UdpBasicApp::initialize(stage);
    if (stage == inet::INITSTAGE_LOCAL) {
        enableHorizonThreshold = par("enableHorizonThreshold");
        horizonThreshold = par("horizonThreshold");
        WATCH(enableHorizonThreshold);
        WATCH(horizonThreshold);
    }
    if (stage == inet::INITSTAGE_LOCAL + 1) {
        // get pointer to SGP4Mobility
        sgp4Mobility = SGP4MobilityBaseAccess().get(getParentModule());
    }
}

void UdpTxOverHorizon::processSend()
{
    if (enableHorizonThreshold) {
        if (sgp4Mobility->getCurrentPosition().z >= horizonThreshold) {
            // TODO: only send packets when we are in a relevant period
            // only send packets when we are close to the horizon
            sendPacket();
        }
    } else {
        sendPacket();
    }
    simtime_t d = simTime() + par("sendInterval");
    if (stopTime < SIMTIME_ZERO || d < stopTime) {
        selfMsg->setKind(SEND);
        scheduleAt(d, selfMsg);
    }
    else {
        selfMsg->setKind(STOP);
        scheduleAt(stopTime, selfMsg);
    }
}