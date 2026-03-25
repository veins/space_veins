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
# SPDX-FileCopyrightText: 2026 Mario Franke <research@m-franke.net>
#
# SPDX-License-Identifier: GPL-2.0-or-later

import argparse
import math
import sys

parser = argparse.ArgumentParser(description='Create a Manhattan grid SUMO edge file.')
parser.add_argument('dimension', metavar='DIMENSION', type=int, default=5, help='Dimension of the Manhattan grid.')
parser.add_argument('sideLength', metavar='SIDE_LENGTH', type=float, help='The Manhattan grid\'s side length in km.')
parser.add_argument('-n', '--numLanes', dest='numLanes', type=int, default=1, help="Number of lanes per edge. [default: 1].")
parser.add_argument('-l', '--laneWidth', dest='laneWidth', type=float, default=3.2, help="SUMO default lane width in meter. [default: 3.2m].")
parser.add_argument('-b', '--buildingMargin', dest='buildingMargin', type=float, default=5.0, help="Margin between end of the outermost lane end the buildings in meter. [default: 5.0m].")
parser.add_argument('-c', '--buildingColor', dest='buildingColor', type=str, default="red", help="The buildings' color. [default: red].")
parser.add_argument('-lat', '--latitude', dest='latitude', type=float, default=0.0, help="Latitude of top left node in deg [default: 0.0deg]")
parser.add_argument('-lon', '--longitude', dest='longitude', type=float, default=0.0, help="Longitude of top left node in deg [default: 0.0m]")
parser.add_argument('-d', '--degreePerKilometer', dest='degreePerKilometer', type=str, default='calculation', help="Methode to use for calculating the degree per kilometer values. Possible values [\"nullIsland\", \"reykjavik\", \"hamburg\", \"calculation\"] [default: calculation]")

args = parser.parse_args()

if not (args.degreePerKilometer == "nullIsland" or
        args.degreePerKilometer == "reykjavik" or
        args.degreePerKilometer == "hamburg" or
        args.degreePerKilometer == "calculation"):
    print("Error: Unsupported method for defining the degree per kilometer value.")
    exit(1)

degreePerKilometerX_NullIsland = 0.008974365
degreePerKilometerY_NullIsland = 0.009034808
degreePerKilometerXReykjavik = 0.02025814365907971
degreePerKilometerYReykjavik = 0.008850902690732099
degreePerKilometerXHamburg = 0.014883647519555166
degreePerKilometerYHamburg = 0.008864606487403906
degreePerKilometerX = 0.02025814365907971
degreePerKilometerY = 0.008850902690732099

# TODO: improve precision
# promissing approach: https://en.wikipedia.org/wiki/Geographic_coordinate_system
def calculateDegreePerKilometer(latitude):
    # in meters; see https://en.wikipedia.org/wiki/World_Geodetic_System
    wgs84_semi_major_axis = 6378137.0
    wgs84_flattening = 1 / 298.257223563
    wgs84_semi_minor_axis = 6356752.314245
    wgs84_semi_minor_axis = wgs84_semi_major_axis * (1 - wgs84_flattening)
    # print("wgs84_semi_minor_axis: ", wgs84_semi_minor_axis)
    # radius at given latitude in km
    # see https://www.quora.com/How-do-I-calculate-the-circumference-of-latitude-70-N
    radiusAtLat_km = wgs84_semi_major_axis * math.cos(math.atan(wgs84_semi_minor_axis / wgs84_semi_major_axis * math.tan(math.radians(latitude)))) / 1000
    # circumference at given latitude in km
    circumferenceAtLat_km = 2 * math.pi * radiusAtLat_km
    # print("circumferenceAtLat_km: ", circumferenceAtLat_km)
    # lon/x degree per kilometer at given latitude
    degreePerKilometerX = (360 / circumferenceAtLat_km)
    print("degreePerKilometerX: ", degreePerKilometerX, file=sys.stderr)

    # assume Earth is a perfect sphere
    degreePerKilometerY = (360 / (2 * math.pi * wgs84_semi_minor_axis / 1000))
    print("degreePerKilometerY: ", degreePerKilometerY, file=sys.stderr)
    return degreePerKilometerX, degreePerKilometerY

if args.degreePerKilometer == "reykjavik":
    print("degreePerKilometer use reykjavik values.", file=sys.stderr)
    degreePerKilometerX = degreePerKilometerXReykjavik
    degreePerKilometerY = degreePerKilometerYReykjavik
