#!/usr/bin/env perl
#
# Copyright (C) 2008-2021 Christoph Sommer <sommer@cms-labs.org>
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
# opp_sca2csv.pl -- converts OMNeT++ .sca files to csv format, collating values from multiple scalars into one column each
#
# (Refer to POD sections at end of file for documentation)
#
use strict;
use warnings;
use Getopt::Long qw(:config no_ignore_case bundling auto_version auto_help);
use Pod::Usage;

$main::VERSION = 4.10;

my $verbose = 0;
my %scalarNames = ();
my %attrNames = ();
my %paramNames = ();
my %itervarNames = ();
my %configNames = ();
my $moduleRegex = "";
my $noHeader = 0;
my $list = "";
my @listLines = ();
GetOptions (
	"filter|F:s" => \%scalarNames,
	"attr|A:s" => \%attrNames,
	"param|P:s" => \%paramNames,
	"itervar|I:s" => \%itervarNames,
	"config|C:s" => \%configNames,
	"module-regex|M:s" => \$moduleRegex,
	"verbose|v" => \$verbose,
	"no-header|H" => \$noHeader,
	"list|l:s" => \$list
) or pod2usage("$0: Bad command line options.\n");
pod2usage("$0: No scalar files given.\n") if (@ARGV < 1);


# get list of file names from command line
my @fileNames;
while (my $fileName = shift @ARGV) {
	push (@fileNames, $fileName);
}

# output CSV header
if ($noHeader) {
}
else {
	# print header
	print "node";
	foreach my $attrName (sort keys %attrNames) {
		my $value = $attrNames{$attrName};
		$value = $attrName unless (defined $value and $value ne "");
		print "\t".$value;
	}
	foreach my $itervarName (sort keys %itervarNames) {
		my $value = $itervarNames{$itervarName};
		$value = $itervarName unless (defined $value and $value ne "");
		print "\t".$value;
	}
	foreach my $configName (sort keys %configNames) {
		my $value = $configNames{$configName};
		$value = $configName unless (defined $value and $value ne "");
		print "\t".$value;
	}
	foreach my $paramName (sort keys %paramNames) {
		my $value = $paramNames{$paramName};
		$value = $paramName unless (defined $value and $value ne "");
		print "\t".$value;
	}
	foreach my $scalarName (sort keys %scalarNames) {
		my $value = $scalarNames{$scalarName};
		$value = $scalarName unless (defined $value and $value ne "");
		print "\t".$value;
	}
	print "\n";
}

# remember if we already warned about overwriting a value
my $warnedOverwriteValue = 0;

