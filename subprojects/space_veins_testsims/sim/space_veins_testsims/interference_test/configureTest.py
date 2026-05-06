#
# Copyright (C) 2026 Mario Franke <research@m-franke.net>
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

# define the distance between ground station and satellite by the signal
# propagation delay of 5ms and speed of light 299792458 m/s
propagationDelaySecond = 0.005
propagationSpeedMeterPerSecond = 299792458

def getSatPos(pElevationAngleDeg, pPosGroundStation, pPropagationDelaySecond):
    print("compute position for: ", pElevationAngleDeg, "deg -----------------------------------")
    # compute the postion of three points building a triangle, as y == 0 for all points
    # position of the satellite
    posSatellite = dict(x=-1, y=0, z=-1)
    # position of the satellite projected on the same z-coordinate like pos_ground_station
    posSatelliteGroundProjection = dict(x=-1, y=0, z=0)

    # define distance between ground station and satelite by the propagation delay of 5ms and speed of light
    distanceGroundStationToSatelliteMeter = propagationSpeedMeterPerSecond * pPropagationDelaySecond
    print("Distance between ground station and LEO satellite: ", distanceGroundStationToSatelliteMeter, "m")

    # compute the distance between ground station and the projection of the LEO satellite on a flat plane on which the ground station is
    distanceGroundStationToSatelliteGroundProjectionMeter = math.cos(degToRad(pElevationAngleDeg)) * distanceGroundStationToSatelliteMeter
    print("Distance between ground station and LEO satellite projection: ", distanceGroundStationToSatelliteGroundProjectionMeter, "m")
    posSatellite['x'] = distanceGroundStationToSatelliteGroundProjectionMeter
    posSatelliteGroundProjection['x'] = distanceGroundStationToSatelliteGroundProjectionMeter

    # compute the height of the LEO satellite above the plane on which the ground station is (the horizon)
    distanceSatelliteOverHorizon = math.sin(degToRad(pElevationAngleDeg)) * distanceGroundStationToSatelliteMeter
    print("Distance between LEO satellite and the local horizon: ", distanceSatelliteOverHorizon, "m")
    posSatellite['z'] = distanceSatelliteOverHorizon
    print('\nPositions')
    print("Position of the ground station: ", pPosGroundStation)
    print("Position of the LEO satellite: ", posSatellite)
    print("Position of the LEO satellite projection: ", posSatelliteGroundProjection)

    # elevate all points by the height of the ground station antenna
    pPosGroundStation['z'] = pPosGroundStation['z'] + groundStationAntennaHeight
    posSatellite['z'] = posSatellite['z'] + groundStationAntennaHeight
    posSatelliteGroundProjection['z'] = posSatelliteGroundProjection['z'] + groundStationAntennaHeight
    print('\nPositions considering the ground station antenna height')
    print("Position of the ground station: ", pPosGroundStation)
    print("Position of the LEO satellite: ", posSatellite)
    print("Position of the LEO satellite projection: ", posSatelliteGroundProjection)

    print('\nDouble check elevation angle')
    # check by calculating the tangent
    elevationDegTan = radToDeg(math.atan((posSatellite['z'] - posSatelliteGroundProjection['z']) / (posSatelliteGroundProjection['x'] - pPosGroundStation['x'])))
    print("Elevation angle calculated by math.atan: ", elevationDegTan, "deg")
    if (abs(pElevationAngleDeg - elevationDegTan) > tolerance):
        # fail
        raise ValueError("abs(pElevationAngleDeg - elevationDegTan) > tolerance is true")
    # check by calculating the sine
    # calculate the hypotenuse
    distanceGroundStationToSatelliteMeter = ((posSatellite['x'] - pPosGroundStation['x'])**2 + (posSatellite['z'] - pPosGroundStation['z'])**2)**0.5
    elevationDegSin = radToDeg(math.asin((posSatellite['z'] - posSatelliteGroundProjection['z']) / distanceGroundStationToSatelliteMeter))
    print("Elevation angle calculated by math.asin: ", elevationDegSin, "deg")
    if (abs(pElevationAngleDeg - elevationDegSin) > tolerance):
        # fail
        raise ValueError("abs(pElevationAngleDeg - elevationDegSin) > tolerance is true")
    return posSatellite


