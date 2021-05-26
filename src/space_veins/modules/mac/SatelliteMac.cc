//
// Copyright (C) 2016 David Eckhoff <david.eckhoff@fau.de>
// Copyright (C) 2018 Fabian Bronner <fabian.bronner@ccs-labs.org>
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

#include "space_veins/modules/mac/SatelliteMac.h"
// #include <iterator>

#include "space_veins/modules/phy/DeciderSatelliteResult.h"
// #include "veins/base/phyLayer/PhyToMacControlInfo.h"
// #include "veins/modules/messages/PhyControlMessage_m.h"
// #include "veins/modules/messages/AckTimeOutMessage_m.h"

using namespace space_veins;

using std::unique_ptr;

Define_Module(space_veins::SatelliteMac);

const simsignal_t SatelliteMac::sigChannelBusy = registerSignal("space_veins_modules_mac_sigChannelBusy");
const simsignal_t SatelliteMac::sigCollision = registerSignal("org_car2x_veins_modules_mac_sigCollision");
const simsignal_t SatelliteMac::sigSentPacket = registerSignal("org_car2x_veins_modules_mac_sigSentPacket");
const simsignal_t SatelliteMac::sigSentAck = registerSignal("org_car2x_veins_modules_mac_sigSentAck");
const simsignal_t SatelliteMac::sigRetriesExceeded = registerSignal("org_car2x_veins_modules_mac_sigRetriesExceeded");

void SatelliteMac::initialize(int stage)
{
    BaseMacLayer::initialize(stage);
    if (stage == 0) {

        // read parameter
        macDelay = par("macDelay").doubleValue();

        phySat = veins::FindModule<SatelliteMacToSatellitePhyInterface*>::findSubModule(getParentModule());
        ASSERT(phySat);

        // this is required to circumvent double precision issues with constants from CONST80211p.h
        ASSERT(simTime().getScaleExp() == -12);

        txPower = par("txPower").doubleValue();
        int bitrate = par("bitrate");
        setParametersForBitrate(bitrate);

        myId = getParentModule()->getParentModule()->getFullPath();
        headerLength = par("headerLength");
        idleChannel = true;
        lastBusy = simTime();
        channelIdle(true);
    }
}

void SatelliteMac::handleSelfMsg(cMessage* msg)
{
    EV_DEBUG << "SatelliteMac of vehicle " << myMacAddr << ": received self message." << std::endl;
}

void SatelliteMac::handleUpperControl(cMessage* msg)
{
    ASSERT(false);
}

void SatelliteMac::handleUpperMsg(cMessage* msg)
{
    EV_DEBUG << "SatelliteMac of vehicle " << myMacAddr <<": Received message from SatelliteApplLayer: " << msg->getName() << std::endl;
    BaseSatelliteFrame* thisMsg = check_and_cast<BaseSatelliteFrame*>(msg);
    // send the packet
    veins::MacPkt* mac = new veins::MacPkt(thisMsg->getName(), thisMsg->getKind());
    if (thisMsg->getRecipientAddress() != veins::LAddress::L2BROADCAST()) {
        mac->setDestAddr(thisMsg->getRecipientAddress());
    }
    else {
        mac->setDestAddr(veins::LAddress::L2BROADCAST());
    }
    mac->setSrcAddr(myMacAddr);
    mac->encapsulate(thisMsg->dup());

    veins::MCS usedMcs = mcs;
    double txPower_mW;
    veins::PhyControlMessage* controlInfo = dynamic_cast<veins::PhyControlMessage*>(thisMsg->getControlInfo());
    if (controlInfo) {
        // if MCS is not specified, just use the default one
        EV_DEBUG << "SatelliteMac::handleUpperMsg: MCS is not specified, use default MCS." << std::endl;
        veins::MCS explicitMcs = static_cast<veins::MCS>(controlInfo->getMcs());
        if (explicitMcs != veins::MCS::undefined) {
            usedMcs = explicitMcs;
        }
        // apply the same principle to tx power
        txPower_mW = controlInfo->getTxPower_mW();
        if (txPower_mW <= 0) {
            txPower_mW = txPower;
        }
    }
    else {
        txPower_mW = txPower;
    }

    SatelliteChannel channelNr = SatelliteChannel::dvbs1;
    double freq = SatelliteChannelFrequencies.at(channelNr);

    sendFrame(mac, macDelay, channelNr, usedMcs, txPower_mW);
    EV_DEBUG << "SatelliteMac of vehicle " << myMacAddr << ": is sending a Satellite Packet. Frequency: " << freq << ", MCS: " << (int)usedMcs << ", txPower_mW: " << txPower_mW << std::endl;
}

