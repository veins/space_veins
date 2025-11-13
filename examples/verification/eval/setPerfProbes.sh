#!/bin/bash
# main
perf probe --exec /opt/omnetpp/bin/opp_run_dbg --add main_dbg=main --add main_dbg=main%return --no-demangle
perf probe --exec /opt/omnetpp/bin/opp_run --add main_rls=main --add main_rls=main%return --no-demangle

# SGP4Mobilities initialize
# NoGetCurrentPostionUpdates
perf probe --exec /workspace/space_veins/src/libspace_veins_dbg.so --add ngcp_init_dbg=_ZN11space_veins39SGP4MobilityNoGetCurrentPositionUpdates10initializeEi --add ngcp_init_dbg=_ZN11space_veins39SGP4MobilityNoGetCurrentPositionUpdates10initializeEi%return --no-demangle
perf probe --exec /workspace/space_veins/src/libspace_veins.so --add ngcp_init_rls=_ZN11space_veins39SGP4MobilityNoGetCurrentPositionUpdates10initializeEi --add ngcp_init_rls=_ZN11space_veins39SGP4MobilityNoGetCurrentPositionUpdates10initializeEi%return --no-demangle

# WithGetCurrentPositionUpdate
perf probe --exec /workspace/space_veins/src/libspace_veins_dbg.so --add wgcp_init_dbg=_ZN11space_veins41SGP4MobilityWithGetCurrentPositionUpdates10initializeEi --add wgcp_init_dbg=_ZN11space_veins41SGP4MobilityWithGetCurrentPositionUpdates10initializeEi%return --no-demangle
perf probe --exec /workspace/space_veins/src/libspace_veins.so --add wgcp_init_rls=_ZN11space_veins41SGP4MobilityWithGetCurrentPositionUpdates10initializeEi --add wgcp_init_rls=_ZN11space_veins41SGP4MobilityWithGetCurrentPositionUpdates10initializeEi%return --no-demangle

# WithLinearInterpolationUpdate
perf probe --exec /workspace/space_veins/src/libspace_veins_dbg.so --add wlip_init_dbg=_ZN11space_veins42SGP4MobilityWithLinearInterpolationUpdates10initializeEi --add wlip_init_dbg=_ZN11space_veins42SGP4MobilityWithLinearInterpolationUpdates10initializeEi%return --no-demangle
perf probe --exec /workspace/space_veins/src/libspace_veins.so --add wlip_init_rls=_ZN11space_veins42SGP4MobilityWithLinearInterpolationUpdates10initializeEi --add wlip_init_rls=_ZN11space_veins42SGP4MobilityWithLinearInterpolationUpdates10initializeEi%return --no-demangle

# WithNearestPeriodicPositionUpdate
perf probe --exec /workspace/space_veins/src/libspace_veins_dbg.so --add wnpp_init_dbg=_ZN11space_veins46SGP4MobilityWithNearestPeriodicPositionUpdates10initializeEi --add wnpp_init_dbg=_ZN11space_veins46SGP4MobilityWithNearestPeriodicPositionUpdates10initializeEi%return --no-demangle
perf probe --exec /workspace/space_veins/src/libspace_veins.so --add wnpp_init_rls=_ZN11space_veins46SGP4MobilityWithNearestPeriodicPositionUpdates10initializeEi --add wnpp_init_rls=_ZN11space_veins46SGP4MobilityWithNearestPeriodicPositionUpdates10initializeEi%return --no-demangle

# SGP4MobilityBase getCurrentPosition() - for random time updates
perf probe --exec /workspace/space_veins/src/libspace_veins_dbg.so --no-inlines --add gcp_dbg=_ZN11space_veins16SGP4MobilityBase18getCurrentPositionEv --add gcp_dbg=_ZN11space_veins16SGP4MobilityBase18getCurrentPositionEv%return --no-demangle
perf probe --exec /workspace/space_veins/src/libspace_veins.so --no-inlines --add gcp_rls=_ZN11space_veins16SGP4MobilityBase18getCurrentPositionEv --add gcp_rls=_ZN11space_veins16SGP4MobilityBase18getCurrentPositionEv%return --no-demangle

# SGP4MobilityBase handleSelfMessage() - for periodic updates
perf probe --exec /workspace/space_veins/src/libspace_veins_dbg.so --add hsm_dbg=_ZN11space_veins16SGP4MobilityBase17handleSelfMessageEPN7omnetpp8cMessageE --add hsm_dbg=_ZN11space_veins16SGP4MobilityBase17handleSelfMessageEPN7omnetpp8cMessageE%return --no-demangle
perf probe --exec /workspace/space_veins/src/libspace_veins.so --add hsm_rls=_ZN11space_veins16SGP4MobilityBase17handleSelfMessageEPN7omnetpp8cMessageE --add hsm_rls=_ZN11space_veins16SGP4MobilityBase17handleSelfMessageEPN7omnetpp8cMessageE%return --no-demangle

echo "List of all perf probes:"
perf probe --list