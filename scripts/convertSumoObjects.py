# Copyright (C) 2023 Dakota Leslie

# SPDX-License-Identifier: GPL-2.0-or-later

# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

import xml.etree.ElementTree as ET
import math
import random
from xml.dom import minidom
import argparse

# converts the convboundary string from the net file into doubles to transform all the coordinates into INET coordinates
def coordinateConversion(locData):
    locData = locData.split(',')
    
    if (len(locData) > 4):
        quit()

    x_1 = float(locData[0])
    y_1 = float(locData[1])
    x_2 = float(locData[2])
    y_2 = float(locData[3])

    return [[x_1,y_1],[x_2,y_2]]

# creates a dictionary of useable coordinate values from the poly file using the SUMO ids as keys for each object
def objectCoords(temp):
    #creates a dictionary of lists that contains coordinate pairs as strings
    temp_list = {}
    dict(temp_list)
    for value in temp.keys():
        temp_list[value] = temp[value].split()

    #creates a dictionary of lists that contains single values as strings
    coordList_string = {}
    dict(coordList_string)
    for i in temp_list.keys():
        temp_string = []
        temp_coord = []
        for value in temp_list[i]:
            temp_string = value.split(',')
            for item in temp_string:
                temp_coord.append(item)
        coordList_string[i] = temp_coord

    del temp_list
    
    # turn the dictionary of lists containing single value strings into a dictionary of lists containing doubles
    coordList = {}
    dict(coordList)
    for i in coordList_string.keys():
        temp_double = []
        for value in coordList_string[i]:
            temp_double.append(float(value))
        coordList[i] = temp_double

    del coordList_string
    del temp
    
    for i in coordList.keys():
        coordList[i].pop(-1)
        coordList[i].pop(-1)

    return coordList

# transforms the sumo coords into coorisponding inet values keeping the SUMO ids attatched with the correct objects
# margin default is 25
def transformCoords(coordList, topLeft, dimensions, margin):
    newCoordList = {}
    dict(newCoordList)
    for obj in coordList.keys():
        newObj = []
        for i in range(len(coordList[obj])):
            if (i % 2 == 0):
                newObj.append(coordList[obj][i] - topLeft[0] + margin)
            else:
                newObj.append(dimensions[1] - (coordList[obj][i] - topLeft[1]) + margin)
        newCoordList[obj] = newObj.copy()
            
    return newCoordList

# finds the minimum coord for each object
# needed for the INeT object file
def findMinCoord(objects):
    minCoord = [5000000000, 5000000000]

    tempCoord = [0, 0]
    for i in range(len(objects)):
        if (i % 2 != 0):
            if (objects[i] < minCoord[1]):
                minCoord[1] = objects[i]
        else:
            if (objects[i] < minCoord[0]):
                minCoord[0] = objects[i]
    minCoord.append(0)

    return minCoord 

# writes the object file that INeT uses for objects
def writeFile(finalCoordList, outputFile, material, color, height):
    root = ET.Element('environment')

    tree = ET.ElementTree()
    tree._setroot(root)
    listEle = []
    for objects in finalCoordList.keys():
        minCoord = findMinCoord(finalCoordList[objects]) 
        temp = ET.Element('object')
        temp.set('id', str(objects))
        temp.set('material', material)
        temp.set('line-color', str(color[0]) + " " + str(color[1]) + " " + str(color[2]))
        temp.set('opacity','0.8')
        temp.set('position', 'min ' + str(minCoord[0]) + " " + str(minCoord[1]) + " " + str(minCoord[2]))
        shape_string = "prism " + str(height)
        for items in finalCoordList[objects]:
            shape_string = shape_string + " " + str(items)
        temp.set('shape', shape_string)
        listEle.append(temp)

    tree.getroot().extend(listEle)
    
    #This is for more organized xml output (easier for humans to read)
    xmlstr = minidom.parseString(ET.tostring(tree.getroot())).toprettyxml(indent=" ")
    
    with open(outputFile, "w") as f:
        f.write(xmlstr)

    return None

