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
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "space_veins/modules/statistics/VehicleStatistics/VehicleStatistics.h"
#include "space_veins_testsims/satpos_test/SatPosTestApp.h"

#include "space_veins_testsims/utils/asserts.h"
using space_veins::SatPosTestApp;

Define_Module(SatPosTestApp);

void SatPosTestApp::initialize(int stage)
{
    if (stage == 0) {
        testNumber = par("testNumber");
     
        // getSystemModule()->subscribe(VehicleStatistics::omnetCoordX, this);
        // getSystemModule()->subscribe(VehicleStatistics::omnetCoordY, this);
        // getSystemModule()->subscribe(VehicleStatistics::omnetCoordZ, this);

        //findHost()->subscribe(BaseMobility::mobilityStateChangedSignal, this);
        EV_DEBUG << "SatPosTestApp initialized with testNumber=" << testNumber << std::endl;

        std::ifstream file("validateSatPos.csv");

        if (!file.is_open()) {
            std::cerr << "Cannot open file: " << std::endl;
            return;
        }

        //remove header
        std::string text;
        std::getline(file, text);

        //read file
        std::stringstream buffer;
        buffer << file.rdbuf();
        text = buffer.str();

        //seperate values
        std::vector<std::string> csvValues;
        size_t start = text.find_first_not_of(" \t\n");
        while (start != std::string::npos) {
            size_t end = text.find_first_of(" \t\n", start);
            csvValues.push_back(text.substr(start, end - start));
            start = text.find_first_not_of(" \t\n", end);
        }

        //keep only omnetCoords
        int stage = 0;
        for (const std::string& file_coord : csvValues) {            
            switch (stage){
                case 0: break;                                                          //second
                case 1: break;                                                          //satellite
                case 2:                                                                 //omnetCoordX
                case 3:                                                                 //omnetCoordY
                case 4: file_coords.insert(file_coords.begin(), std::stod(file_coord)); //omnetCoordZ
            }
            
            stage = (stage + 1) % 5;       
        }    

        file.close();
    }
}

void SatPosTestApp::finish()
{
    performTest(999);
}

void SatPosTestApp::handleSelfMsg(cMessage* msg)
{
}

void SatPosTestApp::handleLowerMsg(cMessage* msg)
{
    delete msg;
}

void SatPosTestApp::receiveSignal(cComponent* source, simsignal_t signalID, double coord, cObject* details)
{
    //ignore values during warmup period
    if(simTime().dbl() < 10){
        return;
    }

    currentCoord = coord;
    performTest(simTime());
}

void SatPosTestApp::performTest(const simtime_t t)
{   
    if(testCounter < 273){
        std::string msg = "Check Satellite Position ";
        assertClose(msg.append(std::to_string(testCounter)), file_coords.back(), currentCoord);
        if(file_coords.size() > 0){
            file_coords.pop_back();
        } else {
            assertTrue("No Element in Position Vector!", false);
        }
    }
    
    if(t == 999){
        assertEqual("Remaining unchecked values", 0, file_coords.size());

        //91 seconds simulation time (without warmup period)
        assertEqual("Testcounter", 91*3, testCounter);
    }

    testCounter = testCounter +1;

    //
    // End
    //
}
