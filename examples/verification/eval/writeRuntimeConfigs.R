experiments <- list("RuntimeEnUpPosOnlyAboveHorizon", "RuntimeDisUpPosOnlyAboveHorizon")
getCurrentPositionBehavior <- list("NGCP", "WGCP", "WLIP", "WNPP")
updateInterval <- list("1.0", "10.0", "30.0", "60.0")


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
            config <- paste0(paste0(paste0(config, "*.leo*[*].mobility.updateInterval = ${updateInterval="), u), "}s\n\n")
            cat(config)
        }
    }
}
cat(paste(configNames, collapse = " "), "\n")