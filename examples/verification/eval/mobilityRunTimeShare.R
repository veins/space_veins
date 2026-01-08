require(data.table)
require(dplyr)
require(tidyverse)

print(sessionInfo())

args = commandArgs(trailingOnly=TRUE)

# FULL DEBUG VERSION of create_complete_perf_table
create_complete_perf_table <- function(filenames) {
  cat("=== DEBUG: Starting create_complete_perf_table ===\n")
  cat("Filenames (", length(filenames), "):", paste(filenames, collapse = ", "), "\n\n")

  # Step 1: Create base table with metadata
  cat("=== STEP 1: Extracting perf parts ===\n")
  perf_parts <- lapply(filenames, extract_perf_parts)
  cat("perf_parts length:", length(perf_parts), "\n")
  print(perf_parts)

  perf_dt <- data.table(
    filename = filenames,
    precalculation = sapply(perf_parts, function(x) ifelse(is.null(x$part1), NA, x$part1)),
    gcpBehavior = sapply(perf_parts, function(x) ifelse(is.null(x$part2), NA, x$part2)),
    repetition = sapply(perf_parts, function(x) ifelse(is.null(x$number), NA, x$number))
  )
  cat("\nBase perf_dt created:\n")
  print(perf_dt)

  # Step 2: Read ALL file contents
  cat("\n=== STEP 2: Reading file contents ===\n")
  perf_data <- lapply(filenames, read_perf_csv)
  cat("perf_data length:", length(perf_data), "\n")
  for(i in 1:min(3, length(perf_data))) {
    cat("perf_data[[", i, "]] names:", paste(names(perf_data[[i]]), collapse = ", "), "\n")
  }

  # Step 3: Initialize category columns
  cat("\n=== STEP 3: Initializing columns ===\n")
  categories <- c("hsm", "gcp", "main", "init")
  for (cat in categories) {
    perf_dt[, (cat) := as.numeric(NA)]
    cat("Initialized column:", cat, "\n")
  }

  # Step 4: Fill categorized values
  cat("\n=== STEP 4: Filling category values ===\n")
  for (i in 1:nrow(perf_dt)) {
    cat("\n--- Processing row", i, "---\n")
    file_data <- perf_data[[i]]
    cat("File", i, "has", length(file_data), "probes\n")

    for (probe_name in names(file_data)) {
      category <- get_probe_category(probe_name)
      if (!is.na(category)) {
        value <- file_data[probe_name]
        perf_dt[i, (category) := value]
        cat("  Probe:", probe_name, "->", category, "=", round(value, 3), "\n")
      } else {
        cat("  Probe:", probe_name, "-> NO category match\n")
      }
    }
  }

  cat("\n=== FINAL RESULT ===\n")
  print(perf_dt)

  # Sort final result
  setorder(perf_dt, precalculation, gcpBehavior, repetition)
  cat("\n=== SORTED RESULT ===\n")
  print(perf_dt)

  # Step 6: VALIDATION on SORTED final results
  cat("\n=== STEP 6: FINAL VALIDATION (SORTED) ===\n")
  for (i in 1:nrow(perf_dt)) {
    filename <- perf_dt[i, filename]
    expected_precalc <- perf_dt[i, precalculation]
    expected_gcp <- perf_dt[i, gcpBehavior]
    expected_rep <- perf_dt[i, repetition]

    # Re-read file content for this specific row
    file_data <- read_perf_csv(filename)

    # Metadata validation
    actual <- extract_perf_parts(filename)
    stopifnot(expected_precalc == actual$part1,
              expected_gcp == actual$part2,
              expected_rep == actual$number)

    # Numerical validation
    hsm_val <- file_data[grepl("hsm", names(file_data), fixed=TRUE)][1]
    main_val <- file_data[grepl("main", names(file_data), fixed=TRUE)][1]
    init_val <- file_data[grepl("init", names(file_data), fixed=TRUE)][1]
    gcp_val <- file_data[grepl("gcp_dbg", names(file_data), fixed=TRUE)][1]

    if (!is.na(perf_dt[i, hsm]))  stopifnot(abs(perf_dt[i, hsm] - hsm_val) < 1e-6)
    if (!is.na(perf_dt[i, gcp]))  stopifnot(abs(perf_dt[i, gcp] - gcp_val) < 1e-6)
    if (!is.na(perf_dt[i, main])) stopifnot(abs(perf_dt[i, main] - main_val) < 1e-6)
    if (!is.na(perf_dt[i, init])) stopifnot(abs(perf_dt[i, init] - init_val) < 1e-6)

    cat(sprintf("Row %d: ✓ ALL VALIDATED\n", i))
  }

  cat("\n=== VALIDATION COMPLETE ===\n")

  return(perf_dt)
}


# Helper functions (unchanged)
get_probe_category <- function(probe_name) {
  if (grepl("hsm", probe_name, fixed = TRUE)) return("hsm")
  if (grepl("main", probe_name, fixed = TRUE)) return("main")
  if (grepl("init", probe_name, fixed = TRUE)) return("init")
  if (grepl("gcp_dbg", probe_name, fixed = TRUE)) return("gcp")
  return(NA_character_)
}

read_perf_csv <- function(filepath) {
  dt <- fread(filepath, sep = ",", header = TRUE, fill = TRUE)
  values <- as.numeric(dt[[2]])
  names(values) <- dt[[1]]
  return(values)
}

extract_perf_parts <- function(filename) {
  # Extract basename first (ignore path)
  basename_only <- basename(filename)

  # Pattern: Verify(CAPTURE1)Up.*Horizon(CAPTURE2)_(NUMBER)\.perf
  pattern <- "Verify([^U]+)Up[^H]*Horizon([^_]+)_(\\d+)\\.perf"

  matches <- regexec(pattern, basename_only, perl = TRUE)
  extracted <- regmatches(basename_only, matches)

  if (length(extracted[[1]]) < 4) {
    return(list(part1 = NA_character_, part2 = NA_character_, number = NA_integer_))
  }

  return(list(
    part1 = trimws(extracted[[1]][2]),      # Verify...Up → Dis
    part2 = trimws(extracted[[1]][3]),      # Horizon..._ → WLIP
    number = as.integer(trimws(extracted[[1]][4]))  # _NUMBER.perf → 1
  ))
}

perf_dt <- create_complete_perf_table(args)

# calculate share in percent
perf_dt[, `:=`(
  hsm  = fifelse(!is.na(hsm),  hsm,  NA_real_),
  gcp  = fifelse(!is.na(gcp),  gcp,  NA_real_),
  init = fifelse(!is.na(init), init, NA_real_),
  share = 100 * (hsm + gcp + init) / main
)]

print(perf_dt)
fwrite(perf_dt, "plots/mobilityRunTimeShare.csv")