# SPDX-FileCopyrightText: 2023 Mario Franke <research@m-franke.net>
#
# SPDX-License-Identifier: GPL-2.0-or-later

import argparse
import subprocess

parser = argparse.ArgumentParser(description='Create a Manhattan.')
parser.add_argument('dimension', metavar='DIMENSION', type=int, default=3, help='Dimension of the Manhattan grid.')
parser.add_argument('sideLength', metavar='SIDE_LENGTH', type=float, help='The Manhattan grid\'s side length in km.')
parser.add_argument('scenarioName', metavar='scenarioName', help='The name for the scenario.')
parser.add_argument('-lat', '--latitude', dest='latitude', type=float, default=0.0, help="Latitude of top left node in deg [default: 0.0deg]")
parser.add_argument('-lon', '--longitude', dest='longitude', type=float, default=0.0, help="Longitude of top left node in deg [default: 0.0m]")
parser.add_argument('-d', '--degreePerKilometer', dest='degreePerKilometer', type=str, default='NullIsland', help="Methode to use for calculating the degree per kilometer values. Possible values [\"NullIsland\", \"reykjavik\", \"hamburg\", \"calculation\"] [default: NullIsland]")
parser.add_argument('-n', '--numLanes', dest='numLanes', type=int, default=1, help="Number of lanes per edge. [default: 1].")
parser.add_argument('-p', '--priority', dest='priority', type=int, default=10, help="Edge priority. [default: 10].")
parser.add_argument('-s', '--speed', dest='speed', type=float, default=13.89, help="Allowed vehile speed. [default: 13.89].")
parser.add_argument('-lw', '--laneWidth', dest='laneWidth', type=float, default=3.2, help="SUMO default lane width in meter. [default: 3.2m].")
parser.add_argument('-rr', '--randomRoutes', metavar='randomRoutes', type=bool, default=False, help='Create random routes. [default: False].')
parser.add_argument('-bm', '--buildingMargin', dest='buildingMargin', type=float, default=5.0, help="Margin between end of the outermost lane end the buildings in meter. [default: 5.0m].")
parser.add_argument('-cs', '--buildingColorSUMO', dest='buildingColorSUMO', type=str, default="red", help="The buildings' color. [default: red].")
parser.add_argument('-bh', '--buildingHeightINET', metavar='height', type=float, default=9.0, help='input the heihgt you would like your buildings to be (float variable), default is 5. NOTE: If randHeight is set to True, this height value will NOT be taken into account')
parser.add_argument('-vm', '--veinsMargin', metavar='margin', type=float, default=25, help='if you have changed the margin parameter in your simulation, denote that new value here (default is 25).')
parser.add_argument('-r', '--randomBuildingHeightINET', metavar='randomBuildingHeightINET', type=bool, default=False, help='if you want the height to be random type "True", default is false. NOTE: This arg will overwrite a user decided height.')
parser.add_argument('-mi', '--materialINET', metavar='buildingMaterialINET', type=str, default="concrete", help='Change the material the object is made out of from the list of built in INeT materials: vacuum, air, copper, aluminum, wood, forest, brick, concrete, glass. Default is concrete. If the material does not match the list, the value will default to conctete.')
parser.add_argument('-ci', '--buildingColorINET', metavar='buildingColorINET', type=str, default="255 0 0", help='Change the color of the objects by using the decimal code of the color R G B. Default is red 255 0 0. Invalid color inputs default to red 255 0 0. Expected input: "R G B"')

args = parser.parse_args()

# create node file
nodeFile = open(args.scenarioName + ".nod.xml", "w")
p = subprocess.Popen(["python3", "createManhattanGridNodeFile.py", str(args.dimension), str(args.sideLength), "--latitude="+str(args.latitude), "--longitude="+str(args.longitude), "--degreePerKilometer", args.degreePerKilometer], stdout=nodeFile)
p.wait()
# exit if return code is not 0
if p.returncode != 0:
    print("Error: Could not create node file. Error message is written in the output file.")
    exit(p.returncode)

# create edge file
edgeFile = open(args.scenarioName + ".edg.xml", "w")
p = subprocess.Popen(["python3", "createManhattanGridEdgeFile.py", str(args.dimension), "-n", str(args.numLanes), "-p", str(args.priority), "-s", str(args.speed)], stdout=edgeFile)
p.wait()

# create geo poly file
polyFile = open(args.scenarioName + ".geo.poly.xml", "w")
p = subprocess.Popen(["python3", "createManhattanGridBuildings.py", str(args.dimension), str(args.sideLength), "--latitude="+str(args.latitude), "--longitude="+str(args.longitude), "--degreePerKilometer", args.degreePerKilometer, "-n", str(args.numLanes), "-l", str(args.laneWidth), "-b", str(args.buildingMargin), "-c", str(args.buildingColorSUMO)], stdout=polyFile)
p.wait()

# create net file
p = subprocess.Popen(["netconvert", "--node-files=" + args.scenarioName + ".nod.xml", "--edge-files=" + args.scenarioName + ".edg.xml", "--proj.utm", "--output-file=" + args.scenarioName + ".net.xml"])
p.wait()

# create sumo poly file
p = subprocess.Popen(["polyconvert", "--xml-files=" + args.scenarioName + ".geo.poly.xml", "--net-file=" + args.scenarioName + ".net.xml", "--output-file=" + args.scenarioName + ".poly.xml"])
p.wait()

# create inet object file
if (not args.randomBuildingHeightINET):
    p = subprocess.Popen(["python3", "convertSumoObjects.py", args.scenarioName + ".net.xml", "-height", str(args.buildingHeightINET), "-margin", str(args.veinsMargin), "-material", str(args.materialINET), "-color", str(args.buildingColorINET)])
else:
    p = subprocess.Popen(["python3", "convertSumoObjects.py", args.scenarioName + ".net.xml", "-margin", str(args.veinsMargin), "-randHeight", str(args.randomBuildingHeightINET), "-material", str(args.materialINET), "-color", str(args.buildingColorINET)])
p.wait()

# create route files
if (args.randomRoutes):
    p = subprocess.Popen(["bash", "randomRoutes4VehicleInserter.sh", args.scenarioName + ".net.xml", args.scenarioName + ".trips.xml", args.scenarioName + ".rou.xml"])
    p.wait()