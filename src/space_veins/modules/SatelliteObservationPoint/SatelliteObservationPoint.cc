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

#include "space_veins/modules/SatelliteObservationPoint/SatelliteObservationPoint.h"

using namespace space_veins;

Define_Module(space_veins::SatelliteObservationPoint)

std::string SatelliteObservationPoint::getProjectionString(const cXMLElement* sumoNetXmlFile) const
{
    if (sumoNetXmlFile == nullptr) {
        throw cRuntimeError("No .net.xml file specified.");
    }

    cXMLElementList locationList = sumoNetXmlFile->getElementsByTagName("location");

    if (locationList.empty()) {
        throw cRuntimeError("No location tag found in .net.xml file.");
    }

    if (locationList.size() > 1) {
        throw cRuntimeError("More than one location tag found in .net.xml file.");
    }

    cXMLElement* location = locationList.front();
    return std::string(location->getAttribute("projParameter"));
}

std::string SatelliteObservationPoint::getConvBoundaryString(const cXMLElement* sumoNetXmlFile) const
{
    if (sumoNetXmlFile == nullptr) {
        throw cRuntimeError("No .net.xml file specified.");
    }

    cXMLElementList locationList = sumoNetXmlFile->getElementsByTagName("location");

    if (locationList.empty()) {
        throw cRuntimeError("No location tag found in .net.xml file.");
    }

    if (locationList.size() > 1) {
        throw cRuntimeError("More than one location tag found in .net.xml file.");
    }

    cXMLElement* location = locationList.front();
    return std::string(location->getAttribute("convBoundary"));
}

std::pair<veins::TraCICoord, veins::TraCICoord> SatelliteObservationPoint::convBoundary2TraCICoords(const std::string cb) const
{
    std::vector<std::string> coords;
    std::istringstream iss(cb);
    for (std::string s; std::getline(iss, s, ','); ) {
        coords.push_back(s);
    }
    veins::TraCICoord nb1(std::stod(coords[0]), std::stod(coords[1]));
    veins::TraCICoord nb2(std::stod(coords[2]), std::stod(coords[3]));
    EV_DEBUG << "SOP: network boundaries (" << nb1.x << ", " << nb1.y << ")-(" << nb2.x << ", " << nb2.y << ")" << std::endl;
    return std::pair<veins::TraCICoord, veins::TraCICoord>{nb1, nb2};
}

std::string SatelliteObservationPoint::getNetOffsetString(const cXMLElement* sumoNetXmlFile) const
{
    if (sumoNetXmlFile == nullptr) {
        throw cRuntimeError("No .net.xml file specified.");
    }

    cXMLElementList locationList = sumoNetXmlFile->getElementsByTagName("location");

    if (locationList.empty()) {
        throw cRuntimeError("No location tag found in .net.xml file.");
    }

    if (locationList.size() > 1) {
        throw cRuntimeError("More than one location tag found in .net.xml file.");
    }

    cXMLElement* location = locationList.front();
    return std::string(location->getAttribute("netOffset"));
}

veins::Coord SatelliteObservationPoint::netOffsetString2Coord(const std::string no) const
{
    std::vector<std::string> coords;
    std::istringstream iss(no);
    for (std::string s; std::getline(iss, s, ','); ) {
        coords.push_back(s);
    }
    veins::Coord offset(std::stod(coords[0]), std::stod(coords[1]));
    return offset;
}

void SatelliteObservationPoint::handleMessage(cMessage* message)
{
    if (message->isSelfMessage()) {
        handleSelfMessage(message);
    }else{
        EV_DEBUG << "SOP: Error received cMessage that is not self message." << std::endl;
        delete message;
    }
}

void SatelliteObservationPoint::handleSelfMessage(cMessage* message)
{
    switch (message->getKind()) {
    case 0: {
        // draw SOP in SUMO
        annotations->drawPoint(sop_omnet_coord, "red", "SOP");
        EV_TRACE << "SOP: drawn SOP in SUMO at veins::Coord: " << sop_omnet_coord << std::endl;
        break;
    }
    }
}

