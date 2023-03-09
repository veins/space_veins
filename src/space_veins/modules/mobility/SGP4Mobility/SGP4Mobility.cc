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

void SGP4Mobility::initializePosition() 
{
    //updateSatellitePosition();
}

void SGP4Mobility::initialize(int stage)
{
    MovingMobilityBase::initialize(stage);
    if (stage == 0) {

        EV_DEBUG << "Initializing SGP4Mobility module." << std::endl;
        // Read wall_clock_sim_start_time_utc store it in wall_clock_start_time
        wall_clock_sim_start_time_utc = par("wall_clock_sim_start_time_utc").stringValue();
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

        // Store current wall clock time (wct) as std::chrono::system_clock::time_point
        std::tm tm = {};
        std::stringstream ss(wall_clock_sim_start_time_utc);
        ss >> std::get_time(&tm, "%Y-%m-%d-%H-%M-%S");
        wct = std::chrono::system_clock::from_time_t(std::mktime(&tm));

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

        // Read and store TLE data.
        tle = TLE(par("satelliteName").stringValue(),
                  par("tle_line_one").stringValue(),
                  par("tle_line_two").stringValue()
                  );

        // Initialize SGP4 model
        double startmfe = -1440;  // in min -> 1 day before epoch
        double stopmfe = 1440;  // in min -> 1 day after epoch
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

        // Calculate elapsed time beginning from the tle epoch
        std::tm tm2 = {};
        std::stringstream ss2;
        ss2 << tle_epoch.year << "-" << std::setfill('0') << std::setw(2) << tle_epoch.mon << "-" << tle_epoch.day << "-" << tle_epoch.hour << "-" << tle_epoch.min << "-" << tle_epoch.sec;
        EV_DEBUG << "tle_epoch: " << ss2.str() << std::endl;
        ss2 >> std::get_time(&tm2, "%Y-%m-%d-%H-%M-%S");
        auto ep = std::chrono::system_clock::from_time_t(std::mktime(&tm2)); // tle epoch

        std::time_t toPrint = std::chrono::system_clock::to_time_t(wct);
        EV_DEBUG << "Current wall clock time: wct: " << std::put_time(std::gmtime(&toPrint), "%c %Z") << std::endl;

        toPrint = std::chrono::system_clock::to_time_t(ep);
        EV_DEBUG << "TLE epoch: ep: " << std::put_time(std::gmtime(&toPrint), "%c %Z") << std::endl;

        wall_clock_since_tle_epoch_min = std::chrono::duration<double, std::chrono::minutes::period>(wct - ep);
        EV_DEBUG << "wall_clock_since_tle_epoch_min: wct - ep: " << wall_clock_since_tle_epoch_min.count() << " min" << std::endl;
    }
    else if (stage == 1) {
        // Get access to SOP
        sop = SatelliteObservationPointAccess().get();
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
    // t is the duration from tle epoch until current simTime() in minutes as required by SGP4Funcs::sgp4
    std::chrono::duration<double, std::chrono::minutes::period> t = wall_clock_since_tle_epoch_min + std::chrono::duration<double, std::chrono::seconds::period>(simTime().dbl());
    EV_DEBUG << "SGP4Mobility wall_clock_since_tle_epoch_min: " << wall_clock_since_tle_epoch_min.count() << " min" << std::endl;
    EV_DEBUG << "SGP4Mobility wall_clock_since_tle_epoch_min + simTime(): " << t.count() << " min" << std::endl;

    // Calculate satellite position using SGP4 in TEME coodinate system
    double r_array[3];
    double v_array[3];
    bool ret = SGP4Funcs::sgp4(satrec,
                            (double)t.count(),          // time since epoch in min
                            r_array,
                            v_array);
    // Copy results into a vector
    int n = sizeof(r_array) / sizeof(r_array[0]);
    std::vector<double> r_TEME(r_array, r_array + n);
    std::vector<double> v_TEME(v_array, v_array + n);

    // Calculate the current date_time_t which is required in order to calculate the date according to the Julian calendar
    // SGP4 Model needs the current SGP4Mobility::date_time_t: tle_epoch + t as date -> DD-MM-YYYY:HH-mm-SS.SSS
    // Transform duration t from minutes in needed period
    std::chrono::duration<double, std::chrono::milliseconds::period> tp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t);
    std::chrono::duration<double, std::chrono::seconds::period> tp_s = std::chrono::duration_cast<std::chrono::seconds>(t);
    std::chrono::duration<double, std::chrono::minutes::period> tp_min = std::chrono::duration_cast<std::chrono::minutes>(t);
    std::chrono::duration<double, std::chrono::hours::period> tp_hours = std::chrono::duration_cast<std::chrono::hours>(t);

    // Print epoch
    EV_TRACE << "SGP4Mobility tle_epoch: year: " << tle_epoch.year
             << ",  month: " << tle_epoch.mon
             << ",  day: " << tle_epoch.day
             << ",  hour: " << tle_epoch.hour
             << ",  minute: " << tle_epoch.min
             << ",  second: " << tle_epoch.sec
             << std::endl;

    double milliseconds_to_add = (double)((int)tp_ms.count() % 1000);           // add seconds, take care of carryover
    milliseconds_to_add = milliseconds_to_add / 1000;                           // transform milliseconds into seconds
    double seconds_to_add = (double)((int)tp_s.count() % 60);
    seconds_to_add += milliseconds_to_add;                                      // seconds to add including a fraction of milliseconds
    int minutes_to_add = 0;
    if (tle_epoch.sec + seconds_to_add >= 60) {                                 // take care of carryover
        minutes_to_add = 1;
        current_date_time.sec = tle_epoch.sec + seconds_to_add - 60;
    }else{
        current_date_time.sec = tle_epoch.sec + seconds_to_add;
    }

    minutes_to_add += ((int)tp_min.count() % 60);
    int hours_to_add = 0;
    if (tle_epoch.min + minutes_to_add >= 60) {                                 // add minutes, take care of carryover
        hours_to_add = 1;
        current_date_time.min = tle_epoch.min + minutes_to_add - 60;
    }else{
        current_date_time.min = tle_epoch.min + minutes_to_add;
    }

    hours_to_add += ((int)tp_hours.count() % 24);
    int days_to_add = 0;
    if (tle_epoch.hour + hours_to_add >= 24) {                                  // add hours, take care of carryover
        days_to_add = 1;
        current_date_time.hour = tle_epoch.hour + hours_to_add - 24;
    }else{
        current_date_time.hour = tle_epoch.hour + hours_to_add;
    }

    // TODO: simulations longer than a day are currently no supported because I do not know how long a day is
    int months_to_add = 0;
    int years_to_add = 0;
    if (days_to_add == 0) {
        current_date_time.day = tle_epoch.day;
        current_date_time.mon = tle_epoch.mon;
        current_date_time.year = tle_epoch.year;
    }else{
        // Days to add
        if  (tle_epoch.day + days_to_add > 31 && (tle_epoch.mon == 1 ||
                                                  tle_epoch.mon == 3 ||
                                                  tle_epoch.mon == 5 ||
                                                  tle_epoch.mon == 7 ||
                                                  tle_epoch.mon == 8 ||
                                                  tle_epoch.mon == 10 ||
                                                  tle_epoch.mon == 12 )) {
            current_date_time.day = 1;
            months_to_add = 1;
        }else if  (tle_epoch.day + days_to_add > 30 && (tle_epoch.mon == 4 ||
                                                  tle_epoch.mon == 6 ||
                                                  tle_epoch.mon == 9 ||
                                                  tle_epoch.mon == 11 )) {
            current_date_time.day = 1;
            months_to_add = 1;
        }else if ((tle_epoch.day + days_to_add > 28 && tle_epoch.mon == 2 && tle_epoch.year % 4 != 0) ||    // no leap year
                  (tle_epoch.day + days_to_add > 29 && tle_epoch.mon == 2 && tle_epoch.year % 4 == 0)) {    // leap year
            current_date_time.day = 1;
            months_to_add = 1;
        }else{  // no carryover
            current_date_time.day = tle_epoch.day + days_to_add;
        }

        // Months to add
        if (months_to_add != 0) {
            if (tle_epoch.mon + months_to_add > 12) {
                years_to_add = 1;
                current_date_time.mon = 1;
                current_date_time.day = 1;
            }
        }else{  // no carryover
            current_date_time.mon = tle_epoch.mon + months_to_add;
        }
        // Years to add
        current_date_time.year = tle_epoch.year + years_to_add;
    }
    EV_TRACE << "SGP4Mobility current_date_time: year: " << current_date_time.year
             << ",  month: " << current_date_time.mon
             << ",  day: " << current_date_time.day
             << ",  hour: " << current_date_time.hour
             << ",  minute: " << current_date_time.min
             << ",  second: " << current_date_time.sec
             << std::endl;

    // Calculate julian date_time for TEME to ITRF conversion
    double julian_day, julian_day_frac;
    SGP4Funcs::jday_SGP4(current_date_time.year,
                         current_date_time.mon,
                         current_date_time.day,
                         current_date_time.hour,
                         current_date_time.min,
                         current_date_time.sec,
                         julian_day,
                         julian_day_frac);

    julian_day += julian_day_frac;
    // Coordinate transformation TEME -> ITRF
    std::pair<std::vector<double>, std::vector<double>> itrf = TEME_to_ITRF(julian_day, r_TEME, v_TEME);
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

    emitMobilityStateChangedSignal();
    refreshDisplay();
}

void SGP4Mobility::finish()
{
    MovingMobilityBase::finish();
}
