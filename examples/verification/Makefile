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
getCurrentPositionBehavior := NGCP WGCP WLIP WNPP
# getCurrentPositionBehavior := NGCP WGCP
# getCurrentPositionBehavior := NGCP
experiments := EnUpPosOnlyAboveHorizon DisUpPosOnlyAboveHorizon
demoExperiments := EnUpPosOnlyAboveHorizon DisUpPosOnlyAboveHorizon
runtimeExperiments := EnUpPosOnlyAboveHorizon DisUpPosOnlyAboveHorizon
# experiments := DisUpPosOnlyAboveHorizon
# # combinational expansion of scenarios and experiments
# validationFiles := $(foreach C, $(validationConfigs), csv/$(C)SatOmnetppPosition.csv)
# validationFilesNoEventNumbers := $(patsubst csv/%SatOmnetppPosition.csv,csv/%SatOmnetppPositionNoEventNumbers.csv, $(validationFiles))
# validationOutputFiles := $(foreach S, $(scenarios), validation/$(S)Validation.txt)
# vectorFiles := $(wildcard results/*.vec)
verificationConfigs := $(foreach E, $(experiments), $(foreach G, $(getCurrentPositionBehavior), Verify$E$G))
demoConfigs := DemoEnUpPosOnlyAboveHorizonNGCP_1c0 DemoEnUpPosOnlyAboveHorizonWGCP_1c0 DemoEnUpPosOnlyAboveHorizonWLIP_1c0 DemoEnUpPosOnlyAboveHorizonWNPP_1c0 DemoDisUpPosOnlyAboveHorizonNGCP_1c0 DemoDisUpPosOnlyAboveHorizonWGCP_1c0 DemoDisUpPosOnlyAboveHorizonWLIP_1c0 DemoDisUpPosOnlyAboveHorizonWNPP_1c0
runtimeConfigs := RuntimeDisUpPosOnlyAboveHorizonNGCP_1c0 RuntimeDisUpPosOnlyAboveHorizonNGCP_10c0 RuntimeDisUpPosOnlyAboveHorizonNGCP_30c0 RuntimeDisUpPosOnlyAboveHorizonNGCP_60c0 RuntimeDisUpPosOnlyAboveHorizonWGCP_1c0 RuntimeDisUpPosOnlyAboveHorizonWGCP_10c0 RuntimeDisUpPosOnlyAboveHorizonWGCP_30c0 RuntimeDisUpPosOnlyAboveHorizonWGCP_60c0 RuntimeDisUpPosOnlyAboveHorizonWLIP_1c0 RuntimeDisUpPosOnlyAboveHorizonWLIP_10c0 RuntimeDisUpPosOnlyAboveHorizonWLIP_30c0 RuntimeDisUpPosOnlyAboveHorizonWLIP_60c0 RuntimeDisUpPosOnlyAboveHorizonWNPP_1c0 RuntimeDisUpPosOnlyAboveHorizonWNPP_10c0 RuntimeDisUpPosOnlyAboveHorizonWNPP_30c0 RuntimeDisUpPosOnlyAboveHorizonWNPP_60c0

vectors := lastElevationAngleToSop
vectorCsvFiles := $(foreach C, $(verificationConfigs), $(foreach V, $(vectors), csv/vectors/$(C)-$(V).csv))
scalars := totalAddedTraciModules totalRemovedTraciModules getCurrentPositionUpdateCallsNoWarmupPeriod periodicUpdateCallsNoWarmupPeriod ageOfInformation sendSatellitePacketsOverMinElevationAngle periodicUpdateCalls getCurrentPositionUpdateCalls sgp4Computations linearInterpolationComputations packetSent packetReceived precalculationIntervalsSize relevantPrecalculationIntervalsSize bordersOfRelevantIntervalsSize sgp4ComputationsDuringPrecalculation packetReceptionHash
scalarCsvFiles := $(foreach C, $(verificationConfigs), $(foreach S, $(scalars), csv/scalars/$(C)-$(S).csv))

