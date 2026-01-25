require(data.table)
require(dplyr)
require(tidyverse)

print(sessionInfo())

args = commandArgs(trailingOnly=TRUE)
# print(args)

data <- fread(args[1])

convert_c_to_float <- function(str) {
  # Split string at 'c', convert parts to numeric, treat 'c' as decimal point
  parts <- strsplit(str, "c")[[1]]
  before <- as.numeric(parts[1])
  after <- as.numeric(parts[2])
  
  # Reconstruct as float: before + after shifted by decimal position
  decimal_places <- nchar(parts[2])
  result <- before + after / (10^decimal_places)
  
  return(result)
}

# lapply(data, convert_c_to_float)

unfiltered <- data[filter == "Dis"]
unfiltered$totalPopulation <- unfiltered$fp + unfiltered$fn + unfiltered$tp + unfiltered$tn
unfiltered$accuracy <- (unfiltered$tp + unfiltered$tn) / (unfiltered$tp + unfiltered$tn + unfiltered$fp + unfiltered$fn)
unfiltered$precision <- (unfiltered$tp) / (unfiltered$tp + unfiltered$fp)
unfiltered$conditionPositive <- unfiltered$fn + unfiltered$tp
unfiltered$conditionNegative <- unfiltered$fp + unfiltered$tn
unfiltered$sensitivity <- unfiltered$tp / unfiltered$conditionPositive
unfiltered$fpr <- unfiltered$fp / unfiltered$conditionNegative
unfiltered$plr <- unfiltered$sensitivity / unfiltered$fpr

unfiltered <- unfiltered[order(updateInterval)]
print(unfiltered)

# filtered <- data[filter == "En"]
# filtered$accuracy <- (filtered$tp + filtered$tn) / (filtered$tp + filtered$tn + filtered$fp + filtered$fn)
# filtered$precision <- (filtered$tp) / (filtered$tp + filtered$fp)
# filtered <- filtered[order(updateInterval)]

fwrite(unfiltered, "archive/latest/plots/runtimeAccuracyVsUpdateInterval.unfiltered.pdf.csv")
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

# minX <- 0
# # maxX <- max(max(unfiltered$updateInterval), max(filtered$updateInterval))
# maxX <- max(unfiltered$updateInterval)
# # print(maxX)
# minY <- 0.85
# maxY <- max(max(unfiltered$accuracy), max(unfiltered$accuracy))

# # ----------- unfiltered
# done <- myps(file = "archive/latest/plots/runtimeAcuracyVsUpdateInterval.unfiltered", width=paper.width, height=paper.height)
# plot.new()

# # minX <- 0
# # maxX <- max(unfiltered$updateInterval)
# # minY <- 0
# # maxY <- max(unfiltered$avg)

# par(mar=c(3.0,3.0,1,1.0), mgp=c(3.0, 0.5, 0))
# plot.window(xlim=c(minX, maxX+1),
#             ylim=c(minY, maxY+10),
#             xaxs="i",
#             yaxs="i")

# unique_behaviors <- unique(unfiltered$gcpBehavior)

# i <- 1  # Index counter
# for(behavior in unique_behaviors) {
#   subset_dt <- unfiltered[gcpBehavior == behavior]
#   lines(subset_dt$updateInterval, subset_dt$avg, lty=i)
#   points(subset_dt$updateInterval, subset_dt$avg, pch=i)
#   i <- i + 1  # Increment index
# }

# # Add legend using index-based colors
# legend("bottomright",
#        legend=unique_behaviors,
#        lty=1:length(unique_behaviors),
#        pch=1:length(unique_behaviors),
#        ncol=2,
#        bty="n"
#        )
# box()
# xticks <- seq(minX, maxX+1, length.out = maxX+1)
# xlabels <- sprintf("%.0f", xticks)
# axis(1, at = xticks, labels = xlabels)
# # axis(2, las=1, tck=-0.04)
# axis(2, las=1, tck=-0.04)
# title(xlab="periodic update interval in s", line=1.8)
# title(ylab="accuracy", line=2)

# done()


# # # ----------- filtered -------------------------------------------------------------------------------------------------------
# # done <- myps(file = "archive/latest/plots/runtimeUserTimeVsUpdateInterval.filtered", width=paper.width, height=paper.height)
# # plot.new()

# # # minX <- 0
# # # maxX <- max(filtered$updateInterval)
# # # minY <- 0
# # # maxY <- max(filtered$avg)

# # par(mar=c(3.0,3.0,1,1.0), mgp=c(3.0, 0.5, 0))
# # plot.window(xlim=c(minX, maxX+1),
# #             ylim=c(minY, maxY+10),
# #             xaxs="i",
# #             yaxs="i")

# # unique_behaviors <- unique(filtered$getCurrentPositionBehavior)

# # i <- 1  # Index counter
# # for(behavior in unique_behaviors) {
# #   subset_dt <- filtered[getCurrentPositionBehavior == behavior]
# #   lines(subset_dt$updateInterval, subset_dt$avg, lty=i)
# #   points(subset_dt$updateInterval, subset_dt$avg, pch=i)
# #   i <- i + 1  # Increment index
# # }

# # # Add legend using index-based colors
# # legend("bottomright",
# #        legend=unique_behaviors,
# #        lty=1:length(unique_behaviors),
# #        pch=1:length(unique_behaviors),
# #        ncol=2,
# #        bty="n"
# #        )
# # box()
# # xticks <- seq(minX, maxX+1, length.out = maxX+1)
# # xlabels <- sprintf("%.0f", xticks)
# # axis(1, at = xticks, labels = xlabels)
# # # axis(2, las=1, tck=-0.04)
# # axis(2, las=1, tck=-0.04)
# # title(xlab="periodic update interval in s", line=1.8)
# # title(ylab="user space run time in s", line=2)

# # done()