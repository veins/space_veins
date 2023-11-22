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
#include "space_veins/modules/mobility/SGP4Mobility/TEME2ITRF.h"

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
    }
}

void SGP4Mobility::updateSatellitePosition()
{
    // add simTime() to diffTleEpochWctMin to calculate the satellite position according to the current simulation time
    EV_TRACE << "simTime in minutes: " << simTime().dbl() / 60 << " min" << std::endl;
    double t = diffTleEpochWctMin + simTime().dbl() / 60;
    EV_TRACE << "t: " << t << " min" << std::endl;

    // Calculate satellite position using SGP4 in TEME coodinate system
    double r_array[3];
    double v_array[3];
    bool ret = SGP4Funcs::sgp4(satrec,
                               t,          // time since epoch in min
                               r_array,
                               v_array);
    // Copy results into a vector
    EV_TRACE << "time since epoch: " << t << " min" << std::endl;
    int n = sizeof(r_array) / sizeof(r_array[0]);
    std::vector<double> r_TEME(r_array, r_array + n);
    std::vector<double> v_TEME(v_array, v_array + n);
    EV_TRACE << "TEME x: " << r_TEME[0] << ", y: " << r_TEME[1] << ", z: " << r_TEME[2] << std::endl;

    // Calculate the current wall clock time as julian date for TEME to ITRF conversion
    // ignore leap seconds for now
    double daysToAdd = std::trunc((wall_clock_sim_start_time_frac + simTime().dbl()) / 86400.0);
    EV_TRACE << "daysToAdd: " << daysToAdd << std::endl;
    current_wall_clock_time_frac = wall_clock_sim_start_time_frac + simTime().dbl() / 86400.0;
    current_wall_clock_time_jd = wall_clock_sim_start_time_jd + daysToAdd;
    EV_TRACE << std::fixed << "current_wall_clock_time_jd: " << current_wall_clock_time_jd << ", current_wall_clock_time_frac: " << current_wall_clock_time_frac << std::endl;

    // Coordinate transformation TEME -> ITRF
    std::pair<std::vector<double>, std::vector<double>> itrf = TEME_to_ITRF(current_wall_clock_time_jd, r_TEME, v_TEME, 0.0, 0.0, current_wall_clock_time_frac);
    EV_TRACE << "SGP4Mobility itrf: x: " << itrf.first[0] << ", y: " << itrf.first[1] << ", z: " << itrf.first[2] << std::endl;
    // Coordinate transformation ITRF -> WGS84
    PJ_COORD toTransfer = proj_coord(itrf.first[0] * 1000, itrf.first[1] * 1000, itrf.first[2] * 1000, 0); // conversion km -> m!
    PJ_COORD geo = proj_trans(itrf2008_to_wgs84_projection, PJ_FWD, toTransfer);
    WGS84Coord sat_pos_wgs84 = WGS84Coord(geo.lpz.phi, geo.lpz.lam, geo.lpz.z);
    vehicleStatistics->recordWGS84Coord(sat_pos_wgs84);
    EV_TRACE << "SGP4Mobility simTime(): " << simTime() << std::endl;
    EV_TRACE << "SGP4Mobility sat_pos_wgs84: " << sat_pos_wgs84 << std::endl;

    // Transform satellite's WGS84 coordinate from geodetic to cartesian representation, proj needs Radians for an unknown reason
    // see https://proj.org/operations/conversions/cart.html
    toTransfer = proj_coord(sat_pos_wgs84.lon * (PI/180), sat_pos_wgs84.lat * (PI/180), sat_pos_wgs84.alt, 0);
    PJ_COORD geo_cart = proj_trans(wgs84_to_wgs84cartesian_projection, PJ_FWD, toTransfer);
    vehicleStatistics->recordWGS84CartCoord(geo_cart);
    EV_TRACE << "SGP4Mobility sat_pos_wgs84 cartesian: x: " << geo_cart.xyz.x << ", y: " << geo_cart.xyz.y << ", z: " << geo_cart.xyz.z << std::endl;

    // Geocentric to topocentric, see https://proj.org/operations/conversions/topocentric.html
    PJ_COORD topo_cart = proj_trans(wgs84cartesian_to_topocentric_projection, PJ_FWD, geo_cart);
    EV_TRACE << "SGP4Mobility topo as cartesian coordinates: e: " << topo_cart.enu.e << ", n:" << -topo_cart.enu.n << ", u: " << topo_cart.enu.u << std::endl;
    vehicleStatistics->recordSopRelativeCoord(veins::Coord(topo_cart.enu.e, -topo_cart.enu.n, topo_cart.enu.u));

    // Note the minus operator at the northing: The reason is OMNeT++'s coordinate system. The origin is in the upper left corner,
    // the x-axis goes from west to east in the positiv direction and the y-axis goes from north to south in the positiv direction.
    // According to the figure at https://proj.org/operations/conversions/topocentric.html the enu.n-axis needs to be inverted.
    //
    // Further, the position of the SOP is added such that the satellite position is relative to OMNeT++'s origin.
    auto sop_omnet_coord = sop->get_sop_omnet_coord();
    inet::Coord satellitePosition(topo_cart.enu.e + sop_omnet_coord.x, -topo_cart.enu.n + sop_omnet_coord.y, topo_cart.enu.u + sop_omnet_coord.z);
    EV_TRACE << "SGP4Mobility new lastPosition: " << satellitePosition << std::endl;

    lastPosition = satellitePosition;
    vehicleStatistics->recordOmnetCoord(veins::Coord(lastPosition.x, lastPosition.y, lastPosition.z));
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
