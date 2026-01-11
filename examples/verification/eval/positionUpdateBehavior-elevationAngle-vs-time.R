require(data.table)
require(dplyr)
require(tidyverse)

print(sessionInfo())

args = commandArgs(trailingOnly=TRUE)
# if (length(args) < 4) {
#     stop("Please provide four csv files!")
# }
print(args)

minTime <- 2804
maxTime <- 2809

eleWindowMin <- 7.85
eleWindowMax <- 8.15

# read in elevation angle data
# filter based on simulation time
eleAngle <- fread(args[1])
eleAngle <- subset(eleAngle, time >= minTime)
eleAngle <- subset(eleAngle, time <= maxTime)
eleAngle <- subset(eleAngle, select = -c(node, repetition))
print("eleAngle:")
print(eleAngle)

# read time points of getCurrentPositionUpdateCalls (time points when we want to know the position of a satellite)
# filter based on simulation time
gcpuc <- fread(args[2])
gcpuc <- subset(gcpuc, time >= minTime)
gcpuc <- subset(gcpuc, time <= maxTime)
gcpuc <- subset(gcpuc, select = -c(node, repetition))
print("gcpuc:")
print(gcpuc)

# read time points and calculated elevationAngle relative to the SOP of SGP4 updates
# filter based on simulation time
sgp4 <- fread(args[3])
sgp4 <- subset(sgp4, time >= minTime)
sgp4 <- subset(sgp4, time <= maxTime)
sgp4 <- subset(sgp4, select = -c(node, repetition))
print("sgp4:")
print(sgp4)

# read time points and calculated elevationAngle relative to the SOP of linear interpolation updates
# filter based on simulation time
# li <- fread(args[4])
# li <- subset(li, time >= minTime)
# li <- subset(li, time <= maxTime)
# li <- subset(li, select = -c(node, repetition))
# print("li:")
# print(li)

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
# reduce the size of the plot to 60% of the paper size such that 3 plots fit on one page
# paper.width <- paper.width * 0.6
# paper.height <- paper.height * 0.6

lwds = list(default=1)
pchs = list(circle=1,
            cross=4,
            diamond=5)
legends = list(sgp4Update=list(lwd=lwds[["default"]], pch=pchs[["circle"]], label= expression("SGP4 update")),
               gcpUpdate=list(lty=2, label= expression("gcp update")))

done <- myps(file = args[4], width=paper.width, height=paper.height)
plot.new()
par(mar=c(3.25,3.0,.5,.75), mgp=c(3.0, 0.5, 0))
plot.window(xlim=c(minTime + 1, maxTime - 1), ylim=c(eleWindowMin ,eleWindowMax), xaxs="i", yaxs="i")
lines(eleAngle$time, eleAngle$lastElevationAngleToSop, type = "s", lwd = legends[["sgp4Update"]][["lwd"]])
points(sgp4$time, sgp4$sgp4Computations, pch = legends[["sgp4Update"]][["pch"]])
# points(li$time, li$linearInterpolationComputations, pch = legends[["liUpdate"]][["pch"]])

# plot a vertical line for every value in gcp$time
for (i in 1:nrow(gcpuc)) {
    abline(v=gcpuc$time[i], col="black", lty=2)
}

box()
axis(1, xaxp=c(minTime + 1, maxTime -1, 4))
yticks <- seq(eleWindowMin, eleWindowMax, length.out = 7)
ylabels <- sprintf("%.2f", yticks)
axis(2, at = yticks, labels = ylabels, las=1, tck = -0.05)
title(xlab="simulation time in s", line=2.2)
title(ylab="elevation angle in deg", line=2)

# legend position in the top middle
legendPosition <- "bottomleft"
insetValue <- c(0, 0)

# if (unlist(strsplit(args[5], '/'))[4] == "VerifyDisableUpdatePositionOnlyAboveHorizonLastPosition") {
#     legendPosition <- "bottom"
#     # insetValue <- c(0, -0.05)
# }

legend(legendPosition,
        # add a frame ``
       inset=insetValue,
       legend=c(legends[["sgp4Update"]][["label"]], legends[["gcpUpdate"]][["label"]], expression("sim. state")),
       pch=c(legends[["sgp4Update"]][["pch"]], NA, NA),
       lty=c(NA, 2, 1),
       bty="o",
    #    cex=0.9,
       x.intersp = 0.15,
       seg.len = 0.9,
       ncol=1)
done()