# initialize
testNumbers = range(6)

satellite0X = []
satellite0Y = []
satellite0Z = []

satellite1X = []
satellite1Y = []
satellite1Z = []

ignoreInterference = []

expectedReceivedPacketsSatCar = []
startTimeSat0 = []
startTimeSat1 = []

# configure Tests
# test 0
# get position for satellite 0
posSat0 = getSatPos(75, dict(x=0, y=0, z=0), 0.002)
posSat1 = posSat0.copy()
# mirror the x-axis for satellite 1
posSat1['x'] = posSat1['x'] * -1
# set satellite position for test 0
satellite0X.append(posSat0['x'])
satellite0Y.append(posSat0['y'])
satellite0Z.append(posSat0['z'])
satellite1X.append(posSat1['x'])
satellite1Y.append(posSat1['y'])
satellite1Z.append(posSat1['z'])
# set ignoreInterference
ignoreInterference.append('true')
# set expected received packets of the SatCar
expectedReceivedPacketsSatCar.append('1')
# set app start times
startTimeSat0.append("0s")
startTimeSat1.append("0s")

# test 1
# get position for satellite 0
posSat0 = getSatPos(75, dict(x=0, y=0, z=0), 0.002)
posSat1 = posSat0.copy()
# mirror the x-axis for satellite 1
posSat1['x'] = posSat1['x'] * -1
# set satellite position for test 0
satellite0X.append(posSat0['x'])
satellite0Y.append(posSat0['y'])
satellite0Z.append(posSat0['z'])
satellite1X.append(posSat1['x'])
satellite1Y.append(posSat1['y'])
satellite1Z.append(posSat1['z'])
# set ignoreInterference
ignoreInterference.append('false')
# set expected received packets of the SatCar
expectedReceivedPacketsSatCar.append('0')
# set app start times
startTimeSat0.append("0s")
startTimeSat1.append("0s")

# test 2
# get position for satellite 0
posSat0 = getSatPos(75, dict(x=0, y=0, z=0), 0.002)
posSat1 = getSatPos(20, dict(x=0, y=0, z=0), 0.005)
# set satellite position for test 0
satellite0X.append(posSat0['x'])
satellite0Y.append(posSat0['y'])
satellite0Z.append(posSat0['z'])
satellite1X.append(posSat1['x'])
satellite1Y.append(posSat1['y'])
satellite1Z.append(posSat1['z'])
# set ignoreInterference
ignoreInterference.append('true')
# set expected received packets of the SatCar
expectedReceivedPacketsSatCar.append('1')
# set app start times
startTimeSat0.append("0.003s")
startTimeSat1.append("0s")

# test 3
# get position for satellite 0
posSat0 = getSatPos(75, dict(x=0, y=0, z=0), 0.002)
posSat1 = getSatPos(20, dict(x=0, y=0, z=0), 0.005)
# set satellite position for test 0
satellite0X.append(posSat0['x'])
satellite0Y.append(posSat0['y'])
satellite0Z.append(posSat0['z'])
satellite1X.append(posSat1['x'])
satellite1Y.append(posSat1['y'])
satellite1Z.append(posSat1['z'])
# set ignoreInterference
ignoreInterference.append('false')
# set expected received packets of the SatCar
expectedReceivedPacketsSatCar.append('1')
# set app start times
startTimeSat0.append("0.003s")
startTimeSat1.append("0s")

# test 4
# get position for satellite 0
posSat0 = getSatPos(75, dict(x=0, y=0, z=0), 0.002)
posSat1 = getSatPos(20, dict(x=0, y=0, z=0), 0.005)
# mirror the z-axis for satellite 1 to be below the horizon
posSat1['z'] = posSat1['z'] * -1
if (posSat1['z'] >= 0):
    # fail
    raise ValueError("test4: posSat1['z'] >= 0 is true")