void SatelliteMac::handleLowerControl(cMessage* msg)
{
    if (msg->getKind() == veins::MacToPhyInterface::PHY_RX_START) {
        EV_DEBUG << "SatelliteMac handleLowerControl: veins::MacToPhyInterface::PHY_RX_START" << std::endl;
    }
    else if (msg->getKind() == veins::MacToPhyInterface::PHY_RX_END_WITH_SUCCESS) {
        // PHY_RX_END_WITH_SUCCESS will get packet soon! Nothing to do here
        EV_DEBUG << "SatelliteMac handleLowerControl: PHY_RX_END_WITH_SUCCESS will get packet soon! Nothing to do here." << std::endl;
    }
    else if (msg->getKind() == veins::MacToPhyInterface::PHY_RX_END_WITH_FAILURE) {
        // RX failed at phy. Time to retransmit
        EV_DEBUG << "SatelliteMac handleLowerControl: RX failed at phy." << std::endl;
        // phy11p->notifyMacAboutRxStart(false);
        // rxStartIndication = false;
        // handleRetransmit(lastAC);
    }
    else if (msg->getKind() == veins::MacToPhyInterface::TX_OVER) {

        EV_TRACE << "SatelliteMac handleLowerControl: Successfully transmitted packet." << std::endl;
        EV_DEBUG << "SatelliteMac handleLowerControl: Successfully transmitted packet." << std::endl;

        phy->setRadioState(veins::Radio::RX);
    }
    else if (msg->getKind() == SatelliteMacToSatellitePhyInterface::CHANNEL_BUSY) {
        EV_DEBUG << "SatelliteMac handleLowerControl: SatelliteMacToSatellitePhyInterface::CHANNEL_BUSY." << std::endl;
        channelBusy();
    }
    else if (msg->getKind() == SatelliteMacToSatellitePhyInterface::CHANNEL_IDLE) {
        EV_DEBUG << "SatelliteMac handleLowerControl: SatelliteMacToSatellitePhyInterface::CHANNEL_IDLE." << std::endl;
        // Decider80211p::processSignalEnd() sends up the received packet to MAC followed by control message CHANNEL_IDLE in the same timestamp.
        channelIdle();
    }
    else if (msg->getKind() == DeciderSatellite::BITERROR || msg->getKind() == DeciderSatellite::COLLISION) {
        EV_TRACE << "A packet was not received due to biterrors" << std::endl;
        EV_DEBUG << "SatelliteMac handleLowerControl: A packet was not received due to biterrors" << std::endl;
    }
    else if (msg->getKind() == DeciderSatellite::RECWHILESEND) {
        EV_TRACE << "A packet was not received because we were sending while receiving" << std::endl;
        EV_DEBUG << "SatelliteMac handleLowerContro: A packet was not received because we were sending while receiving" << std::endl;
    }
    else if (msg->getKind() == veins::MacToPhyInterface::RADIO_SWITCHING_OVER) {
        EV_TRACE << "Phylayer said radio switching is done" << std::endl;
        EV_DEBUG << "SatelliteMac handleLowerContro: Phylayer said radio switching is done" << std::endl;
    }
    else if (msg->getKind() == veins::BaseDecider::PACKET_DROPPED) {
        phy->setRadioState(veins::Radio::RX);
        EV_TRACE << "Phylayer said packet was dropped" << std::endl;
        EV_DEBUG << "SatelliteMac handleLowerControl: Phylayer said packet was dropped" << std::endl;
    }
    else {
        EV_WARN << "Invalid control message type (type=NOTHING) : name=" << msg->getName() << " modulesrc=" << msg->getSenderModule()->getFullPath() << "." << std::endl;
        EV_DEBUG << "Invalid control message type (type=NOTHING) : name=" << msg->getName() << " modulesrc=" << msg->getSenderModule()->getFullPath() << "." << std::endl;
        ASSERT(false);
    }

    EV_DEBUG << "SatelliteMac of vehicle " << myMacAddr << ": Received message from SatellitePhy control gate." << std::endl;
    delete msg;
}

