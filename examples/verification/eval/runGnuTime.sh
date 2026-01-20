#!/bin/bash
set -x

# $1 location where to store gnu time results
# $2 command to be measured
# $3 OMNeT++ config
# $4 repetition

/usr/bin/time \
-o $1 \
--format "%C,%U,%S,%e,%M,%F,%W,%P" \
$2

sleep 1
###############################
#           clean up          #
###############################
# mkdir -p archive/latest/results
# mkdir -p archive/latest/csv/receptionsMap/rawData

# mv /ramdisk/results/${3}*${4}.* archive/latest/results
# mv /ramdisk/ReceptionsMap_${3}_${4}.csv archive/latest/csv/receptionsMap/rawData

# sleep 1