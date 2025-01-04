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

#include "space_veins/modules/mobility/SGP4Mobility/SGP4Mobility.h"
#include "inet/common/Units.h"
#include "space_veins/modules/mobility/SGP4Mobility/SGP4.h"
#include "space_veins/modules/mobility/SGP4Mobility/TEME2ITRF.h"
#include <cmath>
#include <iterator>
#include <proj.h>
#include <sstream>
#include <string>

using namespace space_veins;

Define_Module(space_veins::SGP4Mobility);

void SGP4Mobility::setInitialPosition()
{
    EV_DEBUG << "SGP4Mobility setInitialPosition()" << std::endl;
    // not possible at the moment because of dependencies in the initialize phase
    //updateSatellitePosition();
}

void SGP4Mobility::initializePosition()
{
    EV_DEBUG << "SGP4Mobility initialPosition()" << std::endl;
    MovingMobilityBase::initializePosition();
}

void SGP4Mobility::preInitialize(TLE pTle, std::string pWall_clock_sim_start_time_utc)
{
    EV_DEBUG << "SGP4Mobility preInitialize()" << std::endl;
    tle = pTle;
    wall_clock_sim_start_time_utc = pWall_clock_sim_start_time_utc;
    isPreInitialized = true;
}

void SGP4Mobility::initialize(int stage)
{
    EV_DEBUG << "SGP4Mobility stage: " << stage << std::endl;
    MovingMobilityBase::initialize(stage);
    if (stage == 0) {
        EV_DEBUG << "Initializing SGP4Mobility module." << std::endl;
        EV_DEBUG << "isPreInitialized: " << isPreInitialized << std::endl;
        ASSERT(isPreInitialized);
        WATCH(tle.satellite_name);
        WATCH(tle.tle_line1);
        WATCH(tle.tle_line2);
        WATCH(wall_clock_sim_start_time_utc);

        // parse ned parameters
        updatePositionOnlyAboveHorizon = par("updatePositionOnlyAboveHorizon").boolValue();
        WATCH(updatePositionOnlyAboveHorizon);
        precalculationInterval_s = par("precalculationInterval_s");
        WATCH(precalculationInterval_s);

        // create projection context
        pj_ctx = proj_context_create();

        // create itrf2008_to_wgs84_projection
        itrf2008_to_wgs84_projection = proj_create_crs_to_crs(
                pj_ctx,
                "EPSG:5332",    // from ITRF2008
                "EPSG:4326",    // to WGS84
                NULL);

        // fix longitude, latitude order, see https://proj.org/development/quickstart.html
        itrf2008_to_wgs84_projection = proj_normalize_for_visualization(pj_ctx, itrf2008_to_wgs84_projection);
        if (itrf2008_to_wgs84_projection == 0) {
            throw cRuntimeError("itrf2008_to_wgs84_projection initialization error");
        }

        // create wgs84_to_wgs84cartesian_projection
        wgs84_to_wgs84cartesian_projection = proj_create(
                pj_ctx,
                "+proj=pipeline +step proj=cart +ellps=WGS84");    // from WGS84 geodetic to cartesian

        // Initialize SGP4 model
        double startmfe = -2880;  // in min -> 2 days before epoch
        double stopmfe = 2880;  // in min -> 2 days after epoch
        double deltamin = 1/600; // in min -> 1/60 is 1 second -> 1/600 is 0.1 second
        SGP4Funcs::twoline2rv((char*)tle.get_tle_line1().c_str(),
                              (char*)tle.get_tle_line2().c_str(),
                              'c', // catalog run
                              'd', // type of manual input, d = dayofyr, possible irrelevant because no manual run is performed
                              'i', //improved mode
                              gravconsttype::wgs72,     // used by Skyfield
                              startmfe,
                              stopmfe,
                              deltamin,
                              satrec);

        // Calculate elapsed time beginning from the tle epoch
        // 1. Calculate julian date_time of the tle epoch
        // Store tle epoch as date_time_t object
        tle_epoch.year = (satrec.epochyr >= 57) ? 1900 + satrec.epochyr : 2000 + satrec.epochyr;
        SGP4Funcs::days2mdhms_SGP4(satrec.epochyr,
                                   satrec.epochdays,
                                   tle_epoch.mon,
                                   tle_epoch.day,
                                   tle_epoch.hour,
                                   tle_epoch.min,
                                   tle_epoch.sec
                                   );

        std::stringstream ss2;
        ss2 << tle_epoch.year << "-" << std::setfill('0') << std::setw(2) << tle_epoch.mon << "-" << tle_epoch.day << "-" << tle_epoch.hour << "-" << tle_epoch.min << "-" << tle_epoch.sec;
        EV_DEBUG << "tle_epoch: " << ss2.str() << std::endl;
        SGP4Funcs::jday_SGP4(tle_epoch.year,
                             tle_epoch.mon,
                             tle_epoch.day,
                             tle_epoch.hour,
                             tle_epoch.min,
                             tle_epoch.sec,
                             tle_epoch_jd,
                             tle_epoch_frac);
        EV_DEBUG << std::fixed << "tle_epoch_jd: " << tle_epoch_jd << "; frac: " << tle_epoch_frac << std::endl;

        // 2. Calculate wall clock time as julian date
        EV_DEBUG << "SGP4 model wall_clock_sim_start_time_utc: " << wall_clock_sim_start_time_utc << std::endl;
        std::istringstream iss(wall_clock_sim_start_time_utc);
        std::tm tmp;    // helper struct to read in the wall_clock_sim_start_time_utc
        iss >> std::get_time(&tmp, "%Y-%m-%d-%H-%M-%S");
        wall_clock_start_time.year = tmp.tm_year + 1900;  //tm_year is number of year after 1900 according to struct
        wall_clock_start_time.mon = tmp.tm_mon + 1;       //tm_mon is number of months after Jan (Jan = 0)
        wall_clock_start_time.day = tmp.tm_mday;
        wall_clock_start_time.hour = tmp.tm_hour;
        wall_clock_start_time.min = tmp.tm_min;
        wall_clock_start_time.sec = tmp.tm_sec;

        EV_DEBUG << "SGP4 model time initialized to: year: " << wall_clock_start_time.year
                 << " month: " << wall_clock_start_time.mon
                 << " day: " << wall_clock_start_time.day
                 << " hour: " << wall_clock_start_time.hour
                 << " minute: " << wall_clock_start_time.min
                 << " second: " << wall_clock_start_time.sec
                 << std::endl;

        SGP4Funcs::jday_SGP4(wall_clock_start_time.year,
                             wall_clock_start_time.mon,
                             wall_clock_start_time.day,
                             wall_clock_start_time.hour,
                             wall_clock_start_time.min,
                             wall_clock_start_time.sec,
                             wall_clock_sim_start_time_jd,
                             wall_clock_sim_start_time_frac);
        EV_DEBUG << std::fixed << "wall_clock_sim_start_time_jd: " << wall_clock_sim_start_time_jd << "; frac: " << wall_clock_sim_start_time_frac << std::endl;

        // 3. Calculate elapsed time in minutes
        diffTleEpochWctMin = (wall_clock_sim_start_time_jd - tle_epoch_jd) * 1440 + (wall_clock_sim_start_time_frac - tle_epoch_frac) * 1440;
        EV_DEBUG << "diffTleEpochWctMin: " << diffTleEpochWctMin << " min" << std::endl;
    }
    if (stage == 4) {
        // has to be done after the SOP stage in which the sop_omnet_coord is retrieved from its mobility
        // Get access to SOP
        sop = SatelliteObservationPointAccess().get();
        ASSERT(sop);
        const PJ_COORD sop_wgs84_proj_cart = sop->get_sop_wgs84_proj_cart();
        // create geocentric to topocentric projection (requires sop_wgs84 as Cartesian coordinate)
        std::stringstream ss_proj;
        ss_proj << "+proj=topocentric +ellps=WGS84 +X_0=" << sop_wgs84_proj_cart.xyz.x << " +Y_0=" << sop_wgs84_proj_cart.xyz.y << " +Z_0=" << sop_wgs84_proj_cart.xyz.z;
        EV_DEBUG << "SGP4Mobility: ss_proj: " << ss_proj.str() << std::endl;
        // geocentric to topocentric projection
        wgs84cartesian_to_topocentric_projection = proj_create(pj_ctx, ss_proj.str().c_str());

        // Statistics
        vehicleStatistics = VehicleStatisticsAccess().get(getParentModule());

        // updatePositionOnlyAboveHorizon
        if (updatePositionOnlyAboveHorizon) {
            // precalculate time periods in which the satellite is above the horizon of the SOP
            // set up vector of intervals to investigate
            setUpPrecalculationIntervals();
            // check that precalculation intervals are created, otherwise abort precalculation
            if (!precalculationIntervals.empty()) {
                // calculate start and end position for all intervals
                calculateIntervalPositions();
                // identify relevant intervals in which the position of a satellite needs to be calculated precisely
                identifyRelevantIntervals();
                calculateBordersOfRelevantIntervals();
            } else {
                // disable updatePositionOnlyAboveHorizon
                updatePositionOnlyAboveHorizon = false;
            }
        }
    }
}

