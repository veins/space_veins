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

#include <sstream>
#include <iomanip>
#include <chrono>

#include <proj.h>
#include <utility>
#include <vector>

#include "inet/common/Units.h"
#include "inet/mobility/base/MovingMobilityBase.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/physicallayer/common/packetlevel/MediumLimitCache.h"

#include "space_veins/space_veins.h"
#include "space_veins/modules/utility/WGS84Coord.h"
#include "space_veins/modules/mobility/SGP4Mobility/TLE.h"
#include "space_veins/modules/mobility/SGP4Mobility/SGP4.h"
#include "space_veins/modules/SatelliteObservationPoint/SatelliteObservationPoint.h"

#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/mobility/traci/TraCIConnection.h"
#include "veins/base/utils/FindModule.h"
#include "veins/base/utils/Heading.h"
#include "veins/base/utils/Coord.h"
#include "veins/base/utils/FWMath.h"
#include "veins/modules/world/traci/trafficLight/TraCITrafficLightProgram.h"

namespace space_veins {
class VehicleStatistics;

class SPACE_VEINS_API SGP4MobilityBase : public inet::MovingMobilityBase {

struct date_time_t {
    int year;
    int mon;
    int day;
    int hour;
    int min;
    double sec;

    bool operator==(const date_time_t& t) const
    {
        return (this->year == t.year &&
                this->mon == t.mon &&
                this->day == t.day &&
                this->hour == t.hour &&
                this->min == t.min &&
                this->sec == t.sec);
    }
};

struct border_t {
    simtime_t simulationTime;
    date_time_t utcTime;
};

protected:
enum updateType_t {
    undefined = -1,
    periodicUpdate,
    getCurrentPositionUpdate
    };

struct calculatedSgp4SatellitePositions_t {
    simtime_t simulationTime = SIMTIME_MAX;
    date_time_t wallClockTimeUtc = {-1, -1, -1, -1, -1, -1.0};
    inet::Coord teme = inet::Coord::NIL;
    inet::Coord itrf2008 = inet::Coord::NIL;   // values are in meters
    WGS84Coord wgs84 = WGS84Coord(-1, -1, -1);
    PJ_COORD wgs84CartesianGeocentric = {-1, -1, -1, -1}; // WGS84 coordinate in Cartesian representation
    PJ_COORD wgs84CartesianTopocentric = {-1, -1, -1, -1}; // WGS84 coordinate in Cartesian representation relative to SOP
    inet::Coord omnetppCoordRelativeToSOP = inet::Coord::NIL; // WGS84 coordinate in Cartesian representation relative to SOP in OMNeT++ coordinate system -- y-axis is positiv going south
    inet::Coord omnetppCoord = inet::Coord::NIL; // satellite position in OMNeT++'s coordinate system
    double sgp4SpeedMeterPerSecond = -1; // scalar speed of the satellite calculated by SGP4 in meter per second

    bool operator==(const calculatedSgp4SatellitePositions_t& p) const
    {
        return (this->simulationTime == p.simulationTime &&
                this->wallClockTimeUtc == p.wallClockTimeUtc &&
                this->teme == p.teme &&
                this->itrf2008 == p.itrf2008 &&
                this->wgs84 == p.wgs84 &&
                // this->wgs84CartesianGeocentric == p.wgs84CartesianGeocentric && disabled because PJ_COORD does not support operator==
                // this->wgs84CartesianTopocentric == p.wgs84CartesianTopocentric &&
                this->omnetppCoordRelativeToSOP == p.omnetppCoordRelativeToSOP &&
                this->omnetppCoord == p.omnetppCoord &&
                this->sgp4SpeedMeterPerSecond == p.sgp4SpeedMeterPerSecond);
    }
};

struct precalculationInterval_t {
    simtime_t start = SIMTIME_MAX; // simulation time of the start of the interval
    simtime_t end = SIMTIME_MAX; // simulation time of the end of the interval
    bool isStartDummy = false; // is true if this time point is out of the simulation time and only used to check if the satellite is rising or falling at SimTime == 0
    bool isEndDummy = false; // is true if the time point is out of the simulation time and only used to check if the satellite is rising or fallting at SimTime == <SimTimeLimit>
    calculatedSgp4SatellitePositions_t startPosition;
    calculatedSgp4SatellitePositions_t endPosition;
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




public:
    SGP4MobilityBase()
        : MovingMobilityBase()
        , isPreInitialized(false)
    {
    }

