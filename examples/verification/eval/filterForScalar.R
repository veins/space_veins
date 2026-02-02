require(data.table)
require(dplyr)
require(tidyverse)

print(sessionInfo())

args = commandArgs(trailingOnly=TRUE)

data <- fread(args[1])

itervars <- c("filter", "gcp", "geo", "hour", "leo", "minEle", "mobilityInterval", "simTimeLimit", "txInterval", "warmupPeriod", "wct")
scalars <- c("ageOfInformation_last")
columns <- c("node", itervars, scalars)
data <- data[, ..columns]

# all node vehicle statistics
pattern <- "ManhattanBuildingShadowing\\.node\\[\\d+\\]\\.vehicleStatistics"
data <- data[node %like% pattern]
print(data)