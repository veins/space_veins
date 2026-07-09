#!/bin/bash
poetry install
poetry run python3 ./computeWgs84PositionsWithSkyfield.py demosat.txt 2025-12-31:00-12-15 200.0 0.01
poetry run python3 ./computeUT1.py 2025-12-31:00-12-15 200.0 0.01