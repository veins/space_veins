# SPDX-FileCopyrightText: 2021 Mario Franke <research@m-franke.net>
#
# SPDX-License-Identifier: GPL-2.0-or-later

"""
Classes which store different kinds of processing units.
"""

class ProcessingUnit:
    def __init__(self, unitId=-1):
        self.unitId = unitId

#---------------------------------------------------------------------
# Requests

class SatelliteProcessingRequests(ProcessingUnit):
    def __init__(self, unitId=-1, time_s=-1):
        super(SatelliteProcessingRequests, self).__init__(unitId)
        self.time_s = time_s
        self.requests = []

class SatelliteRequest():
    def __init__(self, requestId=-1):
        self.requestId = requestId

class WGS84Coord:
    def __init__(self, latitude=-1, longitude=-1):
        self.latitude = latitude
        self.longitude = longitude

class RelativeSatellitePositionRequest(SatelliteRequest):
    def __init__(self, requestId=-1, satelliteName="", wgs84coord=None):
        super(RelativeSatellitePositionRequest, self).__init__(requestId)
        self.satelliteName = satelliteName
        self.referencePosition = wgs84coord

#---------------------------------------------------------------------
# Replys

class SatelliteProcessingReplys(ProcessingUnit):
    def __init__(self, unitId=-1, time_s=-1):
        super(SatelliteProcessingReplys, self).__init__(unitId)
        self.time_s = time_s
        self.replys = []

class SatelliteReply():
    def __init__(self, replyId=-1):
        self.replyId = replyId

class RelativeSatellitePosition:
    def __init__(self, altitude=-1, azimuth=-1, distance_km=-1):
        self.altitude = altitude
        self.azimuth = azimuth
        self.distance_km = distance_km

class RelativeSatellitePositionReply(SatelliteReply):
    def __init__(self, replyId=-1, satelliteName="", relativeSatellitePosition=None):
        super(RelativeSatellitePositionReply, self).__init__(replyId)
        self.satelliteName = satelliteName
        self.relativeSatellitePosition = relativeSatellitePosition

#---------------------------------------------------------------------
# Simulation messages

class SkyfieldInitCmd():
    def __init__(self, time_utc=None, path_to_tle_file="", space_veins_root=""):
        self.time_utc = time_utc
        self.path_to_tle_file = path_to_tle_file
        self.space_veins_root = space_veins_root

class SkyfieldShutdownCmd():
    pass

#---------------------------------------------------------------------
# Acknowledge message

class SpaceVeinsAck():
    def __init__(self, ackId=-1):
        self.ackId = ackId
