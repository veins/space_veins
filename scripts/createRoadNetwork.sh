#!/bin/bash

# SPDX-FileCopyrightText: 2023 Mario Franke <research@m-franke.net>
#
# SPDX-License-Identifier: GPL-2.0-or-later

# $1 = *.nod.xml input file
# $2 = *.edg.xml input file
# $3 = *.net.xml output file

if [ $# -lt 3 ]; then
    echo "Not enough arguments provided. Usage:\n bash createRoadNetwork.sh Null-Island-3by3-grid.nod.xml Null-Island-3by3-grid.edg.xml Null-Island-3by3-grid.net.xml"
    exit 1
fi

netconvert --node-files=$1 --edge-files=$2 --proj.utm --output-file=$3
