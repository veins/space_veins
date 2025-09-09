//
// Copyright (C) 2006-2012 Christoph Sommer <christoph.sommer@uibk.ac.at>
// Copyright (C) 2025 Mario Franke <research@m-franke.net>
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

#pragma once

#include "space_veins/modules/mobility/SGP4Mobility/SGP4MobilityBase.h"

namespace space_veins {

class SPACE_VEINS_API SGP4MobilityWithGetCurrentPositionUpdates : public SGP4MobilityBase {

public:
    SGP4MobilityWithGetCurrentPositionUpdates()
        : SGP4MobilityBase()
    {
    }

    ~SGP4MobilityWithGetCurrentPositionUpdates() override
    {
    }

protected:
    // methods

    void initialize(int) override;
    void finish() override;
    bool nextPeriodicPositionUpdate() override;
    bool nextGetCurrentPositionUpdate() override;

    class SPACE_VEINS_API SGP4MobilityWithGetCurrentPositionUpdatesAccess {
    public:
        SGP4MobilityWithGetCurrentPositionUpdates* get(cModule* host)
        {
            SGP4MobilityWithGetCurrentPositionUpdates* sgp4 = veins::FindModule<SGP4MobilityWithGetCurrentPositionUpdates*>::findSubModule(host);
            ASSERT(sgp4);
            return sgp4;
        };
    };
};
} // namespace space_veins
