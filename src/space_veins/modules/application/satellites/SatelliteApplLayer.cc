//
// Copyright (C) 2011 David Eckhoff <eckhoff@cs.fau.de>
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

#include "space_veins/modules/application/satellites/SatelliteApplLayer.h"

using namespace space_veins;

Define_Module(space_veins::SatelliteApplLayer);

void SatelliteApplLayer::initialize(int stage)
{
    veins::BaseApplLayer::initialize(stage);

    if (stage == 0) {

        // initialize pointers to other modules
        mac = veins::FindModule<SatelliteApplLayerToSatelliteMacInterface*>::findSubModule(getParentModule());
        ASSERT(mac);

        // read parameters
        headerLength = par("headerLength");
        dataLengthBits = par("dataLengthBits");
        satPingLengthBits = par("satPingLengthBits").intValue();
        satPongLengthBits = par("satPongLengthBits").intValue();
    }
    else if (stage == 1) {
        // print satellite module name
        EV_DEBUG << "SatelliteApplLayer: satellite module name: " << this->getParentModule()->getFullName() << std::endl;

        // store MAC address for quick access
        myId = mac->getMACAddress();
        EV_DEBUG << "SatelliteApplLayer: Satellite initiliazed, MAC: " << myId << std::endl;
    }
}

void SatelliteApplLayer::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details)
{
    Enter_Method_Silent();
    EV_DEBUG << "SatelliteApplLayer: Received signal." << std::endl;
}

void SatelliteApplLayer::handleLowerMsg(cMessage* msg)
{
    EV_DEBUG << "SatelliteApplLayer: Received message from mac layer." << std::endl;
    if (BaseSatelliteFrame* bsf = dynamic_cast<BaseSatelliteFrame*>(msg)) {
        ASSERT(bsf);
        EV_DEBUG << "SatelliteApplLayer: Received BaseSatelliteFrame: " << bsf->getData() << std::endl;
        sendPong();
    }
    delete (msg);
}

void SatelliteApplLayer::handleSelfMsg(cMessage* msg)
{
    switch (msg->getKind()) {
    default: {
        EV_DEBUG << "SatelliteApplLayer: Received self message." << std::endl;
        delete msg;
        break;
    }
    }
}

void SatelliteApplLayer::finish()
{
    EV_DEBUG << "SatelliteApplLayer: Called finish." << std::endl;
}

SatelliteApplLayer::~SatelliteApplLayer()
{
}

void SatelliteApplLayer::sendPing()
{
    BaseSatelliteFrame msg = BaseSatelliteFrame("ping", -1);
    msg.setData("ping");
    msg.setBitLength(satPingLengthBits);
    sendDown((&msg)->dup());
    EV_DEBUG << "Transmitted ping." << std::endl;
}

void SatelliteApplLayer::sendPong()
{
    BaseSatelliteFrame msg = BaseSatelliteFrame("pong", -1);
    msg.setData("pong");
    msg.setBitLength(satPongLengthBits);
    sendDown((&msg)->dup());
    EV_DEBUG << "Transmitted pong." << std::endl;
}

void SatelliteApplLayer::sendDown(cMessage* msg)
{
    BaseApplLayer::sendDown(msg);
}

void SatelliteApplLayer::sendDelayedDown(cMessage* msg, simtime_t delay)
{
    BaseApplLayer::sendDelayedDown(msg, delay);
}
