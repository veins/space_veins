#!/bin/bash

#
# Copyright (C) 2025 Mario Franke <research@m-franke.net>
#
# Documentation for these modules is at http://sat.car2x.org/
#
# SPDX-License-Identifier: GPL-2.0-or-later
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#


# install dependencies
apt-get update && export DEBIAN_FRONTEND=noninteractive && apt-get -y install --no-install-recommends \
    pkg-config \
    python3-dev \
    libelf-dev \
    libunwind-dev \
    libbabeltrace-dev \
    libdw-dev \
    systemtap-sdt-dev \
    libslang2-dev \
    libperl-dev \
    libcap-dev \
    libnuma-dev \
    libcapstone-dev \
    openjdk-8-jdk \
    libpfm4-dev \
    libtraceevent-dev \
    libbfd-dev \
    ;

KERNEL_MAIN_VERSION=$( echo $(uname -r) | cut -d '.' -f1 )
KERNEL_FULL_VERSION=$( echo $(uname -r) | cut -d '+' -f1 )

# download linux kernel
mkdir -p /opt/kernel
cd /opt/kernel
wget https://cdn.kernel.org/pub/linux/kernel/v"$KERNEL_MAIN_VERSION".x/linux-"$KERNEL_FULL_VERSION".tar.xz
tar -xf linux-"$KERNEL_FULL_VERSION".tar.xz

# build perf
cd linux-"$KERNEL_FULL_VERSION"/tools/perf
make BUILD_NONDISTRO=1 -j8

# install it
mkdir -p /usr/lib/linux-tools/"$KERNEL_FULL_VERSION"
cp perf /usr/lib/linux-tools/"$KERNEL_FULL_VERSION"/
ln -sf /usr/lib/linux-tools/"$KERNEL_FULL_VERSION"/perf /usr/bin/perf
