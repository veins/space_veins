#!/bin/bash

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
echo "Generating random trips file $2"
python3 $SUMO_HOME/tools/randomTrips.py -n $1 -o $2 -i 3100 --fringe-factor 100 -b 0 --end 10 --period 0.118 --binomial 10
echo "Building routes file $3"
$SUMO_HOME/bin/duarouter -n $1 --route-files $2 -o $3

# create named routes for TraCIVehicleInserter
# echo "Generating random trips file $2"
# python3 $SUMO_HOME/tools/randomTrips.py -n $1 -o $2 -i 3100 --fringe-factor 100 --end 255 --period 10
# echo "Building routes file $3"
# $SUMO_HOME/bin/duarouter -n $1 --route-files $2 -o $3 --named-routes true
# # delete vehicles because the TraCIVehicleInserter creates the vehicles
# sed -i '/^    <vehicle/d' $3
