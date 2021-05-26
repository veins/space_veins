# SPDX-FileCopyrightText: 2021 Mario Franke <research@m-franke.net>
#
# SPDX-License-Identifier: GPL-2.0-or-later

all: space_veins buildPythonSkyfield
	@echo 'space_Veins compiled successfully'

buildSpaceVeins_and_run_example: space_veins
	cd examples/space_veins && ./run x-terminal-emulator -- -c WithoutBeaconing -u Cmdenv

run_example:
	cd examples/space_veins && ./run x-terminal-emulator -- -c WithoutBeaconing -u Cmdenv

run_dbg_example_gui: space_veins
	cd examples/space_veins && ./run x-terminal-emulator -d --tool gdb -- -c WithoutBeaconing

run_dbg_example_cmd: space_veins
	cd examples/space_veins && ./run x-terminal-emulator -d --tool gdb -- -c WithoutBeaconing -u Cmdenv

space_veins: checkmakefiles
	cd src && $(MAKE) MODE=release -j4
	cd src && $(MAKE) MODE=debug -j4

clean: checkmakefiles
	cd src && $(MAKE) clean

cleanall: checkmakefiles
	cd src && $(MAKE) MODE=release clean
	cd src && $(MAKE) MODE=debug clean
	rm -r src/space_veins/base/satellitesConnectionManager/protobuf
	rm -f src/Makefile
	rm -rf out/

makefiles:
	@echo
	@echo '====================================================================='
	@echo 'Warning: make makefiles has been deprecated in favor of ./configure'
	@echo '====================================================================='
	@echo
	./configure
	@echo
	@echo '====================================================================='
	@echo 'Warning: make makefiles has been deprecated in favor of ./configure'
	@echo '====================================================================='
	@echo

checkmakefiles:
	@if [ ! -f src/Makefile ]; then \
	echo; \
	echo '======================================================================='; \
	echo 'src/Makefile does not exist. Please use "make makefiles" to generate it!'; \
	echo '======================================================================='; \
	echo; \
	exit 1; \
	fi

cleanPythonSkyfield:
	@echo 'Clean python skyfield'
	@rm -rf lib/skyfield_protobuf/skyfield_protobuf_protocol.egg-info/
	@rm -rf lib/python_skyfield/.venv/
	@rm -f lib/skyfield_protobuf/MANIFEST
	@rm -rf lib/skyfield_protobuf/dist/
	@rm -rf lib/skyfield_protobuf/skyfield_protobuf/

skyfieldProtobufPackage:
	@echo 'Build skyfield_protobuf package.'
	@cd lib/skyfield_protobuf && python3 setup.py sdist

buildPythonSkyfield: cleanPythonSkyfield skyfieldProtobufPackage lib/python_skyfield/Pipfile.lock
	@echo 'Build Skyfield python environment'
	@cd lib/python_skyfield && pipenv sync

lib/python_skyfield/Pipfile.lock: lib/python_skyfield/Pipfile
	@echo 'Create Pipfile.lock'
	@cd lib/python_skyfield && pipenv lock
