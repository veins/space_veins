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

#pragma once

#include <cstddef>
#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>

#include "space_veins/space_veins.h"
#include "space_veins/modules/statistics/VehicleStatistics/VehicleStatistics.h"

#include "inet/applications/udpapp/UdpSink.h"

using namespace space_veins;

class SPACE_VEINS_API Sgp4MobilityTestApp : public inet::UdpSink, cListener {

protected:
    int testNumber;
    std::string pathToSkyfieldGroundTruth;
    std::ifstream skyfieldFile;
    std::string pathToOmnetppCoordsGroundTruth;
    std::ifstream omnetppCoordsFile;

    double lastTemeX;
    double lastTemeY;
    double lastTemeZ;

    double lastItrfX;
    double lastItrfY;
    double lastItrfZ;

    double lastWgs84Lat;
    double lastWgs84Lon;
    double lastWgs84Alt;

    double lastOmnetppCoordX;
    double lastOmnetppCoordY;
    double lastOmnetppCoordZ;

    double skyfieldTemeX;
    double skyfieldTemeY;
    double skyfieldTemeZ;

    double skyfieldItrfX;
    double skyfieldItrfY;
    double skyfieldItrfZ;

    double skyfieldWgs84Lat;
    double skyfieldWgs84Lon;
    double skyfieldWgs84Alt;

    double groundTruthOmnetppCoordX;
    double groundTruthOmnetppCoordY;
    double groundTruthOmnetppCoordZ;

protected:
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void receiveSignal(cComponent* source, simsignal_t signalID, double value, cObject *details) override;
    void setSkyfieldVariables();
    void setGroundTruthOmnetppCoords();
    void runAsserts();

public:
    Sgp4MobilityTestApp();
    ~Sgp4MobilityTestApp();
};
