require(data.table)
require(dplyr)
require(tidyverse)

print(sessionInfo())
args = commandArgs(trailingOnly=TRUE)
# print(args)

rawData <- fread(args[1])

# print(rawData)
avgs <- rawData[, .(avg_userTime = mean(userTime), avg_kernelTime = mean(kernelTime), avg_wallClockTime = mean(wallClockTime)), by = .(config, updateAboveHorizon, getCurrentPositionBehavior, precalculationInterval)]
toPlot <- avgs[precalculationInterval == 120]
# Sort ascending alphabetically by 'config' column to ensure the correct order
toPlot <- toPlot[order(config)]
print("data.table")
print(toPlot)
toPlot <- matrix(toPlot$avg_wallClockTime, byrow = TRUE, nrow=2, ncol=4, dimnames = list(c("Baseline", "Optimized"), c("NGCP", "WGCP", "WLIP", "WNPP")))
print("matrix")
print(toPlot)
fwrite(toPlot, "archive/latest/csv/gnuTimeResults/wallClockTime-vs-gcpb.csv")

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
# # reduce the size of the plot to 60% of the paper size such that 3 plots fit on one page
# # paper.width <- paper.width * 0.6
# # paper.height <- paper.height * 0.6

# ltys = list(solid=1,
#             dashed=2,
#             dotted=3,
#             dotdash=4
#             )
# lwds = list(default=1)
# pchs = list(circle=1,
#             cross=4,
#             diamond=5,
#             filledCircle=20)
# legends = list(NGCP=list(lty=ltys[["solid"]], lwd=lwds[["default"]], pch=pchs[["circle"]], cex=0.5, col="black", label= expression("NGCP")),
#                WGCP=list(lty=ltys[["dashed"]], lwd=lwds[["default"]], pch=pchs[["cross"]], cex=0.5, col="black", label= expression("WGCP")),
#                WLIP=list(lty=ltys[["dotted"]], lwd=lwds[["default"]], pch=pchs[["diamond"]], cex=0.5, col="black", label= expression("WLIP")),
#                WNPP=list(lty=ltys[["dotdash"]], lwd=lwds[["default"]], pch=pchs[["filledCircle"]], cex=0.5, col="black", label= expression("WNPP"))
#                )

done <- myps(file = "archive/latest/plots/wallClockTime-vs-gcpb", width=paper.width, height=paper.height)
plot.new()
par(mar=c(3.25,3.0,.5,1.1), mgp=c(3.0, 0.5, 0))
plot.window(xlim=c(0, 1), ylim=c(0, 20), xaxs="i", yaxs="i")
barplot(toPlot, beside=TRUE, ylim=c(0, 150), axes=FALSE, legend.text=TRUE, args.legend=list(bty="n"))

box()
axis(2, las=1)
title(xlab="getCurrentPosition() behavior", line=2.2)
title(ylab="Wall clock time in s", line=2)
done()


# box()
# axis(1)
# axis(2, las=1, tck=-0.025)
# title(xlab="precalculation interfval in s", line=2.2)
# title(ylab="speed up", line=2)

# legendPosition <- "topright"
# insetValue <- c(0, 0)

# # if (str_detect(args[2], "StarlinkShell2")) {
# #     legendPosition <- "bottomleft"
# #     insetValue <- c(0, -0.05)
# # }

# legend(legendPosition,
#        inset=insetValue,
#        legend=c(legends[["NGCP"]][["label"]], legends[["WGCP"]][["label"]], legends[["WLIP"]][["label"]], legends[["WNPP"]][["label"]]),
#        col=c(legends[["NGCP"]][["col"]], legends[["WGCP"]][["col"]], legends[["WLIP"]][["col"]], legends[["WNPP"]][["col"]]),
#        lty=c(legends[["NGCP"]][["lty"]], legends[["WGCP"]][["lty"]], legends[["WLIP"]][["lty"]], legends[["WNPP"]][["lty"]]),
#        pch=c(legends[["NGCP"]][["pch"]], legends[["WGCP"]][["pch"]], legends[["WLIP"]][["pch"]], legends[["WNPP"]][["pch"]]),
#        bty="n",
#        ncol=2)
# done()