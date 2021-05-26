# SPDX-FileCopyrightText: 2021 Mario Franke <research@m-franke.net>
#
# SPDX-License-Identifier: GPL-2.0-or-later

"""
A helper class which provides methods for simulating the satellite mobility.
"""

import logging
from skyfield.api import load, wgs84

log = logging.getLogger(__name__)

class SkyfieldSatelliteMobilityManager:
    def __init__(self):
        log.debug("__init__ of SkyfieldSatelliteMobilityManager")

    def getRelativePosition(self, satellite, time, ref_pos):
        geocentric = satellite.at(time)
        log.debug("X, Y, Z coordinates in km of {} in Geocentric Celestial Reference System at {}: {}".format(satellite.name, time.utc_strftime("%Y-%m-%dT%H:%M:%S:%f_%z"), geocentric.position.km))
        wgs84_ref_pos = wgs84.latlon(ref_pos.latitude, ref_pos.longitude)
        diff = satellite - wgs84_ref_pos
        topocentric = diff.at(time)
        log.debug("X, Y, Z coordinates in km of {} in Geocentric Celestial Reference System at {} relative to {}: {}".format(satellite.name, time.utc_strftime("%Y-%m-%dT%H:%M:%S:%f_%z"), wgs84_ref_pos, topocentric.position.km))
        alt, az, distance = topocentric.altaz()
        log.debug("Altitude: {}, azimuth: {}, and distance in km: {}  of {} relative to {} at {}".format(alt.degrees, az.degrees, distance.km, satellite.name, wgs84_ref_pos, time.utc_strftime("%Y-%m-%dT%H:%M:%S:%f_%z")))
        return alt.degrees, az.degrees, distance.km
