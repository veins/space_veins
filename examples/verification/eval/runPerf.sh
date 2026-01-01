#!/bin/bash
set -x

# $1 location of the perf record report; should be placed in RAM as this must be fast memory
# $2 method for generating the call graph; use ```dwarf``` for more reliable and precise stack traces and ```fp``` for less disk usage
# $3 command to be perfed
# $4 location of the perf script output
# -e probe_opp_run_dbg:main_L3 \
# -e probe_opp_run_dbg:main_dbg \
# -e probe_opp_run_dbg:main_dbg__return \
# -e probe_opp_run:main_rls \
# -e probe_opp_run:main_rls__return \


perf record \
-e probe_opp_run_dbg:main_dbg \
-e probe_opp_run_dbg:main_dbg__return \
-e probe_libspace_veins_dbg:ngcp_init_dbg \
-e probe_libspace_veins_dbg:ngcp_init_dbg__return \
-e probe_libspace_veins_dbg:wgcp_init_dbg \
-e probe_libspace_veins_dbg:wgcp_init_dbg__return \
-e probe_libspace_veins_dbg:wlip_init_dbg \
-e probe_libspace_veins_dbg:wlip_init_dbg__return \
-e probe_libspace_veins_dbg:wnpp_init_dbg \
-e probe_libspace_veins_dbg:wnpp_init_dbg__return \
-e probe_libspace_veins_dbg:gcp_dbg \
-e probe_libspace_veins_dbg:gcp_dbg__return \
-e probe_libspace_veins_dbg:hsm_dbg \
-e probe_libspace_veins_dbg:hsm_dbg__return \
-o $1 \
--call-graph $2 \
--verbose \
-a \
$3

# get results in a nice way
perf script -i $1 --ns -F comm,time,event > $4

# clean up
# rm $1

# $1: /ramdisk/perf.data
# $2: fp
# $3: ./run --debug -- -u Cmdenv -c RtpDisUpPosOnlyAboveHorizonWGCP0060 -r 58
# $4: testPerfRecord_dbg.perf

# -k CLOCK_MONOTONIC \
# -m 16384 \
# -a \