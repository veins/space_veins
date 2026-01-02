# This script checks that for every function call and its corresponding return call
# the same number occurences are in the perf data

require(data.table)
require(dplyr)
require(tidyverse)

print(sessionInfo())
args = commandArgs(trailingOnly=TRUE)
print(args)

# print 10 digits when printing a data.table
options(digits = 20)

# might be faster
perf <- read.table(args[1])
setDT(perf)
# count calls
counts <- perf[, .N, by = V3]
fwrite(counts, paste0(args[1],".counts.csv"))

compareOccurences <- function(numberOfCalls, comparingCall) {
    comparingCallResults <- counts[counts$V3 == comparingCall, ]
    print(comparingCallResults)
    if (nrow(comparingCallResults) == 1) {
        numberOfComparingCalls <- comparingCallResults[1, N]
        print(sprintf("numberOfComparingResults: %d", numberOfComparingCalls))
        if (numberOfComparingCalls != numberOfCalls) {
            print(sprintf("Error with comparingCall: %s; There are %d comparingCalls but %d numberOfCalls.", comparingCall, numberOfComparingCalls, numberOfCalls))
            # quit(status = 1)
        } else {
            print(sprintf("numberOfCalls == %d and numberOfComparingCalls == %d", numberOfCalls, numberOfComparingCalls))
        }
    } else {
        print(sprintf("Error with comparingCall: %s; There are %d result lines although only 1 is valid.", comparingCall, nrow(comparingCallResults)))
        # quit(status = 1)
    }
}

repeat {
    if (nrow(counts) >= 2) {
        print(counts)
        call <- counts[1, V3]
        print(call)
        numberOfCalls <- counts[1, N]
        print(sprintf("numberOfCalls: %d", numberOfCalls))
        isReturnCall <- substr(call, nchar(call) - 8 + 1, nchar(call))
        comparingCall <- ""
        if (isReturnCall == "__return") {
            # find the corresponding begin call
            comparingCall <- substr(call, 1, nchar(call) - 8)
            compareOccurences(numberOfCalls, comparingCall)
        } else {
            # find the corresponding return call
            comparingCall <- paste0(substr(call, 1, nchar(call) - 1), "__return:")
            compareOccurences(numberOfCalls, comparingCall)
        }
        # remove the first row and its correspondig one
        counts <- counts[-1, ]
        counts <- counts[counts$V3 != comparingCall, ]
    } else {
        if (nrow(counts) == 1) {
            print(sprintf("Error: there is a single row left in the counts data.table"))
            # quit(status = 1)
            break
        } else {
            break
        }
    }
}
# print(sprintf("perf validation successful."))
quit(status = 0)