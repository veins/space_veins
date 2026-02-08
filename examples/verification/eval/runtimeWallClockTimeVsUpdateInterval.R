require(data.table)
require(dplyr)
require(tidyverse)

print(sessionInfo())

args = commandArgs(trailingOnly=TRUE)

data <- fread(args[1])

plottedData <- data[, c("mobilityInterval", "gcp", "wallClockTime")]
plottedData <- plottedData[!is.na(wallClockTime), 
                        .(avg = mean(wallClockTime, na.rm = TRUE),
                          sd = sd(wallClockTime, na.rm = TRUE)), 
                        by = .(gcp, mobilityInterval)]
plottedData <- plottedData[order(mobilityInterval)]
print(plottedData)

fwrite(plottedData, "archive/latest/plots/runtimeWallClockTimeVsUpdateInterval.pdf.csv")
# # ----------- plot

myps <- function(file, paper="special", width=6, height=4, family="Times") {
    postscript(file = paste(file, ".eps", sep=""), horizontal=F, onefile = FALSE, paper=paper, width=width, height=height, family=family)
    par(mar=c(2.1, 3.1, 0.1, 0.1))
    done <- function() {
        graphics.off()
        system(paste("ps2pdf -dPDFSETTINGS=/prepress -dEPSCrop", paste(file, ".eps", sep=""), paste(file, ".pdf", sep="")))
    }
    return(done)
}

paper.width <- 308.43/72*1.22274*0.5*2
paper.height <- paper.width * .4*1.2

minX <- 0.01
# maxX <- max(max(unfiltered$updateInterval), max(filtered$updateInterval))
maxX <- max(plottedData$mobilityInterval)
minY <- 0
maxY <- 2600

# ----------- unfiltered
done <- myps(file = "archive/latest/plots/runtimeWallClockTimeVsUpdateInterval", width=paper.width, height=paper.height)
plot.new()

par(mar=c(3.0,3.2,1,1.0), mgp=c(3.0, 0.5, 0))
plot.window(xlim=c(minX, maxX),
            ylim=c(minY, maxY),
            xaxs="i",
            yaxs="i",
            log="x")

unique_behaviors <- unique(plottedData$gcp)

i <- 1  # Index counter
leg.txt <- c()
for(behavior in unique_behaviors) {
  subset_dt <- plottedData[gcp == behavior]
  lines(subset_dt$mobilityInterval, subset_dt$avg, lty=i)
  points(subset_dt$mobilityInterval, subset_dt$avg, pch=i)
  arrows(x0=subset_dt$mobilityInterval, y0=subset_dt$avg - subset_dt$sd, x1=subset_dt$mobilityInterval, y1=subset_dt$avg + subset_dt$sd, code=3, angle=90, length=0.05, lwd=1.0, col="darkgrey")
  i <- i + 1  # Increment index
  leg.txt <- append(leg.txt, behavior)
}

leg.txt <-gsub("SGP4MobilityNoGetCurrentPositionUpdates", "baseline", leg.txt)
leg.txt <-gsub("SGP4MobilityWithGetCurrentPositionUpdates", "precise", leg.txt)
leg.txt <-gsub("SGP4MobilityWithLinearInterpolationUpdates", "lin. interp.", leg.txt)
leg.txt <-gsub("SGP4MobilityWithNearestPeriodicPositionUpdates", "nearest neighbor", leg.txt)

# Add legend using index-based colors
legend("bottomright",
      #  legend=c("baseline", "precise", "lin. interp.", "nearest neighbor"),
       legend=leg.txt,
       lty=1:length(unique_behaviors),
       pch=1:length(unique_behaviors),
       ncol=2,
       bty="n"
       )
box()
# xticks <- seq(minX, maxX, length.out = 7)
xticks <- c(0.01, 0.1, 1, 10)
xlabels <- sprintf("%.2f", xticks)
axis(1, at = xticks, labels = xlabels)
# axis(2, las=1, tck=-0.04)
axis(2, las=1, tck=-0.04)
title(xlab="periodic update interval in s", line=1.8)
title(ylab="wall clock run time in s", line=2.2)

done()





# unfiltered <- data[updateAboveHorizon == "Dis"]
# unfiltered <- unfiltered[!is.na(userTime), 
#                         .(avg = mean(userTime, na.rm = TRUE),
#                           sd = sd(userTime, na.rm = TRUE)), 
#                         by = .(getCurrentPositionBehavior, updateInterval)]
# unfiltered <- unfiltered[order(updateInterval)]
# # print(unfiltered)
# filtered <- data[updateAboveHorizon == "En"]
# filtered <- filtered[!is.na(userTime), 
#                         .(avg = mean(userTime, na.rm = TRUE),
#                           sd = sd(userTime, na.rm = TRUE)), 
#                         by = .(getCurrentPositionBehavior, updateInterval)]
# filtered <- filtered[order(updateInterval)]
# # print(filtered)


