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

/*
 * Based on PhyLayer.cc from Karl Wessel
 * and modifications by Christopher Saloman
 */

#include "space_veins/modules/phy/SatellitePhy.h"

// #include "veins/modules/phy/Decider80211p.h"
#include "veins/modules/analogueModel/SimplePathlossModel.h"
// #include "veins/modules/analogueModel/BreakpointPathlossModel.h"
// #include "veins/modules/analogueModel/PERModel.h"
// #include "veins/modules/analogueModel/SimpleObstacleShadowing.h"
// #include "veins/modules/analogueModel/VehicleObstacleShadowing.h"
// #include "veins/modules/analogueModel/TwoRayInterferenceModel.h"
// #include "veins/modules/analogueModel/NakagamiFading.h"
#include "veins/base/connectionManager/BaseConnectionManager.h"
// #include "veins/modules/utility/Consts80211p.h"
// #include "veins/modules/messages/AirFrame11p_m.h"
// #include "veins/modules/utility/MacToPhyControlInfo11p.h"

using namespace space_veins;

using std::unique_ptr;

Define_Module(space_veins::SatellitePhy);

void SatellitePhy::initialize(int stage)
{
    if (stage == 0) {
        // get ccaThreshold before calling BasePhyLayer::initialize() which instantiates the deciders
        ccaThreshold = pow(10, par("ccaThreshold").doubleValue() / 10);
        allowTxDuringRx = par("allowTxDuringRx").boolValue();
        collectCollisionStatistics = par("collectCollisionStatistics").boolValue();

        minSINR_vec.setName("minSINR_dB");

        // Create frequency mappings and initialize spectrum for signal representation
        veins::Spectrum::Frequencies freqs;
        for (auto& channel : SatelliteChannelFrequencies) {
            freqs.push_back(channel.second - 5e6);
            freqs.push_back(channel.second);
            freqs.push_back(channel.second + 5e6);
        }
        overallSpectrum = veins::Spectrum(freqs);

    }
    BaseSatellitePhyLayer::initialize(stage);
}

unique_ptr<veins::AnalogueModel> SatellitePhy::getAnalogueModelFromName(std::string name, ParameterMap& params)
{

    if (name == "SimplePathlossModel") {
        return initializeSimplePathlossModel(params);
    }
    return BaseSatellitePhyLayer::getAnalogueModelFromName(name, params);
}


unique_ptr<veins::AnalogueModel> SatellitePhy::initializeSimplePathlossModel(ParameterMap& params)
{

    // init with default value
    double alpha = 2.0;
    bool useTorus = world->useTorus();
    const veins::Coord& playgroundSize = *(world->getPgs());

    // get alpha-coefficient from config
    ParameterMap::iterator it = params.find("alpha");

    if (it != params.end()) { // parameter alpha has been specified in config.xml
        // set alpha
        alpha = it->second.doubleValue();
        EV_TRACE << "createPathLossModel(): alpha set from config.xml to " << alpha << endl;

        // check whether alpha is not smaller than specified in ConnectionManager
        if (scc->hasPar("alpha") && alpha < scc->par("alpha").doubleValue()) {
            // throw error
            throw cRuntimeError("TestPhyLayer::createPathLossModel(): alpha can't be smaller than specified in \
                   ConnectionManager. Please adjust your config.xml file accordingly");
        }
    }
    else // alpha has not been specified in config.xml
    {
        if (scc->hasPar("alpha")) { // parameter alpha has been specified in ConnectionManager
            // set alpha according to ConnectionManager
            alpha = cc->par("alpha").doubleValue();
            EV_TRACE << "createPathLossModel(): alpha set from ConnectionManager to " << alpha << endl;
        }
        else // alpha has not been specified in ConnectionManager
        {
            // keep alpha at default value
            EV_TRACE << "createPathLossModel(): alpha set from default value to " << alpha << endl;
        }
    }

    return space_veins::make_unique<veins::SimplePathlossModel>(this, alpha, useTorus, playgroundSize);
}

unique_ptr<veins::Decider> SatellitePhy::getDeciderFromName(std::string name, ParameterMap& params)
{
    if (name == "DeciderSatellite") {
        protocolId = SATELLITE;
        return initializeDeciderSatellite(params);
    }
    return BaseSatellitePhyLayer::getDeciderFromName(name, params);
}

