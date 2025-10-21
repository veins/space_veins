#!/bin/bash

echo "List of all perf probes:"
perf probe --list
echo "Deleting all perf probes!:"
perf probe -d '*'
echo "List of all perf probes:"
perf probe --list