# fwrite(unfiltered, "archive/latest/plots/runtimeUserTimeVsUpdateInterval.unfiltered.pdf.csv")
# fwrite(filtered, "archive/latest/plots/runtimeUserTimeVsUpdateInterval.filtered.pdf.csv")

# # ----------- plot

# myps <- function(file, paper="special", width=6, height=4, family="Times") {
#     postscript(file = paste(file, ".eps", sep=""), horizontal=F, onefile = FALSE, paper=paper, width=width, height=height, family=family)
#     par(mar=c(2.1, 3.1, 0.1, 0.1))
#     done <- function() {
#         graphics.off()
#         system(paste("ps2pdf -dPDFSETTINGS=/prepress -dEPSCrop", paste(file, ".eps", sep=""), paste(file, ".pdf", sep="")))
#     }
#     return(done)
# }

# paper.width <- 308.43/72*1.22274*0.5*2
# paper.height <- paper.width * .4*1.2

# minX <- 0.01
# # maxX <- max(max(unfiltered$updateInterval), max(filtered$updateInterval))
# maxX <- max(unfiltered$updateInterval)
# minY <- 0
# maxY <- max(max(unfiltered$avg), max(unfiltered$avg))
# print(maxY)
# maxY <- 600

# # ----------- unfiltered
# done <- myps(file = "archive/latest/plots/runtimeUserTimeVsUpdateInterval.unfiltered", width=paper.width, height=paper.height)
# plot.new()

# par(mar=c(3.0,3.0,1,1.0), mgp=c(3.0, 0.5, 0))
# plot.window(xlim=c(minX, maxX),
#             ylim=c(minY, maxY),
#             xaxs="i",
#             yaxs="i",
#             log="x")

# unique_behaviors <- unique(unfiltered$getCurrentPositionBehavior)

# i <- 1  # Index counter
# for(behavior in unique_behaviors) {
#   subset_dt <- unfiltered[getCurrentPositionBehavior == behavior]
#   lines(subset_dt$updateInterval, subset_dt$avg, lty=i)
#   points(subset_dt$updateInterval, subset_dt$avg, pch=i)
#   i <- i + 1  # Increment index
# }

# # Add legend using index-based colors
# legend("bottomright",
#        legend=c("baseline", "precise", "lin. interp.", "nearest neighbor"),
#        lty=1:length(unique_behaviors),
#        pch=1:length(unique_behaviors),
#        ncol=2,
#        bty="n"
#        )
# box()
# # xticks <- seq(minX, maxX, length.out = 7)
# xticks <- c(0.01, 0.1, 1, 10)
# xlabels <- sprintf("%.2f", xticks)
# axis(1, at = xticks, labels = xlabels)
# # axis(2, las=1, tck=-0.04)
# axis(2, las=1, tck=-0.04)
# title(xlab="periodic update interval in s", line=1.8)
# title(ylab="user space run time in s", line=2)

# done()


# # ----------- filtered -------------------------------------------------------------------------------------------------------
# done <- myps(file = "archive/latest/plots/runtimeUserTimeVsUpdateInterval.filtered", width=paper.width, height=paper.height)
# plot.new()

# par(mar=c(3.0,3.0,1,1.0), mgp=c(3.0, 0.5, 0))
# plot.window(xlim=c(minX, maxX),
#             ylim=c(minY, maxY),
#             xaxs="i",
#             yaxs="i",
#             log="x")

# unique_behaviors <- unique(filtered$getCurrentPositionBehavior)

# i <- 1  # Index counter
# for(behavior in unique_behaviors) {
#   subset_dt <- filtered[getCurrentPositionBehavior == behavior]
#   lines(subset_dt$updateInterval, subset_dt$avg, lty=i)
#   points(subset_dt$updateInterval, subset_dt$avg, pch=i)
#   i <- i + 1  # Increment index
# }

# # Add legend using index-based colors
# legend("bottomright",
#        legend=c("baseline", "precise", "lin. interp.", "nearest neighbor"),
#        lty=1:length(unique_behaviors),
#        pch=1:length(unique_behaviors),
#        ncol=2,
#        bty="n"
#        )
# box()
# # xticks <- seq(minX, maxX, length.out = 7)
# xticks <- c(0.01, 0.1, 1, 10)
# xlabels <- sprintf("%.2f", xticks)
# axis(1, at = xticks, labels = xlabels)
# # axis(2, las=1, tck=-0.04)
# axis(2, las=1, tck=-0.04)
# title(xlab="periodic update interval in s", line=1.8)
# title(ylab="user space run time in s", line=2)
# done()