require(data.table)
require(dplyr)
require(tidyverse)

print(sessionInfo())

args = commandArgs(trailingOnly=TRUE)

perf_dt <- fread(args[1])
avg_share <- perf_dt[, .(avg_share = mean(share, na.rm = TRUE)), 
                     by = .(precalculation, gcpBehavior)]
print(avg_share)
setorder(avg_share, gcpBehavior, precalculation)
fwrite(avg_share, "plots/runTimeShareVsConfig.pdf.csv")

# ----------- plot

# Create matrix for grouped barplot: gcpBehavior (x-axis), precalculation (groups)
plot_data <- dcast(avg_share, gcpBehavior ~ precalculation, value.var = "avg_share")
plot_matrix <- as.matrix(plot_data[, .(Dis, En)])
rownames(plot_matrix) <- plot_data$gcpBehavior
colnames(plot_matrix) <- c("baseline", "improved")

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
done <- myps(file = "plots/runTimeShareVsConfig", width=paper.width, height=paper.height)
plot.new()

par(mar=c(2.0,3.0,1,1.0), mgp=c(3.0, 0.5, 0))
# Create 4 grey shades (light → dark)
greys <- gray.colors(4, start = 0.9, end = 0.1)  # 90% → 10% grey

print(greys)
# Grouped barplot using base R
barplot(plot_matrix,
        beside = TRUE,                    # Grouped bars side-by-side
        col = greys,
        axes = FALSE,
        legend.text = TRUE,
        args.legend = list(ncol = 2, bty="n"),
        ylim = c(0, 100))                   # Assuming share is in percent
box()
axis(2, las=1, tck=-0.04)
title(ylab="mobility run time share in %", line=2)
# Add value labels on top of bars
text(x = barplot(plot_matrix, beside = TRUE, plot = FALSE),
     y = plot_matrix + 0.02,
     labels = round(plot_matrix, 3),
     pos = 3)

done()