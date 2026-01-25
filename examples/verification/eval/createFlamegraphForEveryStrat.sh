#!/bin/bash
set -x

./eval/createFlamegraph.sh /ramdisk/VerifyDisUpPosOnlyAboveHorizonWGCP_0_release.perf.data fp "./run --mode release -- -u Cmdenv -c VerifyDisUpPosOnlyAboveHorizonWGCP -r 0"
# ./eval/createFlamegraph.sh /ramdisk/VerifyDisUpPosOnlyAboveHorizonNGCP_0_release.perf.data fp "./run --mode release -- -u Cmdenv -c VerifyDisUpPosOnlyAboveHorizonNGCP -r 0"
# ./eval/createFlamegraph.sh /ramdisk/VerifyDisUpPosOnlyAboveHorizonWLIP_0_release.perf.data fp "./run --mode release -- -u Cmdenv -c VerifyDisUpPosOnlyAboveHorizonWLIP -r 0"
# ./eval/createFlamegraph.sh /ramdisk/VerifyDisUpPosOnlyAboveHorizonWNPP_0_release.perf.data fp "./run --mode release -- -u Cmdenv -c VerifyDisUpPosOnlyAboveHorizonWNPP -r 0"
