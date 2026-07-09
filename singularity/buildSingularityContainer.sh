#!/bin/bash

#
# Copyright (C) 2026 Mario Franke <research@m-franke.net>
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

# build docker container
cd ../
SPACE_VEINS_VERSION=$(./print-space_Veins-version)
SPACE_VEINS_GIT_HASH=$(git rev-parse --short HEAD)
cd .devcontainer
SPACE_VEINS_ID="${SPACE_VEINS_VERSION}-${SPACE_VEINS_GIT_HASH}"
SPACE_VEINS_DOCKER_CONTAINER_NAME="space_veins:${SPACE_VEINS_ID}"
SPACE_VEINS_DOCKER_CONTAINER_ARCHIVE_NAME="space_veins-${SPACE_VEINS_ID}.tar"
SPACE_VEINS_DOCKER_CONTAINER_LOCATION="localhost"
SPACE_VEINS_DOCKER_CONTAINER="${SPACE_VEINS_DOCKER_CONTAINER_LOCATION}/${SPACE_VEINS_DOCKER_CONTAINER_NAME}"
# Always build the docker container, due to caching this does not cost much time if nothing changed in the Dev Container
docker build -t "$SPACE_VEINS_DOCKER_CONTAINER" .

# remove container tarball if it exists
if [ -f "${SPACE_VEINS_DOCKER_CONTAINER_ARCHIVE_NAME}" ]
    then
        rm ${SPACE_VEINS_DOCKER_CONTAINER_ARCHIVE_NAME}
fi
# Write container tarball such that it always contains the newest changes, even if the changes are not part of a git commit
docker save -o "${SPACE_VEINS_DOCKER_CONTAINER_ARCHIVE_NAME}" "${SPACE_VEINS_DOCKER_CONTAINER}"

# set correct docker image tag in singularity definition file, always use latest version and commit hash
cd ../singularity
sed -i 's/From: \.\.\/\.devcontainer\/space_veins-[0-9]*\.[0-9]*-[a-zA-Z0-9]*.tar/From: \.\.\/\.devcontainer\/'"${SPACE_VEINS_DOCKER_CONTAINER_ARCHIVE_NAME}"'/g' singularity-space_veins.def

# set correct version
sed -i "s/    Version [0-9]*\.[0-9]*-[a-zA-Z0-9]*/    Version ${SPACE_VEINS_ID}/g" singularity-space_veins.def

# build singularity container
singularity build --force --fakeroot singularity-space_veins.sif singularity-space_veins.def

# reset singularity-space_veins.def such that Git does not detect changes.
sed -i 's/From: \.\.\/\.devcontainer\/'"${SPACE_VEINS_DOCKER_CONTAINER_ARCHIVE_NAME}"'/From: \.\.\/\.devcontainer\/space_veins-0\.0-XXXXXXX.tar/g' singularity-space_veins.def
sed -i "s/    Version ${SPACE_VEINS_ID}/    Version 0\.0-XXXXXXX/g" singularity-space_veins.def
