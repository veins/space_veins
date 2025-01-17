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
scenarios := Iridium86Active20240304NullIsland
experiments := EnableUpdatePositionOnlyAboveHorizon DisableUpdatePositionOnlyAboveHorizon
# combinational expansion of scenarios and experiments
validationConfigs := $(foreach S, $(scenarios), $(foreach E, $(experiments), $S$E))
validationFiles := $(foreach C, $(validationConfigs), csv/$(C)SatOmnetppPosition.csv)
validationFilesNoEventNumbers := $(patsubst csv/%SatOmnetppPosition.csv,csv/%SatOmnetppPositionNoEventNumbers.csv, $(validationFiles))
validationOutputFiles := $(foreach S, $(scenarios), validation/$(S)Validation.txt)
vectorFiles := $(wildcard results/*.vec)

configs := $(validationConfigs)
runmakerJobs := 2
runmakerLogLines := 200
simMachines := nsm-sim03 nsm-sim04

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

simMachines.txt:
	touch $@
	for m in $(simMachines) ; do \
		echo $$m >> $@ ; \
	done

# sleep 5 seconds after finishing all runs to wait for files written to disk
runAll: $(experimentId)_runs.log $(experimentId)_runs.txt simMachines.txt gnuTimeResults.csv
	parallel-ssh -i -h simMachines.txt "cd $(mkfile_path) && cd ../../singularity/ && ./singularity-space_veins.sh /workspaces/space_veins/examples/manhattanGrid --launchd -- python3 ../../lib/runmaker/runmaker4.py --jobs $(runmakerJobs) -l $(experimentId)_runs.log --loglines $(runmakerLogLines) $(experimentId)_runs.txt"
	mkdir -p csv
	mv gnuTimeResults.csv csv/
	sleep 5
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

# used secondary expansion to use the stem $* in a wildcard function
vectorFilesForSecondExpansion := $$(wildcard results/$$**.vec) runAll
.SECONDEXPANSION:
$(validationFiles): csv/%SatOmnetppPosition.csv : $(vectorFilesForSecondExpansion)
	@echo "target: $@\nprereq: $^\nstem: $*"
	mkdir -p csv
	perl ../../lib/veins_scripts/eval/opp_vec2csv.pl -v -M 'ManhattanBuildingShadowing\.leo[a-z,A-Z]*\[(?<module>[0-9]+)\]\.vehicleStatistics' --merge-by mt -F omnetCoordX:vector=omnetCoordX -F omnetCoordY:vector=omnetCoordY -F omnetCoordZ:vector=omnetCoordZ -A repetition $(filter-out runAll,$^) > $@

$(validationFilesNoEventNumbers): csv/%NoEventNumbers.csv: csv/%.csv
	@echo "target: $@\nprereq: $^\nstem: $*"
	@echo "Delete column event as these values will differ since there are less events in EnableUpdatePositionOnlyAboveHorizon"
	sed 's/[^\t]*\t//' $^ > $@
	# @echo "Sort file for correct line by line comparison in diff"
	sort -o $@ $@

# https://stackoverflow.com/q/18204904
# || true ignores return value from diff as it returns 1 if files differ in order to indicate the difference, no error
$(validationOutputFiles): validation/%Validation.txt: $(validationFilesNoEventNumbers)
	@echo "target: $@\nprereq: $^\nstem: $*"
	mkdir -p validation
	diff --unchanged-line-format="" --old-line-format="" $(sort $(filter csv/$*%, $^)) > $@ || true
	echo "diff --unchanged-line-format="" --old-line-format="" $(sort $(filter csv/$*%, $^)) > $@" > $@.cmd
	#diff $(sort $(filter csv/$*%, $^)) > $@

# create a csv file summarizing the validation process. A "1" in the column success means that the validation is correct
validation.csv: $(validationOutputFiles)
	@echo "target: $@\nprereq: $^\nstem: $*"
	touch $@
	echo "command,success" > $@
	for f in $^ ; do \
		cmd=$$(cat $${f}.cmd) ; \
		if [ ! -s $$f ]; then \
			echo "$${cmd},1" >> $@ ; \
		else \
			echo "$${cmd},0" >> $@ ; \
		fi \
	done

debugVariables:
	$(info vectorFiles is $(vectorFiles))

collectVectors: csv/endToEndDelay.csv
	touch $@

resultCollection: collectScalars collectVectors
	touch $@

experiment: validation.csv resultCollection
	mkdir -p archive/$(experimentId)
	mv $(experimentId)_runs.log archive/$(experimentId)
	mv $(experimentId)_runs.txt archive/$(experimentId)
	mv results archive/$(experimentId)
	mv csv archive/$(experimentId)
	mv validation archive/$(experimentId)
	mv validation.csv archive/$(experimentId)/validation
	rm runAll resultCollection collectScalars collectVectors