#!/bin/sh
TOOL='../rust/main'
DATFILE='../../data/client_portal.dat'

$TOOL $DATFILE ls \
    | grep -E '^22' \
    | cut -f 1 -d ' ' \
    | xargs -d '\n' -n 1 \
    sh stringsinfo22-inner.sh

