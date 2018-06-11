#!/bin/sh
VERFILE=$1
LOCALVER=`git rev-list HEAD | wc -l |awk '{print $1}'`
VER=$LOCALVER
echo {\"gitver\":$VER}
