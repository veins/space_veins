experiments <- list("RtpEnUpPosOnlyAboveHorizon", "RtpDisUpPosOnlyAboveHorizon")
getCurrentPositionBehavior <- list("NGCP", "WGCP", "WLIP", "WNPP")
# precalculationIntervals <- list("0010", "0020", "0030", "0040", "0050", "0060", "0120", "0180", "0240", "0300", "0360")
precalculationIntervals <- list("0010", "0020", "0030")


configNames <- list()

for (e in experiments) {
    for (gcpb in getCurrentPositionBehavior) {
        for (p in precalculationIntervals) {
            configName <- paste0(paste0(e, gcpb), p)
            configNames <- append(configNames, configName)
            config <- paste0(paste0("[Config ", configName), "]\nextends = RunTimerPerformance, ")
            if (e == experiments[1]) {
                config <- paste0(config, "EnableUpdatePositionOnlyAboveHorizon, ")
            } else {
                config <- paste0(config, "DisableUpdatePositionOnlyAboveHorizon, ")
            }
            if (gcpb == "NGCP") {
                config <- paste0(config, "NoGetCurrentPositionUpdates\n")
            }
            if (gcpb == "WGCP") {
                config <- paste0(config, "WithGetCurrentPositionUpdates\n")
            }
            if (gcpb == "WLIP") {
                config <- paste0(config, "WithLinearInterpolationPositionUpdates\n")
            }
            if (gcpb == "WNPP") {
                config <- paste0(config, "WithNearestPeriodicPositionUpdates\n")
            }
            config <- paste0(paste0(paste0(config, "*.leo*[*].mobility.precalculationInterval_s = "), p), "s\n\n")
            cat(config)
        }
    }
}
cat(paste(configNames, collapse = " "), "\n")