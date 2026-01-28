# SPDX-FileCopyrightText: 2023 Mario Franke <research@m-franke.net>
#
# SPDX-License-Identifier: GPL-2.0-or-later

import argparse
import math

parser = argparse.ArgumentParser(description='Create a Manhattan grid SUMO Node file.\n The top left node is located all Null-Island which is at 0.0, 0.0 in WGS84 coordinates.')
parser.add_argument('dimension', metavar='DIMENSION', type=int, help='Dimension of the Manhattan grid.')
parser.add_argument('sideLength', metavar='SIDE_LENGTH', type=float, help='The Manhattan grid\'s side length in km.')
parser.add_argument('-lat', '--latitude', dest='latitude', type=float, default=0.0, help="Latitude of top left node in deg [default: 0.0deg]")
parser.add_argument('-lon', '--longitude', dest='longitude', type=float, default=0.0, help="Longitude of top left node in deg [default: 0.0m]")
parser.add_argument('-d', '--degreePerKilometer', dest='degreePerKilometer', type=str, default='calculation', help="Methode to use for calculating the degree per kilometer values. Possible values [\"nullIsland\", \"reykjavik\", \"hamburg\", \"calculation\"] [default: calculation]")

args = parser.parse_args()
rangeArg = args.dimension + 1

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
    # print("degreePerKilometerX: ", degreePerKilometerX)

    # assume Earth is a perfect sphere
    degreePerKilometerY = (360 / (2 * math.pi * wgs84_semi_minor_axis / 1000))
    # print("degreePerKilometerY: ", degreePerKilometerY)
    return degreePerKilometerX, degreePerKilometerY

if args.degreePerKilometer == "reykjavik":
    degreePerKilometerX = degreePerKilometerXReykjavik
    degreePerKilometerY = degreePerKilometerYReykjavik
elif args.degreePerKilometer == "nullIsland":
    degreePerKilometerX = degreePerKilometerX_NullIsland
    degreePerKilometerY = degreePerKilometerY_NullIsland
elif args.degreePerKilometer == "hamburg":
    degreePerKilometerX = degreePerKilometerXHamburg
    degreePerKilometerY = degreePerKilometerYHamburg
elif args.degreePerKilometer == "calculation":
    degreePerKilometerX, degreePerKilometerY = calculateDegreePerKilometer(args.latitude)
# print("degreePerKilometerX: ", degreePerKilometerX)
# print("degreePerKilometerY: ", degreePerKilometerY)
interNodeDistance_km = args.sideLength / args.dimension

print("<nodes>")
# print all nodes from left to right and top to bottom
# Coordinate origin is in the top left corner -> y Coords are negative
for y in range(0, rangeArg):
    for x in range(0, rangeArg):
        print("\t<node id=\"n" + str(x) + "_" + str(y) + "\" x=\"" + str(degreePerKilometerX * interNodeDistance_km * x + args.longitude) + "\" y=\"" + str(degreePerKilometerY * interNodeDistance_km * -y + args.latitude) + "\" type=\"priority\"/>")
    print("")
print("</nodes>")