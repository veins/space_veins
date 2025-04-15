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
#include "space_veins_testsims/elevation_angle_test/ElevationAngleTestApp.h"

#include "space_veins_testsims/utils/asserts.h"
using space_veins::ElevationAngleTestApp;

Define_Module(ElevationAngleTestApp);

void ElevationAngleTestApp::initialize(int stage)
{
    //veins::BaseApplLayer::initialize(stage);

    if (stage == 0) {
        testNumber = par("testNumber");
     
        getSystemModule()->subscribe(inet::packetReceivedSignal, this);
        getSystemModule()->subscribe(inet::packetSentSignal, this);

        EV_DEBUG << "ElevationAngleTest initialized with testNumber=" << testNumber << std::endl;

        performTest(0);
    }
}

void ElevationAngleTestApp::finish()
{
    performTest(999);
}

void ElevationAngleTestApp::handleSelfMsg(cMessage* msg)
{
}

void ElevationAngleTestApp::handleLowerMsg(cMessage* msg)
{
    delete msg;
}

void ElevationAngleTestApp::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details)
{
    if(auto packet = check_and_cast<inet::Packet *>(obj)){
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

    
}

void ElevationAngleTestApp::performTest(const simtime_t t)
{
    int testCounter = 0;

    //Simulation 0 with ignored interferece
    if (testNumber == testCounter++) {
        if (t == 999) {
            //25.5 deg satellite and vehicle can communicate for 1 seconds (one packet per second)
            assertEqual("Packets received with minElevationAngle of 25deg", 2, packetReceivedCount);
            assertEqual("Packets sent", 6, packetSentCount);
        }
    }

    //Simulation 1 with interference
    if (testNumber == testCounter++) {
        if (t == 999) {
            //25.5 deg, 24.5 deg, 15.5 deg and car (this packet will be received 3 times) can communicate for 1 seconds (one packet per second)
            assertEqual("Packets received with minElevationAngle of 15deg", 3 + (3*1), packetReceivedCount);
            assertEqual("Packets sent", 6, packetSentCount);
        }
    }
    
    //
    // End
    //
}
