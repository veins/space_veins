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

#include <cmath>
#include <cstdio>
#include <sstream>
#include <string>
#include <string_view>

#include "space_veins/modules/mobility/SGP4Mobility/SGP4MobilityBase.h"
#include "space_veins_testsims/applications/Sgp4MobilityTestApp.h"
#include "space_veins_testsims/utils/asserts.h"

#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/mobility/contract/IMobility.h"

Define_Module(Sgp4MobilityTestApp);

void Sgp4MobilityTestApp::initialize(int stage)
{
    UdpSink::initialize(stage);
    if (stage == 0) {
        WATCH(testNumber);
        WATCH(pathToSkyfieldGroundTruth);
        WATCH(pathToOmnetppCoordsGroundTruth);

        testNumber = par("testNumber");
        pathToSkyfieldGroundTruth = par("pathToSkyfieldGroundTruth").stdstringValue();
        pathToOmnetppCoordsGroundTruth = par("pathToOmnetppCoordsGroundTruth").stdstringValue();

        getParentModule()->subscribe(inet::IMobility::mobilityStateChangedSignal, this);

        skyfieldFile.open(pathToSkyfieldGroundTruth, std::ios::in);
        std::string header;
        std::getline(skyfieldFile, header);

        omnetppCoordsFile.open(pathToOmnetppCoordsGroundTruth, std::ios::in);
        std::getline(omnetppCoordsFile, header);
    }
}

void Sgp4MobilityTestApp::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject *details)
{
    Enter_Method_Silent();
    if (inet::IMobility::mobilityStateChangedSignal == signalID) {
        SGP4MobilityBase* smb = check_and_cast<SGP4MobilityBase*>(obj);
        // get ground truth and compare it
        setGroundTruthOmnetppCoords();
        setSkyfieldVariables();
        runAsserts(smb->getLastCalculatedSgp4SatellitePosition());
    }
}

void Sgp4MobilityTestApp::runAsserts(SGP4MobilityBase::calculatedSgp4SatellitePositions_t pos) {
    // assertTrue("UTC time", skyfieldUtcTime == pos.wallClockTimeUtc);
    assertClose("Skyfield simTime offset", skyfieldSimTimeOffset_s, simTime().dbl());

    assertClose("temeX", skyfieldTemeX, pos.teme.x);
    assertClose("temeY", skyfieldTemeY, pos.teme.y);
    assertClose("temeZ", skyfieldTemeZ, pos.teme.z);

    assertClose("itrfX", skyfieldItrfX, pos.itrf2008.x / 1000); // conversion to kilometer
    assertClose("itrfY", skyfieldItrfY, pos.itrf2008.y / 1000);
    assertClose("itrfZ", skyfieldItrfZ, pos.itrf2008.z / 1000);

    assertClose("latitude", skyfieldWgs84Lat, pos.wgs84.lat);
    assertClose("longitude", skyfieldWgs84Lon, pos.wgs84.lon);
    assertClose("altitude", skyfieldWgs84Alt, pos.wgs84.alt, 0.001); // fail if difference is greater than a millimeter

    assertClose("omnetpp simTime offset", groundTruthOmnetppSimTimeOffset_s, simTime().dbl());
    assertClose("omnetppCoordX", groundTruthOmnetppCoordX, pos.omnetppCoord.x);
    assertClose("omnetppCoordY", groundTruthOmnetppCoordY, pos.omnetppCoord.y);
    assertClose("omnetppCoordZ", groundTruthOmnetppCoordZ, pos.omnetppCoord.z);
}

