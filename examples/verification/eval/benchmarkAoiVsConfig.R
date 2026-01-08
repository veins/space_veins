require(data.table)
require(dplyr)
require(tidyverse)

print(sessionInfo())

args = commandArgs(trailingOnly=TRUE)

print(args)

data <- fread(args[1])
data$precalculation <- factor(data$precalculation, levels = c("Dis", "En"))
data$gcpBehavior <- factor(data$gcpBehavior, levels = c("NGCP", "WGCP", "WLIP", "WNPP"))
# data$group <- interaction(data$precalculation, data$gcpBehavior, sep = " | ")
data$group <- interaction(data$gcpBehavior, data$precalculation, sep = " | ")

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
done <- myps(file = "plots/benchmarkAoiVsConfig", width=paper.width, height=paper.height)
plot.new()
par(mar=c(2.0,3.0,1,1.0), mgp=c(3.0, 0.5, 0))
pos <- c(1:4, 6:9)  # Gaps between precalculation groups
greys <- gray.colors(4, start = 0.9, end = 0.3)  # 90% → 10% grey
boxplot(ageOfInformation ~ group, data = data, at = pos,
        col = greys,
        las = 2,
        ylim = c(0, 10),
        xaxt = "n")
title(ylab="age of information in s", line=2)
axis(1, at = c(2.5, 7.5), labels = c("baseline", "improved"), tick = FALSE)
legend(x = "bottomright", legend = c("NGCP", "WGCP", "WLIP", "WNPP"), fill=greys, ncol = 2, bty="n")
box()
done()