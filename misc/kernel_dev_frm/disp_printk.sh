#!/bin/sh

dmesg -C

while [ 1 -lt 10 ]
do
    dmesg -c
    usleep 5 
done
