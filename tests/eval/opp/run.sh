#!/bin/bash

set -e

../../../eval/opp_vec2csv.pl --no-header --list F results/*.vec | grep --quiet "^filter\tvector1$"
../../../eval/opp_vec2csv.pl --no-header --list F results/*.vec | grep --quiet "^filter\tstatistic1:vector$"
../../../eval/opp_vec2csv.pl --no-header --list A results/*.vec | grep --quiet "^attr\trepetition$"
../../../eval/opp_vec2csv.pl --no-header --list I results/*.vec | grep --quiet "^itervar\titervar1$"

../../../eval/opp_sca2csv.pl --no-header --list F results/*.sca | grep --quiet "^filter\tscalar1$"
../../../eval/opp_sca2csv.pl --no-header --list F results/*.sca | grep --quiet "^filter\tstatistic1:timeavg$"
../../../eval/opp_sca2csv.pl --no-header --list A results/*.sca | grep --quiet "^attr\trepetition$"
../../../eval/opp_sca2csv.pl --no-header --list I results/*.sca | grep --quiet "^itervar\titervar1$"

#../../../eval/opp_vec2csv.pl --merge-by em --attr repetition --iter itervar1 --filter vector1 --filter statistic1:vector=signal1 results/*.vec | sort --reverse > vectors.csv
../../../eval/opp_vec2csv.pl --merge-by em --attr repetition --iter itervar1 --filter vector1 --filter statistic1:vector=signal1 results/*.vec | sort --reverse | diff -u vectors.csv -

#../../../eval/opp_sca2csv.pl --attr repetition --iter itervar1 --filter scalar1 --filter statistic1:timeavg=signal1 results/*.sca | sort --reverse > scalars.csv
../../../eval/opp_sca2csv.pl --attr repetition --iter itervar1 --filter scalar1 --filter statistic1:timeavg=signal1 results/*.sca | sort --reverse | diff -u scalars.csv -


echo success
