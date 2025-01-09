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
experimentTime := $(shell date +%Y-%m-%dT%H'\':%M'\':%S)
experimentCommit := $(shell git rev-parse --short HEAD)
experimentId := $(experimentTime)_$(experimentCommit)
configs := Iridium86Active20240304NullIslandEnableUpdatePositionOnlyAboveHorizon Iridium86Active20240304NullIslandDisableUpdatePositionOnlyAboveHorizon 

all: ../../singularity/singularity-space_veins.sif $(space_veins_binaries)

clean:
	rm *_runs.log *_runs.txt

shell:
	cd ../../singularity && ./singularity-space_veins.sh shell

../../singularity/singularity-space_veins.sif: ../../singularity/buildSingularityContainer.sh ../../singularity/singularity-space_veins.def
	cd ../../singularity && bash buildSingularityContainer.sh

$(space_veins_binaries): ../../Makefile ../../singularity/singularity-space_veins.sif
	cd ../../singularity && ./singularity-space_veins.sh /workspaces/space_veins -- make makefiles
	cd ../../singularity && ./singularity-space_veins.sh /workspaces/space_veins -- make -j8 all

$(experimentId)_runs.log:
	@touch $@

$(experimentId)_runs.txt: ../../lib/veins_scripts/running/generateRunsFile.pl
	cd ../../singularity && ./singularity-space_veins.sh /workspaces/space_veins/examples/manhattanGrid -- perl $< $(configs) > ../examples/manhattanGrid/$@

test:
	@echo $(experimentId)

experiment: ../../singularity/singularity-space_veins.sif $(space_veins_binaries) $(experimentId)_runs.log $(experimentId)_runs.txt
	@echo Run experiment: $(experimentId)