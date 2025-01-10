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
#include <utility>
#include <vector>

#include "inet/common/Units.h"
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

struct calculatedSatellitePositions_t {
    simtime_t simulationTime = SIMTIME_MAX;
    date_time_t wallClockTimeUtc = {-1, -1, -1, -1, -1, -1.0};
    inet::Coord teme = inet::Coord::NIL;
    inet::Coord itrf2008 = inet::Coord::NIL;   // values are in meters
    WGS84Coord wgs84 = WGS84Coord(-1, -1, -1);
    PJ_COORD wgs84CartesianGeocentric = {-1, -1, -1, -1}; // WGS84 coordinate in Cartesian representation
    PJ_COORD wgs84CartesianTopocentric = {-1, -1, -1, -1}; // WGS84 coordinate in Cartesian representation relative to SOP
    inet::Coord omnetppCoordRelativeToSOP = inet::Coord::NIL; // WGS84 coordinate in Cartesian representation relative to SOP in OMNeT++ coordinate system -- y-axis is positiv going south
    inet::Coord omnetppCoord = inet::Coord::NIL; // satellite position in OMNeT++'s coordinate system
};

struct precalculationInterval_t {
    simtime_t start = SIMTIME_MAX; // simulation time of the start of the interval
    simtime_t end = SIMTIME_MAX; // simulation time of the end of the interval
    bool isStartDummy = false; // is true if this time point is out of the simulation time and only used to check if the satellite is rising or falling at SimTime == 0
    bool isEndDummy = false; // is true if the time point is out of the simulation time and only used to check if the satellite is rising or fallting at SimTime == <SimTimeLimit>
    calculatedSatellitePositions_t startPosition;
    calculatedSatellitePositions_t endPosition;
    inet::deg startElevationAngleToSOP = inet::deg(NaN);
    inet::deg endElevationAngleToSOP = inet::deg(NaN);
    bool isRising = false; // is true if the satellite is rising between <start> and <end>
    bool isFalling = false; // is true if the satellite is falling between <start> and <end>
    bool risesAboveHorizon = false; // is true of the satellite rises above the horizon in this interval
    bool setsBelowHorizon = false; // is true of the satellite sets below the horizon in this interval
    bool isPartlyAboveHorizon = false; // is true if either startElevationAngleToSOP or endElevationAngleToSOP is greater than 0
    bool isCompletelyAboveHorizon = false; // is true if both startElevationAngleToSOP and endElevationAngleToSOP is greater than 0
    bool relevantForSatellitePositionUpdates = false; // is true if this interval is relevant for precise position updates after the initialization phase
    bool canContainMaxima = false; // is true if one of the neighboring interval isRising and this interval isFalling or vice versa
};

struct border_t {
    simtime_t simulationTime;
    date_time_t utcTime;
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

    void setUpPrecalculationIntervals();

    void calculateIntervalPositions();

    void identifyRelevantIntervals();

    void calculateBordersOfRelevantIntervals();

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
    calculatedSatellitePositions_t calculateSatellitePosition(simtime_t simulationTime);
    inet::deg calculateElevationAngleRelativeToSop(calculatedSatellitePositions_t pos);
    std::string satellitePositionsToString(calculatedSatellitePositions_t pos);
    std::string precalculationIntervalToString(precalculationInterval_t interval);
    std::string vectorOfPrecalculationIntervalsToString(std::vector<precalculationInterval_t> vec);
    std::string bordersOfRelevantIntervalsToString();
    std::string dateTimeTToString(date_time_t t);
    static bool isSmallerStartBorder(std::pair<border_t, border_t> i, std::pair<border_t, border_t> j);

protected:
    // ned parameters
    bool updatePositionOnlyAboveHorizon;
    simtime_t precalculationInterval_s;

    // precalculation data structures
    std::vector<precalculationInterval_t> precalculationIntervals;
    std::vector<precalculationInterval_t> relevantPrecalculationIntervals;
    std::vector<int> indicesOfRelevantIntervals;
    std::vector<std::pair<border_t, border_t>> bordersOfRelevantIntervals;

    bool isPreInitialized;
    bool firstPositionUpdateDone = false;
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

    /* Signals */
    static const simsignal_t processedEvent;

    /* Statistics */
    VehicleStatistics* vehicleStatistics;
    int calculatedPositionUpdates = 0;
    int processedEvents = 0;

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
