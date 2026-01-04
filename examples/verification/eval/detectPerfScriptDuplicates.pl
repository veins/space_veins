#!/usr/bin/perl

# deduplicate_file.pl - Creates unique file + removed duplicates file
# Usage: ./deduplicate_file.pl input.txt
# Output: input.unique.perf  (first occurrence of each line)
#         input.removed.perf (all duplicate occurrences that were removed)

use strict;
use warnings;

if (@ARGV != 1) {
    die "Usage: $0 <input_file>\n";
}

my $input_file = $ARGV[0];
my ($base_name) = $input_file =~ /^(.*)\.[^.]*$/;
$base_name //= $input_file;

my $unique_file = "$base_name.unique.perl.perf";
my $removed_file = "$base_name.removed.perl.perf";

die "Error: File '$input_file' not found.\n" unless -f $input_file;

print "Processing '$input_file'...\n";
print "- Unique lines → '$unique_file'\n";
print "- Removed duplicates → '$removed_file'\n";

open my $IN, '<', $input_file or die "Cannot open $input_file: $!";
open my $UNIQUE, '>', $unique_file or die "Cannot open $unique_file: $!";
open my $REMOVED, '>', $removed_file or die "Cannot open $removed_file: $!";

my %seen;
my ($unique_count, $removed_count) = (0, 0);

while (my $line = <$IN>) {
    chomp $line;
    if (exists $seen{$line}) {
        print $REMOVED $line, "\n";
        $removed_count++;
    } else {
        $seen{$line} = 1;
        print $UNIQUE $line, "\n";
        $unique_count++;
    }
}

close $IN;
close $UNIQUE;
close $REMOVED;

my $total_count = $unique_count + $removed_count;

print "Done!\n";
print "Unique lines: $unique_count\n";
print "Removed lines: $removed_count (total duplicates)\n";
print "Original total: $total_count\n";
