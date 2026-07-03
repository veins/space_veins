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

#include "space_veins_testsims/applications/Sgp4MobilityTestApp.h"
#include "space_veins/modules/statistics/VehicleStatistics/VehicleStatistics.h"
#include "space_veins_testsims/utils/asserts.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include <string>
#include <string_view>

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

        getParentModule()->subscribe(VehicleStatistics::temeCoordX, this);
        getParentModule()->subscribe(VehicleStatistics::temeCoordY, this);
        getParentModule()->subscribe(VehicleStatistics::temeCoordZ, this);

        getParentModule()->subscribe(VehicleStatistics::itrfCoordX, this);
        getParentModule()->subscribe(VehicleStatistics::itrfCoordY, this);
        getParentModule()->subscribe(VehicleStatistics::itrfCoordZ, this);

        getParentModule()->subscribe(VehicleStatistics::wgs84CoordLat, this);
        getParentModule()->subscribe(VehicleStatistics::wgs84CoordLon, this);
        getParentModule()->subscribe(VehicleStatistics::wgs84CoordAlt, this);

        getParentModule()->subscribe(VehicleStatistics::omnetCoordX, this);
        getParentModule()->subscribe(VehicleStatistics::omnetCoordY, this);
        getParentModule()->subscribe(VehicleStatistics::omnetCoordZ, this);

        skyfieldFile.open(pathToSkyfieldGroundTruth, std::ios::in);
        std::string header;
        std::getline(skyfieldFile, header);

        omnetppCoordsFile.open(pathToOmnetppCoordsGroundTruth, std::ios::in);
        std::getline(omnetppCoordsFile, header);
    }
}

void Sgp4MobilityTestApp::receiveSignal(cComponent* source, simsignal_t signalID, double value, cObject *details)
{
    Enter_Method_Silent();
    if (VehicleStatistics::wgs84CoordLat == signalID) {
        lastWgs84Lat = value;
    } else if (VehicleStatistics::wgs84CoordLon == signalID) {
        lastWgs84Lon = value;
    } else if (VehicleStatistics::wgs84CoordAlt == signalID) {
        lastWgs84Alt = value;
        // get ground truth and compare it
        setSkyfieldVariables();
        runAsserts();
    } else if (VehicleStatistics::temeCoordX == signalID) {
        lastTemeX = value;
    } else if (VehicleStatistics::temeCoordY == signalID) {
        lastTemeY = value;
    } else if (VehicleStatistics::temeCoordZ == signalID) {
        lastTemeZ = value;
    } else if (VehicleStatistics::itrfCoordX == signalID) {
        lastItrfX = value;
    } else if (VehicleStatistics::itrfCoordY == signalID) {
        lastItrfY = value;
    } else if (VehicleStatistics::itrfCoordZ == signalID) {
        lastItrfZ = value;
    } else if (VehicleStatistics::omnetCoordX == signalID) {
        lastOmnetppCoordX = value;
    } else if (VehicleStatistics::omnetCoordY == signalID) {
        lastOmnetppCoordY = value;
    } else if (VehicleStatistics::omnetCoordZ == signalID) {
        lastOmnetppCoordZ = value;
        // get ground truth and compare it
        setGroundTruthOmnetppCoords();
        assertClose("omnetppCoordX", groundTruthOmnetppCoordX, lastOmnetppCoordX);
        assertClose("omnetppCoordY", groundTruthOmnetppCoordY, lastOmnetppCoordY);
        assertClose("omnetppCoordZ", groundTruthOmnetppCoordZ, lastOmnetppCoordZ);
    }
}

void Sgp4MobilityTestApp::runAsserts() {
    assertClose("temeX", skyfieldTemeX, lastTemeX);
    assertClose("temeY", skyfieldTemeY, lastTemeY);
    assertClose("temeZ", skyfieldTemeZ, lastTemeZ);

    assertClose("itrfX", skyfieldItrfX, lastItrfX);
    assertClose("itrfY", skyfieldItrfY, lastItrfY);
    assertClose("itrfZ", skyfieldItrfZ, lastItrfZ);

    assertClose("latitude", skyfieldWgs84Lat, lastWgs84Lat);
    assertClose("longitude", skyfieldWgs84Lon, lastWgs84Lon);
    assertClose("altitude", skyfieldWgs84Alt, lastWgs84Alt, 0.001); // fail if difference is greater than a millimeter
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

    // get time_offset_s
    double time_offset_s;
    getline(line, token, ',');
    EV_TRACE << "Sgp4MobilityTestApp string_time_offset_s: " << token << std::endl;
    time_offset_s = std::stod(token);
    // check time_offset is equal to simTime()
    if (std::abs(simTime().dbl() - time_offset_s) > 0.0000001) {
        throw cRuntimeError("Sgp4MobilityTestApp: time_offset_s != simTime().dbl()");
    }

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

    // event number
    std::getline(line, token, '\t');
    EV_TRACE << "Sgp4MobilityTestApp string event number: " << token << std::endl;

    // time
    std::getline(line, token, '\t');
    EV_TRACE << "Sgp4MobilityTestApp string simulation time: " << token << std::endl;
    double groundTruthSimTime = std::stod(token);
    // check time_offset is equal to simTime()
    if (std::abs(simTime().dbl() - groundTruthSimTime) > 0.0000001) {
        throw cRuntimeError("Sgp4MobilityTestApp: groundTruthSimTime != simTime().dbl()");
    }

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
