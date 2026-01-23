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
-o $1 \
--call-graph $2 \
--no-bpf-event \
--per-thread \
$3

perf script -i "$1" > "${1}.out"

###############################
#         fold stacks         #
###############################

/ramdisk/FlameGraph/stackcollapse-perf.pl "${1}.out" > "${1}.out.folded"

###############################
#      create flamegraph      #
###############################

/ramdisk/FlameGraph/flamegraph.pl "${1}.out.folded" > "${1}.svg"