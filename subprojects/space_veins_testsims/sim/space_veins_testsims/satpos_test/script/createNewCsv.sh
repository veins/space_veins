#!/bin/sh

# SPDX-FileCopyrightText: 2025 Mario Franke <research@m-franke.net>
#
# SPDX-License-Identifier: GPL-2.0-or-later

./opp_vec2csv.pl --merge-by em -F omnetCoordX:vector -F omnetCoordY:vector -F omnetCoordZ:vector ../results/Test-j=0,warmupPeriod=10,simTimeLimit=100,numVehicles=1,payloadSize=1000-\#0.vec | cut -f2-6 > new.csv
sort -n new.csv > newValidateSatPos.csv
rm new.csv