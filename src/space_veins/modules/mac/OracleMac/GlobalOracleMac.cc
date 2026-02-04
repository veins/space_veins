//
// Copyright (C) 2023 Mario Franke <research@m-franke.net>
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

#include "space_veins/modules/mac/OracleMac/GlobalOracleMac.h"

Define_Module(space_veins::GlobalOracleMac);

using namespace space_veins;

void GlobalOracleMac::initialize(int stage)
{
    if (stage == 0) {
        tdmaSlotDuration = par("tdmaSlotDuration");
        channelIdleTime = SimTime();
        channelIdleTimer = new cMessage("channelIdleTimer", CHANNEL_IDLE_TIMER);
        vecSendTimes.setName("sendTimes:vector");
        EV_DEBUG << "GlobalOralceMac initialized." << std::endl;
    }
}

GlobalOracleMac::~GlobalOracleMac()
{
    cancelAndDelete(channelIdleTimer);
}

void GlobalOracleMac::finish() {
}

void GlobalOracleMac::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        handleSelfMessage(msg);
    } else {
        EV_DEBUG << "GlobalOracleMac received unknown message: " << msg << std::endl;
        throw cRuntimeError("GlobalOracleMac received non-self message");
    }
}

void GlobalOracleMac::handleSelfMessage(cMessage *msg)
{
    if (msg->getKind() == CHANNEL_IDLE_TIMER) {
        busy = false;
        ASSERT(channelIdleTime == simTime());
        EV_TRACE << "GlobalOracleMac: channel idle again at: " << simTime() << std::endl;
    }
}

simtime_t GlobalOracleMac::askOracle()
{
    Enter_Method("askOracle()");
    if (!busy) {
        ASSERT(!channelIdleTimer->isScheduled());
        channelIdleTime = simTime() + tdmaSlotDuration;
        scheduleAt(channelIdleTime, channelIdleTimer);
        busy = true;
        EV_TRACE << "GlobalOracleMac: channel idle -> send immediately at: " << simTime() << std::endl;
        vecSendTimes.record(simTime());
        return simTime();
    } else {
        cancelEvent(channelIdleTimer);
        simtime_t sendTime = channelIdleTime;
        scheduleAt(channelIdleTime + tdmaSlotDuration, channelIdleTimer);
        channelIdleTime += tdmaSlotDuration;
        EV_TRACE << "GlobalOracleMac: channel busy -> send at: " << sendTime << std::endl;
        vecSendTimes.record(sendTime);
        return sendTime;
    }
}