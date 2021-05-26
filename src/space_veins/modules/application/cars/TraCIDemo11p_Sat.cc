//
// Copyright (C) 2006-2011 Christoph Sommer <christoph.sommer@uibk.ac.at>
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

#include "space_veins/modules/application/cars/TraCIDemo11p_Sat.h"

#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"

using namespace space_veins;

Define_Module(space_veins::TraCIDemo11p_Sat);

void TraCIDemo11p_Sat::initialize(int stage)
{
    TraCIDemo11p::initialize(stage);
    if (stage == 0) {
        // read parameters
        satPingLengthBits = par("satPingLengthBits").intValue();
        satPongLengthBits = par("satPongLengthBits").intValue();

        // statistics
        ping_s_vec.setName("ping_s");

        // find gate
        satLowerLayerIn = findGate("satLowerLayerIn");
        satLowerLayerOut = findGate("satLowerLayerOut");
        satLowerControlIn = findGate("satLowerControlIn");
        satLowerControlOut = findGate("satLowerControlOut");
        // EV_DEBUG << "TraCIDemo11p_Sat: Initialized TraCIDemo11p_Sat appl layer modules." << std::endl;

        // schedule ping messages every second
        auto t_spec = veins::TimerSpecification([this]() {sendPingViaSatellite();});
        t_spec.absoluteStart(uniform(SimTime(0, SIMTIME_S), SimTime(1, SIMTIME_S)));
        t_spec.interval(SimTime(1, SIMTIME_S));
        timerManager.create(t_spec, "ping");

    }
}

void TraCIDemo11p_Sat::sendPingViaSatellite()
{
    BaseSatelliteFrame msg = BaseSatelliteFrame("ping", -1);
    msg.setData("ping");
    msg.setBitLength(satPingLengthBits);
    sendViaSatellite((&msg)->dup());
    sentPing = simTime();
    EV_DEBUG << "Transmitted ping." << std::endl;
}

void TraCIDemo11p_Sat::sendPongViaSatellite()
{
    BaseSatelliteFrame msg = BaseSatelliteFrame("pong", -1);
    msg.setData("pong");
    msg.setBitLength(satPongLengthBits);
    sendViaSatellite((&msg)->dup());
    EV_DEBUG << "Transmitted pong." << std::endl;
}

void TraCIDemo11p_Sat::sendViaC2X(cMessage* msg)
{
    send(msg, lowerLayerOut);
}

void TraCIDemo11p_Sat::sendViaSatellite(cMessage* msg)
{
    send(msg, satLowerLayerOut);
}

void TraCIDemo11p_Sat::sendDelayedViaC2X(cMessage* msg, simtime_t delay)
{
    sendDelayed(msg, delay, lowerLayerOut);
}

void TraCIDemo11p_Sat::sendDelayedViaSatellite(cMessage* msg, simtime_t delay)
{
    sendDelayed(msg, delay, satLowerLayerOut);
}

void TraCIDemo11p_Sat::handleLowerSatMessage(cMessage* msg)
{
    if (BaseSatelliteFrame* bsf = dynamic_cast<BaseSatelliteFrame*>(msg)) {
        ASSERT(bsf);
        ping_s_vec.record(simTime() - sentPing);
        EV_DEBUG << "TraCIDemo11p_Sat: Received BaseSatelliteFrame: " << bsf->getData() << std::endl;
    }
}

void TraCIDemo11p_Sat::handleLowerSatControlMessage(cMessage* msg)
{
    EV_DEBUG << "Received lowerSatControlMessage: " << msg->getFullName() << std::endl;
}

void TraCIDemo11p_Sat::handleMessage(cMessage* msg)
{
    if (msg->getArrivalGateId() == satLowerLayerIn) {
        handleLowerSatMessage(msg);
    }else if(msg->getArrivalGateId() == satLowerControlIn) {
        handleLowerSatControlMessage(msg);
    }else{
        veins::BaseLayer::handleMessage(msg);
    }
}

void TraCIDemo11p_Sat::handleSelfMsg(cMessage* msg)
{
    if (timerManager.handleMessage(msg)) {
        return;
    }else if (veins::TraCIDemo11pMessage* wsm = dynamic_cast<veins::TraCIDemo11pMessage*>(msg)) {
        // send this message on the service channel until the counter is 3 or higher.
        // this code only runs when channel switching is enabled
        sendViaC2X(wsm->dup());
        wsm->setSerial(wsm->getSerial() + 1);
        if (wsm->getSerial() >= 3) {
            // stop service advertisements
            stopService();
            delete (wsm);
        }
        else {
            scheduleAt(simTime() + 1, wsm);
        }
    }
    else {
        DemoBaseApplLayer::handleSelfMsg(msg);
    }
}

void TraCIDemo11p_Sat::handlePositionUpdate(cObject* obj)
{
        DemoBaseApplLayer::handlePositionUpdate(obj);

        // // stopped for for at least 10s?
        // if (mobility->getSpeed() < 1) {
        //     if (simTime() - lastDroveAt >= 10 && sentMessage == false) {
        //         findHost()->getDisplayString().setTagArg("i", 1, "red");
        //         sentMessage = true;

        //         veins::TraCIDemo11pMessage* wsm = new veins::TraCIDemo11pMessage();
        //         populateWSM(wsm);
        //         wsm->setDemoData(mobility->getRoadId().c_str());

        //         // host is standing still due to crash
        //         if (dataOnSch) {
        //             startService(veins::Channel::sch2, 42, "Traffic Information Service");
        //             // started service and server advertising, schedule message to self to send later
        //             scheduleAt(computeAsynchronousSendingTime(1, veins::ChannelType::service), wsm);
        //         }
        //         else {
        //             //send right away on CCH, because channel switching is disabled
        //             sendDown(wsm);
        //         }
        //     }
        // }
        // else {
        //     lastDroveAt = simTime();
        // }
}
