#!/bin/bash

python3 createManhattanGrid.py 5 1 hamburg --latitude "53.55023422545714" --longitude "9.991979316018119" --degreePerKilometer hamburg --randomRoutes true --buildingHeightINET 15
python3 createManhattanGrid.py 5 1 reykjavik --latitude "64.13317904765591" --longitude "-21.89820811215422" --degreePerKilometer reykjavik --randomRoutes true --buildingHeightINET 15
python3 createManhattanGrid.py 5 1 Null-Island-5by5-grid --degreePerKilometer nullIsland --randomRoutes true --buildingHeightINET 15
python3 createManhattanGrid.py 3 0.75 Null-Island-3by3-grid --degreePerKilometer nullIsland --randomRoutes true --buildingHeightINET 15
