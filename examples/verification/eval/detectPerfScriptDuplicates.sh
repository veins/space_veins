#!/bin/bash

# deduplicate_file.sh - Creates unique file + removed duplicates file
# Usage: ./deduplicate_file.sh input.txt
# Output: input_unique.txt  (first occurrence of each line)
#         input_removed.txt (all duplicate occurrences that were removed)

if [ $# -ne 1 ]; then
    echo "Usage: $0 <input_file>"
    exit 1
fi

input_file="$1"
unique_file="${input_file%.*}.unique.perf"
removed_file="${input_file%.*}.removed.perf"

if [ ! -f "$input_file" ]; then
    echo "Error: File '$input_file' not found."
    exit 1
fi

echo "Processing '$input_file'..."
echo "- Unique lines → '$unique_file'"
echo "- Removed duplicates → '$removed_file'"

# Create unique file (first occurrence of each line, preserves order)
awk '!seen[$0]++' "$input_file" > "$unique_file"

# Create removed duplicates file (all lines minus first occurrence)
awk 'seen[$0]++' "$input_file" > "$removed_file"

unique_count=$(wc -l < "$unique_file")
removed_count=$(wc -l < "$removed_file")
total_count=$(wc -l < "$input_file")

echo "Done!"
echo "Unique lines: $unique_count"
echo "Removed lines: $removed_count (total duplicates)"
echo "Original total: $total_count"