void SGP4Mobility::setUpPrecalculationIntervals()
{
    // get simTimeLimit
    const char* sim_time_limit_str = getEnvir()->getConfig()->getConfigValue("sim-time-limit");
    simtime_t simTimeLimit = SimTime::parse(sim_time_limit_str);
    // set up pre calculation intervals only if simTimeLimit is greater then 5 times the preCalculationInterval_s
    // This should ensure that enough intervals are created, otherwise precalculationIntervals remains empty and precalculation is cancelled.
    if (simTimeLimit > 5 * precalculationInterval_s) {
        // set up first interval
        precalculationInterval_t firstInterval;
        firstInterval.start = SimTime::ZERO - precalculationInterval_s;
        firstInterval.isStartDummy = true;
        firstInterval.end = SimTime::ZERO;
        precalculationIntervals.push_back(firstInterval);
        // set up intermediate intervals
        simtime_t simTimeIterate = SimTime::ZERO;
        while (simTimeIterate < simTimeLimit) {
            precalculationInterval_t intermediateInterval;
            intermediateInterval.start = simTimeIterate;
            intermediateInterval.end = simTimeIterate + precalculationInterval_s;
            precalculationIntervals.push_back(intermediateInterval);
            if (intermediateInterval.end > simTimeLimit) {
                intermediateInterval.isEndDummy = true;
            }
            simTimeIterate = simTimeIterate + precalculationInterval_s;
        }
        // set up last interval
        precalculationInterval_t lastInterval;
        lastInterval.start = simTimeIterate;
        if (lastInterval.start > simTimeLimit) {
            lastInterval.isStartDummy = true;
        }
        lastInterval.end = simTimeIterate + precalculationInterval_s;
        if (lastInterval.end > simTimeLimit) {
            lastInterval.isEndDummy = true;
        }
        precalculationIntervals.push_back(lastInterval);
        // print all precalculation intervals for debugging
        EV_DEBUG << "All precalculattion intervals:" << std::endl << vectorOfPrecalculationIntervalsToString(precalculationIntervals) << std::endl;
    }
}