demoVectors := lastElevationAngleToSop sgp4Computations getCurrentPositionUpdateCalls getCurrentPositionUpdates ageOfInformation
demoVectorCsvFiles := $(foreach C, $(demoConfigs), $(foreach V, $(demoVectors), csv/vectors/$(C)-$(V).csv))
demoScalars := totalAddedTraciModules totalRemovedTraciModules getCurrentPositionUpdateCallsNoWarmupPeriod periodicUpdateCallsNoWarmupPeriod ageOfInformation sendSatellitePacketsOverMinElevationAngle periodicUpdateCalls getCurrentPositionUpdateCalls sgp4Computations linearInterpolationComputations packetSent packetReceived precalculationIntervalsSize relevantPrecalculationIntervalsSize bordersOfRelevantIntervalsSize packetReceptionHash
demoScalarCsvFiles := $(foreach C, $(demoConfigs), $(foreach S, $(demoScalars), csv/scalars/$(C)-$(S).csv))

runtimeVectors := ageOfInformation
runtimeVectorCsvFiles := $(foreach C, $(runtimeConfigs), $(foreach V, $(runtimeVectors), csv/vectors/$(C)-$(V).csv))
runtimeScalars := totalAddedTraciModules totalRemovedTraciModules getCurrentPositionUpdateCallsNoWarmupPeriod periodicUpdateCallsNoWarmupPeriod ageOfInformation sendSatellitePacketsOverMinElevationAngle periodicUpdateCalls getCurrentPositionUpdateCalls sgp4Computations linearInterpolationComputations packetSent packetReceived precalculationIntervalsSize relevantPrecalculationIntervalsSize bordersOfRelevantIntervalsSize sgp4ComputationsDuringPrecalculation packetReceptionHash
runtimeScalarCsvFiles := $(foreach C, $(runtimeConfigs), $(foreach S, $(runtimeScalars), csv/scalars/$(C)-$(S).csv))

runmakerJobs := 1
runmakerLogLines := 50

demoRunmakerJobs := 1
demoRunmakerLogLines := 50

runtimeRunmakerJobs := 1
runtimeRunmakerLogLines := 50

perfMachines := nsm-lab08
demoMachines := nsm-lab08
runtimeMachines := nsm-lab08

compilationMachine := nsm-lab08
demoCompilationMachine := nsm-lab08
runtimeCompilationMachine := nsm-lab08

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
					demoConfigs \
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

demoDebugVariables := demoVectors \
						demoVectorCsvFiles

runtimeDebugVariables := runtimeVectors \
						runtimeVectorCsvFiles \
						runtimeScalars \
						runtimeScalarCsvFiles

.SECONDEXPANSION:
debugVariables:
	$(foreach V, $(debugVariables), $(info $V: $($V)))

.SECONDEXPANSION:
demoDebugVariables:
	$(foreach V, $(demoDebugVariables), $(info $V: $($V)))

.SECONDEXPANSION:
runtimeDebugVariables:
	$(foreach V, $(runtimeDebugVariables), $(info $V: $($V)))

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
# ../../src/Makefile: ../../Makefile ../../singularity/singularity-space_veins.sif
../../src/Makefile: ../../Makefile
	cd ../../singularity && ./singularity-space_veins.sh /workspace/space_veins -- make makefiles
	touch ../../src/Makefile

space_veins: $(ccFiles) $(hFiles) ../../src/Makefile
	ssh $(compilationMachine) "cd $(mkfile_path) && cd ../../singularity && ./singularity-space_veins.sh /workspace/space_veins -- make -j8 space_veins"

../../src/libspace_veins.so: ../../src/Makefile $(ccFiles) $(hFiles)
	ssh $(compilationMachine) "cd $(mkfile_path) && cd ../../singularity && ./singularity-space_veins.sh /workspace/space_veins -- make -j8 all"

demo_space_veins: $(ccFiles) $(hFiles) ../../src/Makefile
	ssh $(demoCompilationMachine) "cd $(mkfile_path) && cd ../../singularity && ./singularity-space_veins.sh /workspace/space_veins -- make -j8 space_veins"

runtime_space_veins: $(ccFiles) $(hFiles) ../../src/Makefile
	ssh $(runtimeCompilationMachine) "cd $(mkfile_path) && cd ../../singularity && ./singularity-space_veins.sh /workspace/space_veins -- make -j48 space_veins"

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

perfMachines.txt:
	touch $@
	for m in $(perfMachines) ; do \
		echo $$m >> $@ ; \
	done

# ------------------------ demoExperiment -------------------------------------------------------------------------------------------------
$(experimentId)_DemoRuns.log:
	@touch $@

