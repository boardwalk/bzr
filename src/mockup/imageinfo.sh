#!/bin/sh
TOOL='../rust/main'
DATFILE='../../data/client_highres.dat'

$TOOL $DATFILE ls \
    | grep -E '^06' \
    | cut -f 1 -d ' ' \
    | xargs -d '\n' -n 1 \
    sh imageinfo-inner.sh