elif args.degreePerKilometer == "nullIsland":
    print("degreePerKilometer use nullIsland values.", file=sys.stderr)
    degreePerKilometerX = degreePerKilometerX_NullIsland
    degreePerKilometerY = degreePerKilometerY_NullIsland
elif args.degreePerKilometer == "hamburg":
    print("degreePerKilometer use hamburg values.", file=sys.stderr)
    degreePerKilometerX = degreePerKilometerXHamburg
    degreePerKilometerY = degreePerKilometerYHamburg
elif args.degreePerKilometer == "calculation":
    print("degreePerKilometer calculate values.", file=sys.stderr)
    degreePerKilometerX, degreePerKilometerY = calculateDegreePerKilometer(args.latitude)

interNodeDistance_km = args.sideLength / args.dimension
print("interNodeDistance_km: ", interNodeDistance_km, file=sys.stderr)
interNodeDistance_x_deg = interNodeDistance_km * degreePerKilometerX
print("interNodeDistance_x_deg: ", interNodeDistance_x_deg, file=sys.stderr)
interNodeDistance_y_deg = interNodeDistance_km * degreePerKilometerY
print("interNodeDistance_y_deg: ", interNodeDistance_y_deg, file=sys.stderr)

buildingSideLength_km = interNodeDistance_km - 2 * (args.laneWidth / 1000) * args.numLanes - 2 * (args.buildingMargin / 1000)
print("buildingSideLength_km: ", buildingSideLength_km, file=sys.stderr)

buildingSideLength_x_deg = buildingSideLength_km * degreePerKilometerX
print("buildingSideLength_x_deg: ", buildingSideLength_x_deg, file=sys.stderr)
buildingSideLength_y_deg = buildingSideLength_km * degreePerKilometerY
print("buildingSideLength_y_deg: ", buildingSideLength_y_deg, file=sys.stderr)

laneWidth_x_deg = args.laneWidth / 1000 * degreePerKilometerX
print("laneWidth_x_deg: ", laneWidth_x_deg, file=sys.stderr)
laneWidth_y_deg = args.laneWidth / 1000 * degreePerKilometerY
print("laneWidth_y_deg: ", laneWidth_y_deg, file=sys.stderr)

buildingMargin_x_deg = args.buildingMargin / 1000 * degreePerKilometerX
print("buildingMargin_x_deg: ", buildingMargin_x_deg, file=sys.stderr)
buildingMargin_y_deg = args.buildingMargin / 1000 * degreePerKilometerY
print("buildingMargin_y_deg: ", buildingMargin_y_deg, file=sys.stderr)

numBuildings = 0

# add one to create buildings that fully surround every street
dimension = args.dimension + 1

print("<additional>")

# x is equal to longitude
# y is equal to latitude
# start at -1 to create buildings outside of the road grid, too
for x in range(-1, dimension):
    for y in range(-1, dimension):

        topLeftBuildingX = args.longitude + x * interNodeDistance_x_deg + args.numLanes * laneWidth_x_deg + buildingMargin_x_deg
        topLeftBuildingY = args.latitude - y * interNodeDistance_y_deg - args.numLanes * laneWidth_y_deg - buildingMargin_y_deg

        bottomLeftBuildingX = topLeftBuildingX
        bottomLeftBuildingY = topLeftBuildingY - buildingSideLength_y_deg

        topRightBuildingX = bottomLeftBuildingX + buildingSideLength_x_deg
        topRightBuildingY = bottomLeftBuildingY + buildingSideLength_y_deg

        bottomRightBuildingX = bottomLeftBuildingX + buildingSideLength_x_deg
        bottomRightBuildingY = bottomLeftBuildingY

        # sumo expects longitude,latitude
        shapeString = str(bottomLeftBuildingX) + "," + str(bottomLeftBuildingY) + " "
        shapeString += str(topLeftBuildingX) + "," + str(topLeftBuildingY) + " "
        shapeString += str(topRightBuildingX) + "," + str(topRightBuildingY) + " "
        shapeString += str(bottomRightBuildingX) + "," + str(bottomRightBuildingY) + " "
        # add bottomLeft corner again, such that the polygon in closed
        # the script "convertSumoObjects.py" deletes the doubled coordinate
        # such that INET creates correct objects
        shapeString += str(bottomLeftBuildingX) + "," + str(bottomLeftBuildingY)

        print("\t<poly id=\"poly_" + str(numBuildings) + "\" type=\"building\" color=\"" + args.buildingColor + "\" fill=\"1\" layer=\"0.00\" geo=\"1\" shape=\"" + shapeString + "\"/>")
        numBuildings = numBuildings + 1
print("</additional>")