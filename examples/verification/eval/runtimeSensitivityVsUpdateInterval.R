require(data.table)
require(dplyr)
require(tidyverse)

print(sessionInfo())

args = commandArgs(trailingOnly=TRUE)
# print(args)

data <- fread(args[1])

data[, updateInterval := as.numeric(gsub("c", ".", updateInterval, fixed = TRUE)) ]

unfiltered <- data[filter == "Dis"]
unfiltered$totalPopulation <- unfiltered$fp + unfiltered$fn + unfiltered$tp + unfiltered$tn
unfiltered$accuracy <- (unfiltered$tp + unfiltered$tn) / (unfiltered$tp + unfiltered$tn + unfiltered$fp + unfiltered$fn)
unfiltered$precision <- (unfiltered$tp) / (unfiltered$tp + unfiltered$fp)
unfiltered$conditionPositive <- unfiltered$fn + unfiltered$tp
unfiltered$conditionNegative <- unfiltered$fp + unfiltered$tn
unfiltered$sensitivity <- unfiltered$tp / unfiltered$conditionPositive
unfiltered$fpr <- unfiltered$fp / unfiltered$conditionNegative
unfiltered$plr <- unfiltered$sensitivity / unfiltered$fpr
fwrite(unfiltered, "archive/latest/plots/classicationResults.csv")
unfiltered <- unfiltered[!is.na(sensitivity), 
                        .(avg = mean(sensitivity, na.rm = TRUE),
                          sd = sd(sensitivity, na.rm = TRUE)), 
                        by = .(gcpBehavior, updateInterval)]
unfiltered <- unfiltered[order(updateInterval)]
print(unfiltered)

# filtered <- data[filter == "En"]
# filtered$accuracy <- (filtered$tp + filtered$tn) / (filtered$tp + filtered$tn + filtered$fp + filtered$fn)
# filtered$precision <- (filtered$tp) / (filtered$tp + filtered$fp)
# filtered <- filtered[order(updateInterval)]

fwrite(unfiltered, "archive/latest/plots/runtimeSensitivityVsUpdateInterval.unfiltered.pdf.csv")
# fwrite(filtered, "archive/latest/plots/runtimeUserTimeVsUpdateInterval.filtered.pdf.csv")

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

minX <- 0
# maxX <- max(max(unfiltered$updateInterval), max(filtered$updateInterval))
maxX <- max(unfiltered$updateInterval)
print(maxX)
minY <- 0.8
maxY <- max(unfiltered$avg)
print(maxY)

# ----------- unfiltered
done <- myps(file = "archive/latest/plots/runtimeSensitivityVsUpdateInterval.unfiltered", width=paper.width, height=paper.height)
plot.new()

# minX <- 0
# maxX <- max(unfiltered$updateInterval)
# minY <- 0
# maxY <- max(unfiltered$avg)

par(mar=c(3.0,3.0,1,1.0), mgp=c(3.0, 0.5, 0))
plot.window(xlim=c(minX, maxX),
            ylim=c(minY, 1) ,
            xaxs="i",
            yaxs="i")

unique_behaviors <- unique(unfiltered$gcpBehavior)

i <- 1  # Index counter
for(behavior in unique_behaviors) {
  subset_dt <- unfiltered[gcpBehavior == behavior]
  print(subset_dt)
  # keep line type and point character similar to user timer vs update interval
  if (i == 2) {
    i <- i + 1
  }
  lines(subset_dt$updateInterval, subset_dt$avg, lty=i)
  points(subset_dt$updateInterval, subset_dt$avg, pch=i)
  i <- i + 1  # Increment index
}

# Add legend using index-based colors
legend("bottomleft",
       legend=c("baseline", "lin. interp.", "nearest neighbor"),
       lty=c(1, 3, 4),
       pch=c(1, 3, 4),
       ncol=2,
       bty="n"
       )
box()
xticks <- seq(minX, maxX, length.out = 7)
xlabels <- sprintf("%.0f", xticks)
axis(1, at = xticks, labels = xlabels)
# axis(2, las=1, tck=-0.04)
# yticks <- seq(0, 1, length.out = 7)
# ylabels <- sprintf("%.1f", yticks)
# axis(2, las=1, tck=-0.04, at = yticks, labels = ylabels)
axis(2, las=1, tck=-0.04)
title(xlab="periodic update interval in s", line=1.8)
title(ylab="sensitivity", line=2)

done()


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