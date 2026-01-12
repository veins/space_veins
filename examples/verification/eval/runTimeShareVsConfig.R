require(data.table)
require(dplyr)
require(tidyverse)

print(sessionInfo())

args = commandArgs(trailingOnly=TRUE)

perf_dt <- fread(args[1])
# avg_share <- perf_dt[, .(avg_share = mean(share, na.rm = TRUE), sd_share = sd(share, na.rm = TRUE)), 
#                      by = .(precalculation, gcpBehavior)]
perf_summary <- perf_dt[!is.na(share), 
                        .(avg_ratio = mean(share, na.rm = TRUE),
                          sd_ratio = sd(share, na.rm = TRUE)), 
                        by = .(precalculation, gcpBehavior)]
print(perf_summary)
setorder(perf_summary, gcpBehavior, precalculation)
fwrite(perf_summary, "plots/runTimeShareVsConfig.pdf.csv")

# ----------- plot

# Create matrix for grouped barplot (rows = precalculation groups, columns = gcpBehavior)
avg_matrix <- dcast(perf_summary, precalculation ~ gcpBehavior, value.var = "avg_ratio")
avg_matrix <- as.matrix(avg_matrix[, -1])  # Remove precalculation column, keep as matrix
rownames(avg_matrix) <- unique(perf_summary$precalculation)

sd_matrix <- dcast(perf_summary, precalculation ~ gcpBehavior, value.var = "sd_ratio")
sd_matrix <- as.matrix(sd_matrix[, -1])  # Remove precalculation column, keep as matrix
rownames(sd_matrix) <- unique(perf_summary$precalculation)

# View the matrix
cat(avg_matrix)

# # Create matrix for grouped barplot: gcpBehavior (x-axis), precalculation (groups)
# # plot_data <- dcast(avg_share, gcpBehavior ~ precalculation, value.var = "avg_share")
# # Reshape for barplot matrix format
# summary_matrix <- dcast(perf_summary, precalculation ~ gcpBehavior, 
#                         value.var = c("avg_ratio", "sd_ratio"))
# # cat(summary_matrix)
# # avg_matrix <- as.matrix(summary_matrix[, grep("avg_ratio", names(summary_matrix))])
# avg_matrix <- as.matrix(summary_matrix[, .(Dis, En)])
# cat(avg_matrix)
# sd_matrix <- as.matrix(summary_matrix[, grep("sd_ratio", names(summary_matrix))])
# # plot_matrix <- as.matrix(plot_data[, .(Dis, En)])
# # rownames(plot_matrix) <- plot_data$gcpBehavior
# # rownames(avg_matrix) <- c("baseline", "precise", "lin. interp.", "nearest neighbor")
# # colnames(avg_matrix) <- c("unfiltered", "filtered")

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
greys <- gray.colors(2, start = 0.9, end = 0.1)  # 90% → 10% grey

print(greys)
# Grouped barplot using base R
bar_centers <- barplot(avg_matrix,
        beside = TRUE,                    # Grouped bars side-by-side
        col = greys,
        axes = FALSE,
        legend.text = TRUE,
        names.arg = c("baseline", "precise", "lin. interp.", "nearest neighbor"),
        args.legend = list(legend = c("unfiltered", "filtered"), ncol = 2, bty="n"),
        ylim = c(0, 100))                   # Assuming share is in percent
group_mids <- colMeans(bar_centers)
# Error bars (unchanged)
for(i in 1:ncol(avg_matrix)) {
    arrows(bar_centers[,i], avg_matrix[,i] - sd_matrix[,i],
           bar_centers[,i], avg_matrix[,i] + sd_matrix[,i],
           angle = 90, code = 3, length = 0.1, lwd = 1.5)
}
box()
# axis(1, at = group_mids, labels = c("baseline", "precise", "lin. interp.", "nearest neighbor"))
axis(2, las=1, tck=-0.04)
title(ylab="mobility run time share in %", line=2)
# Add value labels on top of bars
# text(x = barplot(plot_matrix, beside = TRUE, plot = FALSE),
#      y = plot_matrix + 0.02,
#      labels = round(plot_matrix, 2),
#      pos = 3)

done()