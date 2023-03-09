# SPDX-FileCopyrightText: 2021 Mario Franke <research@m-franke.net>
#
# SPDX-License-Identifier: GPL-2.0-or-later

.PHONY: all space_veins clean_space_veins cleanall_space_veins inet clean_inet cleanall_inet veins_inet clean cleanall makefiles spaceVeinsMakefiles inetMakefiles veinsInetMakefiles checkmakefiles checkInetMakefiles checkVeinsInetMakefiles veins veinsMakefiles checkVeinsMakefiles

all: space_veins
	@echo 'space_Veins compiled successfully'

# command line scripts
bin/space_veins_run: src/scripts/space_veins_run.in.py
	@echo "Creating script \"$@\""
	@mkdir -p bin
	@sed '/# v-- contents of out\/config.py go here/r out/config.py' "$<" > "$@"
	@chmod a+x "$@"

space_veins: checkmakefiles bin/space_veins_run veins_inet libproj
	cd src && $(MAKE) MODE=release
	cd src && $(MAKE) MODE=debug

clean_space_veins: checkmakefiles
	cd src && $(MAKE) clean

cleanall_space_veins: checkmakefiles
	cd src && $(MAKE) MODE=release clean
	cd src && $(MAKE) MODE=debug clean
	rm -f src/Makefile
	rm -rf out/config.py
	rm -rf build/

inet: checkInetMakefiles
	cd lib/inet/src && $(MAKE) MODE=release
	cd lib/inet/src && $(MAKE) MODE=debug

clean_inet: checkInetMakefiles
	cd lib/inet && $(MAKE) clean

cleanall_inet: checkInetMakefiles
	cd lib/inet && $(MAKE) cleanall

veins: checkVeinsMakefiles
	cd lib/veins/src && $(MAKE) MODE=release
	cd lib/veins/src && $(MAKE) MODE=debug
	cd lib/veins && $(MAKE) bin/veins_run

veins_inet: checkVeinsInetMakefiles veins inet
	cd lib/veins/subprojects/veins_inet/src && $(MAKE) MODE=release
	cd lib/veins/subprojects/veins_inet/src && $(MAKE) MODE=debug

clean: checkmakefiles checkInetMakefiles checkVeinsInetMakefiles
	cd src && $(MAKE) clean
	cd lib/inet && $(MAKE) clean
	cd lib/veins/subprojects/veins_inet/src && $(MAKE) clean

libproj:
	cd lib/proj && mkdir -p build
	cd lib/proj/build && cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DBUILD_PROJSYNC=OFF -DENABLE_TIFF=OFF -DENABLE_CURL=OFF -DBUILD_TESTING=OFF -DCMAKE_CXX_FLAGS=-fPIC ..
	cd lib/proj/build && $(MAKE)

cleanall: checkmakefiles checkInetMakefiles checkVeinsInetMakefiles
	rm -rf out/config.py
	rm -rf bin
	rm -rf lib/proj/build
	cd src && $(MAKE) MODE=release clean
	cd src && $(MAKE) MODE=debug clean
	rm -f src/Makefile
	cd lib/inet && $(MAKE) cleanall
	rm -rf lib/inet/out
	cd lib/veins && $(MAKE) cleanall
	cd lib/veins/subprojects/veins_inet && $(MAKE) cleanall

makefiles: inetMakefiles veinsMakefiles veinsInetMakefiles spaceVeinsMakefiles

spaceVeinsMakefiles:
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

inetMakefiles:
	cd lib/inet && opp_featuretool reset
	cd lib/inet && $(MAKE) src/inet/features.h
	cd lib/inet && $(MAKE) makefiles

veinsMakefiles:
	cd lib/veins && ./configure

veinsInetMakefiles:
	cd lib/veins/subprojects/veins_inet && ./configure --with-inet ../../../inet

checkmakefiles:
	@if [ ! -f src/Makefile ]; then \
	echo; \
	echo '======================================================================='; \
	echo 'src/Makefile does not exist. Please use "make makefiles" to generate it!'; \
	echo '======================================================================='; \
	echo; \
	exit 1; \
	fi

checkInetMakefiles:
	cd lib/inet && $(MAKE) checkmakefiles

checkVeinsMakefiles:
	@if [ ! -f lib/veins/src/Makefile ]; then \
	echo; \
	echo '======================================================================='; \
	echo 'lib/veins/src/Makefile does not exist. Please use "make veinsMakefiles" to generate it!'; \
	echo '======================================================================='; \
	echo; \
	exit 1; \
	fi

checkVeinsInetMakefiles:
	@if [ ! -f lib/veins/subprojects/veins_inet/src/Makefile ]; then \
	echo; \
	echo '======================================================================='; \
	echo 'lib/veins/subprojects/veins_inet/src/Makefile does not exist.';\
	echo 'Please use "make veinsInetMakefiles" to generate it!'; \
	echo '======================================================================='; \
	echo; \
	exit 1; \
	fi
