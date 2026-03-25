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

parser = argparse.ArgumentParser(description='Create a Manhattan grid SUMO edge file.')
parser.add_argument('dimension', metavar='DIMENSION', type=int, default=5, help='Dimension of the Manhattan grid.')
parser.add_argument('-n', '--numLanes', dest='numLanes', type=int, default=1, help="Number of lanes per edge. [default: 1].")
parser.add_argument('-p', '--priority', dest='priority', type=int, default=10, help="Edge priority. [default: 10].")
parser.add_argument('-s', '--speed', dest='speed', type=float, default=13.89, help="Allowed vehile speed. [default: 13.89].")

args = parser.parse_args()
rangeArg = args.dimension + 1

print("<edges>")
# print all horizontal edges
for y in range(0, rangeArg):
    for x in range(0, rangeArg):
        if x < args.dimension:
            print("\t<edge id=\"he" + str(x) + "-" + str(y) + "To" + str(x + 1) + "-" + str(y) + "\" from=\"n" + str(x) + "_" + str(y) + "\" to=\"n" + str(x + 1) + "_" + str(y) + "\" priority=\"" + str(args.priority) + "\" numLanes=\"" + str(args.numLanes) + "\" speed=\"" + str(args.speed) + "\"/>")
            print("\t<edge id=\"-he" + str(x) + "-" + str(y) + "To" + str(x + 1) + "-" + str(y) + "\" to=\"n" + str(x) + "_" + str(y) + "\" from=\"n" + str(x + 1) + "_" + str(y) + "\" priority=\"" + str(args.priority) + "\" numLanes=\"" + str(args.numLanes) + "\" speed=\"" + str(args.speed) + "\"/>")
print ("\n")
# print all vertical edges
for x in range(0, rangeArg):
    for y in range(0, rangeArg):
        if y < args.dimension:
            print("\t<edge id=\"ve" + str(x) + "-" + str(y) + "To" + str(x) + "-" + str(y + 1) + "\" from=\"n" + str(x) + "_" + str(y) + "\" to=\"n" + str(x) + "_" + str(y + 1) + "\" priority=\"" + str(args.priority) + "\" numLanes=\"" + str(args.numLanes) + "\" speed=\"" + str(args.speed) + "\"/>")
            print("\t<edge id=\"-ve" + str(x) + "-" + str(y) + "To" + str(x) + "-" + str(y + 1) + "\" to=\"n" + str(x) + "_" + str(y) + "\" from=\"n" + str(x) + "_" + str(y + 1) + "\" priority=\"" + str(args.priority) + "\" numLanes=\"" + str(args.numLanes) + "\" speed=\"" + str(args.speed) + "\"/>")
print("</edges>")