//
// Copyright (C) 2011 David Eckhoff <eckhoff@cs.fau.de>
// Copyright (C) 2012 Bastian Bloessl, Stefan Joerer, Michele Segata <{bloessl,joerer,segata}@ccs-labs.org>
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

/*
 * Based on Decider80211.cc from Karl Wessel
 * and modifications by Christopher Saloman
 * Satellite communication adaptions by Mario Franke
 */

#include "space_veins/modules/phy/DeciderSatellite.h"
#include "space_veins/modules/phy/DeciderSatelliteResult.h"
#include "space_veins/modules/messages/AirFrameSat_m.h"
#include "space_veins/modules/utility/ConstsSatellites.h"

#include "veins/modules/messages/Mac80211Pkt_m.h"
#include "veins/base/toolbox/Signal.h"
#include "veins/modules/phy/NistErrorRate.h"
#include "veins/modules/utility/ConstsPhy.h"

#include "veins/base/toolbox/SignalUtils.h"

using namespace space_veins;

simtime_t DeciderSatellite::processNewSignal(veins::AirFrame* msg)
{
    EV_DEBUG << "DeciderSatellite: processNewSignal: " << msg->getName() << std::endl;
    AirFrameSat* frame = check_and_cast<AirFrameSat*>(msg);

    // get the receiving power of the Signal at start-time and center frequency
    veins::Signal& signal = frame->getSignal();

    signalStates[frame] = EXPECT_END;

    if (signal.smallerAtCenterFrequency(minPowerLevel)) {
        EV_DEBUG << "DeciderSatellite: smallerAtCenterFrequency true, minPowerLevel: " << minPowerLevel << std::endl;
        // annotate the frame, so that we won't try decoding it at its end
        frame->setUnderMinPowerLevel(true);
        // check channel busy status. a superposition of low power frames might turn channel status to busy
        if (cca(simTime(), nullptr) == false) {
            setChannelIdleStatus(false);
        }
        return signal.getReceptionEnd();
    }
    else {

        // This value might be just an intermediate result (due to short circuiting)
        double recvPower = signal.getAtCenterFrequency();
        EV_DEBUG << "DeciderSatellite::processNewSignal recvPower: " << recvPower << " mW" << std::endl;
        setChannelIdleStatus(false);

        if (phySat->getRadioState() == veins::Radio::TX) {
            frame->setBitError(true);
            frame->setWasTransmitting(true);
            EV_TRACE << "AirFrame: " << frame->getId() << " (" << recvPower << ") received, while already sending. Setting BitErrors to true" << std::endl;
        }
        else {

            if (!currentSignal.first) {
                // NIC is not yet synced to any frame, so lock and try to decode this frame
                currentSignal.first = frame;
                EV_TRACE << "AirFrame: " << frame->getId() << " with (" << recvPower << " > " << minPowerLevel << ") -> Trying to receive AirFrame." << std::endl;
                if (notifyRxStart) {
                    phy->sendControlMsgToMac(new cMessage("RxStartStatus", veins::MacToPhyInterface::PHY_RX_START));
                }
            }
            else {
                // NIC is currently trying to decode another frame. this frame will be simply treated as interference
                EV_TRACE << "AirFrame: " << frame->getId() << " with (" << recvPower << " > " << minPowerLevel << ") -> Already synced to another AirFrame. Treating AirFrame as interference." << std::endl;
            }

            // channel turned busy
            // measure communication density
            myBusyTime += signal.getDuration().dbl();
        }
        return signal.getReceptionEnd();
    }
}

int DeciderSatellite::getSignalState(veins::AirFrame* frame)
{

    if (signalStates.find(frame) == signalStates.end()) {
        return NEW;
    }
    else {
        return signalStates[frame];
    }
}

