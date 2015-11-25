#!/bin/bash
PORT=$1
DEST=$2
SCRIPT=$3
DATFILE=$4
export DMTCP_PORT=$PORT
export M5_PATH=/home/dinos/x86/

dmtcp_checkpoint ./build/X86/gem5.opt -r -d $DEST --debug-flags=FaultInjection configs/example/fs.py -b $SCRIPT  --repeat-switch=1 --switch-on-fault=1 --caches -M 0

