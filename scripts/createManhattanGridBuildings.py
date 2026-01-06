# SPDX-FileCopyrightText: 2023 Mario Franke <research@m-franke.net>
#
# SPDX-License-Identifier: GPL-2.0-or-later

import argparse

parser = argparse.ArgumentParser(description='Create a Manhattan grid SUMO edge file.')
parser.add_argument('dimension', metavar='DIMENSION', type=int, default=5, help='Dimension of the Manhattan grid.')
parser.add_argument('sideLength', metavar='SIDE_LENGTH', type=float, help='The Manhattan grid\'s side length in km.')
parser.add_argument('-n', '--numLanes', dest='numLanes', type=int, default=1, help="Number of lanes per edge. [default: 1].")
parser.add_argument('-l', '--laneWidth', dest='laneWidth', type=float, default=3.2, help="SUMO default lane width in meter. [default: 3.2m].")
parser.add_argument('-b', '--buildingMargin', dest='buildingMargin', type=float, default=5.0, help="Margin between end of the outermost lane end the buildings in meter. [default: 5.0m].")
parser.add_argument('-c', '--buildingColor', dest='buildingColor', type=str, default="red", help="The buildings' color. [default: red].")
parser.add_argument('-lat', '--latitude', dest='latitude', type=float, default=0.0, help="Latitude of top left node in deg [default: 0.0deg]")
parser.add_argument('-lon', '--longitude', dest='longitude', type=float, default=0.0, help="Longitude of top left node in deg [default: 0.0m]")
parser.add_argument('-d', '--degreePerKilometer', dest='degreePerKilometer', type=str, default='NullIsland', help="Methode to use for calculating the degree per kilometer values. Possible values [\"NullIsland\", \"reykjavik\", \"hamburg\", \"calculation\"] [default: NullIsland]")

args = parser.parse_args()

if not (args.degreePerKilometer == "NullIsland" or
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

if args.degreePerKilometer == "Reykjavik":
    degreePerKilometerX = degreePerKilometerXReykjavik
    degreePerKilometerY = degreePerKilometerYReykjavik
elif args.degreePerKilometer == "NullIsland":
    degreePerKilometerX = degreePerKilometerX_NullIsland
    degreePerKilometerY = degreePerKilometerY_NullIsland
elif args.degreePerKilometer == "hamburg":
    degreePerKilometerY = degreePerKilometerXHamburg
    degreePerKilometerX = degreePerKilometerYHamburg
elif args.degreePerKilometer == "calculation":
    degreePerKilometerX, degreePerKilometerY = calculateDegreePerKilometer(args.latitude)

interNodeDistance_km = args.sideLength / args.dimension
interNodeDistance_x_deg = interNodeDistance_km * degreePerKilometerX
interNodeDistance_y_deg = interNodeDistance_km * degreePerKilometerY

buildingSideLength_m = interNodeDistance_km * 1000 - 2 * args.laneWidth * args.numLanes - 2 * args.buildingMargin
buildingSideLength_x_deg = buildingSideLength_m / 1000 * degreePerKilometerX
buildingSideLength_y_deg = buildingSideLength_m / 1000 * degreePerKilometerY

laneWidth_x_deg = args.laneWidth / 1000 * degreePerKilometerX
laneWidth_y_deg = args.laneWidth / 1000 * degreePerKilometerY

buildingMargin_x_deg = args.buildingMargin / 1000 * degreePerKilometerX
buildingMargin_y_deg = args.buildingMargin / 1000 * degreePerKilometerY

numBuildings = 0

# add two to create buildings that fully surround every street
dimension = args.dimension + 2

# print("<additional xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"http://sumo.dlr.de/xsd/additional_file.xsd\">")
print("<additional>")
for y in range(0, dimension):
    for x in range(0, dimension):

        topLeftBuildingX = args.latitude - x * interNodeDistance_x_deg + args.numLanes * laneWidth_x_deg + buildingMargin_x_deg
        topLeftBuildingY = args.longitude + y * interNodeDistance_y_deg - args.numLanes * laneWidth_y_deg - buildingMargin_y_deg

        bottomLeftBuildingX = topLeftBuildingX
        bottomLeftBuildingY = topLeftBuildingY - buildingSideLength_y_deg

        topRightBuildingX = bottomLeftBuildingX + buildingSideLength_x_deg
        topRightBuildingY = bottomLeftBuildingY + buildingSideLength_y_deg

        bottomRightBuildingX = bottomLeftBuildingX + buildingSideLength_x_deg
        bottomRightBuildingY = bottomLeftBuildingY

        # bottomLeftBuildingX = args.latitude + x * interNodeDistance_x_deg + args.numLanes * laneWidth_x_deg + buildingMargin_x_deg
        # bottomLeftBuildingY = args.longitude + y * interNodeDistance_y_deg + args.numLanes * laneWidth_y_deg + buildingMargin_y_deg

        # topLeftBuildingX = bottomLeftBuildingX
        # topLeftBuildingY = bottomLeftBuildingY + buildingSideLength_y_deg

        # topRightBuildingX = bottomLeftBuildingX + buildingSideLength_x_deg
        # topRightBuildingY = bottomLeftBuildingY + buildingSideLength_y_deg

        # bottomRightBuildingX = bottomLeftBuildingX + buildingSideLength_x_deg
        # bottomRightBuildingY = bottomLeftBuildingY

        # shapeString = str(bottomLeftBuildingX) + "," + str(bottomLeftBuildingY) + " "
        # shapeString += str(topLeftBuildingX) + "," + str(topLeftBuildingY) + " "
        # shapeString += str(topRightBuildingX) + "," + str(topRightBuildingY) + " "
        # shapeString += str(bottomRightBuildingX) + "," + str(bottomRightBuildingY) + " "
        # # add bottomLeft corner again, such that the polygon in closed
        # # the script "convertSumoObjects.py" deletes the doubled coordinate
        # # such that INET creates correct objects
        # shapeString += str(bottomLeftBuildingX) + "," + str(bottomLeftBuildingY)

        shapeString = str(bottomLeftBuildingY) + "," + str(bottomLeftBuildingX) + " "
        shapeString += str(topLeftBuildingY) + "," + str(topLeftBuildingX) + " "
        shapeString += str(topRightBuildingY) + "," + str(topRightBuildingX) + " "
        shapeString += str(bottomRightBuildingY) + "," + str(bottomRightBuildingX) + " "
        # add bottomLeft corner again, such that the polygon in closed
        # the script "convertSumoObjects.py" deletes the doubled coordinate
        # such that INET creates correct objects
        shapeString += str(bottomLeftBuildingY) + "," + str(bottomLeftBuildingX)


        print("\t<poly id=\"poly_" + str(numBuildings) + "\" type=\"building\" color=\"" + args.buildingColor + "\" fill=\"1\" layer=\"0.00\" geo=\"1\" shape=\"" + shapeString + "\"/>")
        numBuildings = numBuildings + 1
print("</additional>")