void SGP4Mobility::calculateIntervalPositions() {
    inet::deg zeroDegree = inet::deg(0);
    EV_TRACE << "setPrecalculationIntervalRisingOrFallingFlags: zeroDegree: " << zeroDegree << std::endl;
    for (auto it = precalculationIntervals.begin(); it != precalculationIntervals.end(); ++it) {
        // calculate start position
        it->startPosition = calculateSatellitePosition(it->start);
        // calculate elevation angle relative to the SOP for the start position
        it->startElevationAngleToSOP = calculateElevationAngleRelativeToSop(it->startPosition);
        // calculate end position
        it->endPosition = calculateSatellitePosition(it->end);
        // calculate elevation angle relative to the SOP for the end position
        it->endElevationAngleToSOP = calculateElevationAngleRelativeToSop(it->endPosition);
        // check if satellite is rising
        if (it->endElevationAngleToSOP > it->startElevationAngleToSOP) {
            it->isRising = true;
        }
        // check if satellite is falling
        if (it->startElevationAngleToSOP > it->endElevationAngleToSOP) {
            it->isFalling = true;
        }

        // check if satellite rises above the horizon during the interval with the help of the elevation angle of the start and end position
        if (it->endElevationAngleToSOP.get() > zeroDegree.get() && it->startElevationAngleToSOP.get() < zeroDegree.get()) {
            it->risesAboveHorizon = true;
            it->isPartlyAboveHorizon = true;
        }
        // check if satellite sets below the horizon during the interval with the help of the elevation angle of the start and end position
        if (it->endElevationAngleToSOP.get() < zeroDegree.get() && it->startElevationAngleToSOP.get() > zeroDegree.get()) {
            it->setsBelowHorizon = true;
            it->isPartlyAboveHorizon = true;
        }
        // check if the satellite is above the horizon for the duration of the whole interval
        if (it->startElevationAngleToSOP.get() > zeroDegree.get() && it->endElevationAngleToSOP.get() > zeroDegree.get()) {
            it->isCompletelyAboveHorizon = true;
        }
    }
    // print all precalculation intervals for debugging
    EV_DEBUG << "All precalculation intervals: " << std::endl << vectorOfPrecalculationIntervalsToString(precalculationIntervals) << std::endl;
}

