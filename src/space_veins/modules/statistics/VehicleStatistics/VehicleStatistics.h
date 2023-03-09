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

#pragma once

#include <proj.h>

#include "veins/veins.h"

#include "veins/base/utils/FindModule.h"
#include "veins/base/utils/Coord.h"

#include "space_veins/space_veins.h"
#include "space_veins/modules/utility/WGS84Coord.h"

using namespace omnetpp;

namespace space_veins {
class SPACE_VEINS_API VehicleStatistics : public cSimpleModule
{
    protected:
        virtual void initialize(int stage) override;
        virtual void finish() override;
        virtual void handleMessage(cMessage *msg) override;
        ~VehicleStatistics();

        // Mobility
        static const simsignal_t omnetCoordX;
        static const simsignal_t omnetCoordY;
        static const simsignal_t omnetCoordZ;

        static const simsignal_t wgs84CoordLat;
        static const simsignal_t wgs84CoordLon;
        static const simsignal_t wgs84CoordAlt;

        static const simsignal_t wgs84CartCoordX;
        static const simsignal_t wgs84CartCoordY;
        static const simsignal_t wgs84CartCoordZ;

        static const simsignal_t sopRelativeCoordX;
        static const simsignal_t sopRelativeCoordY;
        static const simsignal_t sopRelativeCoordZ;

        // Application layer

        static const simsignal_t sendSatellitePackets;
        static const simsignal_t receivedSatellitePackets;

        static const simsignal_t sendSatelliteRegistrationMessages;
        static const simsignal_t receivedSatelliteRegistrationMessages;

        static const simsignal_t missedSatelliteRegistrationAcks;
        static const simsignal_t receivedSatelliteRegistrationAcks;

        static const simsignal_t sendWlanPackets;
        static const simsignal_t receivedWlanPackets;

        static const simsignal_t sendWlanSchedules;
        static const simsignal_t receivedWlanSchedules;

        static const simsignal_t applLayerRoundTripTime;
        static const simsignal_t applLayerSatelliteLatency;

        static const simsignal_t registeredVehiclesPerSchedule;

        static const simsignal_t wlanPacketEndToEndDelay;

    public:
        void recordOmnetCoord(const veins::Coord c);
        void recordWGS84Coord(const space_veins::WGS84Coord c);
        void recordWGS84CartCoord(const PJ_COORD c);
        void recordSopRelativeCoord(const veins::Coord c);

        void recordSendSatellitePackets();
        void recordReceivedSatellitePackets();

        void recordSendWlanPackets();
        void recordReceivedWlanPackets();

        void recordSendSatelliteRegistrationMessages();
        void recordReceivedSatelliteRegistrationMessages();

        void recordMissedSatelliteRegistrationAcks();
        void recordReceivedSatelliteRegistrationAcks();

        void recordSendWlanSchedules();
        void recordReceivedWlanSchedules();

        void recordApplLayerRoundTripTime(simtime_t rtt);
        void recordApplLayerSatelliteLatency(simtime_t lat);

        void recordRegisteredVehiclesPerSchedule(const long count);

        void recordWlanPacketEndToEndDelay(const simtime_t delay);
};
}

namespace space_veins {
class SPACE_VEINS_API VehicleStatisticsAccess {
public:
    VehicleStatistics* get(cModule* host)
    {
        VehicleStatistics* vs = veins::FindModule<VehicleStatistics*>::findSubModule(host);
        ASSERT(vs);
        return vs;
    };
};
}
