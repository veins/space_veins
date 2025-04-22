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
eleAngleDis <- fread(args[1])
print("eleAngleDis:")
print(eleAngleDis)

eleAngleEn <- fread(args[2])
print("eleAngleEn:")
print(eleAngleEn)

minTime <- min(min(eleAngleDis$time), min(eleAngleEn$time))
maxTime <- max(max(eleAngleDis$time), max(eleAngleEn$time))

minEle <- min(min(eleAngleDis$lastElevationAngleToSop), min(eleAngleEn$lastElevationAngleToSop))
maxEle <- max(max(eleAngleDis$lastElevationAngleToSop), max(eleAngleEn$lastElevationAngleToSop))



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
            dotted=3
            )
lwds = list(default=1)
pchs = list(circle=1,
            cross=4,
            diamond=5)
legends = list(dis=list(lty=ltys[["dotted"]], lwd=lwds[["default"]], pch=pchs[["circle"]], label= expression("disabled")),
               en=list(lty=ltys[["solid"]], lwd=lwds[["default"]], pch=pchs[["cross"]], label= expression("enabled"))
               )

done <- myps(file = args[3], width=paper.width, height=paper.height)
plot.new()
par(mar=c(3.25,3.0,.5,.75), mgp=c(3.0, 0.5, 0))
plot.window(xlim=c(minTime, maxTime), ylim=c(minEle, maxEle), xaxs="i", yaxs="i")
lines(eleAngleDis$time, eleAngleDis$lastElevationAngleToSop, type = "s", lty = legends[["dis"]][["lty"]], lwd = legends[["dis"]][["lwd"]])
lines(eleAngleEn$time, eleAngleEn$lastElevationAngleToSop, type = "s", lty = legends[["en"]][["lty"]], lwd = legends[["en"]][["lwd"]])
# points(pu$time, pu$lastElevationAngleToSop, pch = legends[["periodic"]][["pch"]])
# points(gcp$time, gcp$lastElevationAngleToSop, pch = legends[["gcp"]][["pch"]])
# points(e2e$time, e2e$lastElevationAngleToSop, pch = legends[["received"]][["pch"]])

box()
axis(1, xaxp=c(minTime + 1, maxTime - 1, 3))
#axis(2, las=1, tck=-0.025)
axis(2, las=1)
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
       legend=c(legends[["dis"]][["label"]], legends[["en"]][["label"]]),
    #    lwd=c(legends[["periodic"]][["lwd"]], legends[["horizontal"]][["lwd"]]),
    #    pch=c(legends[["periodic"]][["pch"]], legends[["gcp"]][["pch"]], legends[["received"]]),
       lty=c(legends[["dis"]][["lty"]], legends[["en"]][["lty"]]),
       bty="n",
       ncol=1)
done()