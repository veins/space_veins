require(data.table)
require(dplyr)
require(tidyverse)

print(sessionInfo())

args = commandArgs(trailingOnly=TRUE)

data <- fread(args[1])

itervars <- c("gcp", "geo", "hour", "leo", "minEle", "mobilityInterval", "simTimeLimit", "txInterval", "warmupPeriod", "wct")
scalars <- c("packetReceptionHash_last")
columns <- c("node", itervars, scalars)
data <- data[, ..columns]

# all node vehicle statistics
# pattern <- "ManhattanBuildingShadowing\\.node\\[\\d+\\]\\.vehicleStatistics"
# all node vehicle statistics
# pattern <- "ManhattanBuildingShadowing\\.node\\[\\d+\\]\\.satNic\\[0\\]\\.radio\\.receiver"
# global statistics
pattern <- "ManhattanBuildingShadowing\\.globalStatistics"
data <- data[node %like% pattern]
filename <- paste(scalars, collapse = "_")
path <- paste0("archive/latest/csv/scalars/", filename, ".csv")
fwrite(data, path)