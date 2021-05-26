//
// Copyright (C) 2006-2012 Christoph Sommer <christoph.sommer@uibk.ac.at>
// Copyright (C) 2021 Mario Franke <research@m-franke.net>
//
// Documentation for these modules is at http://veins.car2x.org/
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

#include "space_veins/space_veins.h"
#include "space_veins/base/utils/RelativeSatellitePosition.h"
#include "space_veins/base/satellitesConnectionManager/SatellitesConnectionManager.h"

#include "veins/base/modules/BaseMobility.h"
#include "veins/base/utils/FindModule.h"
#include "veins/modules/utility/TimerManager.h"
#include "veins/base/utils/Heading.h"
#include "veins/base/utils/Coord.h"
#include "veins/base/utils/FWMath.h"


namespace space_veins {

class VEINS_API SkyfieldMobility : public veins::BaseMobility {
public:
    SkyfieldMobility()
        : BaseMobility()
    {
    }
    ~SkyfieldMobility() override
    {
    }
    void initialize(int) override;
    void finish() override;

    void handleSelfMsg(cMessage* msg) override;

    void updateSatellitePosition();

    SatellitesConnectionManager* getSatellitesConnectionManager();

    veins::Heading skyfieldAzimuth2VeinsHeading(double azimuth_deg);

    veins::Coord getUnitDirectionVectorAltitude(double altitude_deg);

    veins::Coord getUnitDirectionVector(veins::Heading azimuth, veins::Coord unitDirectionVectorAltitude);

    // virtual void preInitialize(std::string external_id, const Coord& position, std::string road_id = "", double speed = -1, Heading heading = Heading::nan);
    // virtual void nextPosition(const Coord& position, std::string road_id = "", double speed = -1, Heading heading = Heading::nan, VehicleSignalSet signals = {VehicleSignal::undefined});
    // virtual void changePosition();
    // virtual void setExternalId(std::string external_id)
    // {
    //     this->external_id = external_id;
    // }
    // virtual std::string getExternalId() const
    // {
    //     if (external_id == "") throw cRuntimeError("SkyfieldMobility::getExternalId called with no external_id set yet");
    //     return external_id;
    // }

protected:
    // bool isPreInitialized; /**< true if preInitialize() has been called immediately before initialize() */

    // std::string external_id; /**< updated by setExternalId() */

    // simtime_t lastUpdate; /**< updated by nextPosition() */

    double updateInterval_ms;

    veins::TimerManager timerManager{this};

    SatellitesConnectionManager* scc;
    
    RelativeSatellitePosition rsp;

    veins::Heading heading;

    /**
     * Static reference position used when the position of the satellite has to be determined.
     */
    veins::Coord observerPosition;

    std::string satelliteName;

    /* Statistics */
    cOutVector distance_km_to_observationPosition_vec;
    cOutVector altitude_deg_to_observationPosition_vec;
    cOutVector azimuth_deg_to_observationPosition_vec;

    cOutVector currentPosXVec;
    cOutVector currentPosYVec;
    cOutVector currentPosZVec;

class VEINS_API SkyfieldMobilityAccess {
public:
    SkyfieldMobility* get(cModule* host)
    {
        SkyfieldMobility* skyfield = veins::FindModule<SkyfieldMobility*>::findSubModule(host);
        ASSERT(skyfield);
        return skyfield;
    };
};
};
} // namespace space_veins