$(experimentId)_DemoRuns.txt: ../../lib/veins_scripts/running/generateDemoRunsFile.pl ../../src/libspace_veins.so
	cd ../../singularity && ./singularity-space_veins.sh /workspace/space_veins/examples/$(experimentFolderName) -- perl $< $(demoConfigs) > ../examples/$(experimentFolderName)/$@

demoRunmakerFiles: $(experimentId)_DemoRuns.log $(experimentId)_DemoRuns.txt

demoConfigs.txt: eval/writeDemoConfigs.R
	@singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript $^ > $@

demoMachines.txt:
	touch $@
	for m in $(demoMachines) ; do \
		echo $$m >> $@ ; \
	done

# ------------------------ runtimeExperiment -------------------------------------------------------------------------------------------------
$(experimentId)_RuntimeRuns.log:
	@touch $@

$(experimentId)_RuntimeRuns.txt: ../../lib/veins_scripts/running/generateRuntimeRunsFile.pl ../../src/libspace_veins.so
	cd ../../singularity && ./singularity-space_veins.sh /workspace/space_veins/examples/$(experimentFolderName) -- perl $< $(runtimeConfigs) > ../examples/$(experimentFolderName)/$@

runtimeRunmakerFiles: $(experimentId)_RuntimeRuns.log $(experimentId)_RuntimeRuns.txt

runtimeConfigs.txt: eval/writeRuntimeConfigs.R
	@singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript $^ > $@

runtimeMachines.txt:
	touch $@
	for m in $(runtimeMachines) ; do \
		echo $$m >> $@ ; \
	done

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
	parallel-ssh -i -h perfMachines.txt --timeout=0 "cd $(mkfile_path) && cd ../../singularity/ && ./singularity-space_veins.sh /workspace/space_veins/examples/$(experimentFolderName) --launchd -- python3 ../../lib/runmaker/runmaker4.py --jobs $(runmakerJobs) -l $(experimentId)_VerificationRuns.log --loglines $(runmakerLogLines) $(experimentId)_VerificationRuns.txt" || true
	sleep 1
	touch $@

runDemo: $(experimentId)_DemoRuns.log $(experimentId)_DemoRuns.txt demoMachines.txt
	mkdir -p /ramdisk/results
	ln -s /ramdisk/results results
	parallel-ssh -i -h demoMachines.txt --timeout=0 "cd $(mkfile_path) && cd ../../singularity/ && ./singularity-space_veins.sh /workspace/space_veins/examples/$(experimentFolderName) --launchd -- python3 ../../lib/runmaker/runmaker4.py --jobs $(demoRunmakerJobs) -l $(experimentId)_DemoRuns.log --loglines $(runmakerLogLines) $(experimentId)_DemoRuns.txt" || true
	sleep 1
	touch $@

runRuntime: $(experimentId)_RuntimeRuns.log $(experimentId)_RuntimeRuns.txt runtimeMachines.txt
	mkdir -p /ramdisk/results
	ln -s /ramdisk/results results
	parallel-ssh -i -h runtimeMachines.txt --timeout=0 "cd $(mkfile_path) && cd ../../singularity/ && ./singularity-space_veins.sh /workspace/space_veins/examples/$(experimentFolderName) --launchd -- python3 ../../lib/runmaker/runmaker4.py --jobs $(runtimeRunmakerJobs) -l $(experimentId)_RuntimeRuns.log --loglines $(runmakerLogLines) $(experimentId)_RuntimeRuns.txt" || true
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

# checkPerfResults: $(PERF_RESULTS:.perf=.perf.counts.csv)
# $(PERF_RESULTS:.perf=.perf.results.csv): %.perf.results.csv: %.perf
# 	@echo "target: $@\nprereq: $^\nstem: $*"
# 	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- perl eval/perfResults.pl $<

detectPerfScriptDuplicates: $(PERF_RESULTS:.perf=.perf.unique.perl)
$(PERF_RESULTS:.perf=.perf.unique.perl): %.perf.unique.perl: %.perf
	@echo "target: $@\nprereq: $^\nstem: $*"
	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- perl eval/detectPerfScriptDuplicates.pl $<

