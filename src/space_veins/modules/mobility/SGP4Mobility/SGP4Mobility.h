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

#include <proj.h>

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
        , isPreInitialized(false)
    {
    }
    ~SGP4Mobility() override
    {
    }

    void preInitialize(TLE pTle, std::string pWall_clock_sim_start_time_utc);

    void initialize(int) override;

    void setInitialPosition() override;

    void initializePosition() override;

    int numInitStages() const override
    {
        return std::max(cSimpleModule::numInitStages(), 5);
    }

    void set_TLE(const TLE tle);

    TLE get_TLE() const;

    void finish() override;

    virtual void handleSelfMessage(cMessage* message) override;

    virtual void move() override;

    virtual inet::Coord getCurrentPosition() override { return lastPosition; }
    virtual inet::Coord getCurrentVelocity() override { return inet::Coord::ZERO; }
    virtual inet::Quaternion getCurrentAngularPosition() override { return inet::Quaternion::IDENTITY; }
    virtual inet::Coord getCurrentAcceleration() override { return inet::Coord::ZERO; }

    void updateSatellitePosition();

protected:
    bool isPreInitialized;
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
    // wall clock start time utc
    std::string wall_clock_sim_start_time_utc;  // wall clock start time of the simulation's begin
    date_time_t wall_clock_start_time;          // wall clock start time of the simulation's begin as date_time_t object
    double wall_clock_sim_start_time_jd;        // julian date of the simulation's begin
    double wall_clock_sim_start_time_frac;      // fraction of the day of the simulation's begin for julian date
    // TLE epoch
    date_time_t tle_epoch;                      // date_time_t of the TLE's epoch
    double tle_epoch_jd;                        // julian date of the TLE's epoch
    double tle_epoch_frac;                      // fraction of the day of the TLE's epoch for julian date
    // Current wall clock time as julian data
    double current_wall_clock_time_jd;          // current wall clock time as julian date
    double current_wall_clock_time_frac;        // fraction of the day of the current wall clock time for julian date
    // Difference Tle epoch current wall clock time in minutes
    double diffTleEpochWctMin;                  // difference between the TLE's epoch and the wall clock start time in minutes

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
