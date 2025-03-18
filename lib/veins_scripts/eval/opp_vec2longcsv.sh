#!/bin/bash
#
# Copyright (C) 2019 Dominik S. Buse <buse@ccs-labs.org>
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

# convert an OMNeT++ vector into a long-format CSV
#
# Works like a unix filter and can read the input vector from a file name or stdin.

set -e

function extract_data_records() {
    grep -v '^vector\|attr\|version\|param\|run\|itervar\|config' | grep -v '^$' | sort -s -k 1n,1 -k 2n,2 --buffer-size=5%
}

function extract_vector_definitions() {
    grep '^vector' | sort -s -k 2n | sed 's/vector[[:space:]]\{1,\}\([0-9]\{1,\}\)[[:space:]]\{1,\}\([^[:space:]]\{1,\}\)[[:space:]]\{1,\}\([^[:space:]]\{1,\}\)[[:space:]]\{1,\}[ETV]*/\1	\2	\3/'
}

FNAME="$1"
if [[ -z "$1" ]]; then
    FNAME="-"
fi

TMPDIR=$(mktemp -d)
trap "rm \"$TMPDIR/vec.fifo\"; rmdir \"$TMPDIR\"" EXIT
mkfifo $TMPDIR/vec.fifo

# this does pipeline does the following:
# - read the input file ($FNAME, could be stdin a.k.a "-")
# - duplicate the input by tee-ing into a named pipe (to support stdin streams)
# - extract the data records from one of the input streams
# - extract the vector definitions from the duplicated output stream (the named pipe)
# - join both together to create a csv file
cat $FNAME | tee $TMPDIR/vec.fifo | extract_data_records | join -j1 <(cat $TMPDIR/vec.fifo | extract_vector_definitions) -
