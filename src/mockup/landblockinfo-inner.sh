#!/bin/sh
TOOL='../rust/main'
DATFILE='../../data/client_cell_1.dat'

$TOOL $DATFILE cat $1 | python landblockinfo.py