# set satellite position for test 0
satellite0X.append(posSat0['x'])
satellite0Y.append(posSat0['y'])
satellite0Z.append(posSat0['z'])
satellite1X.append(posSat1['x'])
satellite1Y.append(posSat1['y'])
satellite1Z.append(posSat1['z'])
# set ignoreInterference
ignoreInterference.append('true')
# set expected received packets of the SatCar
expectedReceivedPacketsSatCar.append('1')
# set app start times
startTimeSat0.append("0.003s")
startTimeSat1.append("0s")

# test 5
# get position for satellite 0
posSat0 = getSatPos(75, dict(x=0, y=0, z=0), 0.002)
posSat1 = getSatPos(20, dict(x=0, y=0, z=0), 0.005)
# mirror the z-axis for satellite 1 to be below the horizon
posSat1['z'] = posSat1['z'] * -1
if (posSat1['z'] >= 0):
    # fail
    raise ValueError("test5: posSat1['z'] >= 0 is true")
# set satellite position for test 0
satellite0X.append(posSat0['x'])
satellite0Y.append(posSat0['y'])
satellite0Z.append(posSat0['z'])
satellite1X.append(posSat1['x'])
satellite1Y.append(posSat1['y'])
satellite1Z.append(posSat1['z'])
# set ignoreInterference
ignoreInterference.append('false')
# set expected received packets of the SatCar
expectedReceivedPacketsSatCar.append('1')
# set app start times
startTimeSat0.append("0.003s")
startTimeSat1.append("0s")



# print omnetpp.ini config lines
print('\n omnetpp.ini copnfig lines')
print("# configure radio medium")
print("*.satelliteRadioMedium.mediumLimitCache.minElevationAngleV2S = " + str(minElevationAngleDeg) + "deg")
print("")
print("# configure apps")
print("*.**.app[0].testNumber = ${testNumber=", ', '.join(str(t) for t in testNumbers), "}")
print("# -- satellites")
print("*.leo*[*].app[0].expectedReceivedPackets = ${expectedReceivedPacketsLeo=", ', '.join('0' for e in testNumbers), "! testNumber}")
print("*.leo*[*].app[0].expectedSentPackets = 1")
print("*.leo*[*].app[0].sendInterval = 20s")
print("# -- -- satellite 0")
print("*.leo*[0].app[0].startTime = ${sat0StartTime=", ', '.join(s for s in startTimeSat0), "! testNumber}")
print("# -- -- satellite 1")
print("*.leo*[1].app[0].startTime = ${sat1StartTime=", ', '.join(s for s in startTimeSat1), "! testNumber}")
print("")
print("# -- SatCar")
print("# choose start time > sim-time-limit such that the SatCar does not transmit a packet")
print("*.node[*].app[0].startTime = 0s")
print("# choose send interval > sim-time-limit such that the SatCar does not transmit a packet")
print("*.node[*].app[0].sendInterval = 31s")
print("*.node[*].app[0].expectedReceivedPackets = ${expectedReceivedPacketsGroundStation=", ', '.join(e for e in expectedReceivedPacketsSatCar), "! testNumber}")
print("*.node[*].app[0].expectedSentPackets = 0")
print("")
print("# configure leo satellite positions")
print("# -- satellite 0")
print("*.leo*[0].mobility.initialX = ${sat0X=", ', '.join(str(i) for i in satellite0X), "! testNumber}m")
print("*.leo*[0].mobility.initialY = ${sat0Y=", ', '.join(str(i) for i in satellite0Y), "! testNumber}m")
print("*.leo*[0].mobility.initialZ = ${sat0Z=", ', '.join(str(i) for i in satellite0Z), "! testNumber}m")
print("# -- satellite 1")
print("*.leo*[1].mobility.initialX = ${sat1X=", ', '.join(str(i) for i in satellite1X), "! testNumber}m")
print("*.leo*[1].mobility.initialY = ${sat1Y=", ', '.join(str(i) for i in satellite1Y), "! testNumber}m")
print("*.leo*[1].mobility.initialZ = ${sat1Z=", ', '.join(str(i) for i in satellite1Z), "! testNumber}m")
print("")
print("# configure ignoreInterference")
print("*.**.satNic[0].radio.receiver.ignoreInterference = ${ignoreInterference=,", ', '.join(i for i in ignoreInterference), "! testNumber}")