UNIQUE_PERF_RESULTS := $(shell find perfResults -name "*.perf.unique.perl")
$(PERF_RESULTS:.perf=.perf.unique.perl.counts.csv): %.perf.unique.perl.counts.csv: %.perf.unique.perl
	@echo "target: $@\nprereq: $^\nstem: $*"
	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript eval/checkPerfResults.R $<

$(PERF_RESULTS:.perf=.perf.unique.perl.results.csv): %.perf.unique.perl.results.csv: %.perf.unique.perl
	@echo "target: $@\nprereq: $^\nstem: $*"
	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- perl eval/perfResults.pl $<

# resultCollection: $(vectorCsvFiles) $(scalarCsvFiles) $(PERF_RESULTS:.perf=.perf.counts.csv) $(PERF_RESULTS:.perf=.perf.unique.perl) $(PERF_RESULTS:.perf=.perf.unique.perl.counts.csv) $(PERF_RESULTS:.perf=.perf.unique.perl.results.csv)
resultCollection: $(scalarCsvFiles) $(PERF_RESULTS:.perf=.perf.counts.csv) $(PERF_RESULTS:.perf=.perf.unique.perl) $(PERF_RESULTS:.perf=.perf.unique.perl.counts.csv) $(PERF_RESULTS:.perf=.perf.unique.perl.results.csv)
	touch $@

# ------------------------ demoExperiment -------------------------------------------------------------------------------------------------

$(demoVectorCsvFiles): csv/vectors/%.csv : $(demoVectorFiles)
	@echo "target: $@\nprereq: $^\nstem: $*"
	mkdir -p csv/vectors
# 	../../lib/veins_scripts/eval/opp_vec2csv.pl -v -M 'ManhattanBuildingShadowing\.leo[a-z,A-Z]*\[(?<module>[0-9]+)\]\.*' -F $(word 2, $(subst -, ,$*)):vector=$(word 2, $(subst -, ,$*)) -A repetition results/$(word 1, $(subst -, ,$*))*.vec > $@
	../../lib/veins_scripts/eval/opp_vec2csv.pl -v -M 'ManhattanBuildingShadowing\.*' -F $(word 2, $(subst -, ,$*)):vector=$(word 2, $(subst -, ,$*)) -A repetition results/$(word 1, $(subst -, ,$*))*.vec > $@

$(demoScalarCsvFiles): csv/scalars/%.csv : $(demoScalarFiles)
	@echo "target: $@\nprereq: $^\nstem: $*"
	mkdir -p csv/scalars
	../../lib/veins_scripts/eval/opp_sca2csv.pl -v -M 'ManhattanBuildingShadowing\.*' -F $(word 2, $(subst -, ,$*)):count=$(word 2, $(subst -, ,$*)) -F $(word 2, $(subst -, ,$*)):last=$(word 2, $(subst -, ,$*)) -A repetition results/$(word 1, $(subst -, ,$*))*.sca > $@

demoResultCollection: $(demoScalarCsvFiles) $(demoVectorCsvFiles)
	touch $@

# ------------------------ runtimeExperiment -------------------------------------------------------------------------------------------------

$(runtimeVectorCsvFiles): csv/vectors/%.csv : $(runtimeVectorFiles)
	@echo "target: $@\nprereq: $^\nstem: $*"
	mkdir -p csv/vectors
# 	../../lib/veins_scripts/eval/opp_vec2csv.pl -v -M 'ManhattanBuildingShadowing\.leo[a-z,A-Z]*\[(?<module>[0-9]+)\]\.*' -F $(word 2, $(subst -, ,$*)):vector=$(word 2, $(subst -, ,$*)) -A repetition results/$(word 1, $(subst -, ,$*))*.vec > $@
	../../lib/veins_scripts/eval/opp_vec2csv.pl -v -M 'ManhattanBuildingShadowing\.*' -F $(word 2, $(subst -, ,$*)):vector=$(word 2, $(subst -, ,$*)) -A repetition results/$(word 1, $(subst -, ,$*))*.vec > $@

$(runtimeScalarCsvFiles): csv/scalars/%.csv : $(runtimeScalarFiles)
	@echo "target: $@\nprereq: $^\nstem: $*"
	mkdir -p csv/scalars
	../../lib/veins_scripts/eval/opp_sca2csv.pl -v -M 'ManhattanBuildingShadowing\.*' -F $(word 2, $(subst -, ,$*)):count=$(word 2, $(subst -, ,$*)) -F $(word 2, $(subst -, ,$*)):last=$(word 2, $(subst -, ,$*)) -A repetition -I "updateInterval"=updateInterval results/$(word 1, $(subst -, ,$*))*.sca > $@

