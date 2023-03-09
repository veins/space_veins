//
// Copyright (C) 2006-2018 Christoph Sommer <sommer@ccs-labs.org>
// Copyright (C) 2022 Mario Franke <research@m-franke.net>
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

//
// Veins Mobility module for the INET Framework
// Based on veins::VeinsInetMobility
//

#pragma once

namespace omnetpp {
}
using namespace omnetpp;

#include <proj.h>

#include "veins/base/utils/FindModule.h"
#include "veins/base/utils/Coord.h"

#include "veins_inet/VeinsInetMobility.h"

#include "space_veins/modules/SatelliteObservationPoint/SatelliteObservationPoint.h"
#include "space_veins/modules/utility/WGS84Coord.h"
#include "space_veins/modules/statistics/VehicleStatistics/VehicleStatistics.h"

namespace space_veins {

class SPACE_VEINS_API VeinsInetMobilityWGS84 : public veins::VeinsInetMobility {
public:
    VeinsInetMobilityWGS84();

    virtual ~VeinsInetMobilityWGS84();

    virtual void initialize(int stage) override;

    /** @brief called by class VeinsInetManager */
    virtual void nextPosition(const inet::Coord& position, std::string road_id, double speed, double angle);

protected:
    virtual void handleSelfMessage(cMessage* message) override;

    SatelliteObservationPoint* sop;

    VehicleStatistics* vehicleStatistics;

};

} // namespace space_veins

namespace space_veins {
class SPACE_VEINS_API VeinsInetMobilityWGS84Access {
public:
    VeinsInetMobilityWGS84* get(cModule* host)
    {
        VeinsInetMobilityWGS84* m = veins::FindModule<VeinsInetMobilityWGS84*>::findSubModule(host);
        ASSERT(m);
        return m;
    };
};
} // namespace space_veins
