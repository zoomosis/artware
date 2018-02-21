#!/bin/sh

COMMON="../COPYING doc/chrstuff.zip doc/advanced.cfg doc/timed.cfg doc/readme.txt doc/changes.txt doc/timed.txt"

mkdir -p dist
rm -f dist/tim*.zip
zip -jovX dist/tim112.zip $COMMON timed.exe fdbpack/fdbpack.exe doc/dos/file_id.diz
zip -jovX dist/tim112x.zip $COMMON tim386.exe dos4gw.exe dos4gw.doc fdbpack/fdbpack.exe doc/d386/file_id.diz
zip -jovX dist/tim112p.zip $COMMON timedp.exe fdbpack/fdbpackp.exe doc/os2/file_id.diz
zip -jovX dist/tim112n.zip $COMMON timednt.exe fdbpack/fdbpackn.exe doc/nt/file_id.diz
