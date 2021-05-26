//
// Copyright (C) 2016 Alexander Brummer <alexander.brummer@fau.de>
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

#pragma once

#include "veins/base/phyLayer/Antenna.h"

namespace space_veins {

class SPACE_VEINS_API SpaceXAntenna : public veins::Antenna {
public:

    SpaceXAntenna(double pAntenna_gain_dBi)
        : antenna_gain_dBi(pAntenna_gain_dBi){};

    ~SpaceXAntenna() override
    {
    }

    /**
     * @brief Calculates this antenna's gain. Parameters are ignored!
     *
     * @param ownPos        - coordinates of this antenna
     * @param ownOrient     - states the direction the antenna (i.e. the car) is pointing at
     * @param otherPos      - coordinates of the other antenna which this antenna is currently communicating with
     * @return Returns antenna_gain_dBi
     */
    double getGain(veins::Coord ownPos, veins::Coord ownOrient, veins::Coord otherPos) override;

    double getLastAngle() override;

private:
    double antenna_gain_dBi;
};

} // namespace space_veins