DeciderSatelliteResult* DeciderSatellite::checkIfSignalOk(veins::AirFrame* frame)
{
    auto frameSat = check_and_cast<AirFrameSat*>(frame);

    veins::Signal& s = frame->getSignal();
    simtime_t start = s.getReceptionStart();
    simtime_t end = s.getReceptionEnd();

    /** Currently, there is no preamble for satellite communication */
    //start = start + veins::PHY_HDR_PREAMBLE_DURATION; // its ok if something in the training phase is broken

    AirFrameVector airFrames;
    getChannelInfo(start, end, airFrames);

    double noise = phy->getNoiseFloorValue();

    // Make sure to use the adjusted starting-point (which ignores the preamble)
    double sinrMin = veins::SignalUtils::getMinSINR(start, end, frame, airFrames, noise);
    EV_DEBUG << "DeciderSatellite::checkIfSignalOk: sinrMin: " << sinrMin << std::endl;
    phySat->recordMinSINR(sinrMin);
    double snrMin;
    if (collectCollisionStats) {
        // snrMin = SignalUtils::getMinSNR(start, end, frame, noise);
        snrMin = s.getDataMin() / noise;
    }
    else {
        // just set to any value. if collectCollisionStats != true
        // it will be ignored by packetOk
        snrMin = 1e200;
    }

    double payloadBitrate = getOfdmDatarate(static_cast<veins::MCS>(frameSat->getMcs()), BANDWIDTH_SAT);

    DeciderSatelliteResult* result = nullptr;

    // compute receive power
    double recvPower_dBm = 10 * log10(s.getAtCenterFrequency());
    EV_DEBUG << "DeciderSatellite::checkIfSignalOk recvPower_dBm: " << recvPower_dBm << " dBm" << std::endl;

    // TODO: record SNR and SINR

    switch (packetOk(sinrMin, snrMin, frame->getBitLength(), payloadBitrate)) {

    case DECODED:
        EV_TRACE << "Packet is fine! We can decode it" << std::endl;
        result = new DeciderSatelliteResult(true, payloadBitrate, sinrMin, recvPower_dBm, false);
        break;

    case NOT_DECODED:
        if (!collectCollisionStats) {
            EV_TRACE << "Packet has bit Errors. Lost " << std::endl;
        }
        else {
            EV_TRACE << "Packet has bit Errors due to low power. Lost " << std::endl;
        }
        result = new DeciderSatelliteResult(false, payloadBitrate, sinrMin, recvPower_dBm, false);
        break;

    case COLLISION:
        EV_TRACE << "Packet has bit Errors due to collision. Lost " << std::endl;
        collisions++;
        result = new DeciderSatelliteResult(false, payloadBitrate, sinrMin, recvPower_dBm, true);
        break;

    default:
        ASSERT2(false, "Impossible packet result returned by packetOk(). Check the code.");
        break;
    }

//     DeciderSatelliteResult* result = nullptr;
//     result = new DeciderSatelliteResult(true, 1000000, 2, 20, false);
    return result;
}

enum DeciderSatellite::PACKET_OK_RESULT DeciderSatellite::packetOk(double sinrMin, double snrMin, int lengthMPDU, double bitrate)
{
    double packetOkSinr;
    double packetOkSnr;

    EV_DEBUG << "DeciderSatellite::packetOk lengthMPDU: " << lengthMPDU << std::endl;

    // compute success rate depending on mcs and bw
    //packetOkSinr = veins::NistErrorRate::getChunkSuccessRate(bitrate, BANDWIDTH_11P, sinrMin, PHY_HDR_SERVICE_LENGTH + lengthMPDU + PHY_TAIL_LENGTH);
    packetOkSinr = veins::NistErrorRate::getChunkSuccessRate(bitrate, BANDWIDTH_SAT, sinrMin, lengthMPDU);
    EV_DEBUG << "DeciderSatellite::packetOk packetOkSinr: " << packetOkSinr << std::endl;

    // check if header is broken
    //double headerNoError = veins::NistErrorRate::getChunkSuccessRate(PHY_HDR_BITRATE, BANDWIDTH_11P, sinrMin, PHY_HDR_PLCPSIGNAL_LENGTH);

    //double headerNoErrorSnr;
    // compute PER also for SNR only
    if (collectCollisionStats) {

        //packetOkSnr = NistErrorRate::getChunkSuccessRate(bitrate, BANDWIDTH_11P, snrMin, PHY_HDR_SERVICE_LENGTH + lengthMPDU + PHY_TAIL_LENGTH);
        packetOkSnr = veins::NistErrorRate::getChunkSuccessRate(bitrate, BANDWIDTH_SAT, snrMin, lengthMPDU);
        EV_DEBUG << "DeciderSatellite::packetOk packetOkSnr: " << packetOkSnr << std::endl;
        //headerNoErrorSnr = NistErrorRate::getChunkSuccessRate(PHY_HDR_BITRATE, BANDWIDTH_11P, snrMin, PHY_HDR_PLCPSIGNAL_LENGTH);

        // the probability of correct reception without considering the interference
        // MUST be greater or equal than when consider it
        ASSERT(packetOkSnr >= packetOkSinr);
        //ASSERT(headerNoErrorSnr >= headerNoError);
    }

