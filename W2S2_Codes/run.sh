#!/bin/bash

interval=1
if [ $# -gt 3 ]
then
    interval=$4
fi

for (( c=$2; c<=$3; c+=$interval ))
do
    $1 <<< "$c"$'\n' 
done

