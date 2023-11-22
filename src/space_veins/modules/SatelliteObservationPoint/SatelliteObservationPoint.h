//
// Copyright (C) 2021 Mario Franke <research@m-franke.net>
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
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

#include <string>
#include <sstream>
#include <utility>
#include <memory>

#include <proj.h>

#include "inet/mobility/static/StationaryMobility.h"

#include "veins/base/utils/FindModule.h"
#include "veins/base/utils/Coord.h"
#include "veins/modules/mobility/traci/TraCICoord.h"
#include "veins/modules/mobility/traci/TraCICoordinateTransformation.h"
#include "veins/modules/world/annotations/AnnotationManager.h"

#include "space_veins/modules/mobility/SGP4Mobility/constants.h"
#include "space_veins/modules/utility/WGS84Coord.h"

namespace space_veins {

class SPACE_VEINS_API SatelliteObservationPoint : public cSimpleModule, public cListener {

public:

    PJ_COORD get_sop_wgs84_proj_cart() const;

    WGS84Coord get_sop_wgs84() const;

    veins::TraCICoord get_sop_utm_coord() const;

    veins::TraCICoord get_sop_traci_coord() const;

    veins::Coord get_sop_omnet_coord() const;

    PJ* provideSumoToWGS84Projection() const;

    PJ* provideWGS84ToWGS84CartesianProjection() const;

    PJ* provideRelativeToSOPProjection() const;

    veins::TraCICoord omnet2UTM(const veins::Coord toTransfer) const;

    WGS84Coord omnet2WGS84(const veins::Coord toTransfer) const;

    PJ_COORD omnet2WGS84Cartesian(const veins::Coord toTransfer) const;

    veins::Coord omnetRelativeSOPCoord(const veins::Coord toTransfer) const;


protected:
    inet::StationaryMobility* mobility;
    veins::AnnotationManager* annotations;
    veins::Coord sop_omnet_coord;
    veins::TraCICoord sop_traci_coord;    // SUMO coordinate
    veins::TraCICoord sop_utm_coord;      // UTM coordinate: sumo coordinates + sumo netOffset
    WGS84Coord sop_wgs84;
    PJ_COORD sop_wgs84_proj_cart;

    veins::Coord netOffset;

    // proj context
    PJ_CONTEXT* pj_ctx;
    PJ* sumo_to_wgs84_projection;
    PJ* wgs84_to_wgs84cartesian_projection;
    PJ* relativeToSOP_projection;

    void initialize(int stage) override;

    int numInitStages() const override
    {
        return std::max(cSimpleModule::numInitStages(), 4);
    }

    std::string getProjectionString(const cXMLElement* sumoNetXmlFile) const;

    std::string getConvBoundaryString(const cXMLElement* sumoNetXmlFile) const;

    std::pair<veins::TraCICoord, veins::TraCICoord> convBoundary2TraCICoords(const std::string cb) const;

    std::string getNetOffsetString(const cXMLElement* sumoNetXmlFile) const;

    veins::Coord netOffsetString2Coord(const std::string no) const;

    virtual void handleMessage(cMessage* message) override;

    virtual void handleSelfMessage(cMessage* message);

private:
    std::unique_ptr<veins::TraCICoordinateTransformation> coordinateTransformation;

};

class SPACE_VEINS_API SatelliteObservationPointAccess {
public:
    SatelliteObservationPoint* get()
    {
        return veins::FindModule<SatelliteObservationPoint*>::findGlobalModule();
    };
};

} //end namespace space_veins
