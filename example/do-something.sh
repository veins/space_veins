#!/bin/bash

LABEL="$1"

echo "process $LABEL starting"
for i in `seq 1 50`
do
	echo "process $LABEL at step $i"
	sleep .1s
done
echo "process $LABEL done"