void SatelliteObservationPoint::initialize(int stage)
{
    if (stage == 0) {
        // get Mobility
        mobility = static_cast<inet::StationaryMobility*>(getSubmodule("mobility"));
        ASSERT(mobility);
        // get AnnotationManager
        annotations = veins::AnnotationManagerAccess().getIfExists();
        ASSERT(annotations);

        cMessage *drawPosition = new cMessage("drawMessage", 0);
        scheduleAt(simTime() + 0.01, drawPosition);
    }
    if (stage == 3) {
        // Initialize projections after lastPosition of the mobility submodule is initialized
        std::string sumo_proj = getProjectionString(par("sumoNetXmlFile").xmlValue());
        EV_DEBUG << "SOP: sumo_proj: " << sumo_proj << std::endl;

        // create projection context
        pj_ctx = proj_context_create();

        // create sumo_to_wgs84_projection
        sumo_to_wgs84_projection = proj_create_crs_to_crs(
                pj_ctx,
                sumo_proj.c_str(),   // from UTM of SUMO
                "EPSG:4326",    // to WGS84
                NULL);
        if (sumo_to_wgs84_projection == 0) {
            std::stringstream err_msg;
            err_msg << "sumo_to_wgs84_projection initialization error: proj_errno: " << proj_errno(sumo_to_wgs84_projection);
            throw cRuntimeError(err_msg.str().c_str());
        }
        // fix longitude, latitude order, see https://proj.org/development/quickstart.html
        sumo_to_wgs84_projection = proj_normalize_for_visualization(pj_ctx, sumo_to_wgs84_projection);
        if (sumo_to_wgs84_projection == 0) {
            std::stringstream err_msg;
            err_msg << "sumo_to_wgs84_projection normalization error: proj_errno: " << proj_errno(sumo_to_wgs84_projection);
            throw cRuntimeError(err_msg.str().c_str());
        }

        // create wgs84_to_wgs84cartesian_projection
        wgs84_to_wgs84cartesian_projection = proj_create(
                pj_ctx,
                "+proj=pipeline +step proj=cart +ellps=WGS84");    // from WGS84 geodetic to cartesian

        // Set satellite observer position (SOP)
        sop_omnet_coord = veins::Coord(
                            mobility->getCurrentPosition().x,
                            mobility->getCurrentPosition().y,
                            mobility->getCurrentPosition().z);
        EV_DEBUG << "SOP: sop_omnet_coord: " << sop_omnet_coord << std::endl;

        // Move SOP to the right position on the canvas
        getDisplayString().setTagArg("p", 0, sop_omnet_coord.x);
        getDisplayString().setTagArg("p", 1, sop_omnet_coord.y);

        // Get network boundary
        std::string convBoundary = getConvBoundaryString(par("sumoNetXmlFile").xmlValue());
        auto convBoundaryCoords = convBoundary2TraCICoords(convBoundary);

        // Read SUMO netOffset
        std::string netOffsetStr = getNetOffsetString(par("sumoNetXmlFile").xmlValue());
        netOffset = netOffsetString2Coord(netOffsetStr);

        // Create TraCICoordinateTransformation
        coordinateTransformation.reset(new veins::TraCICoordinateTransformation(convBoundaryCoords.first, convBoundaryCoords.second, par("margin").intValue()));
        ASSERT(coordinateTransformation.get());

        // Transform sop_omnet_coord into sop_traci_coord
        sop_traci_coord = coordinateTransformation->omnet2traci(sop_omnet_coord);
        EV_DEBUG << "SOP: sop_traci_coord: " << sop_traci_coord.x << ", " << sop_traci_coord.y << std::endl;

        // Apply netOffset to get UTM coords
        sop_utm_coord = veins::TraCICoord(sop_traci_coord.x - netOffset.x, sop_traci_coord.y - netOffset.y);
        EV_DEBUG << "SOP: sop_utm_coord: " << sop_utm_coord.x << ", " << sop_utm_coord.y << std::endl;

        // Transform sop_traci_coord (UTM) into a WGS84 coordinate
        PJ_COORD toTransfer = proj_coord(sop_utm_coord.x, sop_utm_coord.y, 0, 0);
        PJ_COORD sop_wgs84_proj = proj_trans(sumo_to_wgs84_projection, PJ_FWD, toTransfer);
        EV_TRACE << "SOP: sop_wgs84_proj: latitude: " << sop_wgs84_proj.lp.phi << ", longitude: " << sop_wgs84_proj.lp.lam << std::endl;
        sop_wgs84 = WGS84Coord(sop_wgs84_proj.lp.phi, sop_wgs84_proj.lp.lam, 0.0);
        EV_DEBUG << "SOP: sop_wgs84: " << sop_wgs84 << std::endl;

        // Convert sop_wgs84 from geodetic (lon, lat, alt) to cartesian (x, y, z)
        // proj somehow needs radians for this conversion
        PJ_COORD sop_wgs84_proj_rad = proj_coord(sop_wgs84.lon * (PI/180), sop_wgs84.lat * (PI/180), sop_wgs84.alt, 0);
        sop_wgs84_proj_cart = proj_trans(wgs84_to_wgs84cartesian_projection, PJ_FWD, sop_wgs84_proj_rad);
        EV_DEBUG << "SOP: sop_wgs84_proj_cart: x: " << sop_wgs84_proj_cart.xyz.x << ", y: " << sop_wgs84_proj_cart.xyz.y << ", z: " << sop_wgs84_proj_cart.xyz.z << std::endl;

        // Create relativeToSOP_projection
        // create geocentric to topocentric projection (requires sop_wgs84 as Cartesian coordinate)
        std::stringstream ss_proj;
        ss_proj << "+proj=topocentric +ellps=WGS84 +X_0=" << sop_wgs84_proj_cart.xyz.x << " +Y_0=" << sop_wgs84_proj_cart.xyz.y << " +Z_0=" << sop_wgs84_proj_cart.xyz.z;
        // geocentric to topocentric projection
        relativeToSOP_projection = proj_create(pj_ctx, ss_proj.str().c_str());
    }
}

