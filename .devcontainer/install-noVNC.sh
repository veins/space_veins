#!/usr/bin/env bash

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

set -e

export DEBIAN_FRONTEND=noninteractive

apt-get update && apt-get -y install --no-install-recommends \
    tigervnc-standalone-server \
    tigervnc-common \
    tigervnc-tools \
    x11-xserver-utils \
    xterm \
    xvfb \
    novnc \
    fluxbox

#=============================================================================
# Create index.html link to automatically display the VNC in the web browser
ln -s /usr/share/novnc/vnc.html /usr/share/novnc/index.html

#=============================================================================
# Create VNC helper scripts
mkdir -p /opt/vnc

#=============================================================================
# Manual start script
cat << 'EOF' > /opt/vnc/startVNC.sh
#!/bin/bash

# default parameters
VNC_DISPLAY=":4"
VNC_PORT="5904"
NOVNC_PORT="6080"
RESOLUTION="1280x720"

# Parameter read-in
while getopts v:n:r:d: flag
do
    case "${flag}" in
    v) VNC_PORT=${OPTARG};;
    n) NOVNC_PORT=${OPTARG};;
    r) RESOLUTION=${OPTARG};;
    d) VNC_DISPLAY=${OPTARG};;
    *) usage;;
    esac
done

# configure a password for the VNC server
# -s: hides the input
# -r: prevents backslashes from being interpreted as escape characters
# -p: display the prompt
read -s -r -p "Specify a password for the VNC server (at least 6 characters): " S_VNC_PASSWORD
echo

if [ ${#S_VNC_PASSWORD} -ge 6 ]; then
  echo "Password accepted."
else
  echo "Your password is too short. Six characters are required. This script will now exit."
  exit 1
fi
echo

echo "${S_VNC_PASSWORD}" | vncpasswd -f > /$HOME/.vnc-passwd

# start vnc
tigervncserver ${VNC_DISPLAY} -desktop fluxbox -localhost yes -SecurityTypes VncAuth -rfbport ${VNC_PORT} -rfbauth /$HOME/.vnc-passwd -geometry ${RESOLUTION} > /tmp/vnc.log 2>&1 &

# start noVNC
/usr/share/novnc/utils/launch.sh --listen ${NOVNC_PORT} --vnc localhost:${VNC_PORT} > /tmp/vnc.log 2>&1 &

echo "NoVNC is running on localhost:${NOVNC_PORT}"
EOF

#=============================================================================
# stop VNC server
cat << 'EOF' > /opt/vnc/stopVNC.sh
#!/bin/bash

pkill -f vnc
EOF

#=============================================================================
# make helper scripts executable
chmod +x /opt/vnc/startVNC.sh /opt/vnc/stopVNC.sh

# Display the message
echo "noVNC successfully installed!"