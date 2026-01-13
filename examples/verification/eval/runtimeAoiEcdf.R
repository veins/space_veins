require(data.table)
require(dplyr)
require(tidyverse)

print(sessionInfo())

args = commandArgs(trailingOnly=TRUE)

data <- fread(args[1])
print(data)

# ngcp_dt <- data[updateInterval == 10, precalculation == "Dis", gcpBehavior == "NGCP", repetition == 0]
# wgcp_dt <- data[updateInterval == 10, precalculation == "Dis", gcpBehavior == "WGCP", repetition == 0]
# wlip_dt <- data[updateInterval == 10, precalculation == "Dis", gcpBehavior == "WLIP", repetition == 0]
# wnpp_dt <- data[updateInterval == 10, precalculation == "Dis", gcpBehavior == "WNPP", repetition == 0]

# ngcp_ecdf <- ecdf(ngcp_dt$ageOfInformation)
# wgcp_ecdf <- ecdf(wgcp_dt$ageOfInformation)
# wlip_ecdf <- ecdf(wlip_dt$ageOfInformation)
# wnpp_ecdf <- ecdf(wnpp_dt$ageOfInformation)

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

minX <- max(min(data$ageOfInformation), min(data$ageOfInformation))
maxX <- max(max(data$ageOfInformation), max(data$ageOfInformation))
minY <- 0
maxY <- 1

# ----------- plot
done <- myps(file = "archive/latest/plots/runtimeAoiEcdf", width=paper.width, height=paper.height)
plot.new()

par(mar=c(3.0,3.0,1,1.0), mgp=c(3.0, 0.5, 0))
plot.window(xlim=c(minX, maxX),
            ylim=c(minY, maxY),
            xaxs="i",
            yaxs="i")

unique_behaviors <- unique(data$gcpBehavior)
# unique_behaviors <- list("NGCP")
colors <- rainbow(length(unique_behaviors))

i <- 1  # Index counter
for(behavior in unique_behaviors) {
#   subset_dt <- data[updateInterval == 10 & precalculation == "Dis" & gcpBehavior == behavior & repetition == 0]
  subset_dt <- data[updateInterval == 10 & precalculation == "Dis" & gcpBehavior == behavior]
  subset_ecdf <- ecdf(subset_dt$ageOfInformation)
  lines(subset_ecdf, col=colors[i], do.points = FALSE)
#   points(subset_dt$updateInterval, subset_dt$avg, pch=i)
  i <- i + 1  # Increment index
}

# Add legend using index-based colors
legend("bottomright",
       legend=unique_behaviors,
    #    lty=1:length(unique_behaviors),
       lty=1,
       col=colors,
    #    pch=1:length(unique_behaviors),
       ncol=2,
       bty="n"
       )
box()
xticks <- seq(minX, maxX, length.out = maxX)
xlabels <- sprintf("%.2f", xticks)
axis(1, at = xticks, labels = xlabels)
axis(2, las=1, tck=-0.04)
title(xlab="age of information in s", line=1.8)
title(ylab="ecdf", line=2)

done()