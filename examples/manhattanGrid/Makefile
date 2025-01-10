# SPDX-FileCopyrightText: 2021 Mario Franke <research@m-franke.net>
#
# SPDX-License-Identifier: GPL-2.0-or-later

# build singularity container
# build space_Veins inside the singularity container
# create runs.txt and runs.log
# run simulations on different machines via ssh inside singularity container
# result collection
# build plots

space_veins_binaries := ../../src/libspace_veins.so
mkfile_path := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
experimentTime := $(shell date +%Y-%m-%dT%H'\':%M'\':%S)
experimentCommit := $(shell git rev-parse --short HEAD)
experimentId := $(experimentTime)_$(experimentCommit)
configs := Iridium86Active20240304NullIslandEnableUpdatePositionOnlyAboveHorizon Iridium86Active20240304NullIslandDisableUpdatePositionOnlyAboveHorizon 
runmakerJobs := 2
runmakerLogLines := 200

all: ../../singularity/singularity-space_veins.sif $(space_veins_binaries)

cleanAll:
	rm *_runs.log *_runs.txt runAll collectScalars resultCollection

cleanResultCollection:
	rm -rf csv collectScalars resultCollection

shell:
	cd ../../singularity && ./singularity-space_veins.sh shell

../../singularity/singularity-space_veins.sif: ../../singularity/buildSingularityContainer.sh ../../singularity/singularity-space_veins.def
	cd ../../singularity && bash buildSingularityContainer.sh

$(space_veins_binaries): ../../Makefile ../../singularity/singularity-space_veins.sif
	cd ../../singularity && ./singularity-space_veins.sh /workspaces/space_veins -- make makefiles
	cd ../../singularity && ./singularity-space_veins.sh /workspaces/space_veins -- make -j8 all

$(experimentId)_runs.log:
	@touch $@

$(experimentId)_runs.txt: ../../lib/veins_scripts/running/generateRunsFile.pl ../../singularity/singularity-space_veins.sif $(space_veins_binaries)
	cd ../../singularity && ./singularity-space_veins.sh /workspaces/space_veins/examples/manhattanGrid -- perl $< $(configs) > ../examples/manhattanGrid/$@

gnuTimeResults.csv:
	touch $@
	echo "command,user,system,wallClock" > $@

runAll: $(experimentId)_runs.log $(experimentId)_runs.txt simMachines.txt gnuTimeResults.csv
	parallel-ssh -i -h simMachines.txt "cd $(mkfile_path) && cd ../../singularity/ && ./singularity-space_veins.sh /workspaces/space_veins/examples/manhattanGrid --launchd -- python3 ../../lib/runmaker/runmaker4.py --jobs $(runmakerJobs) -l $(experimentId)_runs.log --loglines $(runmakerLogLines) $(experimentId)_runs.txt"
	mkdir -p csv
	mv gnuTimeResults.csv csv/
	touch $@

csv/packetSentCount.csv: runAll
	mkdir -p csv
	perl ../../lib/veins_scripts/eval/opp_sca2csv.pl -v -M 'ManhattanBuildingShadowing\.node\[0\]\.app\[(?<module>[0-9]+)\]' -F packetSent:count=packetSentCount -A repetition -A configname -I simTimeLimit -I warmupPeriod results/*.sca > $@

csv/packetReceivedCount.csv: runAll
	mkdir -p csv
	perl ../../lib/veins_scripts/eval/opp_sca2csv.pl -v -M 'ManhattanBuildingShadowing\.leo[a-z,A-Z]*\[(?<module>[0-9]+)\]\.app\[0\]' -F packetReceived:count=packetReceivedCount -A repetition -A configname -I simTimeLimit -I warmupPeriod results/*.sca > $@

csv/processedLeoSatelliteMobilityEventsCount.csv: runAll
	mkdir -p csv
	perl ../../lib/veins_scripts/eval/opp_sca2csv.pl -v -M 'ManhattanBuildingShadowing\.leo[a-z,A-Z]*\[(?<module>[0-9]+)\]\.mobility' -F processedEvents:count=processedEventsCount -A repetition -A configname -I simTimeLimit -I warmupPeriod results/*.sca > $@

csv/calculatedLeoSatellitePositionUpdatesCount.csv: runAll
	mkdir -p csv
	perl ../../lib/veins_scripts/eval/opp_sca2csv.pl -v -M 'ManhattanBuildingShadowing\.leo[a-z,A-Z]*\[(?<module>[0-9]+)\]\.mobility' -F calculatedPositionUpdates:count=calculatedPositionUpdatesCount -A repetition -A configname -I simTimeLimit -I warmupPeriod results/*.sca > $@

collectScalars: csv/packetSentCount.csv csv/packetReceivedCount.csv csv/processedLeoSatelliteMobilityEventsCount.csv csv/calculatedLeoSatellitePositionUpdatesCount.csv
	touch $@

csv/endToEndDelay.csv: runAll
	mkdir -p csv
	perl ../../lib/veins_scripts/eval/opp_vec2csv.pl -v -M 'ManhattanBuildingShadowing\.leo[a-z,A-Z]*\[(?<module>[0-9]+)\]\.app\[0\]' -F endToEndDelay:vector=endToEndDelay -A repetition -A configname -I simTimeLimit -I warmupPeriod results/*.vec > $@

csv/satOmnetppPosition.csv: runAll
	mkdir -p csv
	perl ../../lib/veins_scripts/eval/opp_vec2csv.pl -v -M 'ManhattanBuildingShadowing\.leo[a-z,A-Z]*\[(?<module>[0-9]+)\]\.vehicleStatistics' --merge-by mt -F omnetCoordX:vector=omnetCoordX -F omnetCoordY:vector=omnetCoordY -F omnetCoordZ:vector=omnetCoordZ -A repetition results/*.vec > $@

collectVectors: csv/endToEndDelay.csv csv/satOmnetppPosition.csv
	touch $@

resultCollection: collectScalars collectVectors
	touch $@

archive: resultCollection
	mkdir -p archive/$(experimentId)
	mv $(experimentId)_runs.log archive/$(experimentId)
	mv $(experimentId)_runs.txt archive/$(experimentId)
	mv results archive/$(experimentId)
	mv csv archive/$(experimentId)
	rm runAll resultCollection collectScalars collectVectors