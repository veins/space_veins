# SPDX-FileCopyrightText: 2021 Mario Franke <research@m-franke.net>
#
# SPDX-License-Identifier: GPL-2.0-or-later

"""
A class which manages the current simulation.
"""

from skyfield.api import load
from datetime import datetime, timedelta
import logging
import skyfieldProtobufInterface
import skyfieldProcessingUnit
import skyfieldSatelliteMobilityManager

log = logging.getLogger(__name__)

class SkyfieldSessionManager:
    def __init__(self, path_to_tle_file, simulation_time, space_veins_port):
        log.debug("__init__ SkyfieldSessionManager")
        self.skyfield_timescale = load.timescale()
        self.skyfield_time = self.skyfield_timescale.from_datetime(datetime.strptime(simulation_time, "%Y-%m-%dT%H:%M:%S:%f_%z"))
        self.spi = skyfieldProtobufInterface.SkyfieldProtobufInterface(space_veins_port)
        self.ssmm = skyfieldSatelliteMobilityManager.SkyfieldSatelliteMobilityManager()
        self.tle_set = self.load_tle_file(path_to_tle_file)
        self.satelliteDictByName = {sat.name: sat for sat in self.tle_set}
        self.satelliteDictByNumber = {sat.model.satnum: sat for sat in self.tle_set}
        self.running = False

    def load_tle_file(self, path_to_tle_file):
        log.debug("load tle file: {}.".format(path_to_tle_file))
        return load.tle_file(path_to_tle_file)

    def calculateReplys(self, request):
        log.debug("calculateReplys")
        spr = skyfieldProcessingUnit.SatelliteProcessingReplys(
                request.unitId,
                request.time_s
                )
        for r in request.requests:
            alt, az, distance_km = self.ssmm.getRelativePosition(
                self.satelliteDictByName[r.satelliteName],
                self.skyfield_timescale.utc(
                    self.skyfield_time.utc_datetime() + timedelta(seconds=request.time_s)),
                r.referencePosition
                )
            spr.replys.append(
                skyfieldProcessingUnit.RelativeSatellitePositionReply(
                    r.requestId,
                    r.satelliteName,
                    skyfieldProcessingUnit.RelativeSatellitePosition(
                        alt,
                        az,
                        distance_km
                        )
                    )
                )
        return spr

    def process_new_message(self):
        msg = self.spi.waitForMessage()
        if type(msg) is skyfieldProcessingUnit.SatelliteProcessingRequests:
            log.debug("Process SatelliteProcessingRequests.")
            reply = self.calculateReplys(msg)
            self.spi.reply(reply)
        elif type(msg) is skyfieldProcessingUnit.SkyfieldInitCmd:
            log.debug("Process SkyfieldInitCmd")
        elif type(msg) is skyfieldProcessingUnit.SkyfieldShutdownCmd:
            log.info("Received shut down command from Space Veins.")
            self.running = False
        elif type(msg) is skyfieldProcessingUnit.SpaceVeinsAck:
            log.debug("Received ack from Space Veins with Id: {}".format(msg.ackId))
        else:
            log.error("Process unknown message type: {}".format(type(msg)))

    def run(self):
        """
        Processing loop: Wait for new requests until Space Veins closes the connection.
        """
        self.running = True
        log.info("Processing loop started.")
        while self.running:
            self.process_new_message()
