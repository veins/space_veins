#!/usr/bin/perl
use strict;
use warnings;

# get command line args
my $filename = shift @ARGV or die "Usage: $0 <input_file>\n";

# open file
open(my $fh, '<', $filename) or die "Cannot open file '$filename': $!\n";

# data structures
my @stack = ();
my %results = ();
my $lineNumber = 0;

# print environment info (similar to sessionInfo)
print "Perl version: $]\n";
print "Arguments: @ARGV\n";

while (my $line = <$fh>) {
    chomp $line;
    $lineNumber++;
    print "Begin line number: $lineNumber -----------------------------------------\n";
    print "Call stack at the beginning of processing this line:\n";
    my $stack_size = scalar @stack;
    print "Stack size: $stack_size\n";

    my @lineSplits = split(/:/, $line);
    next unless @lineSplits >= 3;

    my $perfEvent = $lineSplits[1] . "-" . $lineSplits[2];
    $perfEvent =~ s/^\s+//;  # trim left whitespace
    print "perfEvent: $perfEvent\n";

    my @timeSplits = split(/\s+/, $lineSplits[0]);
    my $timestamp = $timeSplits[-1];
    $timestamp += 0; # numeric
    print "timestamp: $timestamp\n";

    my $isReturnCall = substr($perfEvent, -8);
    my ($perfEventOnStack, $timestampOnStack) = ("", undef);

    if (@stack) {
        ($perfEventOnStack, $timestampOnStack) = @{$stack[0]};
    }

    if ($isReturnCall eq "__return") {
        # return event
        if ($perfEventOnStack . "__return" eq $perfEvent) {
            my $runtime = $timestamp - $timestampOnStack;
            printf("%s, runtime: %.20f\n", $perfEventOnStack, $runtime);

            # update results
            if (exists $results{$perfEventOnStack}) {
                $results{$perfEventOnStack} += $runtime;
            } else {
                $results{$perfEventOnStack} = $runtime;
            }

            # remove event from stack
            shift @stack;
        } else {
            die sprintf("Error in line %d: wrong event on stack for return event. On stack: %s <--> current line: %s\n",
                        $lineNumber, $perfEventOnStack, $perfEvent);
        }
    } else {
        # new call event
        if ($perfEventOnStack eq $perfEvent) {
            die sprintf("Error in line %d: two similar calls: %s\n", $lineNumber, $perfEvent);
        } else {
            unshift @stack, [$perfEvent, $timestamp];
        }
    }

    print "Call stack at the end of processing this line (size: " . scalar(@stack) . ")\n";
    print "end line number: $lineNumber -----------------------------------------\n";
}

# write results to CSV
my $out_file = $filename . ".perl.results.csv";
open(my $out, '>', $out_file) or die "Cannot open output file: $!\n";
print $out "call,accumulatedRunTime\n";
foreach my $call (keys %results) {
    print $out "$call,$results{$call}\n";
}
close($out);

close($fh);
print "Results written to $out_file\n";
