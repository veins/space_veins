//
// Copyright (C) 2006-2018 Christoph Sommer <sommer@ccs-labs.org>
// Copyright (C) 2022 Mario Franke <research@m-franke.net>
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

//
// Veins Mobility module for the INET Framework
// Based on veins_inet::VeinsInetMobility
//

#include "space_veins/modules/mobility/VeinsInetMobilityWGS84/VeinsInetMobilityWGS84.h"

using namespace space_veins;

Define_Module(space_veins::VeinsInetMobilityWGS84);

VeinsInetMobilityWGS84::VeinsInetMobilityWGS84()
{
}

VeinsInetMobilityWGS84::~VeinsInetMobilityWGS84()
{
}

void VeinsInetMobilityWGS84::initialize(int stage)
{
    VeinsInetMobility::initialize(stage);
    if (stage == 1) {
        // Get access to SOP
        sop = SatelliteObservationPointAccess().get();
        vehicleStatistics = VehicleStatisticsAccess().get(getParentModule());
    }
}

void VeinsInetMobilityWGS84::nextPosition(const inet::Coord& position, std::string road_id, double speed, double angle)
{
    Enter_Method_Silent();
    VeinsInetMobility::nextPosition(position, road_id, speed, angle);

    EV_TRACE << getFullPath() << ", simTime(): " << simTime() << ", next position:" << std::endl;

    veins::Coord vehiclePositionOmnet = veins::Coord(lastPosition.x, lastPosition.y, lastPosition.z);
    vehicleStatistics->recordOmnetCoord(vehiclePositionOmnet);
    EV_TRACE << "lastPosition: " << lastPosition << std::endl;

    WGS84Coord vehicle_WGS84_coord = sop->omnet2WGS84(vehiclePositionOmnet);
    vehicleStatistics->recordWGS84Coord(vehicle_WGS84_coord);

    PJ_COORD vehicle_WGS84_cart_coord = sop->omnet2WGS84Cartesian(vehiclePositionOmnet);
    vehicleStatistics->recordWGS84CartCoord(vehicle_WGS84_cart_coord);
    EV_TRACE << "vehicle_WGS84_cart_coord: x: " << vehicle_WGS84_cart_coord.xyz.x << ", y: " << vehicle_WGS84_cart_coord.xyz.y << ", z: " << vehicle_WGS84_cart_coord.xyz.z << std::endl;

    veins::Coord relativeSOPCoord = sop->omnetRelativeSOPCoord(vehiclePositionOmnet);
    vehicleStatistics->recordSopRelativeCoord(relativeSOPCoord);

    EV_TRACE << "relativeSOPCoord: " << relativeSOPCoord << std::endl;

}

void VeinsInetMobilityWGS84::handleSelfMessage(cMessage* message)
{
    VeinsInetMobility::handleSelfMessage(message);
}
