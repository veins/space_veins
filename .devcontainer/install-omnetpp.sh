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

# The following list shows changes made by Mario Franke
# Copyright (C) 2023 Mario Franke <research@m-franke.net>
# Documentation for space_Veins is at http://sat.car2x.org/
# SPDX-License-Identifier: GPL-2.0-or-later
#
# 2023-08-17 change PYTHONPATH variable because Ubuntu Jammy brings
#            python3.10
#            disable WITH_OSGEARTH
#            Set OMNeT++ download URL
#
# 2024-11-12 do not install ccache here, moved to install-misc.sh
#
# 2024-12-11 install ccache here, such that this script has no dependencies
#
# 2024-12-12 change default IDE workspace directory

# This script needs the following environment variables to be set:
#   - OPP_VERSION: the version of OMNeT++ to install (e.g. 6.0.1)

# abort if one of the environment variables is not set
if [ -z "$OPP_VERSION" ]; then
    echo "OPP_VERSION is not set"
    exit 1
fi

set -e

# install dependencies (needed before java will install)
apt-get update && export DEBIAN_FRONTEND=noninteractive && apt-get -y install --no-install-recommends \
    ca-certificates \
    ca-certificates-java \
    ;

# install dependencies
# note the use of clang-13 to allow valgrind 3.19 to interoperate with binaries
apt-get update && export DEBIAN_FRONTEND=noninteractive && apt-get -y install --no-install-recommends \
    bison \
    build-essential \
    ccache \
    clang-13 \
    curl \
    doxygen \
    flex \
    g++ \
    gcc \
    gdb \
    git \
    graphviz \
    libopenscenegraph-dev \
    libqt5opengl5-dev \
    libswt-gtk-4-java \
    libswt-webkit-gtk-4-jni \
    libxml2-dev \
    lld \
    make \
    openjdk-17-jre \
    perl \
    python3 \
    python3-matplotlib \
    python3-numpy \
    python3-pandas \
    python3-pip \
    python3-scipy \
    python3-seaborn \
    qtbase5-dev \
    libqt5svg5 \
    tcl-dev \
    tk-dev \
    wget \
    xdg-utils \
    zlib1g-dev \
    ;

# ensure xdg-utils is happy with the system
mkdir -p /usr/share/desktop-directories/

# install posix_ipc
mkdir -p /opt/pip
python3 -m pip install --root=/opt/pip posix_ipc
export PYTHONPATH=/opt/pip/usr/local/lib/python3.10/dist-packages:$PYTHONPATH

# get architecture (x86_64 or aarch64)
ARCH=$(uname -m)

# download and unpack OMNeT++ if not already done
cd /opt
if [ ! -d omnetpp-${OPP_VERSION} ]; then
    mkdir -p omnetpp-${OPP_VERSION}
    cd omnetpp-${OPP_VERSION}
    case "$OPP_VERSION" in
    5.7)
        curl --location https://github.com/omnetpp/omnetpp/releases/download/omnetpp-${OPP_VERSION}/omnetpp-${OPP_VERSION}-src-linux.tgz | tar -xzv --strip-components=1
        ;;
    # 6.0.2)
    #     curl --location https://github.com/omnetpp/omnetpp/releases/download/omnetpp-${OPP_VERSION}/omnetpp-${OPP_VERSION}-linux-${ARCH}.tgz | tar -xzv --strip-components=1
    *)
        echo "Unsupported OMNeT++ version"
    esac
fi

cd /opt
ln -sf omnetpp-${OPP_VERSION} omnetpp

# enable ccache
export PATH=/usr/lib/ccache:$PATH

# disable OSG EARTH because we did not installed the dependency
sed -i 's/WITH_OSGEARTH=yes/WITH_OSGEARTH=no/g' /opt/omnetpp/configure.user

# build OMNeT++
cd /opt/omnetpp
export PATH=$PATH:/opt/omnetpp/bin
source setenv
CC=clang-13 CXX=clang++-13 ./configure
make -j$(nproc) MODE=debug
make -j$(nproc) MODE=release

# make directory world-writable (as the IDE writes to its install location)
chmod -R a+wX /opt/omnetpp/

# change the default OMNeT++ workspace path to a path mounted into the dev container
case "$OPP_VERSION" in
5.7)
    OPP_DEFAULT_WORKSPACE_CONFIG_FILE=/opt/omnetpp/bin/omnetpp
    ;;
6.*)
    OPP_DEFAULT_WORKSPACE_CONFIG_FILE=/opt/omnetpp/bin/opp_ide
    ;;
*)
    echo "Unsupported OMNeT++ version"
esac
sed -i 's/DEFAULT_WORKSPACE_ARGS="-vmargs -Dosgi\.instance\.area\.default=\$IDEDIR\/\.\.\/samples"/DEFAULT_WORKSPACE_ARGS="-vmargs -Dosgi\.instance\.area\.default=\/opt\/omnetpp.workspace"/g' $OPP_DEFAULT_WORKSPACE_CONFIG_FILE