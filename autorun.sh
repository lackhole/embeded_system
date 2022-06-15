#!/bin/bash

while :
do
    t="`TZ=":ROK" date +%Y_%m_%d_%H:%M:%S:%2N`"
    log_file="log_${t}.txt"
    cmake -B build
    cmake --build build -- -j 4
    ./build/watcher lackhole.com 7000 >> ${log_file} 2>&1 &
    wait
done
