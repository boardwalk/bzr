#!/bin/sh
TOOL='../rust/main'
DATFILE='../../data/client_cell_1.dat'

$TOOL $DATFILE ls \
    | grep -E '^....ffff' \
    | cut -f 1 -d ' ' \
    | xargs -d '\n' -n 1 \
    sh landblockinfo-inner.sh
