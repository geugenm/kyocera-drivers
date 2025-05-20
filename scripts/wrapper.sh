#!/usr/bin/env bash

jobname=$(echo $3 | egrep -o '[[:alnum:]]' | tr -d "\n" | tail -c 20)
/home/e_gleba/job/kyocera-drivers/build/debug/src/rastertokpsl "$1" "$2" "$jobname" "$4" "$5"