void SatelliteMac::finish()
{
    EV_DEBUG << "SatelliteMac of vehicle " << myMacAddr << " called finish method." << std::endl;
}

SatelliteMac::~SatelliteMac()
{
};

void SatelliteMac::sendFrame(veins::MacPkt* frame, simtime_t delay, SatelliteChannel channelNr, veins::MCS mcs, double txPower_mW)
{
    EV_DEBUG << "SatelliteMac of vehicle " << myMacAddr << ": send msg down to SatellitePhy: " << frame->getName() << std::endl;
    phy->setRadioState(veins::Radio::TX); // give time for the radio to be in Tx state before transmitting

    delay = std::max(delay, RADIODELAY_SAT); // wait at least for the radio to switch

    attachControlInfo(frame, channelNr, mcs, txPower_mW);
    check_and_cast<MacToPhyControlInfoSatellite*>(frame->getControlInfo());

    sendDelayed(frame, delay, lowerLayerOut);
}

void SatelliteMac::attachControlInfo(veins::MacPkt* mac, SatelliteChannel channelNr, veins::MCS mcs, double txPower_mW)
{
    auto cinfo = new MacToPhyControlInfoSatellite(channelNr, mcs, txPower_mW);
    mac->setControlInfo(cinfo);
}

void SatelliteMac::setTxPower(double txPower_mW)
{
    txPower = txPower_mW;
}

void SatelliteMac::setMCS(veins::MCS mcs)
{
    ASSERT2(mcs != veins::MCS::undefined, "invalid MCS selected");
    this->mcs = mcs;
}

// void SatelliteMac::setCCAThreshold(double ccaThreshold_dBm)
// {
//     phy11p->setCCAThreshold(ccaThreshold_dBm);
// }

void SatelliteMac::handleLowerMsg(cMessage* msg)
{
    veins::MacPkt* macPkt = check_and_cast<veins::MacPkt*>(msg);
    EV_DEBUG << "SatelliteMac: received message from SatellitePhy." << std::endl;
    unique_ptr<BaseSatelliteFrame> bsf(check_and_cast<BaseSatelliteFrame*>(macPkt->decapsulate()));
    sendUp(bsf.release());
    EV_DEBUG << "SatelliteMac: Received message from lower gate (SatellitePhy)." << std::endl;
}

void SatelliteMac::channelBusy()
{

    if (!idleChannel) return;

    // the channel turned busy because someone else is sending
    idleChannel = false;
    EV_DEBUG << "SatelliteMac: Channel turned busy: External sender" << std::endl;
    lastBusy = simTime();
    //TODO: who needs to receive this signal!
    emit(sigChannelBusy, true);
}

void SatelliteMac::channelIdle(bool afterSwitch)
{
    idleChannel = afterSwitch;
}

void SatelliteMac::setParametersForBitrate(uint64_t bitrate)
{
    mcs = getMCS(bitrate, BANDWIDTH_SAT);
    if (mcs == veins::MCS::undefined) {
        throw cRuntimeError("Chosen Bitrate is not valid for satellite communication: Valid rates are: 3Mbps, 4.5Mbps, 6Mbps, 9Mbps, 12Mbps, 18Mbps, 24Mbps and 27Mbps. Please adjust your omnetpp.ini file accordingly.");
    }
}