void SGP4Mobility::identifyRelevantIntervals() {
    for (auto it = precalculationIntervals.begin(); it != precalculationIntervals.end(); ++it) {
        // SPECIAL CASE: the first interval is not relevant as it is outside of the simulation period.
        if (it->isStartDummy != true) {
            // SPECIAL CASE: an interval with isStartDummy == true and isEndDummy == true is not relevant, because it is completely outside of the simulation period.
            if (it->isStartDummy == false && it->isEndDummy == false) {
                // 1. An interval is relevant if the satellite is partly or completely above the horizon
                if (it->isPartlyAboveHorizon || it->isCompletelyAboveHorizon) {
                    it->relevantForSatellitePositionUpdates = true;
                    // save index in vector
                    indicesOfRelevantIntervals.push_back(it - precalculationIntervals.begin());
                    // save a copy in relevantPrecalculationIntervals
                    relevantPrecalculationIntervals.push_back(*it);
                } else {
                    // 2. An interval is relevant if a satellite is above the horizon for some time of the interval although it is below the horizon at startPosition and endPosition
                    // This can happen when the duration of an interval is longer than the time the satellite is above the horizon.
                    // if the previous interval isRising == true and this interval isFalling == true indicates a maximum
                    // Both, the previous interval and this interval are relevant since we do not know when the satellite rises above and sets below the horizon during these two intervals.
                    // TODO: Test like this, gives us all intervals that may contain a maximum, but it is not ensured that the maximum is greater than 0deg.
                    // TODO: So, we either flag intervals as relevant although they are not because the satellite never rises above the horizon, or we have to find the maximum in order to identify if the satellite rises above the horizon at all and if so, in which of the two intervals.
                    auto pv = std::prev(it, 1);
                    auto next = std::next(it, 1);
                    if ((pv->isRising && it->isFalling) || (it->isRising && next->isFalling)) {
                        it->relevantForSatellitePositionUpdates = true;
                        it->canContainMaxima = true;
                        indicesOfRelevantIntervals.push_back(it - precalculationIntervals.begin());
                        // save a copy in relevantPrecalculationIntervals
                        relevantPrecalculationIntervals.push_back(*it);
                    }
                }
            }
        }
    }
    // print all relevant precalculation intervals for debugging
    EV_DEBUG << "All relevant precalculation intervals: " << std::endl << vectorOfPrecalculationIntervalsToString(relevantPrecalculationIntervals) << std::endl;
}