# iterate over scalar files
foreach my $fileName (@fileNames) {

	# this is where we store all the data
	my %events = ();

	print STDERR "reading \"".$fileName."\"...\n" if $verbose;

	my $FILE;
	open($FILE, $fileName) or die("Error opening file \"".$fileName."\"");

	my $fileSize = -s $fileName;

	print STDERR "reading file...\n" if $verbose;

	my $readingHeader = 1;

	# read file
	my %filterValues = ();
	my %attrValues = ();
	my %paramValues = ();
	my %itervarValues = ();
	my %configValues = ();
	while (<$FILE>) {
		my $lineNumber = $.;

		# print progress
		if ($verbose and ($. % 10000 == 0)) {
			print STDERR sprintf("%.1f", tell($FILE)/1024/1024)."M/".sprintf("%.1f", $fileSize/1024/1024)."M (".sprintf("%.1f", tell($FILE)/$fileSize*100)."%)\r";
		}

		# found scalar data
		if (m{
				^scalar
				\s+
				(("(?<nodname1>[^"]+)")|(?<nodname2>[^\s]+))
				\s+
				(("(?<scaname1>[^"]+)")|(?<scaname2>[^\s]+))
				\s+
				(?<value>.*)
				\r?\n$
				}x) {

			my $nod_name = defined($+{nodname1})?$+{nodname1}:"" . defined($+{nodname2})?$+{nodname2}:"";
			my $sca_name = defined($+{scaname1})?$+{scaname1}:"" . defined($+{scaname2})?$+{scaname2}:"";
			my $value = $+{value};

			if (defined($moduleRegex) and ($moduleRegex)) {
				next unless ($nod_name =~ $moduleRegex);
				if (defined($+{module})) {
					$nod_name = $+{module};
				}
			}

			if ($list and (index($list, "F") != -1)) {
				if (not exists($filterValues{$sca_name})) {
					$filterValues{$sca_name} = 1;
					push(@listLines, "filter\t$sca_name\n");
				}
			}

			# sca_name must have been given on command line
			next unless exists($scalarNames{$sca_name});

			my $key = $nod_name;
			if ($verbose and not $warnedOverwriteValue and defined $events{$key}{$sca_name}) {
				print STDERR "WARNING: value on \"".$fileName."\" line ".$lineNumber." overwrites existing value for \"".$key.".".$sca_name."\".\n";
				$warnedOverwriteValue = 1;
			}
			$events{$key}{$sca_name} = $value;

			next;
		}

		# found attr
		if ($readingHeader and m{
				^attr
				\s+
				(?<attr>[^ ]+)
				\s+
				(?<value>.+)
				\r?\n$
				}x) {
			$attrValues{$+{attr}} = $+{value};
			if ($list and (index($list, "A") != -1)) {
				push(@listLines, "attr\t$+{attr}\n");
			}
			next;
		}

		# found par (in body)
		if ((not $readingHeader) and m{
				^par
				\s+
				(("(?<nodname1>[^"]+)")|(?<nodname2>[^\s]+))
				\s+
				(("(?<scaname1>[^"]+)")|(?<scaname2>[^\s]+))
				\s+
				(?<value>.*)
				\r?\n$
				}x) {
			# ignore
			next;
		}

		# found statistic (in body)
		if ((not $readingHeader) and m{
				^statistic
				\s+
				(("(?<nodname1>[^"]+)")|(?<nodname2>[^\s]+))
				\s+
				(?<statname>.*)
				\r?\n$
				}x) {
			# ignore
			next;
		}

		# found field (in body)
		if ((not $readingHeader) and m{
				^field
				\s+
				(?<fieldname>.*)
				\s+
				(?<value>.*)
				\r?\n$
				}x) {
			# ignore
			next;
		}

		# found bin (in body)
		if ((not $readingHeader) and m{
				^bin
				\s+
				(?<position>.*)
				\s+
				(?<value>.*)
				\r?\n$
				}x) {
			# ignore
			next;
		}

		# found attr (in body)
		if ((not $readingHeader) and m{
				^attr
				\s+
				(?<attr>[^ ]+)
				\s+
				(?<value>.+)
				\r?\n$
				}x) {
			# ignore
			next;
		}

		# found itervar
		if ($readingHeader and m{
				^itervar
				\s+
				(?<itervar>[^ ]+)
				\s+
				(?<value>.+)
				\r?\n$
				}x) {
			$itervarValues{$+{itervar}} = $+{value};
			if ($list and (index($list, "I") != -1)) {
				push(@listLines, "itervar\t$+{itervar}\n");
			}
			next;
		}

		# found config
		if ($readingHeader and m{
				^config
				\s+
				(?<config>[^ ]+)
				\s+
				(?<value>.+)
				\r?\n$
				}x) {
			$configValues{$+{config}} = $+{value};
			if ($list and (index($list, "C") != -1)) {
				push(@listLines, "config\t$+{config}\n");
			}
			next;
		}

		# found param
		if ($readingHeader and m{
				^param
				\s+
				(?<param>[^ ]+)
				\s+
				(?<value>.+)
				\r?\n$
				}x) {
			$paramValues{$+{param}} = $+{value};
			if ($list and (index($list, "P") != -1)) {
				push(@listLines, "param\t$+{param}\n");
			}
			next;
		}

		# found run
		if ($readingHeader and m{
				^run
				\s+
				(?<run>.+)
				\r?\n$
				}x) {
			next;
		}

		# found version
		if ($readingHeader and m{
				^version
				\s+
				(?<version>[0-9.]+)
				\r?\n$
				}x) {
			next;
		}

		# found empty line
		if ($readingHeader and m{
				^
				\r?\n$
				}x) {
			$readingHeader = 0;
			next;
		}

		# found empty line (in body)
		if ((not $readingHeader) and m{
				^
				\r?\n$
				}x) {
			# ignore
			next;
		}

		print STDERR "\n\nUnknown line: $_\n\n" if $verbose;


	}

	close($FILE);

	print STDERR "done processing                             \n" if $verbose;

	# print body
	foreach my $key (sort keys %events) {
		my $node = $key;
		print $node;
		foreach my $attrName (sort keys %attrNames) {
			my $value = $attrValues{$attrName};
			print "\t".(defined $value ? $value : "");
		}
		foreach my $itervarName (sort keys %itervarNames) {
			my $value = $itervarValues{$itervarName};
			print "\t".(defined $value ? $value : "");
		}
		foreach my $configName (sort keys %configNames) {
			my $value = $configValues{$configName};
			print "\t".(defined $value ? $value : "");
		}
		foreach my $paramName (sort keys %paramNames) {
			my $value = $paramValues{$paramName};
			print "\t".(defined $value ? $value : "");
		}
		foreach my $scalarName (sort keys %scalarNames) {
			my $value = $events{$key}{$scalarName};
			print "\t".(defined $value ? $value : "");
		}
		print "\n";
	}

	print STDERR "done                                        \n" if $verbose;
}

my %listLinesUnique = map { $_, 1 } @listLines;
foreach my $line (sort (keys %listLinesUnique)) {
	print $line;
}

__END__

=head1 NAME

opp_sca2csv.pl -- converts OMNeT++ .sca files to csv format

=head1 SYNOPSIS

opp_sca2csv.pl [options] [file ...]

-F --filter <name>[=<alias>]

	add a column for scalar <name>, calling it <alias> (if provided)

-A --attr <name>[=<alias>]

	add a column for attribute <name>, calling it <alias> (if provided)

-P --param <name>[=<alias>]

	add a column for parameter <name>, calling it <alias> (if provided)

-I --itervar <name>[=<alias>]

	add a column for itervar <name>, calling it <alias> (if provided)

-C --config <name>[=<alias>]

	add a column for configuration value <name>, calling it <alias> (if provided)

-M --module-regex

	Regular expression to match module names against. If used
	with a named capture group (?<module>...), only this portion
	of the module name is considered
	[default: all]

-v --verbose

	log debug information to stderr

-l --list [FAPIC]+

	list values for -F, -A, -P, -I, -C

-H --no-header

	Do not print header line

--version

	Output version information

--help

	Output this information

e.g.: opp_sca2csv.pl -M '".'^scenario\.host\[(?<module>[0-9]+)\]'."' -F totalRcvd -F totalSent input.sca >output.csv

=cut

