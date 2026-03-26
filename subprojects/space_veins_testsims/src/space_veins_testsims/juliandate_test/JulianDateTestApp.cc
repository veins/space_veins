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

#include <algorithm>
#include <cstring>
#include <iostream>

#include "inet/common/packet/Message.h"

#include "veins/modules/mobility/traci/TraCIMobility.h"

#include "space_veins/modules/mobility/SGP4Mobility/SGP4.h"
#include "space_veins/modules/mobility/SGP4Mobility/SGP4Mobility.h"
#include "space_veins/modules/statistics/GlobalStatistics/GlobalStatistics.h"

#include "space_veins_testsims/utils/asserts.h"
#include "space_veins_testsims/juliandate_test/JulianDateTestApp.h"

using namespace space_veins;

Define_Module(JulianDateTestApp);

void JulianDateTestApp::initialize(int stage)
{
    //Satellite Inserter uses stage 98
    if (stage == 98) {

        testNumber = par("testNumber");

        //findHost()->subscribe(BaseMobility::mobilityStateChangedSignal, this);
        EV_DEBUG << "JulianDateTestApp initialized with testNumber=" << testNumber << std::endl;

        performTest(0);
    }
}

void JulianDateTestApp::finish()
{
    performTest(simTime());
}

void JulianDateTestApp::handleLowerMsg(cMessage* msg)
{
    delete msg;
}

double JulianDateTestApp::roundTo(double value, double precision){
    return std::round(value/precision) * precision;
}

void JulianDateTestApp::performTest(const simtime_t t)
{    
    double jdFrac_02092025 = 0;
    double jd_02092025 = 0;
    SGP4Funcs::jday_SGP4(2025, 9, 2, 15, 45, 42, jd_02092025, jdFrac_02092025);

    //DD-MM-YYYY - HH:MM:SS
    assertEqual("02.09.2025 - 15:45:42", 2460921.156736, roundTo(jd_02092025+jdFrac_02092025, 0.000001));

    double jdFrac_31122024 = 0;
    double jd_31122024 = 0;
    SGP4Funcs::jday_SGP4(2024, 12, 31, 23, 59, 59, jd_31122024, jdFrac_31122024);
    assertEqual("31.12.2024 - 23:59:59", 2460676.499988, roundTo(jd_31122024+jdFrac_31122024, 0.000001));

    double jdFrac_01012025 = 0;
    double jd_01012025 = 0;
    SGP4Funcs::jday_SGP4(2025, 1, 1, 0, 0, 0, jd_01012025, jdFrac_01012025);
    assertEqual("01.01.2025 - 00:00:00", 2460676.500000, roundTo(jd_01012025+jdFrac_01012025, 0.000001));

    double jdFrac_29022024 = 0;
    double jd_29022024 = 0;
    SGP4Funcs::jday_SGP4(2024, 2, 29, 15, 45, 42, jd_29022024, jdFrac_29022024);
    assertEqual("29.02.2024 - 15:45:42", 2460370.156736, roundTo(jd_29022024+jdFrac_29022024, 0.000001));
}
