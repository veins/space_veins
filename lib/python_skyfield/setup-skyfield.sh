#!/bin/bash

# SPDX-FileCopyrightText: 2021 Mario Franke <research@m-franke.net>
#
# SPDX-License-Identifier: GPL-2.0-or-later

# create python virtual environment
python3 -mvenv .venv

# enter virtual environment
source .venv/bin/activate

# build skyfield_protobuf python package
cd ../skyfield_protobuf
python setup.py sdist
cd ../python_skyfield

echo $(pwd)

# install requirements / libraries
pipenv install --sequential
