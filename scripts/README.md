# General
It is assumed that all commands in this file are executed in either the docker or the singularity container.
Documentation to the single components is provided as help texts with the option ```--help``` e.g., ```poetry run python3 createManhattanGrid.py --help```

# How to create a Manhattan grid at a random location
1. Install the dependencies using poetry ```poetry install```
2. Run this script ```poetry run ./createManhattanGrid.sh```

# How to find out when a satellite is over a random location
1. Install the dependencies using poetry ```poetry install```
2. Run this script ```poetry run python3 findSuitableSatellite.py -e 25 ../examples/space_veins/space_Veins-1.txt 0.00 0.00 2022-02-22:00-00-01 2022-02-22:23-59-59```