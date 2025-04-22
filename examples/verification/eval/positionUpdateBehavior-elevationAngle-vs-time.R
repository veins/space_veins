require(data.table)
require(dplyr)
require(tidyverse)

print(sessionInfo())

args = commandArgs(trailingOnly=TRUE)
# if (length(args) < 4) {
#     stop("Please provide four csv files!")
# }
print(args)

minTime <- 1902.5
maxTime <- 1906

# read in elevation angle data
# filter based on simulation time
eleAngle <- fread(args[1])
eleAngle <- subset(eleAngle, time >= minTime)
eleAngle <- subset(eleAngle, time <= maxTime)
eleAngle <- subset(eleAngle, select = -c(node, repetition))
# -------- test ------
# eleAngle <- add_row(eleAngle, event=1, time=1899.791, lastElevationAngleToSop=9)
print("eleAngle:")
print(eleAngle)

# read in getCurrentPosition data
# filter based on simulation time
gcp <- fread(args[2])
gcp <- subset(gcp, time >= minTime)
gcp <- subset(gcp, time <= maxTime)
gcp <- subset(gcp, select = -c(node, repetition))
gcp <- eleAngle %>% inner_join(gcp, by=c('event', 'time'))
# print("gcp:")
# print(gcp)
# -------- test ------
# gcp <- add_row(gcp, event=1, time=1899.791, lastElevationAngleToSop=9, getCurrentPositionUpdates=1)
print("gcp:")
print(gcp)

# read in periodicUpdates data
# filter based on simulation time
pu <- fread(args[3])
pu <- subset(pu, time >= minTime)
pu <- subset(pu, time <= maxTime)
pu <- subset(pu, select = -c(node, repetition))
pu <- eleAngle %>% inner_join(pu, by=c('event', 'time'))
print("pu:")
print(pu)

# read in E2E latency data
# filter based on simulation time
e2e <- fread(args[4])
e2e <- subset(e2e, time >= minTime)
e2e <- subset(e2e, time <= maxTime)
e2e <- subset(e2e, select = -c(node, repetition))
print("e2e:")
print(e2e)
# -------- test ------
# e2e <- add_row(e2e, event=2, time=1899.791, endToEndDelay=0.009)
# print("e2e:")
# print(e2e)

# identify current elevation angle while the packet was received
if (unlist(strsplit(args[5], '/'))[4] == "VerifyDisableUpdatePositionOnlyAboveHorizonLastPosition") {
    e2e$floorTime <- floor(e2e$time)
    e2e <- eleAngle %>% inner_join(e2e, by=c('time' = 'floorTime'), suffix=c('.ele', '.e2e'))
    e2e$time <- NULL
    names(e2e)[names(e2e) == 'time.e2e'] <- 'time'
}
if (unlist(strsplit(args[5], '/'))[4] == "VerifyDisableUpdatePositionOnlyAboveHorizonLinearInterpolationPosition") {
    e2e <- eleAngle %>% inner_join(e2e, by=c('time'), suffix=c('.ele', '.e2e'))
}
if (unlist(strsplit(args[5], '/'))[4] == "VerifyDisableUpdatePositionOnlyAboveHorizonNearestPeriodicPosition") {
    e2e$floorTime <- floor(e2e$time)
    eleAngle$floorTime <- floor(eleAngle$time)
    e2e <- eleAngle %>% inner_join(e2e, by=c('floorTime'), suffix=c('.ele', '.e2e'))
    names(e2e)[names(e2e) == 'time.e2e'] <- 'time'
}
if (unlist(strsplit(args[5], '/'))[4] == "VerifyDisableUpdatePositionOnlyAboveHorizonSgp4Position") {
    e2e <- eleAngle %>% inner_join(e2e, by=c('time'), suffix=c('.ele', '.e2e'))
}
print("eleAngle:")
print(eleAngle)
print("e2e:")
print(e2e)
# names(tmp)[names(tmp) == 'time'] <- 'time.e2e'
# print("tmp")
# print(tmp)
# search for matches within periodic updates
# e2e$floorTime <- floor(e2e$time)
#e2e <- bind_rows(tmp, pu %>% inner_join(e2e, by=c('time' = 'floorTime'), suffix=c('.pu', '.e2e')))
# e2e <- bind_rows(tmp, pu %>% inner_join(e2e, by=c('time' = 'floorTime'), suffix=c('.pu', '.e2e')))
# e2e[is.na(e2e)] <- 0
# e2e <- subset(e2e, select = c(time.e2e, time, lastElevationAngleToSop, getCurrentPositionUpdates, periodicUpdates, endToEndDelay))
# print("e2e")
# print(e2e)

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
legends = list(periodic=list(lwd=lwds[["default"]], pch=pchs[["circle"]], label= expression("periodic update")),
               received=list(pch=pchs[["cross"]], label= expression("packet received")),
               gcp=list(pch=pchs[["diamond"]], label= expression("getCurrentPosition update")))

done <- myps(file = args[5], width=paper.width, height=paper.height)
plot.new()
par(mar=c(3.25,3.0,.5,.75), mgp=c(3.0, 0.5, 0))
plot.window(xlim=c(minTime + 1, maxTime - 0.9), ylim=c(7.9 ,8.2), xaxs="i", yaxs="i")
lines(eleAngle$time, eleAngle$lastElevationAngleToSop, type = "s", lwd = legends[["periodic"]][["lwd"]])
points(pu$time, pu$lastElevationAngleToSop, pch = legends[["periodic"]][["pch"]])
points(gcp$time, gcp$lastElevationAngleToSop, pch = legends[["gcp"]][["pch"]])
points(e2e$time, e2e$lastElevationAngleToSop, pch = legends[["received"]][["pch"]])

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
       legend=c(legends[["periodic"]][["label"]], legends[["gcp"]][["label"]], legends[["received"]][["label"]]),
    #    lwd=c(legends[["periodic"]][["lwd"]], legends[["horizontal"]][["lwd"]]),
       pch=c(legends[["periodic"]][["pch"]], legends[["gcp"]][["pch"]], legends[["received"]]),
       bty="n",
       ncol=1)
done()