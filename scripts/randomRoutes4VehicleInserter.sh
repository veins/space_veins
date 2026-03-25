#!/bin/bash

# Copyright (C) 2023 Mario Franke <research@m-franke.net>
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
# SPDX-FileCopyrightText: 2023 Mario Franke <research@m-franke.net>
#
# SPDX-License-Identifier: GPL-2.0-or-later

# $1 = *.net.xml input file
# $2 = *.trips.xml output file
# $3 = *.rou.xml output file

if [ $# -lt 3 ]; then
    echo "Not enough arguments provided. Usage:\n bash randomRoutes4VehicleInserter Null-Island-3by3-grid.net.xml Null-Island-3by3-grid.trips.xml Null-Island-3by3-grid.rou.xml"
    exit 1
fi

# Create vehicles with random routes such that SUMO creates the vehicles
# echo "Generating random trips file $2"
# python3 $SUMO_HOME/tools/randomTrips.py -n $1 -o $2 -i 3100 --fringe-factor 100 -b 0 --end 10 --period 0.118 --binomial 10
# echo "Building routes file $3"
# $SUMO_HOME/bin/duarouter -n $1 --route-files $2 -o $3

# create named routes for TraCIVehicleInserter
echo "Generating random trips file $2"
python3 $SUMO_HOME/tools/randomTrips.py -n $1 -o $2 -i 3100 --fringe-factor 100 --end 255 --period 10
echo "Building routes file $3"
$SUMO_HOME/bin/duarouter -n $1 --route-files $2 -o $3 --named-routes true
# delete vehicles because the TraCIVehicleInserter creates the vehicles
sed -i '/^    <vehicle/d' $3