    ~SGP4MobilityBase() override
    {
    }

    void preInitialize(TLE pTle, std::string pWall_clock_sim_start_time_utc);

    virtual inet::Coord getCurrentPosition() override;
    virtual inet::Coord getCurrentVelocity() override { return inet::Coord::ZERO; }
    virtual inet::Quaternion getCurrentAngularPosition() override { return inet::Quaternion::IDENTITY; }
    virtual inet::Coord getCurrentAcceleration() override { return inet::Coord::ZERO; }
    inet::deg getLastElevationAngleToSop() { return lastElevationAngleToSop; }

protected:
    // methods
    void initialize(int) override;
    int numInitStages() const override { return std::max(cSimpleModule::numInitStages(), 5); }
    TLE get_TLE() const;
    void finish() override;
    void set_TLE(const TLE tle);
    virtual void handleSelfMessage(cMessage* message) override;
    virtual void move() override;
    void moveAndUpdate();
    void scheduleUpdate();
    void scheduleRelevantUpdate();  // schedule next event according to the precalculation results
    void setUpPrecalculationIntervals(simtime_t pUpperTimeLimit);
    void calculateIntervalPositions();
    void identifyRelevantIntervals();
    void calculateBordersOfRelevantIntervals();
    void setInitialPosition() override;
    void initializePosition() override;
    virtual bool nextPeriodicPositionUpdate();
    virtual bool nextGetCurrentPositionUpdate();
    void SGP4PositionUpdate(simtime_t sgp4InputTime);
    calculatedSgp4SatellitePositions_t calculateSgp4SatellitePosition(simtime_t simulationTime);
    inet::deg calculateElevationAngleRelativeToSop(inet::Coord pos);
    virtual void updateManagementDataAndStatistics(updateType_t type, bool updated);
    std::string sgp4SatellitePositionToString(calculatedSgp4SatellitePositions_t pos);
    std::string precalculationIntervalToString(precalculationInterval_t interval);
    std::string vectorOfPrecalculationIntervalsToString(std::vector<precalculationInterval_t> vec);
    std::string bordersOfRelevantIntervalsToString();
    bool isRelevantInterval(simtime_t t);   // returns true if t is inside a relevant interval, false if not
    std::string dateTimeTToString(date_time_t t);
    static bool isSmallerStartBorder(std::pair<border_t, border_t> i, std::pair<border_t, border_t> j);

    // statistics helper functions
    void setVectorNames();
    void recordOmnetCoord(const inet::Coord c);
    void recordWGS84Coord(const space_veins::WGS84Coord c);
    void recordWGS84CartCoord(const PJ_COORD c);
    void recordSopRelativeCoord(const inet::Coord c);
    void recordSgp4SpeedMeterPerSecond(const double speed);

    // attributes
    // ned parameters
    bool updatePositionOnlyAboveHorizon;
    simtime_t precalculationInterval_s;
    simtime_t precalculationUpperTimeLimit_s;

    // base
    calculatedSgp4SatellitePositions_t lastCalculatedSgp4SatellitePosition;
    simtime_t lastPeriodicUpdate; // the last time the satellite position was updated because of a periodic update according to updateInterval.
    simtime_t lastGetCurrentPositionUpdate; // the last time the satellite position was updated because of a getCurrentPosition() update.
    updateType_t updateType = undefined; // stores the last used update type: either periodic or getCurrentPosition
    bool isAboveHorizon = false;
    simtime_t lastRisingAboveHorizon;
    simtime_t lastDipBelowHorizon;
    inet::deg lastElevationAngleToSop; // elevation angle of lastPosition relative to the SOP

