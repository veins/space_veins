#!/bin/bash

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