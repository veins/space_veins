//
// Copyright (C) 2026 Mario Franke <research@m-franke.net>
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

#include "space_veins_testsims/applications/JulianDateTestApp.h"
#include "space_veins_testsims/utils/asserts.h"

#include "space_veins/modules/mobility/SGP4Mobility/SGP4Mobility.h"
#include <math.h>

Define_Module(JulianDateTestApp);

JulianDateTestApp::~JulianDateTestApp()
{
}

void JulianDateTestApp::initialize(int stage)
{
    ApplicationBase::initialize(stage);
    if (stage == 0) {
        testNumber = par("testNumber");

        getSystemModule()->subscribe(SGP4Mobility::julianDateDaySignal, this);
        getSystemModule()->subscribe(SGP4Mobility::julianDateFracSignal, this);

        WATCH(testNumber);
    }
}

void JulianDateTestApp::handleMessageWhenUp(cMessage *msg)
{
    delete msg;
}

void JulianDateTestApp::handleStartOperation(inet::LifecycleOperation *operation)
{
}

void JulianDateTestApp::handleStopOperation(inet::LifecycleOperation *operation)
{
}

void JulianDateTestApp::handleCrashOperation(inet::LifecycleOperation *operation)
{
}


// ground truths obtained from https://ssd.jpl.nasa.gov/tools/jdc/#/cd
void JulianDateTestApp::receiveSignal(cComponent* source, simsignal_t signalID, double value, cObject *details)
{
    Enter_Method_Silent();
    // double simTimeDayFrac = simTime().dbl() / 86400.0;
    if (SGP4Mobility::julianDateDaySignal == signalID) {
        currentJulianDateDay = value;

        // check if translating the wall_clock_sim_start_time works
        if (simTime() == 0) {
            if (testNumber == 0) {
                assertClose("translating day of wall_clock_sim_start_time", 2459632.5, value);
            }
        }

        // check if translating the current_wall_clock_time works
        if (simTime() == 0.01) {
            if (testNumber == 0) {
                assertClose("Julian date day", 2459632.5, value);
            }
        }
        if (simTime() == 3600) {
            if (testNumber == 0) {
                assertClose("Julian date day", 2459633.5, value);
            }
        }
        if (simTime() == 3600.01) {
            if (testNumber == 0) {
                assertClose("Julian date day", 2459633.5, value);
            }
        }
        if (simTime() == 86400) {
            if (testNumber == 0) {
                assertClose("Julian date day", 2459633.5, value);
            }
        }
        if (simTime() == 86400.01) {
            if (testNumber == 0) {
                assertClose("Julian date day", 2459633.5, value);
            }
        }
        if (simTime() == 90000) {
            if (testNumber == 0) {
                assertClose("Julian date day", 2459634.5, value);
            }
        }
        if (simTime() == 90000.01) {
            if (testNumber == 0) {
                assertClose("Julian date day", 2459634.5, value);
            }
        }
        if (simTime() == 172800) {
            if (testNumber == 0) {
                assertClose("Julian date day", 2459634.5, value);
            }
        }
        if (simTime() == 172800.01) {
            if (testNumber == 0) {
                assertClose("Julian date day", 2459634.5, value);
            }
        }
    } else if (SGP4Mobility::julianDateFracSignal == signalID) {
        // check if translating the wall_clock_sim_start_time works
        if (simTime() == 0) {
            if (testNumber == 0) {
                assertClose("translating frac of wall_clock_sim_start_time", 0.9583333, value);
                assertClose("check full date", 2459633.4583333, currentJulianDateDay + value);
            }
        }

        // check if translating the current_wall_clock_time works
        if (simTime() == 0.01) {
            if (testNumber == 0) {
                assertClose("Julian date frac", 0.9583334, value);
                assertClose("check full date", 2459633.4583334, currentJulianDateDay + value);
            }
        }
        if (simTime() == 3600) {
            if (testNumber == 0) {
                assertClose("Julian date frac", 0.0, value);
                assertClose("check full date", 2459633.5, currentJulianDateDay + value);
            }
        }
        if (simTime() == 3600.01) {
            if (testNumber == 0) {
                assertClose("Julian date frac", 0.0000001, value);
                assertClose("check full date", 2459633.5000001, currentJulianDateDay + value);
            }
        }
        if (simTime() == 86400) {
            if (testNumber == 0) {
                assertClose("Julian date frac", 0.9583333, value);
                assertClose("check full date", 2459634.4583333, currentJulianDateDay + value);
            }
        }
        if (simTime() == 86400.01) {
            if (testNumber == 0) {
                assertClose("Julian date frac", 0.9583334, value);
                assertClose("check full date", 2459634.4583334, currentJulianDateDay + value);
            }
        }
        if (simTime() == 90000) {
            if (testNumber == 0) {
                assertClose("Julian date frac", 0.0, value);
                assertClose("check full date", 2459634.5, currentJulianDateDay + value);
            }
        }
        if (simTime() == 90000.01) {
            if (testNumber == 0) {
                assertClose("Julian date frac", 0.0000001, value);
                assertClose("check full date", 2459634.5000001, currentJulianDateDay + value);
            }
        }
        if (simTime() == 172800) {
            if (testNumber == 0) {
                assertClose("Julian date frac", 0.9583333, value);
                assertClose("check full date", 2459635.4583333, currentJulianDateDay + value);
            }
        }
        if (simTime() == 172800.01) {
            if (testNumber == 0) {
                assertClose("Julian date frac", 0.9583334, value);
                assertClose("check full date", 2459635.4583334, currentJulianDateDay + value);
            }
        }
    }
}

void JulianDateTestApp::finish()
{
    ApplicationBase::finish();
}
