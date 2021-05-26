//
// Copyright (C) 2007 Technische Universitaet Berlin (TUB), Germany, Telecommunication Networks Group
// Copyright (C) 2007 Technische Universiteit Delft (TUD), Netherlands
// Copyright (C) 2007 Universitaet Paderborn (UPB), Germany
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

#include "space_veins/base/satellitesConnectionManager/SatellitesConnectionManager.h"

#include <cmath>

#include "veins/base/modules/BaseWorldUtility.h"

namespace space_veins{

using Latitude = SatellitesConnectionManager::Latitude;
using Longitude = SatellitesConnectionManager::Longitude;
using WGS84Coordinate = SatellitesConnectionManager::WGS84Coordinate;

Define_Module(space_veins::SatellitesConnectionManager);

double SatellitesConnectionManager::calcInterfDist()
{
    /* With the introduction of antenna models, calculating the maximum
     * interference distance only based on free space loss doesn't make any sense
     * any more as it could also be much bigger due to positive antenna gains.
     * Therefore, the user has to provide a reasonable maximum interference
     * distance himself. */
    if (hasPar("maxInterfDist")) {
        double interfDistance = par("maxInterfDist").doubleValue();
        EV_INFO << "max interference distance:" << interfDistance << endl;
        return interfDistance;
    }
    else {
        throw cRuntimeError("SatellitesConnectionManager: No value for maximum interference distance (maxInterfDist) provided.");
    }
}

void SatellitesConnectionManager::initialize(int stage)
{
    if (stage == 0) {

        sendDirect = par("sendDirect").boolValue();

        minAltitudeAngle_deg = par("minAltitudeAngle").doubleValue();
        smc = make_unique<SkyfieldMobilityClient>("localhost");
        smc->connect();
        EV_DEBUG << "SkyfieldMobilityClient connected." << std::endl;

        std::string proj = getProjectionString(par("sumoNetXmlFile").xmlValue());
        EV_DEBUG << "SatellitesConnectionManager proj: " << proj << std::endl;

        // create projection context
        pj_ctx = proj_context_create();
        // create projection
        projection = proj_create_crs_to_crs(
                pj_ctx,
                proj.c_str(),   // from UTM of SUMO
                "EPSG:4326",    // to WGS84
                NULL);
        // fix longitude, latitude order, see https://proj.org/development/quickstart.html
        projection = proj_normalize_for_visualization(pj_ctx, projection);
        if (projection == 0) {
            throw cRuntimeError("Uniform axis order error.");
        }

        /* Statistics */
        dist_m_sender_receiver_vec.setName("dist_m_sender_receiver");
        height_diff_m_sender_receiver_vec.setName("height_diff_m_sender_receiver");
        altitude_deg_sender_receiver_vec.setName("altitude_deg_sender_receiver");

        EV_DEBUG << "SatellitesConnectionManager is initialized." << std::endl;
    }
}

const veins::NicEntry::GateList& SatellitesConnectionManager::getGateList(int nicID) const
{
    NicEntries::const_iterator ItNic = nics.find(nicID);
    if (ItNic == nics.end()) throw cRuntimeError("No nic with this ID (%d) is registered with this SatellitesConnectionManager.", nicID);

    return ItNic->second->getGateList();
}

std::string SatellitesConnectionManager::getProjectionString(const cXMLElement* sumoNetXmlFile) const
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

WGS84Coordinate SatellitesConnectionManager::omnetCoord2GeoCoord(const veins::Coord omnetCoord)
{
    EV_DEBUG << "SatellitesConnectionManager: Omnet coord: " << omnetCoord << std::endl;

    // traciConnection has to be initialized here because the TraCIConenction is established after the initialization phase.
    if (!traciConnectionEstablished) {
        if (!veins::TraCIScenarioManagerAccess().get()->isConnected()) {
            throw cRuntimeError("TraCIScenarioManager has no connection to SUMO.");
        }
        traciConnection.reset(veins::TraCIScenarioManagerAccess().get()->getConnection());
        EV_DEBUG << "SatellitesConnectionManager has access to TraCIConenction." << std::endl;
        traciConnectionEstablished = true;
    }
    veins::TraCICoord traciCoord = traciConnection->omnet2traci(omnetCoord);
    EV_DEBUG << "SatellitesConnectionManager: TraCI coord: " << traciCoord.x << ", " << traciCoord.y << std::endl;
    PJ_COORD toTransfer = proj_coord(traciCoord.x, traciCoord.y, 0, 0);
    PJ_COORD geo = proj_trans(projection, PJ_FWD, toTransfer);
    EV_DEBUG << "SatellitesConnectionManager: wgs84 coord: latitude: " << geo.lp.phi << ", longitude: " << geo.lp.lam << std::endl;
    return WGS84Coordinate(geo.lp.phi, geo.lp.lam);
}

RelativeSatellitePosition SatellitesConnectionManager::getRelativeSatellitePosition(const WGS84Coordinate geo, const std::string satelliteName)
{
    return smc->getRelativeSatellitePosition(satelliteName, simTime(), geo.first, geo.second);
}

bool SatellitesConnectionManager::isTraCIConnectionEstablished() const
{
    return traciConnectionEstablished;
}

bool SatellitesConnectionManager::isInRange(NicEntries::mapped_type pFromNic, NicEntries::mapped_type pToNic)
{
    cModule* fromNicVehicle = pFromNic->nicPtr->getParentModule();
    cModule* toNicVehicle = pToNic->nicPtr->getParentModule();

    // exclude car to car communication via the satellite link
    if (std::string(toNicVehicle->getModuleType()->getFullName()).compare("space_veins.nodes.Satellite") != 0
        && std::string(fromNicVehicle->getModuleType()->getFullName()).compare("space_veins.nodes.Satellite") != 0) {
        EV_DEBUG << "Neither toNicVehicle nor fromNicVehicle is a satellite. Communication not possible." << std::endl;
        return false;
    }

    // determine altitude angle between satellite and car or vice versa
    // Currently module positions are used, TODO: incorporate antenna position
    double dist_m = pToNic->pos.distance(pFromNic->pos);
    EV_DEBUG << "SatellitesConnectionManager: isInRange: dist_m: " << dist_m << std::endl;
    double height_diff_m = abs(pToNic->pos.z - pFromNic->pos.z);
    EV_DEBUG << "SatellitesConnectionManager: isInRange: height_diff_m: " << height_diff_m << std::endl;
    double altitude_deg = asin(height_diff_m / dist_m) * (180 / M_PI);
    EV_DEBUG << "SatellitesConnectionManager: isInRange: altitude_deg: " << altitude_deg << std::endl;

    /* Statistics */
    dist_m_sender_receiver_vec.record(dist_m);
    height_diff_m_sender_receiver_vec.record(height_diff_m);
    altitude_deg_sender_receiver_vec.record(altitude_deg);

    return altitude_deg > minAltitudeAngle_deg;
}

void SatellitesConnectionManager::updateNicConnections(NicEntries::mapped_type nic)
{
    int id = nic->nicId;

    for (NicEntries::iterator i = nics.begin(); i != nics.end(); ++i) {
        NicEntries::mapped_type nic_i = i->second;

        // no recursive connections
        if (nic_i->nicId == id) continue;

        bool inRange = isInRange(nic, nic_i);
        bool connected = nic->isConnected(nic_i);

        if (inRange && !connected) {
            // nodes within communication range: connect
            // nodes within communication range && not yet connected
            EV_TRACE << "SatellitesConnectionManager: nic #" << id << " and #" << nic_i->nicId << " are in range" << endl;
            nic->connectTo(nic_i);
            nic_i->connectTo(nic);
        }
        else if (!inRange && connected) {
            // out of range: disconnect
            // out of range, and still connected
            EV_TRACE << "SatellitesConnectionManager: nic #" << id << " and #" << nic_i->nicId << " are NOT in range" << endl;
            nic->disconnectFrom(nic_i);
            nic_i->disconnectFrom(nic);
        }
    }
}

bool SatellitesConnectionManager::registerNic(cModule* nic, SatelliteChannelAccess* chAccess, veins::Coord nicPos, veins::Heading heading)
{
    ASSERT(nic != nullptr);

    int nicID = nic->getId();
    EV_TRACE << "SatellitesConnectionManager: registering nic #" << nicID << endl;

    // create new NicEntry
    NicEntries::mapped_type nicEntry;

    if (sendDirect)
        nicEntry = new SatelliteNicEntryDirect(this);
    else
        nicEntry = new SatelliteNicEntryDebug(this);

    // fill nicEntry
    nicEntry->nicPtr = nic;
    nicEntry->nicId = nicID;
    nicEntry->hostId = nic->getParentModule()->getId();
    nicEntry->pos = nicPos;
    nicEntry->heading = heading;
    nicEntry->satChAccess = chAccess;

    // add to map
    nics[nicID] = nicEntry;

    updateNicConnections(nicEntry);

    return sendDirect;
}

bool SatellitesConnectionManager::unregisterNic(cModule* nicModule)
{
    ASSERT(nicModule != nullptr);

    // find nicEntry
    int nicID = nicModule->getId();
    EV_TRACE << "SatellitesConnectionManager: unregistering nic #" << nicID << endl;

    // we assume that the module was previously registered with this CM
    // TODO: maybe change this to an omnet-error instead of an assertion
    ASSERT(nics.find(nicID) != nics.end());
    NicEntries::mapped_type nicEntry = nics[nicID];

    // disconnect from all NICs
    for (NicEntries::iterator i = nics.begin(); i != nics.end(); ++i) {
        NicEntries::mapped_type other = i->second;
        if (other == nicEntry) continue;
        if (!other->isConnected(nicEntry)) continue;
        other->disconnectFrom(nicEntry);
        nicEntry->disconnectFrom(other);
    }

    // erase from list of known nics
    nics.erase(nicID);

    delete nicEntry;

    return true;
}

void SatellitesConnectionManager::updateNicPos(int nicID, veins::Coord newPos, veins::Heading heading)
{
    NicEntries::iterator ItNic = nics.find(nicID);
    if (ItNic == nics.end()) throw cRuntimeError("No satellite nic with this ID (%d) is registered with this SatellitesConnectionManager.", nicID);

    ItNic->second->pos = newPos;
    ItNic->second->heading = heading;

    updateNicConnections(nics[nicID]);
}

} // end namespace space_veins
