#!/bin/sh

TOOL='../rust/main'
DATFILE='../../data/client_portal.dat'

$TOOL $DATFILE cat $1 | python stringsinfo22.py
#$TOOL $DATFILE cat $1 > $1.dat
