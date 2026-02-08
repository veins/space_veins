require(data.table)
require(dplyr)
require(tidyverse)

print(sessionInfo())

args = commandArgs(trailingOnly=TRUE)
# print(args)

data <- fread(args[1])

data$totalPopulation <- data$fp + data$fn + data$tp + data$tn
data$accuracy <- (data$tp + data$tn) / (data$tp + data$tn + data$fp + data$fn)
data$precision <- (data$tp) / (data$tp + data$fp)
data$conditionPositive <- data$fn + data$tp
data$conditionNegative <- data$fp + data$tn
data$sensitivity <- data$tp / data$conditionPositive
data$falseNegativeRate <- data$fn / data$conditionPositive
data$falsePositiveRate <- data$fp / data$conditionNegative
data$positiveLikelihoodRateio <- data$sensitivity / data$falsePositiveRate
fwrite(data, "archive/latest/csv/classificationResults.data.csv")

# add filter here

plottedData <- data[, c("mobilityInterval", "gcp", "falseNegativeRate")]
plottedData <- plottedData[!is.na(falseNegativeRate), 
                        .(avg = mean(falseNegativeRate, na.rm = TRUE),
                          sd = sd(falseNegativeRate, na.rm = TRUE)), 
                        by = .(gcp, mobilityInterval)]
plottedData <- plottedData[order(mobilityInterval)]
fwrite(plottedData, "archive/latest/plots/runtimeFnrVsUpdateInterval.pdf.csv")

# ----------- plot

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
print(maxX)
minY <- 0.000001
maxY <- 1
print(maxY)

# ----------- unfiltered
done <- myps(file = "archive/latest/plots/runtimeFnrVsUpdateInterval", width=paper.width, height=paper.height)
plot.new()
par(mar=c(3.0,4.0,1,1.0), mgp=c(3.0, 0.5, 0))
plot.window(xlim=c(minX, maxX),
            ylim=c(minY, maxY) ,
            xaxs="i",
            yaxs="i",
            log="xy")

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
leg.txt <-gsub("SGP4MobilityWithLinearInterpolationUpdates", "lin. interp.", leg.txt)
leg.txt <-gsub("SGP4MobilityWithNearestPeriodicPositionUpdates", "nearest neighbor", leg.txt)

# Add legend using index-based colors
legend("topleft",
       legend=leg.txt,
       lty=1:length(unique_behaviors),
       pch=1:length(unique_behaviors),
       ncol=2,
       bty="n"
       )
box()
xticks <- c(0.01, 0.1, 1, 10)
xlabels <- sprintf("%.2f", xticks)
axis(1, at = xticks, labels = xlabels)
# axis(2, las=1, tck=-0.04)
# yticks <- seq(0, 1, length.out = 7)
# ylabels <- sprintf("%.1f", yticks)
# axis(2, las=1, tck=-0.04, at = yticks, labels = ylabels)
axis(2, las=1, tck=-0.04)
title(xlab="periodic update interval in s", line=1.8)
title(ylab="false negative rate (miss rate)", line=3)

done()

# filtered <- data[filter == "Dis"]
# filtered <- filtered[!is.na(falseNegativeRate), 
#                         .(avg = mean(falseNegativeRate, na.rm = TRUE),
#                           sd = sd(falseNegativeRate, na.rm = TRUE)), 
#                         by = .(gcpBehavior, updateInterval)]
# filtered <- filtered[order(updateInterval)]
# print(filtered)

# fwrite(unfiltered, "archive/latest/plots/runtimeFnrVsUpdateInterval.unfiltered.pdf.csv")
# fwrite(filtered, "archive/latest/plots/runtimeFnrVsUpdateInterval.filtered.pdf.csv")

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
# print(maxX)
# minY <- 0.000001
# maxY <- 1
# print(maxY)

# # ----------- unfiltered
# done <- myps(file = "archive/latest/plots/runtimeFnrVsUpdateInterval.unfiltered", width=paper.width, height=paper.height)
# plot.new()
# par(mar=c(3.0,4.0,1,1.0), mgp=c(3.0, 0.5, 0))
# plot.window(xlim=c(minX, maxX),
#             ylim=c(minY, maxY) ,
#             xaxs="i",
#             yaxs="i",
#             log="xy")

# unique_behaviors <- unique(unfiltered$gcpBehavior)

# i <- 1  # Index counter
# for(behavior in unique_behaviors) {
#   subset_dt <- unfiltered[gcpBehavior == behavior]
#   print(subset_dt)
#   # keep line type and point character similar to user timer vs update interval
#   if (i == 2) {
#     i <- i + 1
#   }
#   lines(subset_dt$updateInterval, subset_dt$avg, lty=i)
#   points(subset_dt$updateInterval, subset_dt$avg, pch=i)
#   i <- i + 1  # Increment index
# }

# # Add legend using index-based colors
# legend("topleft",
#        legend=c("baseline", "lin. interp.", "nearest neighbor"),
#        lty=c(1, 3, 4),
#        pch=c(1, 3, 4),
#        ncol=2,
#        bty="n"
#        )
# box()
# xticks <- c(0.01, 0.1, 1, 10)
# xlabels <- sprintf("%.2f", xticks)
# axis(1, at = xticks, labels = xlabels)
# # axis(2, las=1, tck=-0.04)
# # yticks <- seq(0, 1, length.out = 7)
# # ylabels <- sprintf("%.1f", yticks)
# # axis(2, las=1, tck=-0.04, at = yticks, labels = ylabels)
# axis(2, las=1, tck=-0.04)
# title(xlab="periodic update interval in s", line=1.8)
# title(ylab="false negative rate (miss rate)", line=3)

# done()


# # ----------- filtered -------------------------------------------------------------------------------------------------------
# done <- myps(file = "archive/latest/plots/runtimeFnrUpdateInterval.filtered", width=paper.width, height=paper.height)
# plot.new()
# par(mar=c(3.0,4.0,1,1.0), mgp=c(3.0, 0.5, 0))
# plot.window(xlim=c(minX, maxX),
#             ylim=c(minY, maxY) ,
#             xaxs="i",
#             yaxs="i",
#             log="xy")

# unique_behaviors <- unique(filtered$gcpBehavior)

# i <- 1  # Index counter
# for(behavior in unique_behaviors) {
#   subset_dt <- filtered[gcpBehavior == behavior]
#   print(subset_dt)
#   # keep line type and point character similar to user timer vs update interval
#   if (i == 2) {
#     i <- i + 1
#   }
#   lines(subset_dt$updateInterval, subset_dt$avg, lty=i)
#   points(subset_dt$updateInterval, subset_dt$avg, pch=i)
#   i <- i + 1  # Increment index
# }

# # Add legend using index-based colors
# legend("topleft",
#        legend=c("baseline", "lin. interp.", "nearest neighbor"),
#        lty=c(1, 3, 4),
#        pch=c(1, 3, 4),
#        ncol=2,
#        bty="n"
#        )
# box()
# xticks <- c(0.01, 0.1, 1, 10)
# xlabels <- sprintf("%.2f", xticks)
# axis(1, at = xticks, labels = xlabels)
# # axis(2, las=1, tck=-0.04)
# # yticks <- seq(0, 1, length.out = 7)
# # ylabels <- sprintf("%.1f", yticks)
# # axis(2, las=1, tck=-0.04, at = yticks, labels = ylabels)
# axis(2, las=1, tck=-0.04)
# title(xlab="periodic update interval in s", line=1.8)
# title(ylab="false negative rate (miss rate)", line=3)
# done()