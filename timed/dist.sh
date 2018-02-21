#!/bin/sh

COMMON="../COPYING doc/chrstuff.zip doc/advanced.cfg doc/timed.cfg doc/readme.txt doc/changes.txt doc/timed.txt"
ICONSW="doc/icons/windows/timed.ico doc/icons/windows/timed2.ico"
ICONSO="doc/icons/os2/timed.ico doc/icons/os2/timed2.ico"

mkdir -p dist
rm -f dist/tim*.zip
zip -jovX dist/tim112.zip $COMMON $ICONSW timed.exe fdbpack/fdbpack.exe doc/dos/file_id.diz
zip -jovX dist/tim112x.zip $COMMON $ICONSW tim386.exe dos4gw.exe dos4gw.doc fdbpack/fdbpack.exe doc/d386/file_id.diz
zip -jovX dist/tim112p.zip $COMMON $ICONSO timedp.exe fdbpack/fdbpackp.exe doc/os2/file_id.diz
zip -jovX dist/tim112n.zip $COMMON $ICONSW timednt.exe fdbpack/fdbpackn.exe doc/nt/file_id.diz
