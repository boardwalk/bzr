#!/bin/sh
TOOL='../rust/main'
DATFILE='../../data/client_portal.dat'

$TOOL $DATFILE cat $1 | python imageinfo.py

