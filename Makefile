# SPDX-FileCopyrightText: 2021 Mario Franke <research@m-franke.net>
#
# SPDX-License-Identifier: GPL-2.0-or-later

all: space_veins
	@echo 'space_Veins compiled successfully'

run_example:
	cd examples/space_veins && ./run -- -c WithoutBeaconing -u Cmdenv

run_dbg_example_gui: space_veins
	cd examples/space_veins && ./run -d --tool gdb -- -c WithoutBeaconing

run_dbg_example_cmd: space_veins
	cd examples/space_veins && ./run -d --tool gdb -- -c WithoutBeaconing -u Cmdenv

space_veins: checkmakefiles checkConanFiles
	cd src && $(MAKE) MODE=release -j4
	cd src && $(MAKE) MODE=debug -j4

clean: checkmakefiles
	cd src && $(MAKE) clean

conan_deps:
	mkdir build && cd build && conan install --build=proj ..

cleanall: checkmakefiles
	cd src && $(MAKE) MODE=release clean
	cd src && $(MAKE) MODE=debug clean
	rm -f src/Makefile
	rm -rf out/
	rm -rf build/

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

checkConanFiles:
	@if [ ! -d build ]; then \
	echo; \
	echo '======================================================================='; \
	echo 'build/ does not exist. Please use "make conan_deps" to generate it!'; \
	echo '======================================================================='; \
	echo; \
	exit 1; \
	fi
