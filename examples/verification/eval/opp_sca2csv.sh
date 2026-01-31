#!/bin/bash

INPUT=$(/home/mafr432d/git/space_veins/lib/veins_scripts/eval/opp_sca2csv.pl --list FI ${1}/results/*.sca)
# echo "INPUT: ${INPUT}"

# Skip first line, split into array by lines
readarray -t LINES < <(printf '%s\n' "$INPUT" | tail -n +2)

# Process lines directly (no subshell!)
FILTERS=() ITERS=()
for line in "${LINES[@]}"; do
    IFS=$'\t' read -r type name <<< "$line"
    case "$type" in
        filter)
            # Replace : with _ in filter names
            name="-F ${name}=${name//:/_}"
            FILTERS+=("$name")
            ;;
        itervar)
            name="-I ${name}"
            ITERS+=("$name")
            ;;
    esac
done

# echo "Filters (${#FILTERS[@]}): ${FILTERS[*]}"
# echo "Itervars (${#ITERS[@]}): ${ITERS[@]}"

/home/mafr432d/git/space_veins/lib/veins_scripts/eval/opp_sca2csv.pl -M 'ManhattanBuildingShadowing\.*' ${ITERS[@]} ${FILTERS[*]} ${1}/results/*.sca > ${1}/scalars.csv