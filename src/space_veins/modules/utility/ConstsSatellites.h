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

#include <stdint.h>

#include "space_veins/space_veins.h"

#include "veins/modules/utility/ConstsPhy.h"

using omnetpp::SimTime;

namespace space_veins {

/** @brief Bit rates for 802.11p
 *
 * as defined in Table 17-14 MIB attribute default values/ranges in the IEEE 802.11-2007 standard
 */
const uint64_t NUM_BITRATES_SATELLITE = 8;
const uint64_t BITRATES_SATELLITE[] = {3000000, 4500000, 6000000, 9000000, 12000000, 18000000, 24000000, 27000000};

/** @brief Number of Data Bits Per Symbol (N_NBPS) corresponding to bitrates in BITRATES_SATELLITE
 *
 * as defined in Table 17-3 in the IEEE 802.11-2007 standard
 */
const uint32_t N_DBPS_SATELLITE[] = {24, 36, 48, 72, 96, 144, 192, 216};

/** @brief Symbol interval
 *
 * as defined in Table 17-4 in the IEEE 802.11-2007 standard
 */
const double T_SYM_SATELLITE = 8e-6;

/** @brief Slot Time for 10 MHz channel spacing
 *
 * as defined in Table 17-15 OFDM PHY characteristics in the IEEE 802.11-2007 standard
 */
const SimTime SLOTLENGTH_SAT = SimTime().setRaw(13000000UL);

/** @brief Short interframe space
 *
 * as defined in Table 17-15 OFDM PHY characteristics in the IEEE 802.11-2007 standard
 */
const SimTime SIFS_SAT = SimTime().setRaw(32000000UL);

/** @brief Time it takes to switch from Rx to Tx Mode
 *
 * as defined in Table 17-15 OFDM PHY characteristics in the IEEE 802.11-2007 standard
 */
const SimTime RADIODELAY_SAT = SimTime().setRaw(1000000UL);

/** @brief Contention Window minimal size
 *
 * as defined in Table 17-15 OFDM PHY characteristics in the IEEE 802.11-2007 standard
 */
const unsigned CWMIN_SAT = 15;

/** @brief Contention Window maximal size
 *
 * as defined in Table 17-15 OFDM PHY characteristics in the IEEE 802.11-2007 standard
 */
const unsigned CWMAX_SAT = 1023;

/** @brief 1609.4 slot length
 *
 * as defined in Table H.1 in the IEEE 1609.4-2010 standard
 */
const SimTime SWITCHING_INTERVAL_SAT = SimTime().setRaw(50000000000UL);

/** @brief 1609.4 slot length
 *
 * as defined in Table H.1 in the IEEE 1609.4-2010 standard
 * It is the sum of SyncTolerance and MaxChSwitchTime as defined in 6.2.5 in the IEEE 1609.4-2010 Standard
 */
const SimTime GUARD_INTERVAL_SAT = SimTime().setRaw(4000000000UL);

const veins::Bandwidth BANDWIDTH_SAT = veins::Bandwidth::ofdm_10_mhz;

/** @brief Channels as reserved by the FCC
 *
 */
enum class SatelliteChannel {
    dvbs1 = 234,
};

/**
 * Maps channel identifier to the corresponding center frequency.
 *
 * @note Not all entries are defined.
 */
const std::map<SatelliteChannel, double> SatelliteChannelFrequencies = {
    {SatelliteChannel::dvbs1, 10.7e9},
};

enum class SatelliteChannelType {
    control = 0,
    service,
};
} // namespace space_veins
