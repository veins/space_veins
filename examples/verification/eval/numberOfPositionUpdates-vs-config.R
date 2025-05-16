require(data.table)
require(dplyr)
require(tidyverse)

print(sessionInfo())

args = commandArgs(trailingOnly=TRUE)
print(args)

puDisLast <- fread(args[1])
print("puDisLast:")
print(puDisLast)
puDisLin <- fread(args[2])
print("puDisLin:")
print(puDisLin)
puDisNear <- fread(args[3])
print("puDisNear:")
print(puDisNear)
puDisSgp4 <- fread(args[4])
print("puDisSgp4:")
print(puDisSgp4)

puEnLast <- fread(args[5])
print("puEnLast:")
print(puEnLast)
puEnLin <- fread(args[6])
print("puEnLin:")
print(puEnLin)
puEnNear <- fread(args[7])
print("puEnNear:")
print(puEnNear)
puEnSgp4 <- fread(args[8])
print("puEnSgp4:")
print(puEnSgp4)

# baseline <- puDisLast$packetReceived
# print("baseline:")
# print(baseline)
# # for every data.table (use a loop), calculate packetReceived / baseline and store in a new column called "pdr"
# puDisLast$pdr <- (puDisLast$packetReceived / baseline) * 100
# puDisLin$pdr <- (puDisLin$packetReceived / baseline) * 100
# puDisNear$pdr <- (puDisNear$packetReceived / baseline) * 100
# puDisSgp4$pdr <- (puDisSgp4$packetReceived / baseline) * 100
# puEnLast$pdr <- (puEnLast$packetReceived / baseline) * 100
# puEnLin$pdr <- (puEnLin$packetReceived / baseline) * 100
# puEnNear$pdr <- (puEnNear$packetReceived / baseline) * 100
# puEnSgp4$pdr <- (puEnSgp4$packetReceived / baseline) * 100
# # print every data.table
# print("puDisLast:")
# print(puDisLast)
# print("puDisLin:")
# print(puDisLin)
# print("puDisNear:")
# print(puDisNear)
# print("puDisSgp4:")
# print(puDisSgp4)
# print("puEnLast:")
# print(puEnLast)
# print("puEnLin:")
# print(puEnLin)
# print("puEnNear:")
# print(puEnNear)
# print("puEnSgp4:")
# print(puEnSgp4)

maxSgp4Computations <- ceiling(max(puDisLast$sgp4Computations, puEnLast$sgp4Computations, puDisLin$sgp4Computations, puEnLin$sgp4Computations, puDisNear$sgp4Computations, puEnNear$sgp4Computations, puDisSgp4$sgp4Computations, puEnSgp4$sgp4Computations) / 50000) * 50000
print("maxSgp4Computations:")
print(maxSgp4Computations)

# create matrix for grouped bar plot
data <- matrix(c(puDisLast$sgp4Computations, puEnLast$sgp4Computations, puDisLin$sgp4Computations, puEnLin$sgp4Computations, puDisNear$sgp4Computations, puEnNear$sgp4Computations, puDisSgp4$sgp4Computations, puEnSgp4$sgp4Computations),
              nrow=2, ncol=4, byrow=FALSE)
colnames(data) <- c("last position", "lin. interpol.", "NPP", "SGP4")
rownames(data) <- c("baseline", "optimized")
print("data:")
print(data)

# plot grouped bar plots

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

done <- myps(file = args[9], width=paper.width, height=paper.height)
plot.new()
par(mar=c(3.25,4.5,.5,.75), mgp=c(3.0, 0.5, 0))
plot.window(xlim=c(0,1), ylim=c(0,maxSgp4Computations), xaxs="i", yaxs="i")
barplot(data, beside=TRUE, ylim=c(0, maxSgp4Computations), axes=FALSE, legend.text=TRUE, args.legend=list(bty="n", x="topleft"))
box()
axis(2, las=1)
title(xlab="Config", line=2.2)
title(ylab="Number of updates", line=3)
done()