void Sgp4MobilityTestApp::setSkyfieldVariables()
{
    // read line
    std::string token;
    std::getline(skyfieldFile, token);
    std::stringstream line(token);

    // get time_utc
    std::string time_utc;
    getline(line, time_utc, ',');
    EV_TRACE << "Sgp4MobilityTestApp time_utc: " << time_utc << std::endl;
    std::stringstream sUtc(time_utc);
    std::string utcDay, utcTime;
    //  parse day
    getline(sUtc, utcDay, ' ');
    int year, mon, day;
    std::stringstream sDate(utcDay);
    getline(sDate, token, '-');
    year = std::stoi(token);
    getline(sDate, token, '-');
    mon = std::stoi(token);
    getline(sDate, token, '-');
    day = std::stoi(token);
    //  parse time
    getline(sUtc, utcTime, ' ');
    int hour, min;
    double sec;
    std::stringstream sTime(utcTime);
    getline(sTime, token, ':');
    hour = std::stoi(token);
    getline(sTime, token, ':');
    min = std::stoi(token);
    getline(sTime, token, ':');
    sec = std::stod(token);
    sec = std::ceil(sec * 100) / 100;   // ceil to two decimal places
    skyfieldUtcTime.year = year;
    skyfieldUtcTime.mon = mon;
    skyfieldUtcTime.day = day;
    skyfieldUtcTime.hour = hour;
    skyfieldUtcTime.min = min;
    skyfieldUtcTime.sec = sec;

    // get time_offset_s
    getline(line, token, ',');
    EV_TRACE << "Sgp4MobilityTestApp string_time_offset_s: " << token << std::endl;
    skyfieldSimTimeOffset_s = std::stod(token);

    // get teme_x_km
    getline(line, token, ',');
    EV_TRACE << "Sgp4MobilityTestApp string teme_x_km: " << token << std::endl;
    skyfieldTemeX = std::stod(token);
    // get teme_y_km
    getline(line, token, ',');
    EV_TRACE << "Sgp4MobilityTestApp string teme_y_km: " << token << std::endl;
    skyfieldTemeY = std::stod(token);
    // get teme_z_km
    getline(line, token, ',');
    EV_TRACE << "Sgp4MobilityTestApp string teme_z_km: " << token << std::endl;
    skyfieldTemeZ = std::stod(token);

    // get itrf_x_km
    getline(line, token, ',');
    EV_TRACE << "Sgp4MobilityTestApp string itrf_x_km: " << token << std::endl;
    skyfieldItrfX = std::stod(token);
    // get itrf_y_km
    getline(line, token, ',');
    EV_TRACE << "Sgp4MobilityTestApp string itrf_y_km: " << token << std::endl;
    skyfieldItrfY = std::stod(token);
    // get itrf_z_km
    getline(line, token, ',');
    EV_TRACE << "Sgp4MobilityTestApp string itrf_z_km: " << token << std::endl;
    skyfieldItrfZ = std::stod(token);

    // get latitude_deg
    getline(line, token, ',');
    EV_TRACE << "Sgp4MobilityTestApp string WGS84 latitude_deg: " << token << std::endl;
    skyfieldWgs84Lat = std::stod(token);
    // get longitude_deg
    getline(line, token, ',');
    EV_TRACE << "Sgp4MobilityTestApp string WGS84 longitude_deg: " << token << std::endl;
    skyfieldWgs84Lon = std::stod(token);
    // get altitude
    getline(line, token, ',');
    EV_TRACE << "Sgp4MobilityTestApp string WGS84 altitude_m: " << token << std::endl;
    skyfieldWgs84Alt = std::stod(token);
}

void Sgp4MobilityTestApp::setGroundTruthOmnetppCoords()
{
    // read line
    std::string token;
    std::getline(omnetppCoordsFile, token);
    std::stringstream line(token);

    // time
    std::getline(line, token, '\t');
    EV_TRACE << "Sgp4MobilityTestApp string simulation time: " << token << std::endl;
    groundTruthOmnetppSimTimeOffset_s = std::stod(token);

    // node
    std::getline(line, token, '\t');
    EV_TRACE << "Sgp4MobilityTestApp string node: " << token << std::endl;

    // omnetCoordX
    std::getline(line, token, '\t');
    EV_TRACE << "Sgp4MobilityTestApp string omnetCoordX: " << token << std::endl;
    groundTruthOmnetppCoordX = std::stod(token);

    // omnetCoordY
    std::getline(line, token, '\t');
    EV_TRACE << "Sgp4MobilityTestApp string omnetCoordY: " << token << std::endl;
    groundTruthOmnetppCoordY = std::stod(token);

    // omnetCoordZ
    std::getline(line, token, '\t');
    EV_TRACE << "Sgp4MobilityTestApp string omnetCoordZ: " << token << std::endl;
    groundTruthOmnetppCoordZ = std::stod(token);
}

void Sgp4MobilityTestApp::finish()
{
    UdpSink::finish();
    skyfieldFile.close();
    omnetppCoordsFile.close();
}

Sgp4MobilityTestApp::Sgp4MobilityTestApp() {}

Sgp4MobilityTestApp::~Sgp4MobilityTestApp() {}
