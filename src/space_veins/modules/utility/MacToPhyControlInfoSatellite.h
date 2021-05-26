//
// Copyright (C) 2018-2019 Dominik S. Buse <buse@ccs-labs.org>
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

#include "space_veins/space_veins.h"

#include "space_veins/modules/utility/ConstsSatellites.h"

namespace space_veins {

/**
 * Stores information which is needed by the physical layer
 * when sending a MacPkt.
 *
 * @ingroup phyLayer
 * @ingroup macLayer
 */
struct SPACE_VEINS_API MacToPhyControlInfoSatellite : public cObject {
    SatelliteChannel channelNr; ///< Channel number/index used to select frequency.
    veins::MCS mcs; ///< The modulation and coding scheme to employ for the associated frame.
    double txPower_mW; ///< Transmission power in milliwatts.

    MacToPhyControlInfoSatellite(SatelliteChannel channelNr, veins::MCS mcs, double txPower_mW)
        : channelNr(channelNr)
        , mcs(mcs)
        , txPower_mW(txPower_mW)
    {
    }
};

} // namespace space_veins
