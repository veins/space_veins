#!/bin/bash
poetry install
poetry run python3 ./computeWgs84PositionsWithSkyfield.py space_Veins-1.txt 2022-02-22:13-42-00 165.0 0.01
poetry run python3 ./computeUT1.py 2022-02-22:13-42-00 165.0 0.01