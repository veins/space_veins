#!/usr/bin/env perl
#
# Copyright (C) 2010 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
#
# SPDX-License-Identifier: GPL-2.0-or-later
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#

#
# opp_csvcat.pl - Concatenate .csv files (printing the header line only once)
#

use strict;
use warnings;

undef my $firstHeader;  ##< first line in first file #
undef my $firstCount;  ##< number of lines in first file #

foreach my $fname (@ARGV) {

	open(FILE, $fname) || die("Could not open $fname\n");

	# read/print first line, make sure it's always the same
	my $thisHeader = <FILE>;
	if (not defined $firstHeader) {
		$firstHeader = $thisHeader;
		print $firstHeader;
	} elsif ($firstHeader ne $thisHeader) {
		print STDERR "$fname: differing header: \"".$thisHeader."\"\n";
	}

	# print out the rest, counting lines
	my $thisCount = 0;
	while(my $line = <FILE>) {
		print $line;
		$thisCount++;
	}

	# make sure line count is always the same
	if (not defined $firstCount) {
		$firstCount = $thisCount;
	} elsif ($firstCount ne $thisCount) {
		print STDERR "$fname: differing line count: $thisCount\n";
	}

	close(FILE);
}