void SGP4Mobility::calculateBordersOfRelevantIntervals() {
    auto it = relevantPrecalculationIntervals.begin();
    while (it != relevantPrecalculationIntervals.end()) {
        std::pair<border_t, border_t> borders;
        border_t start = {it->start, it->startPosition.wallClockTimeUtc};
        borders.first = start;
        auto tmp = it;
        auto nxt = std::next(it, 1);
        // if nxt == relevantPrecalculationIntervals.end() then the last interval marks its own relevant period
        if (nxt == relevantPrecalculationIntervals.end()) {
            border_t end = {it->end, it->endPosition.wallClockTimeUtc};
            borders.second = end;
        } else {
            // if tmp->end == nxt->start both intervals are consecutive, otherwise a new period of relevant intervals begins
            while (nxt != relevantPrecalculationIntervals.end() && tmp->end == nxt->start) {
                tmp = nxt;
                nxt = std::next(nxt, 1);
            }
            // nxt == relevantPrecalculationIntervals.end(): tmp is the last relevant interval
            // tmp->end != nxt->start: we found the end of a relevant period because the end time does not match the next start time
            border_t end = {tmp->end, tmp->endPosition.wallClockTimeUtc};
            borders.second = end;
            bordersOfRelevantIntervals.push_back(borders);
            // set it to the interval marking the beginning of a new relevant period
            it = nxt;
        }
    }
    // print all borders for debugging
    EV_DEBUG << bordersOfRelevantIntervalsToString();
}

std::string SGP4Mobility::bordersOfRelevantIntervalsToString() {
    std::stringstream ss;
    ss << "borders of relevant intervals: (simulation start time - simulation end time) | (UTC start time - UTC end time)" << std::endl;
    for (auto it = bordersOfRelevantIntervals.begin(); it != bordersOfRelevantIntervals.end(); ++it) {
        ss << "\t(" << it->first.simulationTime << " - " << it->second.simulationTime << ") | (" << dateTimeTToString(it->first.utcTime) << " - " << dateTimeTToString(it->second.utcTime) << ")" << std::endl;
    }
    return ss.str();
}

void SGP4Mobility::updateSatellitePosition() {
    // calculate current satellite position and record statistics
    calculatedSatellitePositions_t satellitePosition = calculateSatellitePosition(simTime());
    lastPosition = satellitePosition.omnetppCoord;
    // statistics
    vehicleStatistics->recordWGS84Coord(satellitePosition.wgs84);
    vehicleStatistics->recordWGS84CartCoord(satellitePosition.wgs84CartesianGeocentric);
    vehicleStatistics->recordSopRelativeCoord(veins::Coord(satellitePosition.omnetppCoordRelativeToSOP.x, satellitePosition.omnetppCoordRelativeToSOP.y, satellitePosition.omnetppCoordRelativeToSOP.z));
    vehicleStatistics->recordOmnetCoord(veins::Coord(lastPosition.x, lastPosition.y, lastPosition.z));
    // debug print
    EV_DEBUG << satellitePositionsToString(satellitePosition);
}

