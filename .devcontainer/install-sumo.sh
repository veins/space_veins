#!/bin/bash

#
# Copyright (C) 2023 Christoph Sommer <sommer@cms-labs.org>
#
# Documentation for these modules is at http://veins.car2x.org/
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

# This script needs the following environment variables to be set:
#   - SUMO_VERSION: the version of SUMO to install (e.g. 1.18.0)

# abort if one of the environment variables is not set
if [ -z "$SUMO_VERSION" ]; then
    echo "SUMO_VERSION is not set"
    exit 1
fi

set -e

# install dependencies
apt-get update && export DEBIAN_FRONTEND=noninteractive && apt-get -y install --no-install-recommends \
    git \
    ccache \
    curl \
    cmake \
    libxerces-c-dev \
    libgdal-dev \
    libproj-dev \
    libfox-1.6-dev \
    python3-dev \
    ;

# convert dots to underscores in SUMO_VERSION
SUMO_VERSION_UNDERSCORE=$(echo $SUMO_VERSION | sed 's/\./_/g')

# download and unpack SUMO if not already done
cd /opt
if [ ! -d sumo-$SUMO_VERSION ]; then
    mkdir -p sumo-$SUMO_VERSION
    cd sumo-$SUMO_VERSION
    curl --location https://github.com/eclipse/sumo/archive/refs/tags/v${SUMO_VERSION_UNDERSCORE}.tar.gz | tar -xzv --strip-components=1
fi

cd /opt
ln -sf sumo-$SUMO_VERSION sumo

# enable ccache
export PATH=/usr/lib/ccache:$PATH

# build SUMO
cd /opt/sumo
cd build
cmake ..
make -j$(nproc)