runtimeResultCollection: $(runtimeScalarCsvFiles)
	touch $@

#########################################################################
#                                                                       #
#                                                                       #
#                          prepare plotting                             #
#                                                                       #
#                                                                       #
#########################################################################

ACCUMULATED_PERF_RESULTS := $(shell find perfResults/ -name "*.perf.unique.perl.perl.results.csv")
plots/mobilityRunTimeShare.csv: eval/mobilityRunTimeShare.R $(ACCUMULATED_PERF_RESULTS)
# 	echo $(ACCUMULATED_PERF_RESULTS)
	mkdir -p plots
	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript $< $(ACCUMULATED_PERF_RESULTS)

AGE_OF_INFORMATION_RESULTS := $(shell find csv/scalars -name "*ageOfInformation.csv")
plots/ageOfInformation.csv: eval/ageOfInformation.R $(AGE_OF_INFORMATION_RESULTS)
# 	echo $(AGE_OF_INFORMATION_RESULTS)
	mkdir -p plots
	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript $< $(AGE_OF_INFORMATION_RESULTS)

# ------------------------ demoExperiment/runtimeExperiment -------------------------------------------------------------------------------------------------
archive/latest/csv/gnuTimeResults/gnuTimeResults.csv: eval/gnuTimeResults.R
	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript $^ archive/latest/csv/gnuTimeResults/rawData

archive/latest/csv/receptionsMap/classification.csv: eval/processReceptionsMap.sh
	cd archive/latest/csv/receptionsMap && bash ../../../../$^

# ------------------------ runtimeExperiment -------------------------------------------------------------------------------------------------
RUNTIME_AGE_OF_INFORMATION_RESULTS := $(shell find archive/latest/csv/scalars -name "*ageOfInformation.csv")
archive/latest/plots/ageOfInformation.csv: eval/ageOfInformation.R $(RUNTIME_AGE_OF_INFORMATION_RESULTS)
	echo $(RUNTIME_AGE_OF_INFORMATION_RESULTS)
	mkdir -p archive/latest/plots
	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript $< $(RUNTIME_AGE_OF_INFORMATION_RESULTS)

#########################################################################
#                                                                       #
#                                                                       #
#                                plotting                               #
#                                                                       #
#                                                                       #
#########################################################################
plots/runTimeShareVsConfig.pdf: eval/runTimeShareVsConfig.R plots/mobilityRunTimeShare.csv
	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript $< plots/mobilityRunTimeShare.csv

plots/benchmarkSpeedUp.pdf: eval/benchmarkSpeedUp.R plots/mobilityRunTimeShare.csv
	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript $< plots/mobilityRunTimeShare.csv

plots/benchmarkAoiVsConfig.pdf: eval/benchmarkAoiVsConfig.R plots/ageOfInformation.csv
	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript $< plots/ageOfInformation.csv

perfBenchmarkPlots: plots/runTimeShareVsConfig.pdf plots/benchmarkSpeedUp.pdf plots/benchmarkAoiVsConfig.pdf
	mkdir -p plots
	touch $@

# ------------------------ demoExperiment -------------------------------------------------------------------------------------------------
plotUpdatePositionOnlyAboveHorizon:
	mkdir -p archive/latest/plots
	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript eval/updatePositionOnlyAboveHorizon-elevationAngle-vs-time.R archive/latest/csv/vectors/DemoDisUpPosOnlyAboveHorizonNGCP_1c0-lastElevationAngleToSop.csv \
																																			  archive/latest/csv/vectors/DemoEnUpPosOnlyAboveHorizonNGCP_1c0-lastElevationAngleToSop.csv \
																																			  archive/latest/plots/updatePositionOnlyAboveHorizon_1c0-elevationAngle-vs-time-NGCP

plotPositionUpdateBehavior: eval/positionUpdateBehavior-elevationAngle-vs-time.R
	mkdir -p archive/latest/plots
	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript $< archive/latest/csv/vectors/DemoDisUpPosOnlyAboveHorizonNGCP_1c0-lastElevationAngleToSop.csv \
																					archive/latest/csv/vectors/DemoDisUpPosOnlyAboveHorizonNGCP_1c0-getCurrentPositionUpdateCalls.csv \
																					archive/latest/csv/vectors/DemoDisUpPosOnlyAboveHorizonNGCP_1c0-sgp4Computations.csv \
																					archive/latest/plots/DemoDisUpPosOnlyAboveHorizonNGCP_1c0