SGP4Mobility::calculatedSatellitePositions_t SGP4Mobility::calculateSatellitePosition(simtime_t simulationTime)
{
    calculatedSatellitePositions_t returnValues;
    returnValues.simulationTime = simulationTime;
    EV_TRACE << "------- begin trace log caculateSatellitePosition -------" << std::endl;
    EV_TRACE << "calculateSatellitePosition for simulationTime: " << simulationTime << std::endl;
    // add simulationTime to diffTleEpochWctMin to calculate the satellite position according to the current simulation time
    EV_TRACE << "simulationTime in minutes: " << simulationTime.dbl() / 60 << " min" << std::endl;
    double t = diffTleEpochWctMin + simulationTime.dbl() / 60;
    EV_TRACE << "passed time since TLE epoch: " << t << " min" << std::endl;

    // Calculate satellite position using SGP4 in TEME coodinate system
    double r_array[3];
    double v_array[3];
    bool ret = SGP4Funcs::sgp4(satrec,
                               t,          // time since epoch in min
                               r_array,
                               v_array);
    // Copy results into a vector
    int n = sizeof(r_array) / sizeof(r_array[0]);
    std::vector<double> r_TEME(r_array, r_array + n);
    std::vector<double> v_TEME(v_array, v_array + n);
    EV_TRACE << "TEME x: " << r_TEME[0] << ", y: " << r_TEME[1] << ", z: " << r_TEME[2] << std::endl;
    returnValues.teme = inet::Coord(r_TEME[0], r_TEME[1], r_TEME[2]);

    // Calculate the current wall clock time as julian date for TEME to ITRF conversion
    // ignore leap seconds for now
    double daysToAdd;
    // add elapsed simulation time to wall_clock_sim_start_time_frac to calculate the current wall clock time as julian date
    current_wall_clock_time_frac = wall_clock_sim_start_time_frac + (simulationTime.dbl() / 86400.0);
    // decompose current_wall_clock_time_frac into integral part (whole days) and fractional part (time)
    current_wall_clock_time_frac = std::modf(current_wall_clock_time_frac, &daysToAdd);
    EV_TRACE << "daysToAdd: " << daysToAdd << std::endl;
    current_wall_clock_time_jd = wall_clock_sim_start_time_jd + daysToAdd;
    EV_TRACE << std::fixed << "current_wall_clock_time_jd: " << current_wall_clock_time_jd << ", current_wall_clock_time_frac: " << current_wall_clock_time_frac << std::endl;
    // do inverse calculation to utc time for debug purposes
    date_time_t debugWallClockTimeUtc;
    SGP4Funcs::invjday_SGP4(current_wall_clock_time_jd, current_wall_clock_time_frac, debugWallClockTimeUtc.year, debugWallClockTimeUtc.mon, debugWallClockTimeUtc.day, debugWallClockTimeUtc.hour, debugWallClockTimeUtc.min, debugWallClockTimeUtc.sec);
    EV_TRACE << "debugWallClockTimeUtc: " << dateTimeTToString(debugWallClockTimeUtc) << std::endl;
    returnValues.wallClockTimeUtc = debugWallClockTimeUtc;

    // Coordinate transformation TEME -> ITRF
    std::pair<std::vector<double>, std::vector<double>> itrf = TEME_to_ITRF(current_wall_clock_time_jd, r_TEME, v_TEME, 0.0, 0.0, current_wall_clock_time_frac);
    EV_TRACE << "SGP4Mobility itrf: x: " << itrf.first[0] << ", y: " << itrf.first[1] << ", z: " << itrf.first[2] << std::endl;
    returnValues.itrf2008 = inet::Coord(itrf.first[0] * 1000, itrf.first[1] * 1000, itrf.first[2] * 1000); // conversion km -> m!
    // Coordinate transformation ITRF -> WGS84
    PJ_COORD toTransfer = proj_coord(itrf.first[0] * 1000, itrf.first[1] * 1000, itrf.first[2] * 1000, 0); // conversion km -> m!
    PJ_COORD geo = proj_trans(itrf2008_to_wgs84_projection, PJ_FWD, toTransfer);
    WGS84Coord sat_pos_wgs84 = WGS84Coord(geo.lpz.phi, geo.lpz.lam, geo.lpz.z);
    returnValues.wgs84 = sat_pos_wgs84;
    EV_TRACE << "SGP4Mobility sat_pos_wgs84: " << sat_pos_wgs84 << std::endl;

    // Transform satellite's WGS84 coordinate from geodetic to cartesian representation, proj needs Radians for an unknown reason
    // see https://proj.org/operations/conversions/cart.html
    toTransfer = proj_coord(sat_pos_wgs84.lon * (PI/180), sat_pos_wgs84.lat * (PI/180), sat_pos_wgs84.alt, 0);
    PJ_COORD geo_cart = proj_trans(wgs84_to_wgs84cartesian_projection, PJ_FWD, toTransfer);
    returnValues.wgs84CartesianGeocentric = geo_cart;
    EV_TRACE << "SGP4Mobility sat_pos_wgs84 cartesian: x: " << geo_cart.xyz.x << ", y: " << geo_cart.xyz.y << ", z: " << geo_cart.xyz.z << std::endl;

    // Geocentric to topocentric, see https://proj.org/operations/conversions/topocentric.html
    PJ_COORD topo_cart = proj_trans(wgs84cartesian_to_topocentric_projection, PJ_FWD, geo_cart);
    returnValues.wgs84CartesianTopocentric = topo_cart;
    PJ_COORD topo_cart_omnetpp = topo_cart;
    // Note the minus operator at the northing: The reason is OMNeT++'s coordinate system. The origin is in the upper left corner,
    // the x-axis goes from west to east in the positiv direction and the y-axis goes from north to south in the positiv direction.
    // According to the figure at https://proj.org/operations/conversions/topocentric.html the enu.n-axis needs to be inverted.
    topo_cart_omnetpp.enu.n = topo_cart.enu.n * -1;
    returnValues.omnetppCoordRelativeToSOP = inet::Coord(topo_cart_omnetpp.enu.e, topo_cart_omnetpp.enu.n, topo_cart_omnetpp.enu.u);
    EV_TRACE << "SGP4Mobility topo as cartesian coordinates: e: " << topo_cart.enu.e << ", n:" << topo_cart.enu.n << ", u: " << topo_cart.enu.u << std::endl;

    // Note the minus operator at the northing: The reason is OMNeT++'s coordinate system. The origin is in the upper left corner,
    // the x-axis goes from west to east in the positiv direction and the y-axis goes from north to south in the positiv direction.
    // According to the figure at https://proj.org/operations/conversions/topocentric.html the enu.n-axis needs to be inverted.
    //
    // Further, the position of the SOP is added such that the satellite position is relative to OMNeT++'s origin.
    auto sop_omnet_coord = sop->get_sop_omnet_coord();
    inet::Coord satellitePosition(topo_cart.enu.e + sop_omnet_coord.x, -topo_cart.enu.n + sop_omnet_coord.y, topo_cart.enu.u + sop_omnet_coord.z);
    returnValues.omnetppCoord = satellitePosition;
    EV_TRACE << "SGP4Mobility satellite position at simulation time " << simulationTime << ": " << satellitePosition << std::endl;
    EV_TRACE << "------- end trace log caculateSatellitePosition -------" << std::endl;
    return returnValues;
}

