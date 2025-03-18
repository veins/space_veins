#!/bin/bash

set -e

opp_makemake -f -o sample
make
./sample -u Cmdenv
