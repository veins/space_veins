#!/bin/bash

#!/bin/bash

# Multi-line input stored in a variable
input=$(ls "${1}")

# 1. FIRST: Parse ALL unique values (including repetitions of any count)
declare -A unique_simTimeLimit unique_warmupPeriod unique_geo unique_leo unique_hour 
declare -A unique_minEle unique_mobilityInterval unique_gcp unique_filter unique_txInterval unique_repetitions

while IFS= read -r line; do
    # Extract config part
    config="${line%%-*}"
    
    # Extract middle part (parameters) - FIXED parsing
    middle="${line#*-}"
    middle="${middle%%#*}"           # Cut at "#"
    middle="${middle%-}"             # Remove trailing "-" before "#"
    
    # Split parameters by comma
    IFS=',' read -ra pairs <<< "$middle"
    
    # Parse each key=value
    for pair in "${pairs[@]}"; do
        key="${pair%%=*}"
        value="${pair#*=}"
        case "$key" in
            simTimeLimit) [[ -z "${unique_simTimeLimit[$value]}" ]] && unique_simTimeLimit[$value]=1 ;;
            warmupPeriod) [[ -z "${unique_warmupPeriod[$value]}" ]] && unique_warmupPeriod[$value]=1 ;;
            geo) [[ -z "${unique_geo[$value]}" ]] && unique_geo[$value]=1 ;;
            leo) [[ -z "${unique_leo[$value]}" ]] && unique_leo[$value]=1 ;;
            hour) [[ -z "${unique_hour[$value]}" ]] && unique_hour[$value]=1 ;;
            minEle) [[ -z "${unique_minEle[$value]}" ]] && unique_minEle[$value]=1 ;;
            mobilityInterval) [[ -z "${unique_mobilityInterval[$value]}" ]] && unique_mobilityInterval[$value]=1 ;;
            gcp) [[ -z "${unique_gcp[$value]}" ]] && unique_gcp[$value]=1 ;;
            filter) [[ -z "${unique_filter[$value]}" ]] && unique_filter[$value]=1 ;;
            txInterval) [[ -z "${unique_txInterval[$value]}" ]] && unique_txInterval[$value]=1 ;;
        esac
    done
    
    # Extract repetition (handles 0,1,2,3,... any number)
    repetition="${line##*#}"
    repetition="${repetition%%.rx.csv*}"
    [[ -z "${unique_repetitions[$repetition]}" ]] && unique_repetitions[$repetition]=1
    
done <<< "$input"

# 2. Convert to indexed arrays for looping
declare -a simTimeLimit_values=( "${!unique_simTimeLimit[@]}" )
declare -a warmupPeriod_values=( "${!unique_warmupPeriod[@]}" )
declare -a geo_values=( "${!unique_geo[@]}" )
declare -a leo_values=( "${!unique_leo[@]}" )
declare -a hour_values=( "${!unique_hour[@]}" )
declare -a minEle_values=( "${!unique_minEle[@]}" )
declare -a mobilityInterval_values=( "${!unique_mobilityInterval[@]}" )
declare -a gcp_values=( "${!unique_gcp[@]}" )
declare -a filter_values=( "${!unique_filter[@]}" )
declare -a txInterval_values=( "${!unique_txInterval[@]}" )
declare -a repetition_values=( "${!unique_repetitions[@]}" )  # 0,1,2,3,... automatically

count=0
combinations=""
for simTimeLimit in "${simTimeLimit_values[@]}"; do
    for warmupPeriod in "${warmupPeriod_values[@]}"; do
        for geo in "${geo_values[@]}"; do
            for leo in "${leo_values[@]}"; do
                for hour in "${hour_values[@]}"; do
                    for minEle in "${minEle_values[@]}"; do
                        for mobilityInterval in "${mobilityInterval_values[@]}"; do
                            for filter in "${filter_values[@]}"; do
                                for txInterval in "${txInterval_values[@]}"; do
                                    for repetition in "${repetition_values[@]}"; do  # Works for 2, 5, 10+ repetitions
                                        ((count++))
                                        combinations+="$config $simTimeLimit $warmupPeriod $geo $leo $hour $minEle $mobilityInterval $filter $txInterval $repetition $1"$'\n'
                                    done
                                done
                            done
                        done
                    done
                done
            done
        done
    done
done
# echo "There are ${count} different combinations"

# create csv header
output="config,"
for pair in "${pairs[@]}"; do
    key="${pair%%=*}"
    output+="${key},"
done
output+="repetition,fp,fn,tp,tn"
echo "$output"

# run diff in parallel, -P is the number of jobs
echo "$combinations" | xargs -P 32 -I {} bash -c '
    set -- {}
    config=${1}; simTimeLimit=$2; warmupPeriod=$3; geo=$4; leo=$5; hour=$6; minEle=$7; mobilityInterval=$8; filter=$9; txInterval=${10}; repetition=${11}; path=${12};
    # YOUR WORK HERE;
    /home/mafr432d/git/space_veins/examples/verification/eval/compareReceptionsMapFiles.sh \
    ${path}/${config}-simTimeLimit=${simTimeLimit},warmupPeriod=${warmupPeriod},geo=${geo},leo=${leo},hour=${hour},minEle=${minEle},mobilityInterval=${mobilityInterval},gcp=SGP4MobilityWithGetCurrentPositionUpdates,filter=${filter},txInterval=${txInterval}-#${repetition}.rx.csv \
    ${path}/${config}-simTimeLimit=${simTimeLimit},warmupPeriod=${warmupPeriod},geo=${geo},leo=${leo},hour=${hour},minEle=${minEle},mobilityInterval=${mobilityInterval},gcp=SGP4MobilityNoGetCurrentPositionUpdates,filter=${filter},txInterval=${txInterval}-#${repetition}.rx.csv \
    ${path}/${config}-simTimeLimit=${simTimeLimit},warmupPeriod=${warmupPeriod},geo=${geo},leo=${leo},hour=${hour},minEle=${minEle},mobilityInterval=${mobilityInterval},gcp=SGP4MobilityWithLinearInterpolationUpdates,filter=${filter},txInterval=${txInterval}-#${repetition}.rx.csv \
    ${path}/${config}-simTimeLimit=${simTimeLimit},warmupPeriod=${warmupPeriod},geo=${geo},leo=${leo},hour=${hour},minEle=${minEle},mobilityInterval=${mobilityInterval},gcp=SGP4MobilityWithNearestPeriodicPositionUpdates,filter=${filter},txInterval=${txInterval}-#${repetition}.rx.csv;
' _ {}