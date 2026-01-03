#!/bin/bash
set -x

# $1 location of the perf record report; should be placed in RAM as this must be fast memory
# $2 method for generating the call graph; use ```dwarf``` for more reliable and precise stack traces and ```fp``` for less disk usage
# $3 command to be perfed
# $4 location of the perf script output

###############################
#          set probes         #
###############################

# main
perf probe --exec /opt/omnetpp/bin/opp_run_dbg \
--add main_dbg=main \
--add main_dbg=main%return \
--no-demangle

# SGP4Mobilities initialize
# NoGetCurrentPostionUpdates
perf probe --exec /workspace/space_veins/src/libspace_veins_dbg.so --add ngcp_init_dbg=_ZN11space_veins39SGP4MobilityNoGetCurrentPositionUpdates10initializeEi --add ngcp_init_dbg=_ZN11space_veins39SGP4MobilityNoGetCurrentPositionUpdates10initializeEi%return --no-demangle
# perf probe --exec /workspace/space_veins/src/libspace_veins.so --add ngcp_init_rls=_ZN11space_veins39SGP4MobilityNoGetCurrentPositionUpdates10initializeEi --add ngcp_init_rls=_ZN11space_veins39SGP4MobilityNoGetCurrentPositionUpdates10initializeEi%return --no-demangle

# WithGetCurrentPositionUpdate
perf probe --exec /workspace/space_veins/src/libspace_veins_dbg.so --add wgcp_init_dbg=_ZN11space_veins41SGP4MobilityWithGetCurrentPositionUpdates10initializeEi --add wgcp_init_dbg=_ZN11space_veins41SGP4MobilityWithGetCurrentPositionUpdates10initializeEi%return --no-demangle
# perf probe --exec /workspace/space_veins/src/libspace_veins.so --add wgcp_init_rls=_ZN11space_veins41SGP4MobilityWithGetCurrentPositionUpdates10initializeEi --add wgcp_init_rls=_ZN11space_veins41SGP4MobilityWithGetCurrentPositionUpdates10initializeEi%return --no-demangle

# WithLinearInterpolationUpdate
perf probe --exec /workspace/space_veins/src/libspace_veins_dbg.so --add wlip_init_dbg=_ZN11space_veins42SGP4MobilityWithLinearInterpolationUpdates10initializeEi --add wlip_init_dbg=_ZN11space_veins42SGP4MobilityWithLinearInterpolationUpdates10initializeEi%return --no-demangle
# perf probe --exec /workspace/space_veins/src/libspace_veins.so --add wlip_init_rls=_ZN11space_veins42SGP4MobilityWithLinearInterpolationUpdates10initializeEi --add wlip_init_rls=_ZN11space_veins42SGP4MobilityWithLinearInterpolationUpdates10initializeEi%return --no-demangle

# WithNearestPeriodicPositionUpdate
perf probe --exec /workspace/space_veins/src/libspace_veins_dbg.so --add wnpp_init_dbg=_ZN11space_veins46SGP4MobilityWithNearestPeriodicPositionUpdates10initializeEi --add wnpp_init_dbg=_ZN11space_veins46SGP4MobilityWithNearestPeriodicPositionUpdates10initializeEi%return --no-demangle
# perf probe --exec /workspace/space_veins/src/libspace_veins.so --add wnpp_init_rls=_ZN11space_veins46SGP4MobilityWithNearestPeriodicPositionUpdates10initializeEi --add wnpp_init_rls=_ZN11space_veins46SGP4MobilityWithNearestPeriodicPositionUpdates10initializeEi%return --no-demangle

# SGP4MobilityBase getCurrentPosition() - for random time updates
perf probe --exec /workspace/space_veins/src/libspace_veins_dbg.so --no-inlines --add gcp_dbg=_ZN11space_veins16SGP4MobilityBase18getCurrentPositionEv --add gcp_dbg=_ZN11space_veins16SGP4MobilityBase18getCurrentPositionEv%return --no-demangle
# perf probe --exec /workspace/space_veins/src/libspace_veins.so --no-inlines --add gcp_rls=_ZN11space_veins16SGP4MobilityBase18getCurrentPositionEv --add gcp_rls=_ZN11space_veins16SGP4MobilityBase18getCurrentPositionEv%return --no-demangle

# SGP4MobilityBase handleSelfMessage() - for periodic updates
perf probe --exec /workspace/space_veins/src/libspace_veins_dbg.so --add hsm_dbg=_ZN11space_veins16SGP4MobilityBase17handleSelfMessageEPN7omnetpp8cMessageE --add hsm_dbg=_ZN11space_veins16SGP4MobilityBase17handleSelfMessageEPN7omnetpp8cMessageE%return --no-demangle

sleep 1

###############################
#           run perf          #
###############################
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
--no-bpf-event \
--per-thread \
-F 99 \
$3

sleep 1

# get results in a nice way
perf script -i $1 --ns -F comm,time,event > $4

###############################
#           clean up          #
###############################
sleep 1
# rm $1
mv $1 perfResults/
perf probe -d '*'

sleep 1