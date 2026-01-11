experiments <- list("DemoEnUpPosOnlyAboveHorizon", "DemoDisUpPosOnlyAboveHorizon")
getCurrentPositionBehavior <- list("NGCP", "WGCP", "WLIP", "WNPP")
# precalculationIntervals <- list("0010", "0020", "0030", "0040", "0050", "0060", "0120", "0180", "0240", "0300", "0360")
# precalculationIntervals <- list("0010", "0020", "0030")
# updateInterval <- list("0.5", "1.0", "10.0")
updateInterval <- list("1.0")


configNames <- list()

for (e in experiments) {
    for (gcpb in getCurrentPositionBehavior) {
        for (u in updateInterval) {
            updateIntervalName <- gsub("\\.", "c", u)
            configName <- paste0(paste0(paste0(e, gcpb), "_"), updateIntervalName)
            configNames <- append(configNames, configName)
            config <- paste0(paste0("[Config ", configName), "]\n")
            extends <- paste0("extends = ", paste0(e,gcpb))
            config <- paste0(config,paste0(extends, "\n"))
            config <- paste0(paste0(paste0(config, "*.leo*[*].mobility.updateInterval = "), u), "s\n\n")
            cat(config)
        }
    }
}
cat(paste(configNames, collapse = " "), "\n")