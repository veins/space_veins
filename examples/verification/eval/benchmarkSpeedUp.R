require(data.table)
require(dplyr)
require(tidyverse)

print(sessionInfo())

args = commandArgs(trailingOnly=TRUE)

perf_dt <- fread(args[1])
# First: Calculate main(Dis)/main(En) ratio for EVERY ROW where both exist
perf_dt[, dis_over_en := ifelse(precalculation == "Dis" & .N > 1,
                                main[precalculation == "Dis"] / main[precalculation == "En"],
                                NA_real_),
        by = gcpBehavior]
# Then: Calculate average of those ratios by gcpBehavior
# speedUp <- perf_dt[!is.na(dis_over_en), .(avg_ratio = mean(dis_over_en, na.rm = TRUE)), 
#                       by = gcpBehavior]
speedUp <- perf_dt[!is.na(dis_over_en), 
                        .(avg_ratio = mean(dis_over_en, na.rm = TRUE),
                          sd_ratio = sd(dis_over_en, na.rm = TRUE)), 
                        by = gcpBehavior]

# ToDo: maybe calculate speedUp for hsm, too?
setorder(speedUp, gcpBehavior)
print(speedUp)
fwrite(speedUp, "plots/benchmarkSpeedUp.pdf.csv")

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
done <- myps(file = "plots/benchmarkSpeedUp", width=paper.width, height=paper.height)
plot.new()

par(mar=c(2.0,3.0,1,1.0), mgp=c(3.0, 0.5, 0))
# Create 4 grey shades (light → dark)
greys <- gray.colors(4, start = 0.9, end = 0.1)  # 90% → 10% grey

xpos <- barplot(speedUp$avg_ratio,
        names.arg = speedUp$gcpBehavior,
        col = greys,
        axes = FALSE,
        legend.text = TRUE,
        xaxt = "n",
        ylim = c(0, 2.5))
arrows(xpos, speedUp$avg_ratio + speedUp$sd_ratio,
       xpos, speedUp$avg_ratio - speedUp$sd_ratio,
       angle = 90, code = 3, length = 0.1, lwd = 1.5)
box()
axis(1, at = xpos, labels = c("baseline", "precise", "lin. interp.", "nearest neighbor"))
axis(2, las=1, tck=-0.04)
title(ylab="speed up", line=2)
# Add value labels on top of bars
text(x = xpos,
     y = speedUp$avg_ratio + 0.02,
     labels = round(speedUp$avg_ratio, 2),
     pos = 3,
     cex = 0.8)

done()