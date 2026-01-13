require(data.table)
require(dplyr)
require(tidyverse)

print(sessionInfo())

args = commandArgs(trailingOnly=TRUE)

print(args)

create_complete_data_table <- function(filenames) {
  cat("=== DEBUG: Starting create_complete_data_table ===\n")
  cat("Filenames (", length(filenames), "):", paste(filenames, collapse = ", "), "\n\n")

  cat("\n=== STEP 1: Reading file contents ===\n")
  aoi_data <- lapply(filenames, read_aoi_file)
  aoi_data <- rbindlist(aoi_data)
  print(aoi_data)
  return(aoi_data)
}

read_aoi_file <- function(filename) {
    meta_data <- extract_meta_data(filename)
    print(meta_data)
    aoi_data <- fread(filename)
    # remove empty column for aoi
    aoi_data <- aoi_data[, !apply(aoi_data, 2, function(x) all(is.na(x))), with = FALSE]
    aoi_data$precalculation <- meta_data$part1
    aoi_data$gcpBehavior <- meta_data$part2
    print(aoi_data)
    return(aoi_data)
}

# helper functions
extract_meta_data <- function(filename) {
  # Extract basename first (ignore path)
  basename_only <- basename(filename)

  pattern <- ""
  if (grepl("Verify", filename)) {
    pattern <- "Verify([^U]+)Up[^H]*Horizon([^_]+)-ageOfInformation\\.csv"
    isVerificationExperiment <- TRUE
  } else {
    pattern <- "Runtime([^U]+)Up[^H]*Horizon([^_]+)_\\d+c\\d+-ageOfInformation\\.csv"
    isVerificationExperiment <- FALSE
  }

  matches <- regexec(pattern, basename_only, perl = TRUE)
  extracted <- regmatches(basename_only, matches)

  if (length(extracted[[1]]) < 3) {
    return(list(part1 = NA_character_, part2 = NA_character_))
  }

  return(list(
    part1 = trimws(extracted[[1]][2]),      # Verify...Up → Dis
    part2 = trimws(extracted[[1]][3])       # Horizon..._ → WLIP
  ))
}

isVerificationExperiment <- FALSE
aoi_data <- create_complete_data_table(args)
if (isVerificationExperiment) {
  fwrite(aoi_data, "plots/ageOfInformation.csv")
} else {
  fwrite(aoi_data, "archive/latest/plots/ageOfInformation.csv")
}