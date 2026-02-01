#!/bin/bash



groundTruthFile="${1}"
ngcpFile="${2}"
wlipFile="${3}"
wnppFile="${4}"
allOtherFiles="${ngcpFile} ${wlipFile} ${wnppFile}"

# echo "$groundTruthFile"
# echo "$ngcpFile"
# echo "$wlipFile"
# echo "$wnppFile"

BASEPATH=$(dirname "$groundTruthFile")
# echo "$BASEPATH"
intermediate_path="${BASEPATH}/../intermediate"

output=""

parse_config() {
    local input="$1"

    # 1. Extract config: substring before first "-"
    config="${input%%-*}"

    # 2. Extract middle part (between "-" and "#") - removes the trailing "-" before #
    middle="${input#*-}"
    middle="${middle%%-*#*}"  # This removes everything from last "-" before # onwards
    IFS=',' read -ra pairs <<< "$middle"

    # Declare associative array for key-value pairs
    declare -g -A params

    # Parse each key=value pair
    for pair in "${pairs[@]}"; do
        key="${pair%%=*}"
        value="${pair#*=}"
        params["$key"]="$value"
    done

    # 3. Extract repetition: between "#" and ".rx.csv"
    repetition="${input#*\#}"
    repetition="${repetition%%.rx.csv*}"
}

for f in $allOtherFiles
do
    BASENAME_F=$(basename "$f")
    parse_config "$BASENAME_F"
    output+="$config,"
    # for key in "${!params[@]}"; do
    #     output+="${params[$key]},"
    # done
    # output+="${params['simTimeLimit'],${params['warmupPeriod']},}"
    output+="${params['simTimeLimit']},${params['warmupPeriod']},${params['geo']},${params['leo']},${params['hour']},${params['minEle']},${params['mobilityInterval']},${params['gcp']},${params['filter']},${params['txInterval']},"
    output+="$repetition,"
    # detect all line that are not within the ground truth, these are the false positives and the false negatives
    comm -13 <(sort "${groundTruthFile}") <(sort ${f}) > "${intermediate_path}/${BASENAME_F}.falseLines.txt"
    cat "${intermediate_path}/${BASENAME_F}.falseLines.txt" | wc -l > "${intermediate_path}/${BASENAME_F}.falseLines.counts.txt"
    # detect all false positive lines
    grep '1$' "${intermediate_path}/${BASENAME_F}.falseLines.txt" > "${intermediate_path}/${BASENAME_F}.fp.txt"
    cat "${intermediate_path}/${BASENAME_F}.fp.txt" | wc -l > "${intermediate_path}/${BASENAME_F}.fp.counts.txt"
    fp=$(cat "${intermediate_path}/${BASENAME_F}.fp.txt" | wc -l)
    output+="${fp},"
    # detect all false negative lines
    grep '0$' "${intermediate_path}/${BASENAME_F}.falseLines.txt" > "${intermediate_path}/${BASENAME_F}.fn.txt"
    cat "${intermediate_path}/${BASENAME_F}.fn.txt" | wc -l > "${intermediate_path}/${BASENAME_F}.fn.counts.txt"
    fn=$(cat "${intermediate_path}/${BASENAME_F}.fn.txt" | wc -l)
    output+="${fn},"

    # detect all line that are within the ground truth, these are the true positives and the true negatives
    comm -12 <(sort "${groundTruthFile}") <(sort ${f}) > "${intermediate_path}/${BASENAME_F}.trueLines.txt"
    cat "${intermediate_path}/${BASENAME_F}.trueLines.txt" | wc -l > "${intermediate_path}/${BASENAME_F}.trueLines.counts.txt"
    # detect all true positive lines
    grep '1$' "${intermediate_path}/${BASENAME_F}.trueLines.txt" > "${intermediate_path}/${BASENAME_F}.tp.txt"
    cat "${intermediate_path}/${BASENAME_F}.tp.txt" | wc -l > "${intermediate_path}/${BASENAME_F}.tp.counts.txt"
    tp=$(cat "${intermediate_path}/${BASENAME_F}.tp.txt" | wc -l)
    output+="${tp},"
    # detect all true negative lines
    grep '0$' "${intermediate_path}/${BASENAME_F}.trueLines.txt" > "${intermediate_path}/${BASENAME_F}.tn.txt"
    cat "${intermediate_path}/${BASENAME_F}.tn.txt" | wc -l > "${intermediate_path}/${BASENAME_F}.tn.counts.txt"
    tn=$(cat "${intermediate_path}/${BASENAME_F}.tn.txt" | wc -l)
    output+="${tn}"$'\n'
done

#remove last line break
output=${output%?}
echo "$output"
# echo "$output" > classification.csv