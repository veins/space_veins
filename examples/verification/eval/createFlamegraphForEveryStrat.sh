#!/bin/bash
set -x

# ./eval/createFlamegraph.sh /ramdisk/VerifyDisUpPosOnlyAboveHorizonWGCP_0_debug.perf.data dwarf "./run --mode debug -- -u Cmdenv -c VerifyDisUpPosOnlyAboveHorizonWGCP -r 0"
./eval/createFlamegraph.sh /ramdisk/VerifyDisUpPosOnlyAboveHorizonNGCP_0_debug.perf.data dwarf "./run --mode debug -- -u Cmdenv -c VerifyDisUpPosOnlyAboveHorizonNGCP -r 0"
# ./eval/createFlamegraph.sh /ramdisk/VerifyDisUpPosOnlyAboveHorizonWLIP_0_debug.perf.data dwarf "./run --mode debug -- -u Cmdenv -c VerifyDisUpPosOnlyAboveHorizonWLIP -r 0"
# ./eval/createFlamegraph.sh /ramdisk/VerifyDisUpPosOnlyAboveHorizonWNPP_0_debug.perf.data dwarf "./run --mode debug -- -u Cmdenv -c VerifyDisUpPosOnlyAboveHorizonWNPP -r 0"