    // probability of no bit error in the PLCP header

    double rand = RNGCONTEXT dblrand();

    // if (!collectCollisionStats) {
    //     if (rand > headerNoError) return NOT_DECODED;
    // }
    // else {

    //     if (rand > headerNoError) {
    //         // ups, we have a header error. is that due to interference?
    //         if (rand > headerNoErrorSnr) {
    //             // no. we would have not been able to receive that even
    //             // without interference
    //             return NOT_DECODED;
    //         }
    //         else {
    //             // yes. we would have decoded that without interference
    //             return COLLISION;
    //         }
    //     }
    // }

    // probability of no bit error in the rest of the packet

    rand = RNGCONTEXT dblrand();

    if (!collectCollisionStats) {
        if (rand > packetOkSinr) {
            EV_DEBUG << "DeciderSatellite::packetOk - !collectCollisionStats  NOT_DECODED" << std::endl;
            return NOT_DECODED;
        }
        else {
            EV_DEBUG << "DeciderSatellite::packetOk - !collectCollisionStats  DECODED" << std::endl;
            return DECODED;
        }
    }
    else {

        if (rand > packetOkSinr) {
            // ups, we have an error in the payload. is that due to interference?
            if (rand > packetOkSnr) {
                // no. we would have not been able to receive that even
                // without interference
                EV_DEBUG << "DeciderSatellite::packetOk NOT_DECODED even without interference." << std::endl;
                return NOT_DECODED;
            }
            else {
                // yes. we would have decoded that without interference
                EV_DEBUG << "DeciderSatellite::packetOk NOT_DECODED due to packet collision." << std::endl;
                return COLLISION;
            }
        }
        else {
            EV_DEBUG << "DeciderSatellite::packetOk DECODED." << std::endl;
            return DECODED;
        }
    }
    // EV_DEBUG << "DeciderSatellite: packetOk result: DECODED" << std::endl;
    // return DECODED;
}

bool DeciderSatellite::cca(simtime_t_cref time, AirFrame* exclude)
{

    AirFrameVector airFrames;

    // collect all AirFrames that intersect with [start, end]
    getChannelInfo(time, time, airFrames);

    // In the reference implementation only centerFrequenvy - 5e6 (half bandwidth) is checked!
    // Although this is wrong, the same is done here to reproduce original results
    double minPower = phy->getNoiseFloorValue();
    bool isChannelIdle = minPower < ccaThreshold;
    if (airFrames.size() > 0) {
        size_t usedFreqIndex = airFrames.front()->getSignal().getSpectrum().indexOf(centerFrequency - 5e6);
        isChannelIdle = veins::SignalUtils::isChannelPowerBelowThreshold(time, airFrames, usedFreqIndex, ccaThreshold - minPower, exclude);
    }

    return isChannelIdle;
}

