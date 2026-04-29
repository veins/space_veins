//
// Copyright (C) 2022 Mario Franke <research@m-franke.net>
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

#include "space_veins_testsims/applications/UdpBasicAppTesting.h"
#include "space_veins_testsims/utils/asserts.h"
#include "inet/networklayer/common/L3AddressResolver.h"

Define_Module(UdpBasicAppTesting);

void UdpBasicAppTesting::initialize(int stage)
{
    UdpBasicApp::initialize(stage);
    if (stage == 0) {
        expectedReceivedPackets = par("expectedReceivedPackets");
        expectedSentPackets = par("expectedSentPackets");
        WATCH(expectedReceivedPackets);
        WATCH(expectedSentPackets);
    }
}

void UdpBasicAppTesting::finish()
{
    UdpBasicApp::finish();
    assertEqual("Packets received:", expectedReceivedPackets, numReceived);
    assertEqual("Packets sent:", expectedSentPackets, numSent);
}

void UdpBasicAppTesting::processStart() {
    socket.setOutputGate(gate("socketOut"));
    const char *localAddress = par("localAddress");
    socket.bind(*localAddress ? inet::L3AddressResolver().resolve(localAddress) : inet::L3Address(), localPort);
    setSocketOptions();

    const char *destAddrs = par("destAddresses");
    cStringTokenizer tokenizer(destAddrs);
    const char *token;

    while ((token = tokenizer.nextToken()) != nullptr) {
        destAddressStr.push_back(token);
        inet::L3Address result;
        inet::L3AddressResolver().tryResolve(token, result);
        if (result.isUnspecified())
            EV_ERROR << "cannot resolve destination address: " << token << endl;
        destAddresses.push_back(result);
    }

    if (!destAddresses.empty()) {
        // begin changes of overwritten method
        simtime_t txFirstPacket = simTime() + par("sendInterval");
        if (stopTime < SIMTIME_ZERO || txFirstPacket < stopTime) {
            selfMsg->setKind(SEND);
            scheduleAt(txFirstPacket, selfMsg);
        }
        else {
            selfMsg->setKind(STOP);
            scheduleAt(stopTime, selfMsg);
        }
        // end changes of overwritten method
    }
    else {
        if (stopTime >= SIMTIME_ZERO) {
            selfMsg->setKind(STOP);
            scheduleAt(stopTime, selfMsg);
        }
    }
}

UdpBasicAppTesting::UdpBasicAppTesting() {}

UdpBasicAppTesting::~UdpBasicAppTesting() {}
