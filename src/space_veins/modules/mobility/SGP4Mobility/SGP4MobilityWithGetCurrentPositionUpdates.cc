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

#include "space_veins/modules/mobility/SGP4Mobility/SGP4MobilityWithGetCurrentPositionUpdates.h"

using namespace space_veins;

Define_Module(space_veins::SGP4MobilityWithGetCurrentPositionUpdates);

void SGP4MobilityWithGetCurrentPositionUpdates::initialize(int stage)
{
    EV_DEBUG << "SGP4MobilityWithGetCurrentPositionUpdates stage: " << stage << std::endl;
    SGP4MobilityBase::initialize(stage);
    return;
}

bool SGP4MobilityWithGetCurrentPositionUpdates::nextGetCurrentPositionUpdate() {
    EV_TRACE << "SGP4MobilityWithGetCurrentPositionUpdates::nextGetCurrentPositionUpdate() called" << std::endl;
    simtime_t now = simTime();
    SGP4PositionUpdate(now);
    // Management
    bool updated = true;
    updateManagementDataAndStatistics(getCurrentPositionUpdate, updated);
    return updated;
}

bool SGP4MobilityWithGetCurrentPositionUpdates::nextPeriodicPositionUpdate() {
    EV_TRACE << "SGP4MobilityWithGetCurrentPositionUpdates::nextPeriodicPositionUpdate() called" << std::endl;
    simtime_t now = simTime();
    SGP4PositionUpdate(now);
    // Management
    bool updated = true;
    updateManagementDataAndStatistics(periodicUpdate, updated);
    return updated;
}

void SGP4MobilityWithGetCurrentPositionUpdates::finish()
{
    SGP4MobilityBase::finish();
}