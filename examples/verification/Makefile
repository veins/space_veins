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

# sleep 5 seconds after finishing all runs to wait for files written to disk
# write results of GNU time in a dedicated file for every config to avoid race conditions
# --timeout=0: no timeout
# make results dir on own our because otherwise the radio medium communication log is not written
# 	hosts="nsm-lab01 nsm-lab02 nsm-lab05 nsm-lab07 nsm-lab08"
# 		ssh $$host 'cd $(mkfile_path) && cd ../../singularity/ && ./singularity-space_veins.sh /workspace/space_veins/examples/$(experimentFolderName) --launchd 9996 -- python3 ../../lib/runmaker/runmaker4.py --jobs $(runmakerJobs) -l $(experimentId)_runs.log --loglines $(runmakerLogLines) $(experimentId)_runs.txt' &
# 	hosts="nsm-lab01 nsm-lab05 nsm-lab07 nsm-lab08"
.ONESHELL:
SHELL := /bin/bash
runVnc: $(experimentId)_runs.log $(experimentId)_runs.txt perfMachines.txt
	mkdir -p results
	hosts="nsm-lab01 nsm-lab02 nsm-lab05 nsm-lab07"
	for host in $$hosts; do
		ssh $$host 'cd $(mkfile_path) && cd ../../singularity/ && ./singularity-space_veins.sh /workspace/space_veins/examples/$(experimentFolderName) --launchd 9996 -- python3 ../../lib/runmaker/runmaker4.py --jobs $(runmakerJobs) -l $(experimentId)_runs.log --loglines $(runmakerLogLines) $(experimentId)_runs.txt' &
	done
	wait
	sleep 1
	mv results archive/$(experimentId)
	touch $@

printRuns:
	cd $(mkfile_path) && cd ../../singularity/ && ./singularity-space_veins.sh /workspace/space_veins/examples/$(experimentFolderName) ./run -- -q runs -c VncDebug

killOnAllMachines:
	hosts="nsm-lab01 nsm-lab02 nsm-lab05 nsm-lab07"
	for host in $$hosts; do
		ssh $$host 'pkill -u mafr432d'
	done

exp:
	mkdir -p archive/$(experimentId)
	rm archive/latest
	ln -s $(experimentId) archive/latest
	$(MAKE) -j1 experimentId=$(experimentId) runVnc
	mv $(experimentId)_runs.log archive/$(experimentId)
	mv $(experimentId)_runs.txt archive/$(experimentId)

time = $(shell date +%Y-%m-%dT%H'\':%M'\':%S)
test:
	echo $(time)

parentTarget:
	echo $(time)
	sleep 3
	$(MAKE) time=$(time) test
