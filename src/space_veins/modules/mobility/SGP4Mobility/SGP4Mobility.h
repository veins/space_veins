//
// Copyright (C) 2006-2012 Christoph Sommer <christoph.sommer@uibk.ac.at>
// Copyright (C) 2021 Mario Franke <research@m-franke.net>
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

#include <sstream>
#include <iomanip>
#include <chrono>

#include "proj.h"

#include "inet/mobility/base/MovingMobilityBase.h"
#include "inet/common/geometry/common/Coord.h"

#include "space_veins/space_veins.h"
#include "space_veins/modules/utility/WGS84Coord.h"
#include "space_veins/modules/mobility/SGP4Mobility/TLE.h"
#include "space_veins/modules/mobility/SGP4Mobility/SGP4.h"
#include "space_veins/modules/SatelliteObservationPoint/SatelliteObservationPoint.h"
#include "space_veins/modules/statistics/VehicleStatistics/VehicleStatistics.h"

#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/mobility/traci/TraCIConnection.h"
#include "veins/base/utils/FindModule.h"
#include "veins/base/utils/Heading.h"
#include "veins/base/utils/Coord.h"
#include "veins/base/utils/FWMath.h"

namespace space_veins {

class SPACE_VEINS_API SGP4Mobility : public inet::MovingMobilityBase {

struct date_time_t {
    int year;
    int mon;
    int day;
    int hour;
    int min;
    double sec;
};

public:
    SGP4Mobility()
        : MovingMobilityBase()
    {
    }
    ~SGP4Mobility() override
    {
    }
    void initialize(int) override;

    void initializePosition() override;

    int numInitStages() const override
    {
        return std::max(cSimpleModule::numInitStages(), 3);
    }

    void set_TLE(const TLE tle);

    TLE get_TLE() const;

    void finish() override;

    virtual void handleSelfMessage(cMessage* message) override;

    virtual void move() override;

    virtual inet::Coord getCurrentPosition() override { return lastPosition; }
    virtual inet::Coord getCurrentVelocity() override { return inet::Coord::ZERO; }
    virtual inet::Coord getCurrentAcceleration() override { return inet::Coord::ZERO; }

    void updateSatellitePosition();

protected:
    // proj context
    PJ_CONTEXT* pj_ctx;
    PJ* itrf2008_to_wgs84_projection;
    PJ* wgs84_to_wgs84cartesian_projection;
    PJ* wgs84cartesian_to_topocentric_projection;

    // SGP4
    elsetrec satrec;

    // TLE data
    TLE tle;

    // SOP pointer
    SatelliteObservationPoint* sop;

    // Time management
    date_time_t tle_epoch;                      // date_time_t of the TLE's epoch
    std::string wall_clock_sim_start_time_utc;  // wall clock start time of the simulation's begin
    date_time_t wall_clock_start_time;          // wall clock start time of the simulation's begin as date_time_t object
    std::chrono::system_clock::time_point wct;  // current wall clock time
    std::chrono::duration<double, std::chrono::minutes::period> wall_clock_since_tle_epoch_min;  // elapsed minutes since tle epoch considering configured wall clock start time in UTC and elapsed simulation time
    date_time_t current_date_time;

    /* Statistics */
    VehicleStatistics* vehicleStatistics;

class SPACE_VEINS_API SGP4MobilityAccess {
public:
    SGP4Mobility* get(cModule* host)
    {
        SGP4Mobility* sgp4 = veins::FindModule<SGP4Mobility*>::findSubModule(host);
        ASSERT(sgp4);
        return sgp4;
    };
};
};
} // namespace space_veins