    // precalculation data structures
    std::vector<precalculationInterval_t> precalculationIntervals;
    std::vector<precalculationInterval_t> relevantPrecalculationIntervals;
    std::vector<int> indicesOfRelevantIntervals;
    std::vector<std::pair<border_t, border_t>> bordersOfRelevantIntervals;

    int precalculationIntervalsSize = -1;
    int relevantPrecalculationIntervalsSize = -1;
    int bordersOfRelevantIntervalsSize = -1;

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
    inet::StationaryMobility* sopMobility;

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
    // OMNeT++ events/calls statistics
    unsigned long positionUpdates = 0;
    cOutVector vecPositionUpdates;
    unsigned long processedEvents = 0;
    cOutVector vecProcessedEvents;
    //      periodic updates, done by handleSelfMessage()
    unsigned long periodicUpdates = 0;
    cOutVector vecPeriodicUpdates;
    unsigned long periodicUpdateCalls = 0;
    cOutVector vecPeriodicUpdateCalls;
    unsigned long periodicUpdateCallsNoWarmupPeriod = 0;
    cOutVector vecPeriodicUpdateCallsNoWarmupPeriod;
    //      random updates, done by getCurrentPosition()
    unsigned long getCurrentPositionUpdates = 0;
    cOutVector vecGetCurrentPositionUpdates;
    unsigned long getCurrentPositionUpdateCalls = 0;
    cOutVector vecGetCurrentPositionUpdateCalls;
    unsigned long getCurrentPositionUpdateCallsNoWarmupPeriod = 0;
    cOutVector vecGetCurrentPositionUpdateCallsNoWarmupPeriod;
    //      moveAndUpdate() calls
    unsigned long moveAndUpdateCalls = 0;
    cOutVector vecMoveAndUpdateCalls;
    unsigned long moveAndUpdateLastUpdateNowCalls = 0;
    cOutVector vecMoveAndUpdateLastUpdateNowCalls;
    unsigned long moveAndUpdateLastUpdateNotNowCalls = 0;
    cOutVector vecMoveAndUpdateLastUpdateNotNowCalls;
    //      SGP4 computations
    unsigned long sgp4Computations = 0;
    cOutVector vecSgp4Computations;
    unsigned long sgp4ComputationsDuringPrecalculation = 0;
    // SGP4Mobility statistics
    unsigned long horizonRises = 0;
    cOutVector vecHorizonRises;
    unsigned long horizonSets = 0;
    cOutVector vecHorizonSets;
    unsigned long periodsAboveHorizon = 0;
    cOutVector vecPeriodsAboveHorizon;
    simtime_t totalTimeAboveHorizon;

    cOutVector vecLastElevationAngleToSop;
    cOutVector vecSgp4SpeedMeterPerSecond;

    cOutVector vecOmnetCoordX;
    cOutVector vecOmnetCoordY;
    cOutVector vecOmnetCoordZ;

    cOutVector vecWgs84CoordLat;
    cOutVector vecWgs84CoordLon;
    cOutVector vecWgs84CoordAlt;

    cOutVector vecWgs84CartCoordX;
    cOutVector vecWgs84CartCoordY;
    cOutVector vecWgs84CartCoordZ;

    cOutVector vecSopRelativeCoordX;
    cOutVector vecSopRelativeCoordY;
    cOutVector vecSopRelativeCoordZ;

};

class SPACE_VEINS_API SGP4MobilityBaseAccess {
public:
    SGP4MobilityBase* get(cModule* host)
    {
        SGP4MobilityBase* sgp4MobilityBase = veins::FindModule<SGP4MobilityBase*>::findSubModule(host);
        ASSERT(sgp4MobilityBase);
        return sgp4MobilityBase;
    };
};
} // namespace space_veins