unique_ptr<veins::Decider> SatellitePhy::initializeDeciderSatellite(ParameterMap& params)
{
    double centerFreq = params["centerFrequency"];
    auto dec = space_veins::make_unique<DeciderSatellite>(this, this, minPowerLevel, ccaThreshold, allowTxDuringRx, centerFreq, findHost()->getIndex(), collectCollisionStatistics);
    dec->setPath(getParentModule()->getFullPath());
    return unique_ptr<veins::Decider>(std::move(dec));
}

void SatellitePhy::handleSelfMessage(cMessage* msg)
{
    EV_DEBUG << "SatellitePhy received self message." << std::endl;

    switch (msg->getKind()) {
    // radio transmission over
    case TX_OVER: {
        ASSERT(msg == txOverTimer);
        EV_DEBUG << "SatellitePhy received TX_OVER message." << std::endl;
        sendControlMsgToMac(new cMessage("Transmission over", TX_OVER));
       // check if there is another packet on the chan, and change the chan-state to idle
       DeciderSatellite* dec = dynamic_cast<DeciderSatellite*>(decider.get());
       ASSERT(dec);
       if (dec->cca(simTime(), nullptr)) {
           // chan is idle
           EV_TRACE << "Channel idle after transmit!\n";
           dec->setChannelIdleStatus(true);
       }
       else {
           EV_TRACE << "Channel not yet idle after transmit!\n";
       }
        break;
    }

    // radio switch over
    case RADIO_SWITCHING_OVER:
        EV_DEBUG << "SatellitePhy received RADIO_SWITCHING_OVER message." << std::endl;
        ASSERT(msg == radioSwitchingOverTimer);
        BaseSatellitePhyLayer::finishRadioSwitching();
        break;

    // AirFrame
    case AIR_FRAME:
        EV_DEBUG << "SatellitePhy handleSelfMessage received AIR_FRAME message: " << msg->getName() << std::endl;
        BaseSatellitePhyLayer::handleAirFrame(static_cast<AirFrame*>(msg));
        break;

    default:
        EV_DEBUG << "SatellitePhy received self message of unknown kind." << std::endl;
        break;
    }
}

void SatellitePhy::handleMessage(cMessage* msg)
{
    // self messages
    if (msg->isSelfMessage()) {
        handleSelfMessage(msg);

        // MacPkts <- MacToPhyControlInfo
    }
    else if (msg->getArrivalGateId() == upperLayerIn) {
        EV_DEBUG << "SatellitePhy: received msg: " << msg->getName() << " from upperLayerIn (MAC)" << std::endl;
        handleUpperMessage(msg);

        // controlmessages
    }
    else if (msg->getArrivalGateId() == upperControlIn) {
        handleUpperControlMessage(msg);

        // AirFrames
    }
    else if (msg->getKind() == AIR_FRAME) {
        EV_DEBUG << "SatellitePhy: received AirFrame" << std::endl;
        handleAirFrame(static_cast<AirFrame*>(msg));

        // unknown message
    }
    else {
        EV << "Unknown message received." << endl;
        delete msg;
    }
}

void SatellitePhy::handleUpperMessage(cMessage* msg)
{
    EV_DEBUG << "SatellitePhy handleUpperMessage of msg: " << msg->getName() << std::endl;
    // check if Radio is in TX state
    if (radio->getCurrentState() != veins::Radio::TX) {
        delete msg;
        msg = nullptr;
        throw cRuntimeError("Error: message for sending received, but radio not in state TX");
    }

    // check if not already sending
    if (txOverTimer->isScheduled()) {
        delete msg;
        msg = nullptr;
        throw cRuntimeError("Error: message for sending received, but radio already sending");
    }

    // build the AirFrame to send
    ASSERT(dynamic_cast<cPacket*>(msg) != nullptr);

    unique_ptr<AirFrame> frame = encapsMsg(static_cast<cPacket*>(msg));

    // Prepare a POA object and attach it to the created Airframe
    veins::AntennaPosition pos = antennaPosition;
    veins::Coord orient = antennaHeading.toCoord();
    frame->setPoa({pos, orient, antenna});

    // make sure there is no self message of kind TX_OVER scheduled
    // and schedule the actual one
    ASSERT(!txOverTimer->isScheduled());
    EV_DEBUG << "SatellitePhy schedule txOverTimer" << std::endl;
    sendSelfMessage(txOverTimer, simTime() + frame->getDuration());

    sendMessageDown(frame.release());
}

