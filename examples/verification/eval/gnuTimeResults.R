require(data.table)
require(dplyr)
require(tidyverse)

print(sessionInfo())
args = commandArgs(trailingOnly=TRUE)
print(args)

rawData <- DT <- data.table(
  config = character(),
  updateAboveHorizon = logical(),
  getCurrentPositionBehavior = character(),
  updateInterval = numeric(),
  repetition = integer(),
  userTime = numeric(),
  kernelTime = numeric(),
  wallClockTime = numeric()
)

convert_c_to_float <- function(str) {
  # Split string at 'c', convert parts to numeric, treat 'c' as decimal point
  parts <- strsplit(str, "c")[[1]]
  before <- as.numeric(parts[1])
  after <- as.numeric(parts[2])
  
  # Reconstruct as float: before + after shifted by decimal position
  decimal_places <- nchar(parts[2])
  result <- before + after / (10^decimal_places)
  
  return(result)
}

files <- list.files(path = args[1], full.names = TRUE)

for (filename in files) {
    # Open a connection to the file for reading
    con <- file(filename, open = "r")
    print(filename)

    # 1. Characters between "Demo" and "Up" (only after GnuTimeResults_Demo)
    precalculation <- sub(".*Demo(.*?)Up.*", "\\1", filename)  # "Dis"

    # 2. Characters between "Horizon" and first subsequent "_" 
    gcpBehavior <- sub(".*Horizon(.*?)_.*", "\\1", filename)  # "NGCP"

    # 3. Characters between 3rd and 4th "_" (the FINAL two before .csv)
    underscores <- gregexpr("_", filename)[[1]]
    updateIntervalString <- substr(filename, underscores[2] + 1, underscores[3] - 1)  # "1c0"
    updateInterval <- convert_c_to_float(updateIntervalString)

    # 4. Characters between 2nd-last "_" and first "." 
    dots <- gregexpr("\\.", filename)[[1]]
    repetition <- substr(filename, underscores[length(underscores)] + 1, dots[1] - 1)  # "0"

    # Read and process lines one by one
    repeat {
        line <- readLines(con, n = 1)
        if (identical(line, character(0))) {
            # End of file reached
            break
        }
        lineSplits <- strsplit(line, split = ",")
        cmdSplits <- strsplit(lineSplits[[1]][1], split = " ")
        print(line)
        # extract the first 3 characters of the configs name. Only parse files whose config starts with "Rtp"
        configName <- cmdSplits[[1]][5]
        print(configName)
        configType <- substr(configName, start = 1, stop = 3)
        print(configType)
        # ToDo: Extract the precalculationInterval from the configs name
        dataRow <- list(config = cmdSplits[[1]][5],
                        updateAboveHorizon = precalculation,
                        getCurrentPositionBehavior = gcpBehavior,
                        updateInterval = updateInterval,
                        repetition = repetition,
                        userTime = lineSplits[[1]][2],
                        kernelTime = lineSplits[[1]][3],
                        wallClockTime = lineSplits[[1]][4]
        )
        rawData <- rbind(rawData, dataRow)
    }

    # Close the connection
    close(con)
}

fwrite (rawData, paste0(args[1], "/../gnuTimeResults.csv"))