# Runmaker4

Runmaker is an extremely simple job scheduler.

All state tracking is performed via one single `.txt` file.
In the given file, each line beginning with a dot and a space (`. `) will be
executed. The file is modified to reflect the execution state of each job
(`r`-running, `d`-done, `!`-failed, `e`-error).

One version (`runmaker4.py`) performs all communication and synchronization via a shared filesystem supporting `fcntl()` advisory record locking, such as NFSv3.
Therefore, no dedicated server is needed.

An alternative version (`runmaker4-server.py` and `runmaker4-client.py`) performs synchronization via a simple TCP connection.
Here, no shared file system is needed.

## Preparation

Create a file `runs.txt` containing the following lines (note the first two characters (a dot, followed by a space):
```
. echo a; sleep 60s; echo A
. echo b; sleep 60s; echo B
. echo c; sleep 60s; echo C
. echo d; sleep 60s; echo D
```
These four lines represent four command lines to be run (each printing a letter, waiting a minute, then printing the same letter in upper case).


## Usage (shared filesystem)

Open two terminals, potentially on two different computers (say, `alice` and `bob`).
In each, run
```
./runmaker4.py -j2 runs.txt
```

Output similar to the following should then appear in the first terminal:
```
executing `echo a; sleep 60s; echo A'
executing `echo b; sleep 60s; echo B'
status (alice,601): forked "echo a; sleep 60s; echo A"
status (alice,602): forked "echo b; sleep 60s; echo B"
stdout (alice,601): a
stdout (alice,602): b
```
followed (after a minute) by
```
stdout (alice,601): A
stdout (alice,602): B
status (alice,601): exit 0 "echo a; sleep 60s; echo A"
status (alice,602): exit 0 "echo b; sleep 60s; echo B"
```

In parallel, you should see the first character of each line changing from `.` (new) to `r` (running) and then `d` (done).


## Usage (TCP connection)

Open two terminals, potentially on two different computers (say, `alice` and `bob`).

On machine `alice`, open a terminal and run
```
./runmaker4-server.py runs.txt
```

Output similar to the following should then appear in the first terminal:
```
Token for runmaker4-client.py: 000000 (written to /home/user/.runmaker4.token)
```
Take note of the token value (here, `000000`).

On machine `bob`, open a terminal and run
```
./runmaker4-client.py -j2 --token 000000 alice
```

You should see the same output as above.

## More options

Runmaker4 can also collect output (stdout and stderr) from all processes in a single log file (note that this file must already exist):
```
touch outputs
./runmaker4.py -l outputs runs.txt
```

This will create a file `outputs` with content similar to the following:
```
.-> echo a; sleep 60s; echo A (in /home/user/runmaker)
: stdout (alice,601): a
: stdout (alice,601): A
+ status (alice,601): exit 0 "echo a; sleep 60s; echo A"
.-> echo b; sleep 60s; echo B (in /home/user/runmaker)
: stdout (alice,602): b
: stdout (alice,602): B
+ status (alice,602): exit 0 "echo b; sleep 60s; echo B"
.-> echo c; sleep 60s; echo C (in /home/user/runmaker)
: stdout (bob,601): c
: stdout (bob,601): C
+ status (bob,601): exit 0 "echo c; sleep 60s; echo C"
.-> echo d; sleep 60s; echo D (in /home/user/runmaker)
: stdout (bob,602): d
: stdout (bob,602): D
+ status (bob,602): exit 0 "echo d; sleep 60s; echo D"
```


Runmaker4 also comes with two small helper scripts:

### runset4.py
This script can be used to programmatically modify the `runs.txt` file in place (many text editors can/will not do that, instead replacing the file with a new copy, which then won't be used by already-running processes).
It can be used as follows:

```
./runset4.py --all --set=. runs.txt
```

### runwait4.py
This script can be used to wait for all jobs to finish, optionally printing progress.
It can be used as follows:


```
./runwait4.py --progress runs.txt && sendmail [...]
```

This would print output like the following (here: 2 jobs done, 1 job running, 1 job remaining) and, when all jobs are finished send an e-mail.

```
progress:   2 of   4 jobs processed, 0 errors [========>>>>    ]
```

That's it!
