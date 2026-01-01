# SPDX-FileCopyrightText: 2021 Mario Franke <research@m-franke.net>
#
# SPDX-License-Identifier: GPL-2.0-or-later

# build singularity container
# build space_Veins inside the singularity container
# create runs.txt and runs.log
# run simulations on different machines via ssh inside singularity container
# result collection
# build plots

mkfile_path := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
experimentTime := $(shell date +%Y-%m-%dT%H-%M-%S)
experimentCommit := $(shell git rev-parse --short HEAD)
space := $(subst ,, )
experimentCommitMessage := $(subst /,-,$(subst ',,$(subst :,,$(subst .,,$(subst $(space),_,$(shell git log -1 --pretty=oneline --abbrev-commit))))))
experimentId := $(experimentTime)_$(experimentCommitMessage)
experimentFolderName := verification
# scenarios := Iridium86Active20240304NullIsland
# getCurrentPositionBehavior := NGCP WGCP WLIP WNPP
getCurrentPositionBehavior := NGCP
experiments := EnUpPosOnlyAboveHorizon DisUpPosOnlyAboveHorizon
# # combinational expansion of scenarios and experiments
# validationFiles := $(foreach C, $(validationConfigs), csv/$(C)SatOmnetppPosition.csv)
# validationFilesNoEventNumbers := $(patsubst csv/%SatOmnetppPosition.csv,csv/%SatOmnetppPositionNoEventNumbers.csv, $(validationFiles))
# validationOutputFiles := $(foreach S, $(scenarios), validation/$(S)Validation.txt)
# vectorFiles := $(wildcard results/*.vec)
verificationConfigs = $(foreach E, $(experiments), $(foreach G, $(getCurrentPositionBehavior), Verify$E$G))
# runtimePerformanceConfigs = RtpEnUpPosOnlyAboveHorizonNGCP0360 RtpEnUpPosOnlyAboveHorizonNGCP0720 RtpEnUpPosOnlyAboveHorizonNGCP1080 RtpEnUpPosOnlyAboveHorizonNGCP1440 RtpEnUpPosOnlyAboveHorizonNGCP1800 RtpEnUpPosOnlyAboveHorizonNGCP2160 RtpEnUpPosOnlyAboveHorizonNGCP2520 RtpEnUpPosOnlyAboveHorizonNGCP2880 RtpEnUpPosOnlyAboveHorizonNGCP3240 RtpEnUpPosOnlyAboveHorizonNGCP3600 RtpEnUpPosOnlyAboveHorizonWGCP0360 RtpEnUpPosOnlyAboveHorizonWGCP0720 RtpEnUpPosOnlyAboveHorizonWGCP1080 RtpEnUpPosOnlyAboveHorizonWGCP1440 RtpEnUpPosOnlyAboveHorizonWGCP1800 RtpEnUpPosOnlyAboveHorizonWGCP2160 RtpEnUpPosOnlyAboveHorizonWGCP2520 RtpEnUpPosOnlyAboveHorizonWGCP2880 RtpEnUpPosOnlyAboveHorizonWGCP3240 RtpEnUpPosOnlyAboveHorizonWGCP3600 RtpEnUpPosOnlyAboveHorizonWLIP0360 RtpEnUpPosOnlyAboveHorizonWLIP0720 RtpEnUpPosOnlyAboveHorizonWLIP1080 RtpEnUpPosOnlyAboveHorizonWLIP1440 RtpEnUpPosOnlyAboveHorizonWLIP1800 RtpEnUpPosOnlyAboveHorizonWLIP2160 RtpEnUpPosOnlyAboveHorizonWLIP2520 RtpEnUpPosOnlyAboveHorizonWLIP2880 RtpEnUpPosOnlyAboveHorizonWLIP3240 RtpEnUpPosOnlyAboveHorizonWLIP3600 RtpEnUpPosOnlyAboveHorizonWNPP0360 RtpEnUpPosOnlyAboveHorizonWNPP0720 RtpEnUpPosOnlyAboveHorizonWNPP1080 RtpEnUpPosOnlyAboveHorizonWNPP1440 RtpEnUpPosOnlyAboveHorizonWNPP1800 RtpEnUpPosOnlyAboveHorizonWNPP2160 RtpEnUpPosOnlyAboveHorizonWNPP2520 RtpEnUpPosOnlyAboveHorizonWNPP2880 RtpEnUpPosOnlyAboveHorizonWNPP3240 RtpEnUpPosOnlyAboveHorizonWNPP3600 RtpDisUpPosOnlyAboveHorizonNGCP0360 RtpDisUpPosOnlyAboveHorizonNGCP0720 RtpDisUpPosOnlyAboveHorizonNGCP1080 RtpDisUpPosOnlyAboveHorizonNGCP1440 RtpDisUpPosOnlyAboveHorizonNGCP1800 RtpDisUpPosOnlyAboveHorizonNGCP2160 RtpDisUpPosOnlyAboveHorizonNGCP2520 RtpDisUpPosOnlyAboveHorizonNGCP2880 RtpDisUpPosOnlyAboveHorizonNGCP3240 RtpDisUpPosOnlyAboveHorizonNGCP3600 RtpDisUpPosOnlyAboveHorizonWGCP0360 RtpDisUpPosOnlyAboveHorizonWGCP0720 RtpDisUpPosOnlyAboveHorizonWGCP1080 RtpDisUpPosOnlyAboveHorizonWGCP1440 RtpDisUpPosOnlyAboveHorizonWGCP1800 RtpDisUpPosOnlyAboveHorizonWGCP2160 RtpDisUpPosOnlyAboveHorizonWGCP2520 RtpDisUpPosOnlyAboveHorizonWGCP2880 RtpDisUpPosOnlyAboveHorizonWGCP3240 RtpDisUpPosOnlyAboveHorizonWGCP3600 RtpDisUpPosOnlyAboveHorizonWLIP0360 RtpDisUpPosOnlyAboveHorizonWLIP0720 RtpDisUpPosOnlyAboveHorizonWLIP1080 RtpDisUpPosOnlyAboveHorizonWLIP1440 RtpDisUpPosOnlyAboveHorizonWLIP1800 RtpDisUpPosOnlyAboveHorizonWLIP2160 RtpDisUpPosOnlyAboveHorizonWLIP2520 RtpDisUpPosOnlyAboveHorizonWLIP2880 RtpDisUpPosOnlyAboveHorizonWLIP3240 RtpDisUpPosOnlyAboveHorizonWLIP3600 RtpDisUpPosOnlyAboveHorizonWNPP0360 RtpDisUpPosOnlyAboveHorizonWNPP0720 RtpDisUpPosOnlyAboveHorizonWNPP1080 RtpDisUpPosOnlyAboveHorizonWNPP1440 RtpDisUpPosOnlyAboveHorizonWNPP1800 RtpDisUpPosOnlyAboveHorizonWNPP2160 RtpDisUpPosOnlyAboveHorizonWNPP2520 RtpDisUpPosOnlyAboveHorizonWNPP2880 RtpDisUpPosOnlyAboveHorizonWNPP3240 RtpDisUpPosOnlyAboveHorizonWNPP3600
runtimePerformanceConfigs = RtpEnUpPosOnlyAboveHorizonNGCP0010 RtpEnUpPosOnlyAboveHorizonNGCP0020 RtpEnUpPosOnlyAboveHorizonNGCP0030 RtpEnUpPosOnlyAboveHorizonWGCP0010 RtpEnUpPosOnlyAboveHorizonWGCP0020 RtpEnUpPosOnlyAboveHorizonWGCP0030 RtpEnUpPosOnlyAboveHorizonWLIP0010 RtpEnUpPosOnlyAboveHorizonWLIP0020 RtpEnUpPosOnlyAboveHorizonWLIP0030 RtpEnUpPosOnlyAboveHorizonWNPP0010 RtpEnUpPosOnlyAboveHorizonWNPP0020 RtpEnUpPosOnlyAboveHorizonWNPP0030 RtpDisUpPosOnlyAboveHorizonNGCP0010 RtpDisUpPosOnlyAboveHorizonNGCP0020 RtpDisUpPosOnlyAboveHorizonNGCP0030 RtpDisUpPosOnlyAboveHorizonWGCP0010 RtpDisUpPosOnlyAboveHorizonWGCP0020 RtpDisUpPosOnlyAboveHorizonWGCP0030 RtpDisUpPosOnlyAboveHorizonWLIP0010 RtpDisUpPosOnlyAboveHorizonWLIP0020 RtpDisUpPosOnlyAboveHorizonWLIP0030 RtpDisUpPosOnlyAboveHorizonWNPP0010 RtpDisUpPosOnlyAboveHorizonWNPP0020 RtpDisUpPosOnlyAboveHorizonWNPP0030
allConfigs := $(verificationConfigs) $(runtimePerformanceConfigs)
vectors := lastElevationAngleToSop sgp4Computations linearInterpolationComputations getCurrentPositionUpdateCalls linearInterpolationSpeedMeterPerSecond sgp4SpeedMeterPerSecond
vectorCsvFiles := $(foreach C, $(verificationConfigs), $(foreach V, $(vectors), csv/vectors/$(C)-$(V).csv))
scalars := sgp4Computations linearInterpolationComputations packetSent packetReceived precalculationIntervalsSize relevantPrecalculationIntervalsSize bordersOfRelevantIntervalsSize sgp4CalculationsDuringPrecalculation packetReceptionHash
scalarCsvFiles := $(foreach C, $(verificationConfigs), $(foreach S, $(scalars), csv/scalars/$(C)-$(S).csv))
runmakerJobs := 1
runmakerLogLines := 50
simMachines := nsm-sim05
perfMachines := nsm-lab08
compilationMachine := nsm-lab08
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
					runtimePerformanceConfigs \
					vectors \
					vectorCsvFiles \
					runmakerJobs \
					runmakerLogLines \
					simMachines \
					compilationMachine \
					ccFiles \
					hFiles \
					vectorFiles \

# scenarios \
# experiments \
# validationConfigs \
# allConfigs \
# validationFiles \
# validationFilesNoEventNumbers \
# validationOutputFiles \
# vectorFiles \

.SECONDEXPANSION:
debugVariables:
	$(foreach V, $(debugVariables), $(info $V: $($V)))

cleanAll:
	rm *_runs.log *_runs.txt runAll collectScalars resultCollection plots

cleanResultCollection:
	rm -rf csv collectScalars resultCollection

shell:
	cd ../../singularity && ./singularity-space_veins.sh shell

# TODO: add dependencies: all devcontainer files that are need to build the docker container
../../singularity/singularity-space_veins.sif: ../../singularity/buildSingularityContainer.sh ../../singularity/singularity-space_veins.def
	cd ../../singularity && bash buildSingularityContainer.sh
	touch ../../singularity/singularity-space_veins.sif

#########################################################################
#                                                                       #
#                                                                       #
#                               Compiling                               #
#                                                                       #
#                                                                       #
#########################################################################
# touch ../../src/Makefile to make sure that its timestep is newer than singularity-space_veins.sif after running this target
../../src/Makefile: ../../Makefile ../../singularity/singularity-space_veins.sif
	cd ../../singularity && ./singularity-space_veins.sh /workspace/space_veins -- make makefiles
	touch ../../src/Makefile

space_veins: $(ccFiles) $(hFiles) ../../src/Makefile
	ssh $(compilationMachine) "cd $(mkfile_path) && cd ../../singularity && ./singularity-space_veins.sh /workspace/space_veins -- make -j8 space_veins"

../../src/libspace_veins.so: ../../src/Makefile $(ccFiles) $(hFiles)
	ssh $(compilationMachine) "cd $(mkfile_path) && cd ../../singularity && ./singularity-space_veins.sh /workspace/space_veins -- make -j8 all"

#########################################################################
#                                                                       #
#                                                                       #
#                                runmaker                               #
#                                                                       #
#                                                                       #
#########################################################################
$(experimentId)_VerificationRuns.log:
	@touch $@

$(experimentId)_VerificationRuns.txt: ../../lib/veins_scripts/running/generateRunsFile.pl ../../src/libspace_veins.so
	cd ../../singularity && ./singularity-space_veins.sh /workspace/space_veins/examples/$(experimentFolderName) -- perl $< $(verificationConfigs) > ../examples/$(experimentFolderName)/$@

verificationRunmakerFiles: $(experimentId)_VerificationRuns.log $(experimentId)_VerificationRuns.txt

$(experimentId)_RuntimeRuns.log:
	@touch $@

$(experimentId)_RuntimeRuns.txt: ../../lib/veins_scripts/running/generateRunsFile.pl ../../src/libspace_veins.so
	cd ../../singularity && ./singularity-space_veins.sh /workspace/space_veins/examples/$(experimentFolderName) -- perl $< $(runtimePerformanceConfigs) > ../examples/$(experimentFolderName)/$@

runtimeRunmakerFiles: $(experimentId)_RuntimeRuns.log $(experimentId)_RuntimeRuns.txt

simMachines.txt:
	touch $@
	for m in $(simMachines) ; do \
		echo $$m >> $@ ; \
	done

perfMachines.txt:
	touch $@
	for m in $(perfMachines) ; do \
		echo $$m >> $@ ; \
	done

runtimePerformanceConfigs.txt: eval/writeRuntimePerformanceConfigs.R
	@singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript $^

#########################################################################
#                                                                       #
#                                                                       #
#                         Run simulations                               #
#                                                                       #
#                                                                       #
#########################################################################

# sleep 5 seconds after finishing all runs to wait for files written to disk
# write results of GNU time in a dedicated file for every config to avoid race conditions
# --timeout=0: no timeout
# make results dir on own our because otherwise the radio medium communication log is not written
runVerification: $(experimentId)_VerificationRuns.log $(experimentId)_VerificationRuns.txt perfMachines.txt
	mkdir -p results
	mkdir -p perfResults
	parallel-ssh -i -h perfMachines.txt --timeout=0 "cd $(mkfile_path) && cd ../../singularity/ && ./singularity-space_veins.sh /workspace/space_veins/examples/$(experimentFolderName) -- python3 ../../lib/runmaker/runmaker4.py --jobs $(runmakerJobs) -l $(experimentId)_VerificationRuns.log --loglines $(runmakerLogLines) $(experimentId)_VerificationRuns.txt" || true
	sleep 1
	touch $@

runRuntime: $(experimentId)_RuntimeRuns.log $(experimentId)_RuntimeRuns.txt perfMachines.txt
	mkdir -p results
	mkdir -p perfResults
# 	bash eval/setPerfProbes.sh
	parallel-ssh -i -h perfMachines.txt --timeout=0 "cd $(mkfile_path) && cd ../../singularity/ && ./singularity-space_veins.sh /workspace/space_veins/examples/$(experimentFolderName) -- python3 ../../lib/runmaker/runmaker4.py --jobs 1 -l $(experimentId)_RuntimeRuns.log --loglines $(runmakerLogLines) $(experimentId)_RuntimeRuns.txt" || true
	touch $@

#########################################################################
#                                                                       #
#                                                                       #
#                         Result collection                             #
#                                                                       #
#                                                                       #
#########################################################################


$(vectorCsvFiles): csv/vectors/%.csv : $(vectorFiles)
	@echo "target: $@\nprereq: $^\nstem: $*"
	mkdir -p csv/vectors
	../../lib/veins_scripts/eval/opp_vec2csv.pl -v -M 'ManhattanBuildingShadowing\.leo[a-z,A-Z]*\[(?<module>[0-9]+)\]\.*' -F $(word 2, $(subst -, ,$*)):vector=$(word 2, $(subst -, ,$*)) -A repetition results/$(word 1, $(subst -, ,$*))*.vec > $@

$(scalarCsvFiles): csv/scalars/%.csv : $(scalarFiles)
	@echo "target: $@\nprereq: $^\nstem: $*"
	mkdir -p csv/scalars
	../../lib/veins_scripts/eval/opp_sca2csv.pl -v -M 'ManhattanBuildingShadowing\.*' -F $(word 2, $(subst -, ,$*)):count=$(word 2, $(subst -, ,$*)) -F $(word 2, $(subst -, ,$*)):last=$(word 2, $(subst -, ,$*)) -A repetition results/$(word 1, $(subst -, ,$*))*.sca > $@

PERF_RESULTS := $(shell find perfResults -name "*.perf")
$(PERF_RESULTS:.perf=.perf.counts.csv): %.perf.counts.csv: %.perf
	@echo "target: $@\nprereq: $^\nstem: $*"
	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript eval/checkPerfResults.R $<

checkPerfResults: $(PERF_RESULTS:.perf=.perf.counts.csv)

$(PERF_RESULTS:.perf=.perf.results.csv): %.perf.results.csv: %.perf
	@echo "target: $@\nprereq: $^\nstem: $*"
# 	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript eval/perfResults.R $<
	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- perl eval/perfResults.pl $<

collectPerfResults: $(PERF_RESULTS:.perf=.perf.results.csv)

# gnuTimeResults.csv: eval/gnuTimeResults.R
# 	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript $^ archive/latest/csv/gnuTimeResults/rawData
# 
# speedUp-vs-precalculationInterval.pdf: eval/speedUp-vs-precalculationInterval.R
# 	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript $^ archive/latest/csv/gnuTimeResults/gnuTimeResults.csv
# 
# wallClockTime-vs-gcpb.pdf: eval/wallClockTime-vs-gcpb.R
# 	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript $^ archive/latest/csv/gnuTimeResults/gnuTimeResults.csv
# 
# plotPositionUpdateBehavior:
# 	mkdir -p archive/latest/plots
# 	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript eval/positionUpdateBehavior-elevationAngle-vs-time.R archive/latest/csv/vectors/VerifyDisUpPosOnlyAboveHorizonNGCP-lastElevationAngleToSop.csv \
# 																																	  archive/latest/csv/vectors/VerifyDisUpPosOnlyAboveHorizonNGCP-getCurrentPositionUpdateCalls.csv \
# 																																	  archive/latest/csv/vectors/VerifyDisUpPosOnlyAboveHorizonNGCP-sgp4Computations.csv \
# 																																	  archive/latest/csv/vectors/VerifyDisUpPosOnlyAboveHorizonNGCP-linearInterpolationComputations.csv \
# 																																	  archive/latest/plots/VerifyDisUpPosOnlyAboveHorizonNGCP
# 	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript eval/positionUpdateBehavior-elevationAngle-vs-time.R archive/latest/csv/vectors/VerifyDisUpPosOnlyAboveHorizonWLIP-lastElevationAngleToSop.csv \
# 																																	  archive/latest/csv/vectors/VerifyDisUpPosOnlyAboveHorizonWLIP-getCurrentPositionUpdateCalls.csv \
# 																																	  archive/latest/csv/vectors/VerifyDisUpPosOnlyAboveHorizonWLIP-sgp4Computations.csv \
# 																																	  archive/latest/csv/vectors/VerifyDisUpPosOnlyAboveHorizonWLIP-linearInterpolationComputations.csv \
# 																																	  archive/latest/plots/VerifyDisUpPosOnlyAboveHorizonWLIP
# 	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript eval/positionUpdateBehavior-elevationAngle-vs-time.R archive/latest/csv/vectors/VerifyDisUpPosOnlyAboveHorizonWNPP-lastElevationAngleToSop.csv \
# 																																	  archive/latest/csv/vectors/VerifyDisUpPosOnlyAboveHorizonWNPP-getCurrentPositionUpdateCalls.csv \
# 																																	  archive/latest/csv/vectors/VerifyDisUpPosOnlyAboveHorizonWNPP-sgp4Computations.csv \
# 																																	  archive/latest/csv/vectors/VerifyDisUpPosOnlyAboveHorizonWNPP-linearInterpolationComputations.csv \
# 																																	  archive/latest/plots/VerifyDisUpPosOnlyAboveHorizonWNPP
# 	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript eval/positionUpdateBehavior-elevationAngle-vs-time.R archive/latest/csv/vectors/VerifyDisUpPosOnlyAboveHorizonWGCP-lastElevationAngleToSop.csv \
# 																																	  archive/latest/csv/vectors/VerifyDisUpPosOnlyAboveHorizonWGCP-getCurrentPositionUpdateCalls.csv \
# 																																	  archive/latest/csv/vectors/VerifyDisUpPosOnlyAboveHorizonWGCP-sgp4Computations.csv \
# 																																	  archive/latest/csv/vectors/VerifyDisUpPosOnlyAboveHorizonWGCP-linearInterpolationComputations.csv \
# 																																	  archive/latest/plots/VerifyDisUpPosOnlyAboveHorizonWGCP
# 
# plotUpdatePositionOnlyAboveHorizon:
# 	mkdir -p archive/latest/plots
# 	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript eval/updatePositionOnlyAboveHorizon-elevationAngle-vs-time.R archive/latest/csv/vectors/VerifyDisUpPosOnlyAboveHorizonNGCP-lastElevationAngleToSop.csv \
# 																																			  archive/latest/csv/vectors/VerifyEnUpPosOnlyAboveHorizonNGCP-lastElevationAngleToSop.csv \
# 																																			  archive/latest/plots/updatePositionOnlyAboveHorizon-elevationAngle-vs-time-NGCP
# 	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript eval/updatePositionOnlyAboveHorizon-elevationAngle-vs-time.R archive/latest/csv/vectors/VerifyDisUpPosOnlyAboveHorizonWLIP-lastElevationAngleToSop.csv \
# 																																			  archive/latest/csv/vectors/VerifyEnUpPosOnlyAboveHorizonWLIP-lastElevationAngleToSop.csv \
# 																																			  archive/latest/plots/updatePositionOnlyAboveHorizon-elevationAngle-vs-time-WLIP
# 	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript eval/updatePositionOnlyAboveHorizon-elevationAngle-vs-time.R archive/latest/csv/vectors/VerifyDisUpPosOnlyAboveHorizonWNPP-lastElevationAngleToSop.csv \
# 																																			  archive/latest/csv/vectors/VerifyEnUpPosOnlyAboveHorizonWNPP-lastElevationAngleToSop.csv \
# 																																			  archive/latest/plots/updatePositionOnlyAboveHorizon-elevationAngle-vs-time-WNPP
# 	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript eval/updatePositionOnlyAboveHorizon-elevationAngle-vs-time.R archive/latest/csv/vectors/VerifyDisUpPosOnlyAboveHorizonWGCP-lastElevationAngleToSop.csv \
# 																																			  archive/latest/csv/vectors/VerifyEnUpPosOnlyAboveHorizonWGCP-lastElevationAngleToSop.csv \
# 																																			  archive/latest/plots/updatePositionOnlyAboveHorizon-elevationAngle-vs-time-WGCP
# 
# plotPdrVsConfig:
# 	mkdir -p archive/latest/plots
# 	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript eval/pdr-vs-config.R \
# 		archive/latest/csv/scalars/VerifyDisUpPosOnlyAboveHorizonNGCP-packetReceived.csv \
# 		archive/latest/csv/scalars/VerifyDisUpPosOnlyAboveHorizonWLIP-packetReceived.csv \
# 		archive/latest/csv/scalars/VerifyDisUpPosOnlyAboveHorizonWNPP-packetReceived.csv \
# 		archive/latest/csv/scalars/VerifyDisUpPosOnlyAboveHorizonWGCP-packetReceived.csv \
# 		archive/latest/csv/scalars/VerifyEnUpPosOnlyAboveHorizonNGCP-packetReceived.csv \
# 		archive/latest/csv/scalars/VerifyEnUpPosOnlyAboveHorizonWLIP-packetReceived.csv \
# 		archive/latest/csv/scalars/VerifyEnUpPosOnlyAboveHorizonWNPP-packetReceived.csv \
# 		archive/latest/csv/scalars/VerifyEnUpPosOnlyAboveHorizonWGCP-packetReceived.csv \
# 		archive/latest/plots/pdr-vs-config
# 
# plotNumberOfPositionUpdatesVsConfig:
# 	mkdir -p archive/latest/plots
# 	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript eval/numberOfPositionUpdates-vs-config.R \
# 		archive/latest/csv/scalars/VerifyDisUpPosOnlyAboveHorizonNGCP-sgp4Computations.csv \
# 		archive/latest/csv/scalars/VerifyDisUpPosOnlyAboveHorizonWLIP-sgp4Computations.csv \
# 		archive/latest/csv/scalars/VerifyDisUpPosOnlyAboveHorizonWNPP-sgp4Computations.csv \
# 		archive/latest/csv/scalars/VerifyDisUpPosOnlyAboveHorizonWGCP-sgp4Computations.csv \
# 		archive/latest/csv/scalars/VerifyEnUpPosOnlyAboveHorizonNGCP-sgp4Computations.csv \
# 		archive/latest/csv/scalars/VerifyEnUpPosOnlyAboveHorizonWLIP-sgp4Computations.csv \
# 		archive/latest/csv/scalars/VerifyEnUpPosOnlyAboveHorizonWNPP-sgp4Computations.csv \
# 		archive/latest/csv/scalars/VerifyEnUpPosOnlyAboveHorizonWGCP-sgp4Computations.csv \
# 		archive/latest/plots/numberOfPositionUpdates-vs-config

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

# resultCollection: $(vectorCsvFiles) $(scalarCsvFiles)
resultCollection: $(scalarCsvFiles) $(PERF_RESULTS:.perf=.perf.results.csv)
	touch $@

# ../../lib/singularity-r-env/singularity-R-env.sif: ../../lib/singularity-r-env/singularity-R-env.def
# 	cd ../../lib/singularity-r-env && singularity build --force --fakeroot singularity-R-env.sif singularity-R-env.def

plots: ../../lib/singularity-r-env/singularity-R-env.sif
	$(MAKE) plotPositionUpdateBehavior
	$(MAKE) plotUpdatePositionOnlyAboveHorizon
	$(MAKE) speedUp-vs-precalculationInterval.pdf
	$(MAKE) wallClockTime-vs-gcpb.pdf
	touch plots

verificationExperiment:
	mkdir -p archive/$(experimentId)
	$(MAKE) -j1 experimentId=$(experimentId) runVerification
	$(MAKE) -j8 experimentId=$(experimentId) resultCollection
# 	ToDo: check packetReceptionHashs
# 	$(MAKE) -j8 experimentId=$(experimentId)
	mv $(experimentId)_VerificationRuns.log archive/$(experimentId)
	mv $(experimentId)_VerificationRuns.txt archive/$(experimentId)
	mv results archive/$(experimentId)
	mv perfResults archive/$(experimentId)
	mv csv archive/$(experimentId)
	rm archive/latest
	ln -s $(experimentId) archive/latest
# 	$(MAKE) gnuTimeResults.csv
# 	$(MAKE) plots
	rm runVerification resultCollection
# 	rm runAll resultCollection plots simMachines.txt

runtimeExperiment:
	mkdir -p archive/$(experimentId)
	$(MAKE) -j1 experimentId=$(experimentId) runRuntime

debugInContainer: ../../src/libspace_veins.so
	date +%Y-%m-%dT%H-%M-%S
	/usr/bin/time -o testEn.csv --format "%C,%U,%S,%e" ./run --mode release -- -u Cmdenv -c Debug -r 0
	date +%Y-%m-%dT%H-%M-%S

time = $(shell date +%Y-%m-%dT%H'\':%M'\':%S)
test:
	echo $(time)

parentTarget:
	echo $(time)
	sleep 3
	$(MAKE) time=$(time) test