//
// Copyright (C) 2021 Mario Franke <research@m-franke.net>
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#pragma once

#include "veins/base/utils/FindModule.h"

#include "space_veins/space_veins.h"

using namespace omnetpp;

namespace space_veins {
class SPACE_VEINS_API GlobalOracleMac : public cSimpleModule
{
    protected:
        enum SelfMessageIds {
            CHANNEL_IDLE_TIMER = 0
        };
        double tdmaSlotDuration; // in seconds
        bool busy = false; // true if channel is busy
        simtime_t channelIdleTime; // time when the channel becomes idle again
        cMessage* channelIdleTimer = nullptr; // expires when the channel becomes idle again
        cOutVector vecSendTimes;

        virtual void initialize(int stage) override;
        virtual void finish() override;
        virtual void handleMessage(cMessage *msg) override;
        void handleSelfMessage(cMessage *msg);
        ~GlobalOracleMac();
    
    public:
        simtime_t askOracle();  // get a timestamp when to send a message

};

class SPACE_VEINS_API GlobalOracleMacAccess {
public:
    GlobalOracleMac* get()
    {
        GlobalOracleMac* gom = veins::FindModule<GlobalOracleMac*>::findGlobalModule();
        ASSERT(gom);
        return gom;
    };
};
} // end of namespace space_veins