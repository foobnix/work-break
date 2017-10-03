#!/bin/bash

WB=/home/ivan-dev/git/work-break
NAME="$((1 + $RANDOM % 1000000))"
cp $WB/work-break /tmp/$NAME

#fakes
cp $WB/work-break /tmp/$((1 + $RANDOM % 1000000))
cp $WB/work-break /tmp/$((1 + $RANDOM % 1000000))
cp $WB/work-break /tmp/$((1 + $RANDOM % 1000000))
cp $WB/work-break /tmp/$((1 + $RANDOM % 1000000))

cmd="/tmp/$NAME"
echo $cmd
eval $cmd