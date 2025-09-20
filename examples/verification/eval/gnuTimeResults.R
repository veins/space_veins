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
  precalculationInterval = numeric(),
  repetition = integer(),
  userTime = numeric(),
  kernelTime = numeric(),
  wallClockTime = numeric()
)

files <- list.files(path = args[1], full.names = TRUE)

for (file in files) {
    # Open a connection to the file for reading
    con <- file(file, open = "r")
    print(file)

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
        if (configType == "Rtp") {
            # ToDo: Extract the precalculationInterval from the configs name
            dataRow <- list(config = cmdSplits[[1]][5],
                            updateAboveHorizon = substr(configName, start = 4, stop = 5) == "En",
                            getCurrentPositionBehavior = substr(configName, nchar(configName) - 8 + 1, nchar(configName) - 5 + 1),
                            precalculationInterval = as.numeric(substr(configName, nchar(configName) - 4 + 1, nchar(configName))),
                            repetition = cmdSplits[[1]][7],
                            userTime = lineSplits[[1]][2],
                            kernelTime = lineSplits[[1]][3],
                            wallClockTime = lineSplits[[1]][4]
            )
            rawData <- rbind(rawData, dataRow)
        }
    }

    # Close the connection
    close(con)
}

fwrite (rawData, paste0(args[1], "/../gnuTimeResults.csv"))