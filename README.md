
Date: November 20, 2000
Author: Gerard van Essen (art@artware.net)


Source code release for Artware products under GPL license
----------------------------------------------------------

First of all: all the source created by me is now distributed under
the GPL license (included in the file Copying also in this package).
Please note, that some of the code contained in this package is NOT
made by me, and therefor DOES NOT CARRY THE GPL LICENSING. (The code
in the \exec directory, for example, is NOT GPL).

In this package you will find the source code for the products released
by Artware in the past. Somewhere in the package you will be able to
find source for timEd, NetMgr, Jaminfo and WIMM.

Because it appears there is still at least one person around who wants
me to release this, I am now releasing the sources for my products.
Please note, that these products contain several serious y2k bugs!
If you manage to compile them, you still can not use them safely!

There are two .zip files in this package. They are the original zipfiles
I found on my backup CD from that period. I have deleted some files
from the zip files with personal stuff and some stuff that was
unnecessary. Apart from that, you get the exact mess I used to work
with!

The file tcsrc.zip contains the d:\tc\ tree that existed on my system
at the time. It had several subdirs like d:\tc\api (for the msgapi),
d:\tc\netmgr, d:\tc\wimm etc.

The file timdev.zip contains what existed in my d:\timed directory at
the time. It obviously contains the timEd sources.

As I mentioned, those are the original backup files. I have not touched
them. In most cases there exist dos.bat, 386.bat and os2.cmd batchfiles
to compile the source for that particular platform. All with hardcoded
paths etc. So they will definitely not work on your system.

The compiler I used was Watcom 10.6 (and an older version of Watcom's
linker for the DOS version of timEd. I think I remember it didn't
work with Watcom's 10.6 linker).
Borland's Turbo Assembler was used to compile the assembler files
found in the package.

There are two versions of the msgapi (directory \api and \msgapi).
I included both, because I am not sure what the differences are. The
files in \api seem a bit more recent, with some changes to make the
stuff compile on Linux (with GCC). I remember fiddling with it at the
time. Your best bet might be to use the code in \msgapi, because I
have most likely severely messed up the code in \api to make it
compile (note that I do not say: work) on Linux.

Please note that the msgapi is based on Scott Dudley's original msgapi,
but severy hacked to add JAM and Hudson support and some other
extensions (I think?! :-).

What is NOT included is the code that was contained in \database on my
system. It contained the Mix C Databsse Toolchest. It is a commercial
library that I used for the V7 nodelist browsing in timEd. The license
for that library does not allow me to distribute the source code.

It seems Mix Software is still alive, I found them on the web at
http://www.mixsoftware.com. They even still have the (a?) C Database
Toolchest on sale for USD 50 (source included). The version I have
is 2.0.1. Whether or not this is still the same code/format they are
using now, I don't know..


!!!!! PLEASE NOTE:
------------------

Also contained in the package is a directory with Thomas Wagner's
swapper library. THAT IS NOT MY CODE! There is a DOC included in the
package with more info. I have left it in the package to aid people
who want to mess around with the stuff, as it is required to compile
the DOS versions of NetMgr and timEd. I don't know if I ever made
modifications to that package. I think not, because my assembler
knowledge was very, very limited.

--

That's about it, really. It was quite a few years ago that I messed
around with this stuff. Still, if you have any questions feel free
to mail me at art@artware.net. Who knows, I may have the answers to
your questions hidden somewhere in my grey cells.
I am currently connected to FidoNet again due to the swift actions of
Dallas Hinton who set me up as a point off his sytem, so I can also
be reached in the Artware echomail area for now.