inet::deg SGP4Mobility::calculateElevationAngleRelativeToSop(calculatedSatellitePositions_t pos) {
    inet::Coord vecSopToSatellite;    // a vector pointing from SOP to a satellite position
    // get mobility module of the SOP
    auto sopMobility = static_cast<inet::StationaryMobility*>(sop->getSubmodule("mobility"));
    // calculate vector pointing from SOP to the satellite
    vecSopToSatellite = pos.omnetppCoord - sopMobility->getCurrentPosition();
    // normalize the vector, it's length is 1
    vecSopToSatellite.normalize();
    // since the length of the vector is 1, the elevation angle is the arcsinus of the z-component of vecSopToSatellite
    inet::deg elevationAngle = inet::deg(inet::rad(asin(vecSopToSatellite.z)));
    return elevationAngle;
}

std::string SGP4Mobility::satellitePositionsToString(calculatedSatellitePositions_t pos) {
    std::stringstream ss;
    ss << "Satellite name:              " << tle.satellite_name << std::endl;
    ss << "simulation time:             " << pos.simulationTime << std::endl;
    ss << "wall clock time UTC:         " << dateTimeTToString(pos.wallClockTimeUtc) << std::endl;
    ss << "TEME position:               " << pos.teme.str() << std::endl;
    ss << "ITRF2008 position:           " << pos.itrf2008.str() << std::endl;
    ss << "WGS84 position:              " << pos.wgs84 << std::endl;
    ss << "WGS84 Cartesian Geocentric:  (x: " << pos.wgs84CartesianGeocentric.xyz.x << ", y: " << pos.wgs84CartesianGeocentric.xyz.y << ", z: " << pos.wgs84CartesianGeocentric.xyz.z << ")" << std::endl;
    ss << "WGS84 Cartesian Topocentric: (e: " << pos.wgs84CartesianTopocentric.enu.e << ", n: " << pos.wgs84CartesianTopocentric.enu.n << ", u: " << pos.wgs84CartesianTopocentric.enu.u << ")" << std::endl;
    ss << "omnetCoordRelativeToSOP:     " << pos.omnetppCoordRelativeToSOP << std::endl;
    ss << "omnetCoord:                  " << pos.omnetppCoord << std::endl;
    return ss.str();
}

