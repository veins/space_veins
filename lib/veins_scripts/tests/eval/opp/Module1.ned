simple Module1
{
    parameters:
        @signal[signal1](type=int);
        @statistic[statistic1](source=signal1; record=timeavg, stats, histogram, vector);

        int par1;
}
