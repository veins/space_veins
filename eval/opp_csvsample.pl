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
# opp_csvsample.pl - Generates one-out-of-n samples from .csv files
#

use strict;
use warnings;
use File::Basename;
use Getopt::Long;

my $parN = 0;
my $parC = 0;
GetOptions (
	"oneoutof|n:+" => \$parN,
	"count|c:+" => \$parC,
);

if ((@ARGV < 1) or (($parN <= 0) and ($parC <= 0)) or (($parN > 0) and ($parC > 0))) {
	print STDERR "usage: opp_csvsample.pl [OPTIONS] <file.csv> [<file.csv> ...]\n";
	print STDERR "\n";
	print STDERR "  -n --oneoutof:     Sample size is one out of n\n";
	print STDERR "                     - sample count is random\n";
	print STDERR "                     - order is preserved\n";
	print STDERR "                     - no duplicate lines will be returned\n";
	print STDERR "                     - file is read sequentially\n";
	print STDERR "                     - file is read exactly once\n";
	print STDERR "                     - distribution is uniform\n";
	print STDERR "                     - at least one sample is returned\n";
	print STDERR "  -c --count:        Sample size is c\n";
	print STDERR "                     - sample count is precise\n";
	print STDERR "                     - order is random\n";
	print STDERR "                     - duplicate lines might be returned\n";
	print STDERR "                     - file is accessed in random order\n";
	print STDERR "                     - distribution is uniform only\n";
	print STDERR "                       if line lengths are equal\n";
	print STDERR "\n";
	print STDERR "e.g.: opp_csvsample.pl -n 100 my.csv\n";
	print STDERR "      will write a 1:100 sample of my.csv to sampled-my.csv\n";
	exit;
}

srand();

foreach my $fname (@ARGV) {

	my($basename, $path, $suffix) = fileparse($fname, ('.csv', '.tsv', '.txt'));
	my $ofname = $path.'sampled-'.$basename.$suffix;

	if ($parN > 0) {
		print STDERR "$fname ===[1:$parN]==> $ofname\n";

		open(FILE, $fname) || die("Could not open $fname: $!\n");
		open(OFILE, '>', $ofname) || die("Could not open $ofname: $!\n");
		my $header = readline(FILE);
		if (not defined($header)) {
			die("Could not read header\n");
		}
		print OFILE $header;

		my $readcount = 0;
		my $wrotecount = 0;
		my $line = "";
		while($line = <FILE>) {
			$readcount++;
			next unless int(rand($parN)) == 0;
			print OFILE $line;
			$wrotecount++;
		}
		if (($readcount >= 1) and ($wrotecount < 1)) {
			print OFILE $line;
		}

		close(OFILE);
		close(FILE);
	}
	else {
		print STDERR "$fname ===[$parC]==> $ofname\n";

		open(FILE, $fname) || die("Could not open $fname: $!\n");
		open(OFILE, '>', $ofname) || die("Could not open $ofname: $!\n");
		my $header = readline(FILE);
		if (not defined($header)) {
			die("Could not read header\n");
		}
		print OFILE $header;

		my $bytecount=-s $fname;
		for (my $i = 0; $i < $parC; $i++) {
			seek(FILE, int(rand($bytecount)), 0);
			readline(FILE); # skip partial line (or header)
			my $line = readline(FILE);
			if (not defined($line)) {
				# we hit the last line
				redo;
			}
			print OFILE $line;
		}

		close(OFILE);
		close(FILE);
	}


}
