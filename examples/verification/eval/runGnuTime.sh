#!/bin/bash
set -x

# $1 = OMNeT++ config name
# $2 = run number

# get OMNeT++ iteration variables
itervars=$(./run -- -q runs -c ${1} -r ${2} | grep '^Run' | sed 's/^Run \S*: //')

# Clean itervars and split into key=value pairs
clean_input=$(echo "$itervars" | sed 's/^ *//; s/ *$//; s/ *, */,/g; s/\$//g')

# Create variables from each key=value pair
while IFS=',' read -ra PAIR; do
    for kv in "${PAIR[@]}"; do
        IFS='=' read -r key value <<< "$kv"
        # Handle quoted values properly
        if [[ $value == \"*\" ]]; then
            value=$(echo "$value" | sed 's/^"//; s/"$//')
        fi
        # Create variable dynamically
        printf -v "${key}" "%s" "$value"
        printf -v "keyName_${key}" "%s" "$key"
    done
done <<< "${clean_input}"

# shorten geo and leo iteration variable values as they contain "/" (they are paths)
IFS='/' read -ra parts <<< "$geo"
geo="${parts[0]}"
IFS='/' read -ra parts <<< "$leo"
leo="${parts[1]}"

# filename="${1}-${keyName_simTimeLimit}=${simTimeLimit},${keyName_warmupPeriod}=${warmupPeriod},${keyName_geo}=${geo},${keyName_leo}=${leo},${keyName_hour}=${hour},${keyName_wct}=${wct},${keyName_minEle}=${minEle},${keyName_mobilityInterval}=${mobilityInterval},${keyName_gcp}=${gcp},${keyName_filter}=${filter},${keyName_txInterval}=${txInterval},${keyName_repetition}=${repetition}-#0"
# create base file name
filename="${1}-${keyName_simTimeLimit}=${simTimeLimit},${keyName_warmupPeriod}=${warmupPeriod},${keyName_geo}=${geo},${keyName_leo}=${leo},${keyName_hour}=${hour},${keyName_minEle}=${minEle},${keyName_mobilityInterval}=${mobilityInterval},${keyName_gcp}=${gcp},${keyName_filter}=${filter},${keyName_txInterval}=${txInterval}-#${repetition}"
filenameShort="${1}-${2}"

# create receptions map file name and path
mkdir -p /tmp/franke/rx
receptionsOutputFileName="${filename}.rx.csv"
receptionsOutputFilePath="/tmp/franke/rx/${receptionsOutputFileName}"
## check if file already exists and delete it
# uncomment for testing: touch $receptionsOutputFilePath
if [ -f ${receptionsOutputFilePath} ]; then
    rm ${receptionsOutputFilePath}
    echo "Deleted old receptions file!"
fi

# create gnu time results file name and path
mkdir -p archive/latest/csv/gnuTime
gnuTimeOutputFile="archive/latest/csv/gnuTime/${filenameShort}.time.csv"
gnuTimeFormatString="Config,${keyName_simTimeLimit},${keyName_warmupPeriod},${keyName_geo},${keyName_leo},${keyName_hour},${keyName_minEle},${keyName_mobilityInterval},${keyName_gcp},${keyName_filter},${keyName_txInterval},${keyName_repetition},userTime,kernelTime,wallClockTime,maxRSS,pageFaults,numberSwappedOutOfMainMemory,cpuUsage\n${1},${simTimeLimit},${warmupPeriod},${geo},${leo},${hour},${minEle},${mobilityInterval},${gcp},${filter},${txInterval},${repetition},%U,%S,%e,%M,%F,%W,%P"

# measure simulation run time with gnu time
/usr/bin/time \
-o $gnuTimeOutputFile \
--format $gnuTimeFormatString \
./run --mode debug -- -u Cmdenv -c ${1} -r ${2} --"*.globalStatistics.receptionsMapPath=\"${receptionsOutputFilePath}\""

# copy receptions map file from local disk to NFS
mkdir -p archive/latest/csv/receptionsMap/rawData
# uncomment for testing: touch $receptionsOutputFilePath
cp $receptionsOutputFilePath archive/latest/csv/receptionsMap/rawData/${receptionsOutputFileName}
echo $(pwd)
