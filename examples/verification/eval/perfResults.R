require(data.table)
require(dplyr)
require(tidyverse)

print(sessionInfo())
args = commandArgs(trailingOnly=TRUE)
print(args)

# print 10 digits when printing a data.table
options(digits = 20)

# Open a connection to the file for reading
con <- file(args[1], open = "r")

stack <- list()
lineNumber <- 0
perfEventOnStack <- ""
timestampOnStack <- -1

# create result data.table
results <- data.table(
    call = character(),
    accumulatedRunTime = numeric()
)

# Read and process lines one by one
repeat {
    lineNumber <- lineNumber + 1
    print(sprintf("Line number: %d -----------------------------------------", lineNumber))
    line <- readLines(con, n = 1)
    if (identical(line, character(0))) {
        # End of file reached
        break
    }
    lineSplits <- strsplit(line, split = ":")
    # print(lineSplits)
    perfEvent <- paste(lineSplits[[1]][2], lineSplits[[1]][3], sep = "-")
    perfEvent <- trimws(perfEvent, which = "left")
    # print(perfEvent)
    timeSplits <- strsplit(lineSplits[[1]][1], split = " ")
    timestamp <- tail(timeSplits[[1]], 1)[[1]]
    timestamp <- as.numeric(timestamp)
    # print(timestamp)
    isReturnCall <- substr(perfEvent, nchar(perfEvent) - 8 + 1, nchar(perfEvent))
    if (length(stack) > 0) {
        perfEventOnStack <- stack[[1]][[1]][1]
        timestampOnStack <- stack[[1]][[2]][1]
    }
    if (isReturnCall == "__return") {
        # current line is a return perf event
        # 1. check if the top of the stack is a suitable call: Is the perfEventOnStack + "__return" == perfEvent
        if (paste0(perfEventOnStack,isReturnCall) == perfEvent) {
            # There is a reasonable call on the stack
            # calculate current run time
            runtime <- timestamp - timestampOnStack
            print(sprintf("%s, runtime: %.20f", perfEventOnStack, runtime))
            # update results
            print("Results before the update")
            print(results)
            # check if call already exists in the results
            result_row_number <- which(results$call == perfEventOnStack)
            if (length(result_row_number) == 1) {
                # update accumulated run time
                set(results, i = result_row_number, j = "accumulatedRunTime", results[result_row_number, accumulatedRunTime] + runtime)
            }
            if (length(result_row_number) == 0) {
                # add row for this call
                results <- rbind(results, data.table(call = perfEventOnStack, accumulatedRunTime = runtime))
            }
            if (length(result_row_number) > 1) {
                print("Results until error:")
                print(results)
                errorMessage <- sprintf("Error in line %d: result row already exists: %s", lineNumber, perfEventOnStack)
                stop(errorMessage)
            }
            print("Results after the update")
            print(results)
            # Remove event from stack
            stack[[1]] <- NULL
        } else {
            print("Results until error:")
            print(results)
            errorMessage <- sprintf("Error in line %d: wrong event on the stack for return event: on stack: %s <--> current line: %s", lineNumber, perfEventOnStack, perfEvent)
            stop(errorMessage)
        }
    } else {
        # current line is not a return perf event
        # 1. check that not two similar call follow after each other. (this is not supposed to happen)
        if (perfEventOnStack == perfEvent) {
            print("Results until error:")
            print(results)
            errorMessage <- sprintf("Error in line %d: two similar calls: %s", lineNumber, perfEvent)
            stop(errorMessage)
        } else {
            # 2. put the call on the stack
            call <- list(perfEvent, timestamp)
            # print(call)
            stack <- append(stack, list(call), after = 0)
        }
    }
}
print(stack)

# Close the connection
close(con)