PJ_COORD SatelliteObservationPoint::get_sop_wgs84_proj_cart() const
{
    return sop_wgs84_proj_cart;
}

WGS84Coord SatelliteObservationPoint::get_sop_wgs84() const
{
    return sop_wgs84;
}

veins::TraCICoord SatelliteObservationPoint::get_sop_utm_coord() const
{
    return sop_utm_coord;
}

veins::TraCICoord SatelliteObservationPoint::get_sop_traci_coord() const
{
    return sop_traci_coord;
}

veins::Coord SatelliteObservationPoint::get_sop_omnet_coord() const
{
    return sop_omnet_coord;
}

PJ* SatelliteObservationPoint::provideSumoToWGS84Projection() const
{
    return sumo_to_wgs84_projection;
}

PJ* SatelliteObservationPoint::provideWGS84ToWGS84CartesianProjection() const
{
    return wgs84_to_wgs84cartesian_projection;
}

PJ* SatelliteObservationPoint::provideRelativeToSOPProjection() const
{
    return relativeToSOP_projection;
}

veins::TraCICoord SatelliteObservationPoint::omnet2UTM(const veins::Coord toTransfer) const
{
    // Transform toTransfer into SUMO coordinate system
    veins::TraCICoord toTransferSUMO = coordinateTransformation->omnet2traci(toTransfer);

    // Apply netOffset to get UTM coords
    veins::TraCICoord toTransferUTM = veins::TraCICoord(toTransferSUMO.x - netOffset.x, toTransferSUMO.y - netOffset.y);

    return toTransferUTM;
}

WGS84Coord SatelliteObservationPoint::omnet2WGS84(const veins::Coord toTransfer) const
{
    auto toTransferUTM = omnet2UTM(toTransfer);

    // Transform toTransferUTM into a WGS84 coordinate
    PJ_COORD toTransferProjUTM = proj_coord(toTransferUTM.x, toTransferUTM.y, 0, 0);
    PJ_COORD toTransferWGS84 = proj_trans(sumo_to_wgs84_projection, PJ_FWD, toTransferProjUTM);
    return WGS84Coord(toTransferWGS84.lp.phi, toTransferWGS84.lp.lam, 0.0);
}

PJ_COORD SatelliteObservationPoint::omnet2WGS84Cartesian(const veins::Coord toTransfer) const
{
    auto toTransferWGS84 = omnet2WGS84(toTransfer);

    // Convert toTransferWGS84 from geodetic (lon, lat, alt) to cartesian (x, y, z)
    // proj somehow needs radians for this conversion
    PJ_COORD toTransfer_wgs84_proj_rad = proj_coord(toTransferWGS84.lon * (PI/180), toTransferWGS84.lat * (PI/180), toTransferWGS84.alt, 0);
    PJ_COORD toTransfer_wgs84_proj_cart = proj_trans(wgs84_to_wgs84cartesian_projection, PJ_FWD, toTransfer_wgs84_proj_rad);
    return toTransfer_wgs84_proj_cart;
}

veins::Coord SatelliteObservationPoint::omnetRelativeSOPCoord(const veins::Coord toTransfer) const
{
    auto toTransfer_wgs84_proj_cart = omnet2WGS84Cartesian(toTransfer);

    // Convert toTransfer_wgs84_proj_cart relative to SOP
    PJ_COORD toTransfer_relative_to_sop = proj_trans(relativeToSOP_projection, PJ_FWD, toTransfer_wgs84_proj_cart);
    // Note the minus operator at the northing: The reason is OMNeT++'s coordinate system. The origin is in the upper left corner,
    // the x-axis goes from west to east in the positiv direction and the y-axis goes from north to south in the positiv direction.
    // According to the figure at https://proj.org/operations/conversions/topocentric.html the enu.n-axis needs to be inverted.
    //
    // Further, the position of the SOP is added such that the satellite position is relative to OMNeT++'s origin.
    veins::Coord relativeCoord(toTransfer_relative_to_sop.enu.e + sop_omnet_coord.x, -toTransfer_relative_to_sop.enu.n + sop_omnet_coord.y, toTransfer_relative_to_sop.enu.u + sop_omnet_coord.z);
    return relativeCoord;
}