unique_ptr<AirFrame> SatellitePhy::createAirFrame(cPacket* macPkt)
{
    return space_veins::make_unique<AirFrameSat>(macPkt->getName(), AIR_FRAME);
}

void SatellitePhy::attachSignal(veins::AirFrame* airFrame, cObject* ctrlInfo)
{
    const auto ctrlInfoSat = check_and_cast<MacToPhyControlInfoSatellite*>(ctrlInfo);

    const auto duration = getFrameDuration(airFrame->getEncapsulatedPacket()->getBitLength(), ctrlInfoSat->mcs);
    EV_DEBUG << "getBitLength: " << airFrame->getEncapsulatedPacket()->getBitLength() << std::endl;
    EV_DEBUG << "getFrameDuration: " << duration << std::endl;
    ASSERT(duration > 0);
    veins::Signal signal(overallSpectrum, simTime(), duration);
    auto freqIndex = overallSpectrum.indexOf(SatelliteChannelFrequencies.at(ctrlInfoSat->channelNr));
    signal.at(freqIndex - 1) = ctrlInfoSat->txPower_mW;
    signal.at(freqIndex) = ctrlInfoSat->txPower_mW;
    signal.at(freqIndex + 1) = ctrlInfoSat->txPower_mW;
    signal.setDataStart(freqIndex - 1);
    signal.setDataEnd(freqIndex + 1);
    signal.setCenterFrequencyIndex(freqIndex);
    // copy the signal into the AirFrame
    airFrame->setSignal(signal);
    airFrame->setDuration(signal.getDuration());
    airFrame->setMcs(static_cast<int>(ctrlInfoSat->mcs));
}

int SatellitePhy::getRadioState()
{
    return BaseSatellitePhyLayer::getRadioState();
}

void SatellitePhy::recordMinSINR(double minSINR)
{
    minSINR_vec.record(minSINR);
}

simtime_t SatellitePhy::setRadioState(int rs)
{
    if (rs == veins::Radio::TX) decider->switchToTx();
    return BaseSatellitePhyLayer::setRadioState(rs);
}

void SatellitePhy::setCCAThreshold(double ccaThreshold_dBm)
{
    ccaThreshold = pow(10, ccaThreshold_dBm / 10);
    DeciderSatellite* dec = dynamic_cast<DeciderSatellite*>(decider.get());
    ASSERT(dec);
    dec->setCCAThreshold(ccaThreshold_dBm);
}

double SatellitePhy::getCCAThreshold()
{
    return 10 * log10(ccaThreshold);
}

void SatellitePhy::notifyMacAboutRxStart(bool enable)
{
    DeciderSatellite* dec = dynamic_cast<DeciderSatellite*>(decider.get());
    ASSERT(dec);
    dec->setNotifyRxStart(enable);
}

void SatellitePhy::requestChannelStatusIfIdle()
{
    Enter_Method_Silent();
    DeciderSatellite* dec = dynamic_cast<DeciderSatellite*>(decider.get());
    ASSERT(dec);
    if (dec->cca(simTime(), nullptr)) {
        // chan is idle
        EV_TRACE << "Request channel status: channel idle!\n";
        dec->setChannelIdleStatus(true);
    }
}

simtime_t SatellitePhy::getFrameDuration(int payloadLengthBits, veins::MCS mcs) const
{
    Enter_Method_Silent();
    ASSERT(mcs != veins::MCS::undefined);
    auto ndbps = veins::getNDBPS(mcs);
    // calculate frame duration according to Equation (17-29) of the IEEE 802.11-2007 standard
    return T_SYM_SATELLITE * ceil(static_cast<double>(16 + payloadLengthBits + 6) / (ndbps));
}
