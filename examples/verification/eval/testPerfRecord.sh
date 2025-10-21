#!/bin/bash
set -x

# run perf record, use --call-graph dwarf for more reliable and precise stack traces and --call-graph fp for less disk usage
perf record \
-e probe_opp_run:main_rls \
-e probe_opp_run:main_rls__return \
-e probe_libspace_veins:wgcp_init_rls \
-e probe_libspace_veins:wgcp_init_rls__return \
-e probe_libspace_veins:gcp_rls \
-e probe_libspace_veins:gcp_rls__return \
-e probe_libspace_veins:hsm_rls \
-e probe_libspace_veins:hsm_rls__return \
-m 16384 \
-o /ramdisk/perf.data \
--call-graph fp \
./run -- -u Cmdenv -c RtpDisUpPosOnlyAboveHorizonWGCP0060 -r 58

# get results in a nice way
perf script -i /ramdisk/perf.data --ns -F comm,time,event > testPerfRecord.perf

# clean up
rm /ramdisk/perf.data
