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

runmakerJobs := 1
runmakerLogLines := 50

shell:
	cd ../../singularity && ./singularity-space_veins.sh shell

ccFiles := $(shell find ../../src -name "*.cc")
hFiles := $(shell find ../../src -name "*.h")
space_veins: $(ccFiles) $(hFiles) ../../src/Makefile
	cd ../../singularity && ./singularity-space_veins.sh /workspace/space_veins -- make -j8 space_veins

#########################################################################
#                                                                       #
#                                                                       #
#                                runmaker                               #
#                                                                       #
#                                                                       #
#########################################################################
$(experimentId)_runs.log:
	@touch $@

$(experimentId)_runs.txt: ../../lib/veins_scripts/running/generateRunsVnc.pl ../../src/libspace_veins.so
	cd ../../singularity && ./singularity-space_veins.sh /workspace/space_veins/examples/$(experimentFolderName) -- perl $< VncDebug > ../examples/$(experimentFolderName)/$@

runmakerFiles: $(experimentId)_runs.log $(experimentId)_runs.txt


#########################################################################
#                                                                       #
#                                                                       #
#                         Run simulations                               #
#                                                                       #
#                                                                       #
#########################################################################

# 	hosts="nsm-lab01 nsm-lab02 nsm-lab05 nsm-lab07 nsm-lab08"
# 		ssh $$host 'cd $(mkfile_path) && cd ../../singularity/ && ./singularity-space_veins.sh /workspace/space_veins/examples/$(experimentFolderName) --launchd 9996 -- python3 ../../lib/runmaker/runmaker4.py --jobs $(runmakerJobs) -l $(experimentId)_runs.log --loglines $(runmakerLogLines) $(experimentId)_runs.txt' &
# 	hosts="nsm-lab01 nsm-lab05 nsm-lab07 nsm-lab08"
.ONESHELL:
SHELL := /bin/bash
runVnc: $(experimentId)_runs.log $(experimentId)_runs.txt perfMachines.txt
	mkdir -p results
	hosts="nsm-lab01 nsm-lab02 nsm-lab05 nsm-lab07 nsm-lab08"
	for host in $$hosts; do
		ssh $$host 'cd $(mkfile_path) && cd ../../singularity/ && ./singularity-space_veins.sh /workspace/space_veins/examples/$(experimentFolderName) --launchd 9996 -- python3 ../../lib/runmaker/runmaker4.py --jobs $(runmakerJobs) -l $(experimentId)_runs.log --loglines $(runmakerLogLines) $(experimentId)_runs.txt' &
	done
	wait
	sleep 1
	mv results archive/$(experimentId)
	for host in $$hosts; do
		ssh $$host 'rm -r /tmp/franke/rx' &
	done
	wait
	sleep 1
	touch $@

printRuns:
	cd $(mkfile_path) && cd ../../singularity/ && ./singularity-space_veins.sh /workspace/space_veins/examples/$(experimentFolderName) ./run -- -q runs -c VncDebug

#########################################################################
#                                                                       #
#                                                                       #
#                         Result collection                             #
#                                                                       #
#                                                                       #
#########################################################################

scalarFiles := $(shell find archive/latest/results -name "*.sca")
.ONESHELL:
SHELL := /bin/bash
sca2csv:
	mkdir -p archive/latest/csv/scalars
	ssh nsm-sim05 mkdir -p /tmp/franke/$(experimentId)/results
	scp -r archive/latest/results/*.sca nsm-sim05:/tmp/franke/$(experimentId)/results
	ssh nsm-sim05 /home/mafr432d/git/space_veins/examples/verification/eval/opp_sca2csv.sh "/tmp/franke/$(experimentId)"
	scp nsm-sim05:/tmp/franke/$(experimentId)/scalars.csv archive/latest/csv/scalars/scalars.csv
	ssh nsm-sim05 rm -r /tmp/franke/$(experimentId)

.ONESHELL:
SHELL := /bin/bash
gnuTimeResults: eval/gnuTimeResults.pl
	/home/mafr432d/git/space_veins/examples/verification/eval/gnuTimeResults.pl archive/latest/csv/gnuTime/*.csv

.ONESHELL:
SHELL := /bin/bash
classification: eval/processReceptionsMap.sh eval/compareReceptionsMapFiles.sh
	ssh nsm-sim05 'mkdir -p /tmp/franke/$(experimentId)/receptionsMap'
	ssh nsm-sim05 'mkdir -p /tmp/franke/$(experimentId)/receptionsMap/intermediate'
	rsync -avz --progress archive/latest/csv/receptionsMap/rawData nsm-sim05:/tmp/franke/$(experimentId)/receptionsMap/
	ssh nsm-sim05 '/home/mafr432d/git/space_veins/examples/verification/eval/processReceptionsMap.sh /tmp/franke/$(experimentId)/receptionsMap/rawData > /tmp/franke/$(experimentId)/receptionsMap/classification.csv'
	rsync -avz --progress nsm-sim05:/tmp/franke/$(experimentId)/receptionsMap/intermediate archive/latest/csv/receptionsMap/
	rsync -avz --progress nsm-sim05:/tmp/franke/$(experimentId)/receptionsMap/classification.csv archive/latest/csv/classification.csv
	ssh nsm-sim05 'rm -r /tmp/franke/$(experimentId)'


#########################################################################
#                                                                       #
#                                                                       #
#                               orga                                    #
#                                                                       #
#                                                                       #
#########################################################################
killOnAllMachines:
	hosts="nsm-lab01 nsm-lab02 nsm-lab05 nsm-lab07 nsm-lab08"
	for host in $$hosts; do
		ssh $$host 'pkill -u mafr432d'
	done

cleanTmp:
	hosts="nsm-lab01 nsm-lab02 nsm-lab05 nsm-lab07 nsm-lab08"
	for host in $$hosts; do
		ssh $$host 'rm -r /tmp/franke/rx'
	done

exp:
	mkdir -p archive/$(experimentId)
	rm archive/latest
	ln -s $(experimentId) archive/latest
	$(MAKE) -j1 experimentId=$(experimentId) runVnc
	$(MAKE) -j1 experimentId=$(experimentId) sca2csv
	$(MAKE) -j1 experimentId=$(experimentId) gnuTimeResults
	$(MAKE) -j1 experimentId=$(experimentId) classification
	mv $(experimentId)_runs.log archive/$(experimentId)
	mv $(experimentId)_runs.txt archive/$(experimentId)
	cp omnetpp.ini archive/$(experimentId)
	cp Makefile archive/$(experimentId)

time = $(shell date +%Y-%m-%dT%H'\':%M'\':%S)
test:
	echo $(time)

parentTarget:
	echo $(time)
	sleep 3
	$(MAKE) time=$(time) test