std::string SGP4Mobility::precalculationIntervalToString(precalculationInterval_t interval) {
    std::stringstream ss;
    ss << "Satellite name:                               " << tle.satellite_name << std::endl;
    ss << "interval start time:                          " << interval.start << std::endl;
    ss << "interval isStartDummy:                        " << interval.isStartDummy << std::endl;
    ss << "interval start position:                      " << std::endl << satellitePositionsToString(interval.startPosition);
    ss << "interval end time:                            " << interval.end << std::endl;
    ss << "interval isEndDummy:                          " << interval.isEndDummy << std::endl;
    ss << "interval end position:                        " << std::endl << satellitePositionsToString(interval.endPosition);
    ss << "interval startElevationAngleToSOP:            " << interval.startElevationAngleToSOP << std::endl;
    ss << "interval endElevationAngleToSOP:              " << interval.endElevationAngleToSOP << std::endl;
    ss << "interval isRising:                            " << interval.isRising << std::endl;
    ss << "interval isFalling:                           " << interval.isFalling << std::endl;
    ss << "interval risesAboveHorizon:                   " << interval.risesAboveHorizon << std::endl;
    ss << "interval setsBelowHorizon:                    " << interval.setsBelowHorizon << std::endl;
    ss << "interval isPartlyAboveHorizon:                " << interval.isPartlyAboveHorizon << std::endl;
    ss << "interval isCompletelyAboveHorizon:            " << interval.isCompletelyAboveHorizon << std::endl;
    ss << "interval relevantForSatellitePositionUpdates: " << interval.relevantForSatellitePositionUpdates << std::endl;
    ss << "canContainMaxima:                             " << interval.canContainMaxima << std::endl;
    return ss.str();
}

std::string SGP4Mobility::vectorOfPrecalculationIntervalsToString(std::vector<precalculationInterval_t> vec) {
    std::stringstream ss;
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        ss << precalculationIntervalToString(*it);
        ss << "-----------------------------------" << std::endl;
    }
    return ss.str();
}

std::string SGP4Mobility::dateTimeTToString(date_time_t t) {
    std::stringstream ss;
    ss << "UTC year-month-day-hour-min-sec: " << t.year << "-" << t.mon << "-" << t.day << "-" << t.hour << "-" << t.min << "-" << t.sec;
    return ss.str();
}

void SGP4Mobility::handleSelfMessage(cMessage* message)
{
    MovingMobilityBase::handleSelfMessage(message);
}

void SGP4Mobility::move()
{
    updateSatellitePosition();
    lastVelocity = inet::Coord();                       // TODO: Consider the speed returned by the SGP4 model
    //lastOrientation = inet::Quaternion(0, 0, 0, 0);     // TODO: Currently there are no values for the direction of the satellite
    lastUpdate = simTime();
    nextChange = simTime() + updateInterval;

    EV_DEBUG << "SGP4Mobility moved SimTime: " << simTime() << std::endl;
    updateDisplayStringFromMobilityState();
}

void SGP4Mobility::finish()
{
    MovingMobilityBase::finish();
}