# 																					archive/latest/csv/vectors/DemoDisUpPosOnlyAboveHorizonNGCP_1c0-linearInterpolationComputations.csv \

	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript $< archive/latest/csv/vectors/DemoDisUpPosOnlyAboveHorizonWLIP_1c0-lastElevationAngleToSop.csv \
																					archive/latest/csv/vectors/DemoDisUpPosOnlyAboveHorizonWLIP_1c0-getCurrentPositionUpdateCalls.csv \
																					archive/latest/csv/vectors/DemoDisUpPosOnlyAboveHorizonWLIP_1c0-sgp4Computations.csv \
																					archive/latest/plots/DemoDisUpPosOnlyAboveHorizonWLIP_1c0

# 																					archive/latest/csv/vectors/VerifyDisUpPosOnlyAboveHorizonWLIP-linearInterpolationComputations.csv \

	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript $< archive/latest/csv/vectors/DemoDisUpPosOnlyAboveHorizonWNPP_1c0-lastElevationAngleToSop.csv \
																					archive/latest/csv/vectors/DemoDisUpPosOnlyAboveHorizonWNPP_1c0-getCurrentPositionUpdateCalls.csv \
																					archive/latest/csv/vectors/DemoDisUpPosOnlyAboveHorizonWNPP_1c0-sgp4Computations.csv \
																					archive/latest/plots/DemoDisUpPosOnlyAboveHorizonWNPP_1c0

# 																					archive/latest/csv/vectors/VerifyDisUpPosOnlyAboveHorizonWNPP-linearInterpolationComputations.csv \

	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript $< archive/latest/csv/vectors/DemoDisUpPosOnlyAboveHorizonWGCP_1c0-lastElevationAngleToSop.csv \
																					archive/latest/csv/vectors/DemoDisUpPosOnlyAboveHorizonWGCP_1c0-getCurrentPositionUpdateCalls.csv \
																					archive/latest/csv/vectors/DemoDisUpPosOnlyAboveHorizonWGCP_1c0-sgp4Computations.csv \
																					archive/latest/plots/DemoDisUpPosOnlyAboveHorizonWGCP1_c0

# 																					archive/latest/csv/vectors/VerifyDisUpPosOnlyAboveHorizonWGCP-linearInterpolationComputations.csv \

# demoPlots: plotUpdatePositionOnlyAboveHorizon
demoPlots: plotUpdatePositionOnlyAboveHorizon plotPositionUpdateBehavior
	touch $@

# ------------------------ runtimeExperiment -------------------------------------------------------------------------------------------------

archive/latest/plots/runtimeUserTimeVsUpdateInterval.filtered.pdf: eval/runtimeUserTimeVsUpdateInterval.R
	mkdir -p archive/latest/plots
	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript $< archive/latest/csv/gnuTimeResults/gnuTimeResults.csv

archive/latest/plots/runtimeAccuracyVsUpdateInterval.filtered.pdf: eval/runtimeAccuracyVsUpdateInterval.R archive/latest/csv/receptionsMap/classification.csv
	mkdir -p archive/latest/plots
	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript $< archive/latest/csv/receptionsMap/classification.csv

archive/latest/plots/runtimeSensitivityVsUpdateInterval.filtered.pdf: eval/runtimeSensitivityVsUpdateInterval.R archive/latest/csv/receptionsMap/classification.csv
	mkdir -p archive/latest/plots
	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript $< archive/latest/csv/receptionsMap/classification.csv

archive/latest/plots/runtimeAoiEcdf.pdf: eval/runtimeAoiEcdf.R archive/latest/plots/ageOfInformation.csv
	mkdir -p archive/latest/plots
	singularity run ../../lib/singularity-r-env/singularity-R-env.sif -- Rscript $< archive/latest/plots/ageOfInformation.csv

runtimePlots: archive/latest/plots/runtimeAoiEcdf.pdf archive/latest/plots/runtimeUserTimeVsUpdateInterval.filtered.pdf archive/latest/plots/runtimeSensitivityVsUpdateInterval.filtered.pdf
	touch $@

