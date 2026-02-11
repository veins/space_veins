require(data.table)
require(dplyr)
require(tidyverse)

print(sessionInfo())

args = commandArgs(trailingOnly=TRUE)
# if (length(args) < 4) {
#     stop("Please provide four csv files!")
# }
print(args)

# read in elevation angle data
data <- fread(args[1])

# filter for ngcp
data <- data[grepl("\\\\\"SGP4MobilityNoGetCurrentPositionUpdates\\\\\"", gcp), ]
print("data:")
print(data)

eleAngleDis <- data[filter == FALSE, ]
print("eleAngleDis:")
print(eleAngleDis)

eleAngleEn <- data[filter == TRUE, ]
print("eleAngleEn:")
print(eleAngleEn)

minTime <- min(min(eleAngleDis$time), min(eleAngleEn$time))
# print minimum of eleAngleDis$time and eleAngleEn$time
print(paste("minTimeDis:", min(eleAngleDis$time)))
print(paste("minTimeEn:", min(eleAngleEn$time)))
# print maximum of eleAngleDis$time and eleAngleEn$time
print(paste("maxTimeDis:", max(eleAngleDis$time)))
print(paste("maxTimeEn:", max(eleAngleEn$time)))
minTime <- 0

maxTime <- 3600

# minEle <- min(min(eleAngleDis$lastElevationAngleToSop), min(eleAngleEn$lastElevationAngleToSop))
# maxEle <- max(max(eleAngleDis$lastElevationAngleToSop), max(eleAngleEn$lastElevationAngleToSop))
minEle <- -90
maxEle <- 90



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

ltys = list(solid=1,
            dashed=2,
            dotted=3
            )
lwds = list(default=1)
pchs = list(circle=1,
            cross=4,
            diamond=5,
            filledCircle=20)
legends = list(dis=list(lty=ltys[["dotted"]], lwd=lwds[["default"]], pch=pchs[["circle"]], col="black", label= expression("unfiltered")),
               en=list(lty=ltys[["solid"]], lwd=lwds[["default"]], pch=pchs[["filledCircle"]], cex=0.1, col="black", label= expression("filtered")),
               horizon=list(lty=ltys[["dashed"]], lwd=lwds[["default"]], col="gray", label= expression("horizon")),
               com=list(lty=ltys[["dashed"]], lwd=lwds[["default"]], col="red", label= expression("com. threshold"))
               )

done <- myps(file = args[2], width=paper.width, height=paper.height)
plot.new()
par(mar=c(3.25,3.0,.5,1.1), mgp=c(3.0, 0.5, 0))
plot.window(xlim=c(minTime, maxTime), ylim=c(minEle, maxEle), xaxs="i", yaxs="i")
lines(eleAngleDis$time, eleAngleDis$lastElevationAngleToSop, type = "s", lty = legends[["dis"]][["lty"]], lwd = legends[["dis"]][["lwd"]])
# lines(eleAngleEn$time, eleAngleEn$lastElevationAngleToSop, type = "s", lty = legends[["en"]][["lty"]], lwd = legends[["en"]][["lwd"]])
points(eleAngleEn$time, eleAngleEn$lastElevationAngleToSop, pch = legends[["en"]][["pch"]], cex = 0.09, col = legends[["en"]][["col"]])
# plot horizon
abline(h=0, col="gray", lty=legends[["horizon"]][["lty"]])
# communication threshold
abline(h=8, col="red", lty=legends[["com"]][["lty"]])
# points(pu$time, pu$lastElevationAngleToSop, pch = legends[["periodic"]][["pch"]])
# points(gcp$time, gcp$lastElevationAngleToSop, pch = legends[["gcp"]][["pch"]])
# points(e2e$time, e2e$lastElevationAngleToSop, pch = legends[["received"]][["pch"]])

box()
ticks <- c(0, 600, 1200, 1800, 2400, 3000, 3600)
# axis(1, xaxp=c(minTime, maxTime, 5))
axis(1, at = ticks)
axis(2, las=1, tck=-0.025, yaxp=c(minEle, maxEle, 4))
title(xlab="simulation time in s", line=2.2)
title(ylab="elevation angle in deg", line=2)

legendPosition <- "topright"
insetValue <- c(0, 0)

# if (str_detect(args[2], "StarlinkShell2")) {
#     legendPosition <- "bottomleft"
#     insetValue <- c(0, -0.05)
# }

legend(legendPosition,
       inset=insetValue,
       legend=c(legends[["dis"]][["label"]], legends[["en"]][["label"]], legends[["horizon"]][["label"]], legends[["com"]][["label"]]),
       col=c(legends[["dis"]][["col"]], legends[["en"]][["col"]], legends[["horizon"]][["col"]], legends[["com"]][["col"]]),
       lty=c(legends[["dis"]][["lty"]], legends[["en"]][["lty"]], legends[["horizon"]][["lty"]], legends[["com"]][["lty"]]),
       bty="n",
       x.intersp = 0.2,
       cex = 0.9,
       seg.len = 0.8,
       ncol=2)
done()