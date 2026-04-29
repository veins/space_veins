#
# Copyright (C) 2025 Mario Franke <research@m-franke.net>
#
# Documentation for these modules is at http://sat.car2x.org/
#
# SPDX-License-Identifier: GPL-2.0-or-later
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

import math

# helper functions
def degToRad(deg):
    return deg * math.pi / 180

def radToDeg(rad):
    return rad * 180 / math.pi

# height of the ground station antenna above ground in meter
groundStationAntennaHeight = 1.895

# tolerance
tolerance = 0.0000001
# minElevationAngleDeg
minElevationAngleDeg = 25

# target elevation angle between ground station and LEO satellite
elevationAngles = [-1, -0.001, 0, 0.001, 1, 24, 24.999, 25, 25.001, 26]

# define the distance between ground station and satellite by the signal
# propagation delay of 5ms and speed of light 299792458 m/s
propagationDelaySecond = 0.005
propagationSpeedMeterPerSecond = 299792458

satelliteX = []
satelliteY = []
satelliteZ = []

for elevationAngleDeg in elevationAngles:
    print("compute positions for: ", elevationAngleDeg, "deg -----------------------------------")
    # compute the postion of three points building a triangle, as y == 0 for all points
    # position of the ground station
    posGroundStation = dict(x=0, y=0, z=0)
    # position of the satellite
    posSatellite = dict(x=-1, y=0, z=-1)
    # position of the satellite projected on the same z-coordinate like pos_ground_station
    posSatelliteGroundProjection = dict(x=-1, y=0, z=0)

    # define distance between ground station and satelite by the propagation delay of 5ms and speed of light
    distanceGroundStationToSatelliteMeter = propagationSpeedMeterPerSecond * propagationDelaySecond
    print("Distance between ground station and LEO satellite: ", distanceGroundStationToSatelliteMeter, "m")

    # compute the distance between ground station and the projection of the LEO satellite on a flat plane on which the ground station is
    distanceGroundStationToSatelliteGroundProjectionMeter = math.cos(degToRad(elevationAngleDeg)) * distanceGroundStationToSatelliteMeter
    print("Distance between ground station and LEO satellite projection: ", distanceGroundStationToSatelliteGroundProjectionMeter, "m")
    posSatellite['x'] = distanceGroundStationToSatelliteGroundProjectionMeter
    posSatelliteGroundProjection['x'] = distanceGroundStationToSatelliteGroundProjectionMeter

    # compute the height of the LEO satellite above the plane on which the ground station is (the horizon)
    distanceSatelliteOverHorizon = math.sin(degToRad(elevationAngleDeg)) * distanceGroundStationToSatelliteMeter
    print("Distance between LEO satellite and the local horizon: ", distanceSatelliteOverHorizon, "m")
    posSatellite['z'] = distanceSatelliteOverHorizon
    print('\nPositions')
    print("Position of the ground station: ", posGroundStation)
    print("Position of the LEO satellite: ", posSatellite)
    print("Position of the LEO satellite projection: ", posSatelliteGroundProjection)

    # elevate all points by the height of the ground station antenna
    posGroundStation['z'] = posGroundStation['z'] + groundStationAntennaHeight
    posSatellite['z'] = posSatellite['z'] + groundStationAntennaHeight
    posSatelliteGroundProjection['z'] = posSatelliteGroundProjection['z'] + groundStationAntennaHeight
    print('\nPositions considering the ground station antenna height')
    print("Position of the ground station: ", posGroundStation)
    print("Position of the LEO satellite: ", posSatellite)
    print("Position of the LEO satellite projection: ", posSatelliteGroundProjection)

    print('\nDouble check elevation angle')
    # check by calculating the tangent
    elevationDegTan = radToDeg(math.atan((posSatellite['z'] - posSatelliteGroundProjection['z']) / (posSatelliteGroundProjection['x'] - posGroundStation['x'])))
    print("Elevation angle calculated by math.atan: ", elevationDegTan, "deg")
    if (abs(elevationAngleDeg - elevationDegTan) > tolerance):
        # fail
        raise ValueError("abs(elevationAngleDeg - elevationDegTan) > tolerance is true")
    # check by calculating the sine
    # calculate the hypotenuse
    distanceGroundStationToSatelliteMeter = ((posSatellite['x'] - posGroundStation['x'])**2 + (posSatellite['z'] - posGroundStation['z'])**2)**0.5
    elevationDegSin = radToDeg(math.asin((posSatellite['z'] - posSatelliteGroundProjection['z']) / distanceGroundStationToSatelliteMeter))
    print("Elevation angle calculated by math.asin: ", elevationDegSin, "deg")
    if (abs(elevationAngleDeg - elevationDegSin) > tolerance):
        # fail
        raise ValueError("abs(elevationAngleDeg - elevationDegSin) > tolerance is true")

    satelliteX.append(posSatellite['x'])
    satelliteY.append(posSatellite['y'])
    satelliteZ.append(posSatellite['z'])

# print omnetpp.ini config lines
print('\n omnetpp.ini copnfig lines')
print("# configure radio medium")
print("*.satelliteRadioMedium.runSpaceVeinsTestCode = true")
print("*.satelliteRadioMedium.spaceVeinsTestNumber = ${testNumber=", ', '.join(str(t) for t in range(len(elevationAngles))), "}")
print("*.satelliteRadioMedium.spaceVeinsTestResultFilePrefix = \"elevationAngleTest_\"")
print("*.satelliteRadioMedium.mediumLimitCache.minElevationAngleV2S = " + str(minElevationAngleDeg) + "deg")
print("")
print("# expected number of sent and received packets")
print("*.leo*[*].app[0].expectedReceivedPackets = ${expectedReceivedPacketsLeo=", ', '.join("1" if e >= minElevationAngleDeg else "0" for e in elevationAngles), "! testNumber}")
print("*.node[*].app[0].expectedReceivedPackets = ${expectedReceivedPacketsGroundStation=", ', '.join("1" if e >= minElevationAngleDeg else "0" for e in elevationAngles), "! testNumber}")
print("*.leo*[*].app[0].expectedSentPackets = 1")
print("*.node[*].app[0].expectedSentPackets = 1")
print("")
print("# Position of the LEO satellite such the following elevation angles are created:", ', '.join(str(i) + "deg" for i in elevationAngles))
print("*.leo*[*].mobility.initialX = ${X=", ', '.join(str(i) for i in satelliteX), "! testNumber}m")
print("*.leo*[*].mobility.initialY = ${Y=", ', '.join(str(i) for i in satelliteY), "! testNumber}m")
print("*.leo*[*].mobility.initialZ = ${Z=", ', '.join(str(i) for i in satelliteZ), "! testNumber}m")