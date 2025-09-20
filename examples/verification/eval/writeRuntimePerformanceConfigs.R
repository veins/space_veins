experiments <- list("RtpEnUpPosOnlyAboveHorizon", "RtpDisUpPosOnlyAboveHorizon")
getCurrentPositionBehavior <- list("NGCP", "WGCP", "WLIP", "WNPP")
precalculationIntervals <- list("0360", "0720", "1080", "1440", "1800", "2160", "2520", "2880", "3240", "3600")

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