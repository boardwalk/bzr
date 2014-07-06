#!/bin/sh
TOOL='../rust/main'
DATFILE='../../data/client_portal.dat'

$TOOL $DATFILE ls \
    | grep -E '^2[57]' \
    | cut -f 1 -d ' ' \
    | xargs -d '\n' -n 1 \
    sh stringsinfo25-inner.sh

