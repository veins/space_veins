// SPDX-FileCopyrightText: 2021 Mario Franke <research@m-franke.net>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include <sstream>

#include "space_veins/base/satellitesConnectionManager/SkyfieldMobilityClient.h"

using namespace space_veins;

void SkyfieldMobilityClient::connect()
{
    int rv = zmq_connect(this->sock, getSkyfieldServerAddress().c_str());
    assert(rv == 0);
    EV_DEBUG << "SkyfieldMobilityClient connecting to: " << getSkyfieldServerAddress() << std::endl;
    this->connected = true;
}

std::string SkyfieldMobilityClient::getSkyfieldServerAddress()
{
    std::stringstream ss;
    ss << "tcp://" << this->host << ":" << this->port;
    return ss.str();
}

std::string SkyfieldMobilityClient::buildRequest(const std::string satelliteName, const simtime_t time, const double ref_latitude, const double ref_longitude)
{
    int requestId = 0;
    // Build protobuf message
    skyfield_protobuf::Message proto_msg;
    proto_msg.set_id(this->messageId);
    this->messageId++;
    // Add satellite message
    auto satelliteMsg = proto_msg.mutable_satellite();
    satelliteMsg->set_time_s(time.dbl());
    // Add a request to the satellite message
    auto req = satelliteMsg->add_requestreplymessages();
    req->mutable_request()->set_requestid(requestId);
    requestId++;
    req->mutable_request()->mutable_request_relative_satellite_position()->set_satellitename(satelliteName);
    // Add a reference position to the request
    auto ref_pos = req->mutable_request()->mutable_request_relative_satellite_position()->mutable_ref_pos();
    ref_pos->set_latitude(ref_latitude);
    ref_pos->set_longitude(ref_longitude);
    // Serialize protobuf message
    std::string toSend = proto_msg.SerializeAsString();
    EV_DEBUG << "Created Skyfield request:\n" << proto_msg.DebugString() << std::endl;
    return toSend;
}

void SkyfieldMobilityClient::sendRequest(const std::string proto_msg)
{
    assert(this->connected);
    zmq_msg_t zmq_msg;
    int rv = zmq_msg_init_size(&zmq_msg, proto_msg.length());
    assert(rv == 0);
    memcpy(zmq_msg_data(&zmq_msg), proto_msg.c_str(), proto_msg.length());
    rv = zmq_msg_send(&zmq_msg, this->sock, 0);
    assert(rv > 0);
}

std::string SkyfieldMobilityClient::waitForReply()
{
    std::string reply;
    zmq_msg_t zmq_msg;
    assert(this->connected);
    int rv = zmq_msg_init(&zmq_msg);
    assert(rv == 0);
    rv = zmq_recvmsg(this->sock, &zmq_msg, 0);
    assert(rv > 0);
    reply = std::string((char*)zmq_msg_data(&zmq_msg), zmq_msg_size(&zmq_msg));
    zmq_msg_close(&zmq_msg);
    return reply;
}

RelativeSatellitePosition SkyfieldMobilityClient::protobufToRelativeSatellitePosition(const std::string proto_msg)
{
    skyfield_protobuf::Message msg;
    double altitude_deg, azimuth_deg, distance_km;
    bool rv = msg.ParseFromString(proto_msg);
    assert(rv == true);
    EV_DEBUG << "Received RelativeSatellitePosition reply:\n" << msg.DebugString() << std::endl;
    assert(msg.has_satellite() == true);
    auto satellite_msg = msg.satellite();
    assert(satellite_msg.requestreplymessages_size() > 0);
    auto rrm = satellite_msg.requestreplymessages(0);
    assert(rrm.has_reply() == true);
    assert(rrm.reply().has_reply_relative_satellite_position() == true);
    assert(rrm.reply().reply_relative_satellite_position().has_sat_pos() == true);
    altitude_deg = rrm.reply().reply_relative_satellite_position().sat_pos().altitude();
    azimuth_deg = rrm.reply().reply_relative_satellite_position().sat_pos().azimuth();
    distance_km = rrm.reply().reply_relative_satellite_position().sat_pos().distance_km();
    return RelativeSatellitePosition(altitude_deg, azimuth_deg, distance_km);
}

RelativeSatellitePosition SkyfieldMobilityClient::getRelativeSatellitePosition(const std::string satelliteName, const simtime_t time, const double ref_latitude, const double ref_longitude)
{
    std::string toSend = buildRequest(satelliteName, time, ref_latitude, ref_longitude);
    sendRequest(toSend);
    std::string reply = waitForReply();
    return protobufToRelativeSatellitePosition(reply);

}
