#!/bin/bash

folder_path="rawData"
intermediate_path="intermediate"
mkdir -p "$intermediate_path"
filters="Dis En"

output="filter,updateInterval,repetition,gcpBehavior,fp,fn,tp,tn"$'\n'
for filter in $filters
do
    filter_files=$(ls "$folder_path" | grep "$filter")
    echo "Matching filter files:"
    echo "$filter_files"

    unique_updateIntervals=$(echo "$filter_files" | tr ' ' '\n' | awk -F'_' '{print $(NF-1)}' | sort -u)
    echo "Unique updateIntervals:"
    echo "$unique_updateIntervals"

    unique_repetitions=$(echo "$filter_files" | tr ' ' '\n' | awk -F'[_.]' '{print $(NF-1)}' | sort -u)
    echo "Unique repetitions:"
    echo "$unique_repetitions"

    for ui in $unique_updateIntervals
    do
        matching_updateInterval_files=$(echo "$filter_files" | tr ' ' '\n' | grep "$ui" | tr '\n' ' ')
        for r in $unique_repetitions
        do
            # matching_repetition_files=$(echo "$matching_updateInterval_files" | tr ' ' '\n' | grep "$r" | tr '\n' ' ')
            groundTruthFile="ReceptionsMap_Runtime${filter}UpPosOnlyAboveHorizonWGCP_${ui}_${r}.csv"
            ngcpFile="ReceptionsMap_Runtime${filter}UpPosOnlyAboveHorizonNGCP_${ui}_${r}.csv"
            wlipFile="ReceptionsMap_Runtime${filter}UpPosOnlyAboveHorizonWLIP_${ui}_${r}.csv"
            wnppFile="ReceptionsMap_Runtime${filter}UpPosOnlyAboveHorizonWNPP_${ui}_${r}.csv"
            allOtherFiles="${ngcpFile} ${wlipFile} ${wnppFile}"
            for f in $allOtherFiles
            do
                gcpBehavior=$(echo "$f" | sed 's/.*Horizon\([^_]*\)_\([^_]*\)_[^.]*\.csv/\1/')
                output+="${filter},${ui},${r},${gcpBehavior},"
                # detect all line that are not within the ground truth, these are the false positives and the false negatives
                comm -13 <(sort "${folder_path}/${groundTruthFile}") <(sort ${folder_path}/${f}) > "${intermediate_path}/${f}.falseLines.txt"
                cat "${intermediate_path}/${f}.falseLines.txt" | wc -l > "${intermediate_path}/${f}.falseLines.counts.txt"
                # detect all false positive lines
                grep '1$' "${intermediate_path}/${f}.falseLines.txt" > "${intermediate_path}/${f}.fp.txt"
                cat "${intermediate_path}/${f}.fp.txt" | wc -l > "${intermediate_path}/${f}.fp.counts.txt"
                fp=$(cat "${intermediate_path}/${f}.fp.txt" | wc -l)
                output+="${fp},"
                # detect all false negative lines
                grep '0$' "${intermediate_path}/${f}.falseLines.txt" > "${intermediate_path}/${f}.fn.txt"
                cat "${intermediate_path}/${f}.fn.txt" | wc -l > "${intermediate_path}/${f}.fn.counts.txt"
                fn=$(cat "${intermediate_path}/${f}.fn.txt" | wc -l)
                output+="${fn},"
                # detect all line that are within the ground truth, these are the true positives and the true negatives
                comm -12 <(sort "${folder_path}/${groundTruthFile}") <(sort ${folder_path}/${f}) > "${intermediate_path}/${f}.trueLines.txt"
                cat "${intermediate_path}/${f}.trueLines.txt" | wc -l > "${intermediate_path}/${f}.trueLines.counts.txt"
                # detect all true positive lines
                grep '1$' "${intermediate_path}/${f}.trueLines.txt" > "${intermediate_path}/${f}.tp.txt"
                cat "${intermediate_path}/${f}.tp.txt" | wc -l > "${intermediate_path}/${f}.tp.counts.txt"
                tp=$(cat "${intermediate_path}/${f}.tp.txt" | wc -l)
                output+="${tp},"
                # detect all true negative lines
                grep '0$' "${intermediate_path}/${f}.trueLines.txt" > "${intermediate_path}/${f}.tn.txt"
                cat "${intermediate_path}/${f}.tn.txt" | wc -l > "${intermediate_path}/${f}.tn.counts.txt"
                tn=$(cat "${intermediate_path}/${f}.tn.txt" | wc -l)
                output+="${tn}"$'\n'
            done
        done
    done
done
#remove last line break
output=${output%?}
echo "$output" > classification.csv