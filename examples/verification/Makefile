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
space := $(subst ,, )
experimentCommitMessage := $(subst /,-,$(subst ',,$(subst :,,$(subst .,,$(subst $(space),_,$(shell git log -1 --pretty=oneline --abbrev-commit))))))
experimentId := $(experimentTime)_$(experimentCommitMessage)
experimentFolderName := verification
# scenarios := Iridium86Active20240304NullIsland
getCurrentPositionBehavior := LastPosition Sgp4Position LinearInterpolationPosition NearestPeriodicPosition
experiments := EnableUpdatePositionOnlyAboveHorizon DisableUpdatePositionOnlyAboveHorizon
# runTimerPerformanceConfigs := $(foreach S, $(scenarios), $(foreach E, $(experiments), $S$ERunTimePerformance))
# validationConfigs := $(foreach S, $(scenarios), $(foreach E, $(experiments), $S$EValidation))
# allConfigs := $(validationConfigs) $(runTimerPerformanceConfigs)
# # combinational expansion of scenarios and experiments
# validationFiles := $(foreach C, $(validationConfigs), csv/$(C)SatOmnetppPosition.csv)
# validationFilesNoEventNumbers := $(patsubst csv/%SatOmnetppPosition.csv,csv/%SatOmnetppPositionNoEventNumbers.csv, $(validationFiles))
# validationOutputFiles := $(foreach S, $(scenarios), validation/$(S)Validation.txt)
# vectorFiles := $(wildcard results/*.vec)
verificationConfigs = $(foreach E, $(experiments), $(foreach G, $(getCurrentPositionBehavior), Verify$E$G))
allConfigs := $(verificationConfigs)
vectors := lastElevationAngleToSop sgp4Computations linearInterpolationComputations getCurrentPositionUpdateCalls linearInterpolationSpeedMeterPerSecond sgp4SpeedMeterPerSecond
vectorCsvFiles := $(foreach C, $(verificationConfigs), $(foreach V, $(vectors), csv/vectors/$(C)-$(V).csv))
scalars := sgp4Computations linearInterpolationComputations packetSent packetReceived
scalarCsvFiles := $(foreach C, $(verificationConfigs), $(foreach S, $(scalars), csv/scalars/$(C)-$(S).csv))
runmakerJobs := 3
runmakerLogLines := 200
simMachines := nsm-sim03 nsm-sim04
ccFiles := $(shell find ../../src -name "*.cc")
hFiles := $(shell find ../../src -name "*.h")
vectorFiles := $(shell find results -name "*.vec")

debugVariables := space_veins_binaries \
					mkfile_path \
					experimentTime \
					experimentCommit \
					experimentCommitMessage \
					experimentId \
					experimentFolderName \
					verificationConfigs \
					vectors \
					vectorCsvFiles \
					runmakerJobs \
					runmakerLogLines \
					simMachines \
					ccFiles \
					hFiles \
					vectorFiles \

# scenarios \
# experiments \
# runTimerPerformanceConfigs \
# validationConfigs \
# allConfigs \
# validationFiles \
# validationFilesNoEventNumbers \
# validationOutputFiles \
# vectorFiles \

.SECONDEXPANSION:
debugVariables:
	$(foreach V, $(debugVariables), $(info $V: $($V)))

all: ../../singularity/singularity-space_veins.sif $(space_veins_binaries)

cleanAll:
	rm *_runs.log *_runs.txt runAll collectScalars resultCollection plots

cleanResultCollection:
	rm -rf csv collectScalars resultCollection

shell:
	cd ../../singularity && ./singularity-space_veins.sh shell

../../singularity/singularity-space_veins.sif: ../../singularity/buildSingularityContainer.sh ../../singularity/singularity-space_veins.def
	cd ../../singularity && bash buildSingularityContainer.sh

../../src/Makefile: ../../Makefile ../../singularity/singularity-space_veins.sif
	cd ../../singularity && ./singularity-space_veins.sh /workspaces/space_veins -- make makefiles

# !!! fix ../../src/Makefile to not be build over and over again although nothing changed
space_veins: $(ccFiles) $(hFiles)
	cd ../../singularity && ./singularity-space_veins.sh /workspaces/space_veins -- make -j8 space_veins

$(space_veins_binaries): ../../src/Makefile $(ccFiles) $(hFiles)
	cd ../../singularity && ./singularity-space_veins.sh /workspaces/space_veins -- make -j8 all

$(experimentId)_runs.log:
	@touch $@

$(experimentId)_runs.txt: ../../lib/veins_scripts/running/generateRunsFile.pl ../../singularity/singularity-space_veins.sif $(space_veins_binaries)
	cd ../../singularity && ./singularity-space_veins.sh /workspaces/space_veins/examples/$(experimentFolderName) -- perl $< $(allConfigs) > ../examples/$(experimentFolderName)/$@

runmakerFiles: $(experimentId)_runs.log $(experimentId)_runs.txt

simMachines.txt:
	touch $@
	for m in $(simMachines) ; do \
		echo $$m >> $@ ; \
	done

# sleep 5 seconds after finishing all runs to wait for files written to disk
# write results of GNU time in a dedicated file for every config to avoid race conditions
# --timeout=0: no timeout
# make results dir on own our because otherwise the radio medium communication log is not written
runAll: $(experimentId)_runs.log $(experimentId)_runs.txt simMachines.txt
	mkdir -p results
	parallel-ssh -i -h simMachines.txt --timeout=0 "cd $(mkfile_path) && cd ../../singularity/ && ./singularity-space_veins.sh /workspaces/space_veins/examples/$(experimentFolderName) --launchd -- python3 ../../lib/runmaker/runmaker4.py --jobs $(runmakerJobs) -l $(experimentId)_runs.log --loglines $(runmakerLogLines) $(experimentId)_runs.txt"
	sleep 5
	mkdir -p csv/gnuTimeResults
	mv GnuTimeResults* csv/gnuTimeResults
	sleep 5
	touch $@

$(vectorCsvFiles): csv/vectors/%.csv : $(vectorFiles)
	@echo "target: $@\nprereq: $^\nstem: $*"
	mkdir -p csv/vectors
	../../lib/veins_scripts/eval/opp_vec2csv.pl -v -M 'ManhattanBuildingShadowing\.leo[a-z,A-Z]*\[(?<module>[0-9]+)\]\.*' -F $(word 2, $(subst -, ,$*)):vector=$(word 2, $(subst -, ,$*)) -A repetition results/$(word 1, $(subst -, ,$*))*.vec > $@

$(scalarCsvFiles): csv/scalars/%.csv : $(scalarFiles)
	@echo "target: $@\nprereq: $^\nstem: $*"
	mkdir -p csv/scalars
	../../lib/veins_scripts/eval/opp_sca2csv.pl -v -M 'ManhattanBuildingShadowing\.*' -F $(word 2, $(subst -, ,$*)):count=$(word 2, $(subst -, ,$*)) -A repetition results/$(word 1, $(subst -, ,$*))*.sca > $@

plotPositionUpdateBehavior:
	mkdir -p archive/latest/plots
	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript eval/positionUpdateBehavior-elevationAngle-vs-time.R archive/latest/csv/vectors/VerifyDisableUpdatePositionOnlyAboveHorizonLastPosition-lastElevationAngleToSop.csv archive/latest/csv/vectors/VerifyDisableUpdatePositionOnlyAboveHorizonLastPosition-getCurrentPositionUpdateCalls.csv archive/latest/csv/vectors/VerifyDisableUpdatePositionOnlyAboveHorizonLastPosition-sgp4Computations.csv archive/latest/csv/vectors/VerifyDisableUpdatePositionOnlyAboveHorizonLastPosition-linearInterpolationComputations.csv archive/latest/plots/VerifyDisableUpdatePositionOnlyAboveHorizonLastPosition
	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript eval/positionUpdateBehavior-elevationAngle-vs-time.R archive/latest/csv/vectors/VerifyDisableUpdatePositionOnlyAboveHorizonLinearInterpolationPosition-lastElevationAngleToSop.csv archive/latest/csv/vectors/VerifyDisableUpdatePositionOnlyAboveHorizonLinearInterpolationPosition-getCurrentPositionUpdateCalls.csv archive/latest/csv/vectors/VerifyDisableUpdatePositionOnlyAboveHorizonLinearInterpolationPosition-sgp4Computations.csv archive/latest/csv/vectors/VerifyDisableUpdatePositionOnlyAboveHorizonLinearInterpolationPosition-linearInterpolationComputations.csv archive/latest/plots/VerifyDisableUpdatePositionOnlyAboveHorizonLinearInterpolationPosition
	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript eval/positionUpdateBehavior-elevationAngle-vs-time.R archive/latest/csv/vectors/VerifyDisableUpdatePositionOnlyAboveHorizonNearestPeriodicPosition-lastElevationAngleToSop.csv archive/latest/csv/vectors/VerifyDisableUpdatePositionOnlyAboveHorizonNearestPeriodicPosition-getCurrentPositionUpdateCalls.csv archive/latest/csv/vectors/VerifyDisableUpdatePositionOnlyAboveHorizonNearestPeriodicPosition-sgp4Computations.csv archive/latest/csv/vectors/VerifyDisableUpdatePositionOnlyAboveHorizonNearestPeriodicPosition-linearInterpolationComputations.csv archive/latest/plots/VerifyDisableUpdatePositionOnlyAboveHorizonNearestPeriodicPosition
	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript eval/positionUpdateBehavior-elevationAngle-vs-time.R archive/latest/csv/vectors/VerifyDisableUpdatePositionOnlyAboveHorizonSgp4Position-lastElevationAngleToSop.csv archive/latest/csv/vectors/VerifyDisableUpdatePositionOnlyAboveHorizonSgp4Position-getCurrentPositionUpdateCalls.csv archive/latest/csv/vectors/VerifyDisableUpdatePositionOnlyAboveHorizonSgp4Position-sgp4Computations.csv archive/latest/csv/vectors/VerifyDisableUpdatePositionOnlyAboveHorizonSgp4Position-linearInterpolationComputations.csv archive/latest/plots/VerifyDisableUpdatePositionOnlyAboveHorizonSgp4Position

plotUpdatePositionOnlyAboveHorizon:
	mkdir -p archive/latest/plots
	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript eval/updatePositionOnlyAboveHorizon-elevationAngle-vs-time.R archive/latest/csv/vectors/VerifyDisableUpdatePositionOnlyAboveHorizonLastPosition-lastElevationAngleToSop.csv archive/latest/csv/vectors/VerifyEnableUpdatePositionOnlyAboveHorizonLastPosition-lastElevationAngleToSop.csv archive/latest/plots/updatePositionOnlyAboveHorizon-elevationAngle-vs-time-lastPosition
	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript eval/updatePositionOnlyAboveHorizon-elevationAngle-vs-time.R archive/latest/csv/vectors/VerifyDisableUpdatePositionOnlyAboveHorizonLinearInterpolationPosition-lastElevationAngleToSop.csv archive/latest/csv/vectors/VerifyEnableUpdatePositionOnlyAboveHorizonLinearInterpolationPosition-lastElevationAngleToSop.csv archive/latest/plots/updatePositionOnlyAboveHorizon-elevationAngle-vs-time-linearInterpolationPosition
	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript eval/updatePositionOnlyAboveHorizon-elevationAngle-vs-time.R archive/latest/csv/vectors/VerifyDisableUpdatePositionOnlyAboveHorizonNearestPeriodicPosition-lastElevationAngleToSop.csv archive/latest/csv/vectors/VerifyEnableUpdatePositionOnlyAboveHorizonNearestPeriodicPosition-lastElevationAngleToSop.csv archive/latest/plots/updatePositionOnlyAboveHorizon-elevationAngle-vs-time-NearestPeriodicPosition
	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript eval/updatePositionOnlyAboveHorizon-elevationAngle-vs-time.R archive/latest/csv/vectors/VerifyDisableUpdatePositionOnlyAboveHorizonSgp4Position-lastElevationAngleToSop.csv archive/latest/csv/vectors/VerifyEnableUpdatePositionOnlyAboveHorizonSgp4Position-lastElevationAngleToSop.csv archive/latest/plots/updatePositionOnlyAboveHorizon-elevationAngle-vs-time-Sgp4Position

plotPdrVsConfig:
	mkdir -p archive/latest/plots
	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript eval/pdr-vs-config.R \
		archive/latest/csv/scalars/VerifyDisableUpdatePositionOnlyAboveHorizonLastPosition-packetReceived.csv \
		archive/latest/csv/scalars/VerifyDisableUpdatePositionOnlyAboveHorizonLinearInterpolationPosition-packetReceived.csv \
		archive/latest/csv/scalars/VerifyDisableUpdatePositionOnlyAboveHorizonNearestPeriodicPosition-packetReceived.csv \
		archive/latest/csv/scalars/VerifyDisableUpdatePositionOnlyAboveHorizonSgp4Position-packetReceived.csv \
		archive/latest/csv/scalars/VerifyEnableUpdatePositionOnlyAboveHorizonLastPosition-packetReceived.csv \
		archive/latest/csv/scalars/VerifyEnableUpdatePositionOnlyAboveHorizonLinearInterpolationPosition-packetReceived.csv \
		archive/latest/csv/scalars/VerifyEnableUpdatePositionOnlyAboveHorizonNearestPeriodicPosition-packetReceived.csv \
		archive/latest/csv/scalars/VerifyEnableUpdatePositionOnlyAboveHorizonSgp4Position-packetReceived.csv \
		archive/latest/plots/pdr-vs-config

plotNumberOfPositionUpdatesVsConfig:
	mkdir -p archive/latest/plots
	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript eval/numberOfPositionUpdates-vs-config.R \
		archive/latest/csv/scalars/VerifyDisableUpdatePositionOnlyAboveHorizonLastPosition-sgp4Computations.csv \
		archive/latest/csv/scalars/VerifyDisableUpdatePositionOnlyAboveHorizonLinearInterpolationPosition-sgp4Computations.csv \
		archive/latest/csv/scalars/VerifyDisableUpdatePositionOnlyAboveHorizonNearestPeriodicPosition-sgp4Computations.csv \
		archive/latest/csv/scalars/VerifyDisableUpdatePositionOnlyAboveHorizonSgp4Position-sgp4Computations.csv \
		archive/latest/csv/scalars/VerifyEnableUpdatePositionOnlyAboveHorizonLastPosition-sgp4Computations.csv \
		archive/latest/csv/scalars/VerifyEnableUpdatePositionOnlyAboveHorizonLinearInterpolationPosition-sgp4Computations.csv \
		archive/latest/csv/scalars/VerifyEnableUpdatePositionOnlyAboveHorizonNearestPeriodicPosition-sgp4Computations.csv \
		archive/latest/csv/scalars/VerifyEnableUpdatePositionOnlyAboveHorizonSgp4Position-sgp4Computations.csv \
		archive/latest/plots/numberOfPositionUpdates-vs-config

# csv/packetSentCount.csv: runAll
# 	mkdir -p csv
# 	perl ../../lib/veins_scripts/eval/opp_sca2csv.pl -v -M 'ManhattanBuildingShadowing\.node\[0\]\.app\[(?<module>[0-9]+)\]' -F packetSent:count=packetSentCount -A repetition -A configname -I simTimeLimit -I warmupPeriod results/*.sca > $@

# csv/packetReceivedCount.csv: runAll
# 	mkdir -p csv
# 	perl ../../lib/veins_scripts/eval/opp_sca2csv.pl -v -M 'ManhattanBuildingShadowing\.leo[a-z,A-Z]*\[(?<module>[0-9]+)\]\.app\[0\]' -F packetReceived:count=packetReceivedCount -A repetition -A configname -I simTimeLimit -I warmupPeriod results/*.sca > $@

# csv/processedLeoSatelliteMobilityEventsCount.csv: runAll
# 	mkdir -p csv
# 	perl ../../lib/veins_scripts/eval/opp_sca2csv.pl -v -M 'ManhattanBuildingShadowing\.leo[a-z,A-Z]*\[(?<module>[0-9]+)\]\.mobility' -F processedEvents:count=processedEventsCount -A repetition -A configname -I simTimeLimit -I warmupPeriod results/*.sca > $@

# csv/calculatedLeoSatellitePositionUpdatesCount.csv: runAll
# 	mkdir -p csv
# 	perl ../../lib/veins_scripts/eval/opp_sca2csv.pl -v -M 'ManhattanBuildingShadowing\.leo[a-z,A-Z]*\[(?<module>[0-9]+)\]\.mobility' -F calculatedPositionUpdates:count=calculatedPositionUpdatesCount -A repetition -A configname -I simTimeLimit -I warmupPeriod results/*.sca > $@

# collectScalars: csv/packetSentCount.csv csv/packetReceivedCount.csv csv/processedLeoSatelliteMobilityEventsCount.csv csv/calculatedLeoSatellitePositionUpdatesCount.csv
# 	touch $@

# csv/endToEndDelay.csv: runAll
# 	mkdir -p csv
# 	perl ../../lib/veins_scripts/eval/opp_vec2csv.pl -v -M 'ManhattanBuildingShadowing\.leo[a-z,A-Z]*\[(?<module>[0-9]+)\]\.app\[0\]' -F endToEndDelay:vector=endToEndDelay -A repetition -A configname -I simTimeLimit -I warmupPeriod results/*.vec > $@

# $(validationFiles): csv/%SatOmnetppPosition.csv : runAll
# 	@echo "target: $@\nprereq: $^\nstem: $*"
# 	mkdir -p csv
# 	perl ../../lib/veins_scripts/eval/opp_vec2csv.pl -v -M 'ManhattanBuildingShadowing\.leo[a-z,A-Z]*\[(?<module>[0-9]+)\]\.vehicleStatistics' --merge-by mt -F omnetCoordX:vector=omnetCoordX -F omnetCoordY:vector=omnetCoordY -F omnetCoordZ:vector=omnetCoordZ -A repetition results/*.vec > $@

# $(validationFilesNoEventNumbers): csv/%NoEventNumbers.csv: csv/%.csv
# 	@echo "target: $@\nprereq: $^\nstem: $*"
# 	@echo "Delete column event as these values will differ since there are less events in EnableUpdatePositionOnlyAboveHorizon"
# 	sed 's/[^\t]*\t//' $^ > $@
# 	# @echo "Sort file for correct line by line comparison in diff"
# 	sort -o $@ $@

# # https://stackoverflow.com/q/18204904
# # || true ignores return value from diff as it returns 1 if files differ in order to indicate the difference, no error
# $(validationOutputFiles): validation/%Validation.txt: $(validationFilesNoEventNumbers)
# 	@echo "target: $@\nprereq: $^\nstem: $*"
# 	mkdir -p validation
# 	diff --unchanged-line-format="" --old-line-format="" $(sort $(filter csv/$*%, $^)) > $@ || true
# 	echo "diff --unchanged-line-format="" --old-line-format="" $(sort $(filter csv/$*%, $^)) > $@" > $@.cmd
# 	#diff $(sort $(filter csv/$*%, $^)) > $@

# # create a csv file summarizing the validation process. A "1" in the column success means that the validation is correct
# validation.csv: $(validationOutputFiles)
# 	@echo "target: $@\nprereq: $^\nstem: $*"
# 	touch $@
# 	echo "command,success" > $@
# 	for f in $^ ; do \
# 		cmd=$$(cat $${f}.cmd) ; \
# 		if [ ! -s $$f ]; then \
# 			echo "$${cmd},1" >> $@ ; \
# 		else \
# 			echo "$${cmd},0" >> $@ ; \
# 		fi \
# 	done

# collectVectors: csv/endToEndDelay.csv
# 	touch $@

resultCollection: $(vectorCsvFiles) $(scalarCsvFiles)
# resultCollection: $(scalarCsvFiles)
	touch $@

# ../../lib/singularity-r-env/singularity-R-env.sif: ../../lib/singularity-r-env/singularity-R-env.def
# 	cd ../../lib/singularity-r-env && singularity build --force --fakeroot singularity-R-env.sif singularity-R-env.def

plots: ../../lib/singularity-r-env/singularity-R-env.sif
	$(MAKE) plotPositionUpdateBehavior
	$(MAKE) plotUpdatePositionOnlyAboveHorizon
	touch plots

experiment:
	mkdir -p archive/$(experimentId)
	$(MAKE) -e runAll
	$(MAKE) resultCollection
	mv $(experimentId)_runs.log archive/$(experimentId)
	mv $(experimentId)_runs.txt archive/$(experimentId)
	mv results archive/$(experimentId)
	mv csv archive/$(experimentId)
	rm archive/latest
	ln -s $(experimentId) archive/latest
	$(MAKE) plots
	rm runAll resultCollection plots