// SPDX-FileCopyrightText: 2021 Mario Franke <research@m-franke.net>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "space_veins/space_veins.h"
#include "space_veins/base/utils/RelativeSatellitePosition.h"
#include "space_veins/base/satellitesConnectionManager/protobuf/skyfield_protobuf.pb.h"

#include <zmq.hpp>
#include <cstdint>
#include <string>

namespace space_veins {

class SPACE_VEINS_API SkyfieldMobilityClient {
    private:
        std::string host;
        uint16_t port;
        void* ctx;
        void* sock;

        bool connected = false;
        uint32_t messageId = 0;

    public:
        SkyfieldMobilityClient(std::string pHost)
            : host(pHost)
            , port(40515)  // 40515 is the default port on which the SkyfieldMobilityServer listens.
        {
            this->ctx = zmq_ctx_new();
            assert(this->ctx != NULL);
            this->sock = zmq_socket(this->ctx, ZMQ_REQ);
            assert(this->sock != NULL);
        };

        SkyfieldMobilityClient(std::string pHost, uint16_t pPort)
            : host(pHost)
            , port(pPort)
        {
            this->ctx = zmq_ctx_new();
            assert(this->ctx != NULL);
            this->sock = zmq_socket(this->ctx, ZMQ_REQ);
            assert(this->sock != NULL);
        };

        ~SkyfieldMobilityClient()
        {
            zmq_close(this->sock);
            zmq_ctx_destroy(this->ctx);
        }

        void connect();

        std::string getSkyfieldServerAddress();

        std::string buildRequest(const std::string satelliteName, const simtime_t time, const double ref_latitude, const double ref_longitude);

        void sendRequest(const std::string proto_msg);

        std::string waitForReply();

        RelativeSatellitePosition protobufToRelativeSatellitePosition(const std::string proto_msg);

        RelativeSatellitePosition getRelativeSatellitePosition(const std::string satelliteName, const simtime_t time, const double ref_latitude, const double ref_longitude);
};

} // namespace space_veins
