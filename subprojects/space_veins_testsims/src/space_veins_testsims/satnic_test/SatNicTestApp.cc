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

#include <algorithm>
#include <iostream>

#include "inet/common/Simsignals.h"
#include "space_veins_testsims/satnic_test/SatNicTestApp.h"

#include "space_veins_testsims/utils/asserts.h"
using space_veins::SatNicTestApp;

Define_Module(SatNicTestApp);

void SatNicTestApp::initialize(int stage)
{

    //veins::BaseApplLayer::initialize(stage);
    if (stage == 0) {
        testNumber = par("testNumber");
     
        getSystemModule()->subscribe(inet::packetReceivedSignal, this);
        getSystemModule()->subscribe(inet::packetSentSignal, this);

        
        //findHost()->subscribe(BaseMobility::mobilityStateChangedSignal, this);
        EV_DEBUG << "SatNicTest initialized with testNumber=" << testNumber << std::endl;

        performTest(0);
    }
}

void SatNicTestApp::finish()
{
    performTest(999);
}

void SatNicTestApp::handleSelfMsg(cMessage* msg)
{
}

void SatNicTestApp::handleLowerMsg(cMessage* msg)
{
    delete msg;
}


void SatNicTestApp::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details)
{
    auto packet = check_and_cast<inet::Packet *>(obj);

    if(oldPacketIds.find(packet->getId()) != oldPacketIds.end()){
        //packet has already been counted (due to the hierarchy there are two emits)    
        oldPacketIds.erase(packet->getId());
        return;
    }

    if (signalID == inet::packetReceivedSignal) {        
        packetReceivedCount++;
    } else if (signalID == inet::packetSentSignal) {
        packetSentCount++;
    }

    oldPacketIds.insert(packet->getId());
}


void SatNicTestApp::performTest(const simtime_t t)
{
    int testCounter = 0;

    if (testNumber == testCounter++) {
        if (t == 999) {
            assertEqual("No packets received with satNic", 0, packetReceivedCount);
            assertEqual("Packets sent", 2*20, packetSentCount);
        }
    }

    //
    // End
    //
}
