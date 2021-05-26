# SPDX-FileCopyrightText: 2021 Mario Franke <research@m-franke.net>
#
# SPDX-License-Identifier: GPL-2.0-or-later

"""
An interface which implements uses skyfield_protobuf_protocol
"""

import skyfield_protobuf.skyfield_protobuf_pb2 as skyfield_protobuf
from skyfield_protobuf.skyfield_protobuf import satellite_pb2 as skyfield_satellite
import logging
import zmq
from datetime import datetime

import skyfieldProcessingUnit

log = logging.getLogger(__name__)

class SkyfieldProtobufInterface:
    def __init__(self, port):
        log.debug("__init__ of SkyfieldProtobufInterface")
        self.zmq_context = zmq.Context()
        self.socket = self.zmq_context.socket(zmq.REP)
        self.socket.bind("tcp://*:{}".format(port))
        log.info("Binding socket: tcp://*:{}".format(port))

    def waitForMessage(self):
        log.debug("Wait for request from Space Veins.")
        msg = self.socket.recv()
        protobuf_msg = skyfield_protobuf.Message()
        protobuf_msg.ParseFromString(msg)
        log.debug("Received request from Space Veins:\n{}".format(protobuf_msg))
        return self.processMessage(protobuf_msg)

    def processMessage(self, protobuf_msg):
        if protobuf_msg.WhichOneof("message_type") == "satellite":
            return self.processSatelliteMessage(protobuf_msg)
        elif protobuf_msg.WhichOneof("message_type") == "simulation":
            return self.processSimulationMessage(protobuf_msg)
        elif protobuf_msg.WhichOneof("message_type") == "ack":
            return skyfieldProcessingUnit.SpaceVeinsAck(protobuf_msg.id)
        else:
            log.error("Protobuf message has no message_type.")

    def processSatelliteMessage(self, protobuf_msg):
        log.debug("Process satellite message")
        spr = skyfieldProcessingUnit.SatelliteProcessingRequests(protobuf_msg.id, protobuf_msg.satellite.time_s)
        for r in protobuf_msg.satellite.requestReplyMessages:
            if r.WhichOneof("request_reply_message_type") == "request":
                if r.request.WhichOneof("request_type") == "request_relative_satellite_position":
                    spr.requests.append(skyfieldProcessingUnit.RelativeSatellitePositionRequest(
                        r.request.requestId,
                        r.request.request_relative_satellite_position.satelliteName,
                        skyfieldProcessingUnit.WGS84Coord(
                            r.request.request_relative_satellite_position.ref_pos.latitude,
                            r.request.request_relative_satellite_position.ref_pos.longitude)
                        )
                    )
                else:
                    log.error("Received satellite message with an unknown request.")
            elif r.WhichOneof("request_reply_message_type") == "reply":
                log.error("Received satellite message with a reply.")
            else:
                log.error("Received satellite message which is neither a request nor a reply.")
        return spr

    def processSimulationMessage(self, protobuf_msg):
        log.debug("Process simulation message")
        if protobuf_msg.simulation.WhichOneof("message_type") == "init":
            return skyfieldProcessingUnit.SkyfieldInitCmd(
                datetime.strptime(protobuf_msg.simulation.init.time_utc, "%Y-%m-%dT%H:%M:%S:%f_%z"),
                protobuf_msg.simulation.init.pathToTLEfile,
                protobuf_msg.simulation.init.space_veins_root
            )
        elif protobuf_msg.simulation.WhichOneof("message_type") == "shutdown":
            return skyfieldProcessingUnit.SkyfieldShutdownCmd()
        else:
            log.error("Received simulation message which is neither an init command nor a shutdown command.")

    def reply(self, reply):
        protobuf_reply = skyfield_protobuf.Message()
        protobuf_reply.id = reply.unitId
        protobuf_reply.satellite.time_s = reply.time_s
        for r in reply.replys:
            rrm = skyfield_satellite.RequestReplyMessage()
            rrm.reply.replyId = r.replyId
            if type(r) is skyfieldProcessingUnit.RelativeSatellitePositionReply:
                rrm.reply.reply_relative_satellite_position.satelliteName = r.satelliteName
                rrm.reply.reply_relative_satellite_position.sat_pos.altitude = r.relativeSatellitePosition.altitude
                rrm.reply.reply_relative_satellite_position.sat_pos.azimuth = r.relativeSatellitePosition.azimuth
                rrm.reply.reply_relative_satellite_position.sat_pos.distance_km = r.relativeSatellitePosition.distance_km
            else:
                log.error("Unknown reply type")
            protobuf_reply.satellite.requestReplyMessages.append(rrm)
        toSend = protobuf_reply.SerializeToString()
        log.debug("Transmitting reply:\n{}".format(protobuf_reply))
        self.socket.send(toSend)
