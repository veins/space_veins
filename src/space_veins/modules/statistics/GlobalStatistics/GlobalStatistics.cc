//
// Copyright (C) 2021 Mario Franke <research@m-franke.net>
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "space_veins/modules/statistics/GlobalStatistics/GlobalStatistics.h"

Define_Module(space_veins::GlobalStatistics);

using namespace space_veins;

void GlobalStatistics::initialize(int stage)
{
    if (stage == 0) {
        EV_DEBUG << "GlobalStatistics initialized." << std::endl;

        WATCH(numTotalCars);
        WATCH(numRemovedCars);
        WATCH(numCurrentCars);

        vecCurrentCars.setName("vecCurrentCars:vector");
    }
}

void GlobalStatistics::finish() {
    recordScalar("numTotalCars:count", numTotalCars);
    recordScalar("numRemovedCars:count", numRemovedCars);
    recordScalar("numCurrentCars:count", numCurrentCars);
}

void GlobalStatistics::handleMessage(cMessage *msg)
{
    delete msg; // This module should not get any messages
}

GlobalStatistics::~GlobalStatistics() {
}

void GlobalStatistics::incrementTotalCars()
{
    numTotalCars += 1;
    numCurrentCars += 1;
    vecCurrentCars.record(numCurrentCars);
}

void GlobalStatistics::incrementRemovedCars()
{
    numRemovedCars += 1;
    numCurrentCars -= 1;
    vecCurrentCars.record(numCurrentCars);
}