simtime_t DeciderSatellite::processSignalEnd(AirFrame* msg)
{

    AirFrameSat* frame = check_and_cast<AirFrameSat*>(msg);

    // // here the Signal is finally processed
    veins::Signal& signal = frame->getSignal();

    double recvPower_dBm = 10 * log10(signal.getMax());
    EV_DEBUG << "DeciderSatellite::processSignalEnd recvPower_dBm: " << recvPower_dBm << std::endl;
    EV_DEBUG << "DeciderSatellite::processSignalEnd recvPower_dBm in mW: " << veins::FWMath::dBm2mW(recvPower_dBm) << " mW"  << std::endl;

    bool whileSending = false;

    // remove this frame from our current signals
    signalStates.erase(frame);

    DeciderSatelliteResult* result;

    if (frame->getUnderMinPowerLevel()) {
        // this frame was not even detected by the radio card
        EV_DEBUG << "DeciderSatellite::processSignalEnd: frame was not even detected by the radio card." << std::endl;
        result = new DeciderSatelliteResult(false, 0, 0, recvPower_dBm);
    }
    else if (frame->getWasTransmitting() || phySat->getRadioState() == veins::Radio::TX) {
        // this frame was received while sending
        EV_DEBUG << "DeciderSatellite::processSignalEnd: frame was received while sending." << std::endl;
        whileSending = true;
        result = new DeciderSatelliteResult(false, 0, 0, recvPower_dBm);
    }
    else {

        // first check whether this is the frame NIC is currently synced on
        if (frame == currentSignal.first) {
            // check if the snr is above the Decider's specific threshold,
            // i.e. the Decider has received it correctly
            result = checkIfSignalOk(frame);

            // after having tried to decode the frame, the NIC is no more synced to the frame
            // and it is ready for syncing on a new one
            currentSignal.first = 0;
        }
        else {
            // if this is not the frame we are synced on, we cannot receive it
            EV_DEBUG << "DeciderSatellite::processSignalEnd: not the frame we are synced on, we cannot receive it." << std::endl;
            result = new DeciderSatelliteResult(false, 0, 0, recvPower_dBm);
        }
    }

    if (result->isSignalCorrect()) {
        EV_TRACE << "packet was received correctly, it is now handed to upper layer...\n";
        // go on with processing this AirFrame, send it to the Mac-Layer
        if (notifyRxStart) {
            phy->sendControlMsgToMac(new cMessage("RxStartStatus", veins::MacToPhyInterface::PHY_RX_END_WITH_SUCCESS));
        }
        phy->sendUp(frame, result);
    }
    else {
        if (frame->getUnderMinPowerLevel()) {
            EV_TRACE << "packet was not detected by the card. power was under minPowerLevel threshold\n";
        }
        else if (whileSending) {
            EV_TRACE << "packet was received while sending, sending it as control message to upper layer\n";
            phy->sendControlMsgToMac(new cMessage("Error", RECWHILESEND));
        }
        else {
            EV_TRACE << "packet was not received correctly, sending it as control message to upper layer\n";
            if (notifyRxStart) {
                phy->sendControlMsgToMac(new cMessage("RxStartStatus", veins::MacToPhyInterface::PHY_RX_END_WITH_FAILURE));
            }

            if (((DeciderSatelliteResult*) result)->isCollision()) {
                phy->sendControlMsgToMac(new cMessage("Error", DeciderSatellite::COLLISION));
            }
            else {
                phy->sendControlMsgToMac(new cMessage("Error", BITERROR));
            }
        }
        delete result;
    }

    if (phySat->getRadioState() == veins::Radio::TX) {
        EV_TRACE << "I'm currently sending\n";
    }
    // check if channel is idle now
    // we declare channel busy if CCA tells us so, or if we are currently
    // decoding a frame
    else if (cca(simTime(), frame) == false || currentSignal.first != 0) {
        EV_TRACE << "Channel not yet idle!\n";
    }
    else {
        // might have been idle before (when the packet rxpower was below sens)
        if (isChannelIdle != true) {
            EV_TRACE << "Channel idle now!\n";
            setChannelIdleStatus(true);
        }
    }
    return notAgain;
}

void DeciderSatellite::setChannelIdleStatus(bool isIdle)
{
    isChannelIdle = isIdle;
    if (isIdle)
        phy->sendControlMsgToMac(new cMessage("ChannelStatus", SatelliteMacToSatellitePhyInterface::CHANNEL_IDLE));
    else
        phy->sendControlMsgToMac(new cMessage("ChannelStatus", SatelliteMacToSatellitePhyInterface::CHANNEL_BUSY));
}

void DeciderSatellite::changeFrequency(double freq)
{
    centerFrequency = freq;
}

double DeciderSatellite::getCCAThreshold()
{
    return 10 * log10(ccaThreshold);
}

void DeciderSatellite::setCCAThreshold(double ccaThreshold_dBm)
{
    ccaThreshold = pow(10, ccaThreshold_dBm / 10);
}

void DeciderSatellite::setNotifyRxStart(bool enable)
{
    notifyRxStart = enable;
}

void DeciderSatellite::switchToTx()
{
    if (currentSignal.first != 0) {
        // we are currently trying to receive a frame.
        if (allowTxDuringRx) {
            // if the above layer decides to transmit anyhow, we need to abort reception
            AirFrameSat* currentFrame = dynamic_cast<AirFrameSat*>(currentSignal.first);
            ASSERT(currentFrame);
            // flag the frame as "while transmitting"
            currentFrame->setWasTransmitting(true);
            currentFrame->setBitError(true);
            // forget about the signal
            currentSignal.first = 0;
        }
        else {
            throw cRuntimeError("DeciderSatellite: mac layer requested phy to transmit a frame while currently receiving another");
        }
    }
}

void DeciderSatellite::finish()
{
}

DeciderSatellite::~DeciderSatellite(){};
