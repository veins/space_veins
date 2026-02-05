#!/bin/bash
set -x

mkdir -p /ramdisk/franke/fg
./eval/createFlamegraph.sh /ramdisk/franke/fg/ngcp.perf.data dwarf "./run --mode release -- -u Cmdenv -c VncDebug -r 0"
./eval/createFlamegraph.sh /ramdisk/franke/fg/wgcp.perf.data dwarf "./run --mode release -- -u Cmdenv -c VncDebug -r 5"
./eval/createFlamegraph.sh /ramdisk/franke/fg/wlip.perf.data dwarf "./run --mode release -- -u Cmdenv -c VncDebug -r 10"
./eval/createFlamegraph.sh /ramdisk/franke/fg/wnpp.perf.data dwarf "./run --mode release -- -u Cmdenv -c VncDebug -r 15"