# initialize the parser and its arguments to allow the code to work as well as output being customized
parser = argparse.ArgumentParser(description='converts the SUMO generated poly file into an xml file that can be read by INeT. SUMO generated .net file name and directory required to run script, and the SUMO generated poly file must be in the same directory.')
parser.add_argument('file', metavar='FILE', help='The file (and directory) of the poly file for your simulation -- NOTE: the format of the expected input file is "file-name.net.xml" the generated file will have the same directory as the input file and the naming convention is "input-file-name.object.xml" for the output file.')
parser.add_argument('-height', metavar='HEIGHT', type=float, default=5.0, help='input the heihgt you would like your buildings to be (float variable), default is 5. NOTE: If randHeight is set to True, this height value will NOT be taken into account')
parser.add_argument('-margin', metavar='MARGIN', type=float, default=25, help='if you have changed the margin in your simulation, denote that new value here (default is 25).')
parser.add_argument('-randHeight', metavar='RAND_HEIGHT', type=bool, default=False, help='if you want the height to be random type "True", default is false. NOTE: This arg will overwrite a user decided height.')
parser.add_argument('-material', metavar='MATERIAL', type=str, default="concrete", help='Change the material the object is made out of from the list of built in INeT materials: vacuum, air, copper, aluminum, wood, forest, brick, concrete, glass. Default is concrete. If the material does not match the list, the value will default to conctete.')
parser.add_argument('-color', metavar='COLOR', type=str, default="255 0 0", help='Change the color of the objects by using the decimal code of the color R G B. Default is red 255 0 0. Invalid color inputs default to red 255 0 0. Expected input: "R G B"')

args = parser.parse_args()

# gets the file name and directory name for the SUMO net file and creates the name of the poly file using SUMO's naming convention as well as naming the output file similarly
inputFile = args.file
polyFile = inputFile.replace("net", "poly")
outputFile = polyFile.replace("poly", "object")

# initializes the margin value for coordinate conversion (default is 25)
margin = args.margin

# initializes height acorrding to user input (randHeight overrides manual height)
if (args.randHeight):
    height = random.randint(5, 151)
else:
    height = args.height

# initializes the material according to the pre-made material list within INeT
if (args.material == "vacuum" or args.material == "air" or args.material == "copper" or args.material == "aluminium" or args.material == "wood" or args.material == "forest" or args.material == "brick" or args.material == "concrete" or args.material == "glass"):
    material = args.material
else:
    material = "concrete"

tempColor = args.color
tempColor = tempColor.split(" ")
for i in range(len(tempColor)):
    tempColor[i] = float(tempColor[i])

if ((tempColor[0] > 255 or tempColor[0] < 0) or (tempColor[1] > 255 or tempColor[1] < 0) or (tempColor[2] > 255 or tempColor[2] < 0)):
    color = [255, 0, 0]
else:
    color = tempColor

# opens the net file generated by SUMO
tree = ET.parse(inputFile)
root = tree.getroot()

# finds the convBoundary attribute from the location tag for coordinate transformation the topLeft coord and bottomRight coord are created from the convBoundary and used for calculations
locData = root.find('location').get('convBoundary')
convBound = coordinateConversion(locData)
topLeft = [convBound[0][0], convBound[0][1]]
bottomRight = [convBound[1][0], convBound[1][1]]
dimensions = [(bottomRight[0] - topLeft[0]), (bottomRight[1] - topLeft[1])]

del locData
# opens the object file generated by SUMO
tree = ET.parse(polyFile)
root = tree.getroot()

# finds all the poly attributes called shape and adds them into a dictionary object with SUMO ids as the keys
shapes = tree.findall("./poly")
temp = {}
dict(temp) 
for items in shapes:
    if (items.get('type') == "building"):
        temp[items.get('id')] = items.get('shape')

del shapes
del tree
del root

coordList = objectCoords(temp)
newCoordList = transformCoords(coordList, topLeft, dimensions, margin)

writeFile(newCoordList, outputFile, material, color, height)
