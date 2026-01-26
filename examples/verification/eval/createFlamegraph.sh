#!/bin/bash
set -x

# $1 name of the flamegraph
# $2 call-graph creating method: fp or dwarf
# $3 command that is being perfed

###############################
#           run perf          #
###############################

perf record \
-F 99 \
-e cycles:u \
-o $1 \
--call-graph "${2},65528" \
--no-bpf-event \
--per-thread \
$3

perf script --no-inline -i "$1" > "${1}.out"
mv "$1" archive/latest

###############################
#         fold stacks         #
###############################

/ramdisk/FlameGraph/stackcollapse-perf.pl "${1}.out" > "${1}.out.folded"
mv "${1}.out" archive/latest

###############################
#      create flamegraph      #
###############################

/ramdisk/FlameGraph/flamegraph.pl "${1}.out.folded" > "${1}.svg"
mv "${1}.out.folded" archive/latest
mv "${1}.svg" archive/latest