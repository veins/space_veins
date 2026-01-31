#!/usr/bin/perl
use strict;
use warnings;

# Get all CSV files from command line
my @files = @ARGV;
die "Usage: $0 file1.csv file2.csv ...\n" unless @files;

open my $out, '>', 'archive/latest/csv/gnuTimeResults.csv' or die "Cannot create archive/latest/csv/gnuTimeResults.csv: $!\n";

# Write header from first file
open my $first, '<', $files[0] or die "Cannot open $files[0]: $!\n";
my $header = <$first>;
print $out $header;
close $first;

# Append data rows (skip header) from all files
foreach my $file (@files) {
    print "Reading data from: $file\n";
    open my $in, '<', $file or warn "Cannot open $file: $!\n";
    my $skip_header = <$in>;  # Skip first line
    while (my $line = <$in>) {
        print $out $line;
    }
    close $in;
}

print "Combined ", scalar(@files), " files into archive/latest/csv/gnuTimeResults.csv\n";
