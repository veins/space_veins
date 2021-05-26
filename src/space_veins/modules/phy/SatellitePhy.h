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

#pragma once

#include "space_veins/base/phyLayer/BaseSatellitePhyLayer.h"
// #include "veins/base/toolbox/Spectrum.h"
// #include "veins/modules/mac/ieee80211p/Mac80211pToPhy11pInterface.h"
// #include "veins/modules/phy/Decider80211p.h"
// #include "veins/modules/analogueModel/SimplePathlossModel.h"
// #include "veins/base/connectionManager/BaseConnectionManager.h"
// #include "veins/modules/phy/Decider80211pToPhy80211pInterface.h"
// #include "veins/base/utils/Move.h"

#include "space_veins/space_veins.h"
#include "space_veins/base/satellitesConnectionManager/SatelliteChannelAccess.h"
#include "space_veins/modules/mac/SatelliteMacToSatellitePhyInterface.h"
#include "space_veins/modules/phy/DeciderSatelliteToSatellitePhyInterface.h"
#include "space_veins/modules/phy/DeciderSatellite.h"
#include "space_veins/modules/utility/ConstsSatellites.h"
#include "space_veins/modules/utility/MacToPhyControlInfoSatellite.h"
#include "space_veins/modules/messages/AirFrameSat_m.h"

namespace space_veins {

/**
 * @brief
 * Adaptation of the PhyLayer class for satellites.
 *
 * @ingroup phyLayer
 *
 * @see DemoBaseApplLayer
 * @see Mac1609_4
 * @see PhyLayer80211p
 * @see Decider80211p
 */
class SPACE_VEINS_API SatellitePhy : public BaseSatellitePhyLayer, public SatelliteMacToSatellitePhyInterface, public DeciderSatelliteToSatellitePhyInterface {
public:
    void initialize(int stage) override;
    /**
     * @brief Set the carrier sense threshold
     * @param ccaThreshold_dBm the cca threshold in dBm
     */
    void setCCAThreshold(double ccaThreshold_dBm) override;
    /**
     * @brief Return the cca threshold in dBm
     */
    double getCCAThreshold();
    /**
     * @brief Enable notifications about PHY-RXSTART indication in MAC
     * @param enable true if Mac needs to be notified about it
     */
    void notifyMacAboutRxStart(bool enable) override;
    /**
     * @brief Explicit request to PHY for the channel status
     */
    void requestChannelStatusIfIdle() override;

protected:
    /** @brief CCA threshold. See Decider80211p for details */
    double ccaThreshold;

    /** @brief enable/disable detection of packet collisions */
    bool collectCollisionStatistics;

    /** @brief Vector that records all SINR values of all synchronized frames */
    cOutVector minSINR_vec;

    /** @brief allows/disallows interruption of current reception for txing
     *
     * See detailed description in Decider80211p
     */
    bool allowTxDuringRx;

    enum ProtocolIds {
        SATELLITE = 451506
    };
   /**
    * @brief Creates and returns an instance of the AnalogueModel with the
    * specified name.
    *
    * Is able to initialize the following AnalogueModels:
    */
   virtual std::unique_ptr<veins::AnalogueModel> getAnalogueModelFromName(std::string name, ParameterMap& params) override;

   /**
    * @brief Creates and initializes a SimplePathlossModel with the
    * passed parameter values.
    */
   std::unique_ptr<veins::AnalogueModel> initializeSimplePathlossModel(ParameterMap& params);

    /**
     * @brief Creates and returns an instance of the Decider with the specified
     * name.
     *
     * Is able to initialize the following Deciders:
     *
     * - Decider80211p
     */
    virtual std::unique_ptr<veins::Decider> getDeciderFromName(std::string name, ParameterMap& params) override;

    /**
     * @brief Initializes a new Decider80211 from the passed parameter map.
     */
    virtual std::unique_ptr<veins::Decider> initializeDeciderSatellite(ParameterMap& params);

    /**
     * Create a protocol-specific AirFrame
     * Overloaded to create a specialize AirFrameSat.
     */
    std::unique_ptr<veins::AirFrame> createAirFrame(cPacket* macPkt) override;

    /**
     * Attach a signal to the given AirFrame.
     *
     * The attached Signal corresponds to satellite signal.
     * Parameters for the signal are passed in the control info.
     * The indicated power levels are set up on the specified center frequency, as well as the neighboring 5MHz.
     *
     * @note The control info must be of type MacToPhyControlInfoSatellite
     */
    void attachSignal(veins::AirFrame* airFrame, cObject* ctrlInfo) override;

    virtual simtime_t getFrameDuration(int payloadLengthBits, veins::MCS mcs) const override;

    virtual void handleMessage(cMessage* msg) override;

    virtual void handleUpperMessage(cMessage* msg) override;

    void handleSelfMessage(cMessage* msg) override;
    int getRadioState() override;
    simtime_t setRadioState(int rs) override;

    /**
     * Enables the SatelliteDecider to record the minSINR values in a cOutVector.
     */
    void recordMinSINR(double minSINR) override;
};

} // namespace space_veins