#########################################################################
#                                                                       #
#                                                                       #
#                          run experiments                              #
#                                                                       #
#                                                                       #
#########################################################################
verificationExperiment:
	mkdir -p archive/$(experimentId)
	$(MAKE) -j1 experimentId=$(experimentId) runVerification
	$(MAKE) -j8 experimentId=$(experimentId) resultCollection
	$(MAKE) -j8 experimentId=$(experimentId) perfBenchmarkPlots
	mv $(experimentId)_VerificationRuns.log archive/$(experimentId)
	mv $(experimentId)_VerificationRuns.txt archive/$(experimentId)
	mv results archive/$(experimentId)
	mv perfResults archive/$(experimentId)
	mv csv archive/$(experimentId)
	mv plots archive/$(experimentId)
	rm archive/latest
	ln -s $(experimentId) archive/latest
	rm runVerification resultCollection perfBenchmarkPlots

demoExperiment:
	mkdir -p archive/$(experimentId)
	$(MAKE) -j8 experimentId=$(experimentId) demo_space_veins
	$(MAKE) -j1 experimentId=$(experimentId) runDemo
	$(MAKE) -j8 experimentId=$(experimentId) demoResultCollection
	mv $(experimentId)_DemoRuns.log archive/$(experimentId)
	mv $(experimentId)_DemoRuns.txt archive/$(experimentId)
	mv results archive/$(experimentId)
	mv csv archive/$(experimentId)
	mkdir -p archive/$(experimentId)/csv/gnuTimeResults/rawData
	mv GnuTimeResults_* archive/$(experimentId)/csv/gnuTimeResults/rawData
	mkdir -p archive/$(experimentId)/csv/receptionsMap/rawData
	mv /ramdisk/ReceptionsMap_* archive/$(experimentId)/csv/receptionsMap/rawData
	rm archive/latest
	ln -s $(experimentId) archive/latest
	$(MAKE) -j8 experimentId=$(experimentId) archive/latest/csv/receptionsMap/classification.csv
	$(MAKE) -j1 experimentId=$(experimentId) archive/latest/csv/gnuTimeResults/gnuTimeResults.csv
	$(MAKE) -j8 experimentId=$(experimentId) demoPlots
	rm runDemo demoResultCollection demoPlots

runtimeExperiment:
	mkdir -p archive/$(experimentId)
	rm archive/latest
	ln -s $(experimentId) archive/latest
	$(MAKE) -j6 experimentId=$(experimentId) runtime_space_veins
	$(MAKE) -j1 experimentId=$(experimentId) runRuntime
	sleep 1
	rm -r results
	ln -s archive/latest/results results
	$(MAKE) -j5 experimentId=$(experimentId) runtimeResultCollection
	rm -r results
	mv $(experimentId)_RuntimeRuns.log archive/$(experimentId)
	mv $(experimentId)_RuntimeRuns.txt archive/$(experimentId)
# 	mv results archive/$(experimentId)
	mkdir -p archive/$(experimentId)/csv/
	mv csv/scalars archive/$(experimentId)/csv/
	mkdir -p archive/$(experimentId)/csv/gnuTimeResults/rawData
	mv GnuTimeResults_* archive/$(experimentId)/csv/gnuTimeResults/rawData
# 	mkdir -p archive/$(experimentId)/csv/receptionsMap/rawData
# 	mv /ramdisk/ReceptionsMap_* archive/$(experimentId)/csv/receptionsMap/rawData
	$(MAKE) -j8 experimentId=$(experimentId) archive/latest/csv/receptionsMap/classification.csv
	$(MAKE) -j8 experimentId=$(experimentId) archive/latest/csv/gnuTimeResults/gnuTimeResults.csv
	$(MAKE) -j8 experimentId=$(experimentId) archive/latest/plots/ageOfInformation.csv
	$(MAKE) -j8 experimentId=$(experimentId) runtimePlots
	rm -r csv
	rm runRuntime runtimeResultCollection runtimePlots

flamegraphs: eval/createFlamegraphForEveryStrat.sh
	mkdir -p archive/$(experimentId)
	rm archive/latest
	ln -s $(experimentId) archive/latest
	$(MAKE) -j12 experimentId=$(experimentId) space_veins
	bash $<

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
