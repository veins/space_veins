# Veins scripts

This is a collection of scripts to make using Veins easier.

- Scripts in the `running/` subdirectory deal with running simulations.
- Scripts in the `eval/` subdirectory deal with result evaluation.

## running/

### `generateRunsFile.pl`
runs `./run -a` in the current directory and outputs all possible combinations of `./run -u Cmdenv -c X -r Y` lines (where `X` are all configurations and `Y` are all runs).
Output is formatted for use with runmaker4.py (see <https://github.com/veins/runmaker>).

For example, given an `.ini` file that has a configuration section `[Config WithBeaconing]` which configures the simulation for 6 runs (e.g., 3 repetitions of 2 parameter values each), the following call and output might correspond:

```
$ generateRunsFile.pl | grep WithBeaconing
. ./run -u Cmdenv -c WithBeaconing -r 2
. ./run -u Cmdenv -c WithBeaconing -r 1
. ./run -u Cmdenv -c WithBeaconing -r 4
. ./run -u Cmdenv -c WithBeaconing -r 3
. ./run -u Cmdenv -c WithBeaconing -r 0
. ./run -u Cmdenv -c WithBeaconing -r 5
```

## eval/

### `opp_vec2longcsv.sh`

Converts an OMNeT++ `.vec` file into a long-format CSV.
For example, given a file `results/output.vec` as follows...
```
version 2
run c1-0
attr configname c1
param *.prio 7

vector 0 net.node[0].mob posx ETV
vector 1 net.node[0].mob posy ETV
vector 2 net.node[1].mob posx ETV
vector 3 net.node[1].mob posy ETV
0	101	1	10
0	102	2	11
0	103	3	12
1	101	1	21
1	102	2	22
1	103	3	23
2	102	2	10
2	103	3	11
3	102	2	21
3	103	3	22
```
...the following call and output will correspond:
```
$ ./opp_vec2longcsv.sh results/output.vec
0 net.node[0].mob posx 101 1 10
0 net.node[0].mob posx 102 2 11
0 net.node[0].mob posx 103 3 12
1 net.node[0].mob posy 101 1 21
1 net.node[0].mob posy 102 2 22
1 net.node[0].mob posy 103 3 23
2 net.node[1].mob posx 102 2 10
2 net.node[1].mob posx 103 3 11
3 net.node[1].mob posy 102 2 21
3 net.node[1].mob posy 103 3 22
```
That is, each value of each vector will be output on a separate line, with each line containing

- vector ID
- module path
- vector name
- event number
- time
- value

### `opp_vec2csv.pl`

Converts an OMNeT++ `.vec` file into a wide-format CSV.
For example, given the same vector file as above, the following call and output might correspond:
```
$ ./opp_vec2csv.pl --merge-by em -A configname -P "*.prio" -F posx -F posy results/output.vec
event	time	node	configname	*.prio	posx	posy
101	1	net.node[0].mob	c1	7	10	21
102	2	net.node[0].mob	c1	7	11	22
103	3	net.node[0].mob	c1	7	12	23
102	2	net.node[1].mob	c1	7	10	21
103	3	net.node[1].mob	c1	7	11	22
```
That is, each line contains all requested vector values (here, `posx` and `posy` belonging together (defined by a merge criterion, here: `e`vent number and `m`odule path), along with arbitrary attributes (here: `configname`) and parameters (here: `*.prio`).

### `opp_sca2longcsv.sh`

Converts an OMNeT++ `.sca` file into a long-format CSV.
For example, given a file `results/output.sca` as follows...
```
version 2
run c1-0
attr configname c1
param *.prio 7

scalar net.node[0].app sent 1
scalar net.node[0].app recvd 2
scalar net.node[1].app sent 10
scalar net.node[1].app recvd 20
```
...the following call and output will correspond:
```
./opp_sca2longcsv.sh results/output.sca
net.node[0].app sent 1
net.node[0].app recvd 2
net.node[1].app sent 10
net.node[1].app recvd 20
```

### `opp_sca2csv.pl`

Converts an OMNeT++ `.sca` file into a wide-format CSV.
For example, given the same scalar file as above, the following call and output will correspond:
```
./opp_sca2csv.pl -f sent -f recvd results/output.sca
nod_name	sent	recvd
net.node[0].app	1	2
net.node[1].app	10	20
```

### `opp_csvsample.pl`

Generates one-out-of-n samples from `.csv` files.

### `opp_csvcat.pl`

Concatenates `.csv` files (essentially: concatenates multiple files while printing the header line only once)



That's it!

