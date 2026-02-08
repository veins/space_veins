#!/bin/bash
set -x

mkdir -p /ramdisk/franke/fg
# ngcp, periodic update interval 0.01s
./eval/createFlamegraph.sh /ramdisk/franke/fg/ngcp_ui_0c01_r0.perf.data dwarf "./run --mode release -- -u Cmdenv -c VncDebug -r 0"
# # ngcp, periodic update interval 0.01s
# ./eval/createFlamegraph.sh /ramdisk/franke/fg/ngcp_ui_0c01_r1.perf.data dwarf "./run --mode release -- -u Cmdenv -c VncDebug -r 0"
# # ngcp, periodic update interval 0.01s
# ./eval/createFlamegraph.sh /ramdisk/franke/fg/ngcp_ui_0c01_r2.perf.data dwarf "./run --mode release -- -u Cmdenv -c VncDebug -r 0"
# # ngcp, periodic update interval 0.01s
# ./eval/createFlamegraph.sh /ramdisk/franke/fg/ngcp_ui_0c01_r3.perf.data dwarf "./run --mode release -- -u Cmdenv -c VncDebug -r 0"

# wgcp, periodic update interval 0.01s
./eval/createFlamegraph.sh /ramdisk/franke/fg/wgcp_ui_0c01_r0.perf.data dwarf "./run --mode release -- -u Cmdenv -c VncDebug -r 5"
# # wgcp, periodic update interval 0.01s
# ./eval/createFlamegraph.sh /ramdisk/franke/fg/wgcp_ui_0c01_r1.perf.data dwarf "./run --mode release -- -u Cmdenv -c VncDebug -r 5"
# # wgcp, periodic update interval 0.01s
# ./eval/createFlamegraph.sh /ramdisk/franke/fg/wgcp_ui_0c01_r2.perf.data dwarf "./run --mode release -- -u Cmdenv -c VncDebug -r 5"
# # wgcp, periodic update interval 0.01s
# ./eval/createFlamegraph.sh /ramdisk/franke/fg/wgcp_ui_0c01_r3.perf.data dwarf "./run --mode release -- -u Cmdenv -c VncDebug -r 5"

# ngcp, periodic update interval 1s
./eval/createFlamegraph.sh /ramdisk/franke/fg/ngcp_ui_1c0_r0.perf.data dwarf "./run --mode release -- -u Cmdenv -c VncDebug -r 40"

# wgcp, periodic update interval 1s
./eval/createFlamegraph.sh /ramdisk/franke/fg/wgcp_ui_1c0_r0.perf.data dwarf "./run --mode release -- -u Cmdenv -c VncDebug -r 45"
