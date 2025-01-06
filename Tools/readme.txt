======================================================
              Grandia 2 Translation Tool 0.51
                        by Mat
                      16/03/2007
======================================================
------------------------------------------------------
0 - INDEX 
------------------------------------------------------

1 - Intro
2 - Usage
3 - Technical Notes
4 - Progress
5 - To do
6 - Thanks
7 - Disclaimer


------------------------------------------------------
1 - INTRO
------------------------------------------------------
I wrote these programs in a time period starting almost 10 months ago.
They allow you to translate Grandia 2 WITHOUT text expansion (for now).
I made this package without modifying the original source, I only updated credits.

This package is composed by these programs (they're all tools to be used with the DOS prompt. To visualize their help information, just start them without parameters).

-extAFS    extracts AFS archives and allows to recreate them using extracted (and edited) files
-extL62C   decompresses and compresses compressed files with L62C (an altered LZSS) (sorry for the words joke :P )
-extPVMH   extracts PVHM archives (they contain PVR images) - no recompression
-g2gc      converts menu images from PVR to RAW (and viceversa)
-g2split   splits Grandia 2 files (to reassemble theme use copy /b file1+file2+etc..)
-g2fd      converts font file in RAW
-GR2dump   experimental dumper for the text, currently the expansion makes the game crash.
 It has been tested on a single piece extracted from the file 01 of the 2000.afs (see 01.rtf: Size of Text e Text Pointer) and by rebulding afterwards the original file. GR2dump 0.1Beta9a manages a greater number of opcode but however is not free from crash.


If you need other information, please contact me (mattia.d.r@libero.it).


------------------------------------------------------
2 - USAGE
------------------------------------------------------

This is a command line tool used with the DOS
prompt (Start\Programs or Start\Programs\Accessories).

Write the program name without arguments to show help information.

How to extract text:
- Extract archive contents in a directory with AFS extension.
- Try to decompress files by using extL62C.
- Now, analyze the obtained files, text is usually in the first.
- Translate text (without expansion, at this moment).
- Recompress file(s) by using extL62c.
- Recreate archive by using extAFS.

NOTICE: a small part of the text isn't compress.

How to translate graphics:
- Font is contained in Eng24.fnt, file located in the game main directory, to edit it you have only to use g2fd. 
  Since the game utilizes a VWF (Variable Width Font), you will also have to hack the .exe
  Doing this, you'll be able to create DTE, and this will allow you to have more space.
- Some files extracted from AFSs have as header GBIX or PVRT, to modify them you have only to use the Dreamcast plug-in for Photoshop.
  Other files are not supported, but you can use g2gc, that will convert them in raw files (and, of course, allows to bring them back to their original form in gbix).
- We found that some PVHM archives contains other game graphics.
  They can be extracted with extPVHM; anyway, we did not found until now graphics to translate in these files.
- Some files are archives. If you'll notice that they have an header like GBIX or L62C, it's possible to use g2split to split them.
  Howewer, as above, we didn't find something to translate in these.


------------------------------------------------------
3 - TECHNICAL NOTES
------------------------------------------------------

There would be a lot to write, and I don't want. :P
I'll only say that the exe doesn't allow to utilize
all the 256 font's characters, so Chop (a SadNES cITy's member)
edited it to allow this.
Howewer, for now, I will not publish it.

------------------------------------------------------
4 - PROGRESS
------------------------------------------------------

Version 0.51 (16/03/2007)

- GR2dump 0.1Beta9a

Version 0.5 (11/08/2004)

- First public release


------------------------------------------------------
5 - TO DO
------------------------------------------------------

- Bugs Correction
- Text expansion 

------------------------------------------------------
6 - THANKS
------------------------------------------------------

^Anyone who will use or publish this program :)
^mentz
^Gemini
^Ombra
^Chop
^I figli di Gaucci


------------------------------------------------------
7 - DISCLAIMER
------------------------------------------------------

This program must be used only for LEGAL intents!
Use this program to your risk and danger :).
This program can be freely distributed. I only 
recommend to distribute it in the state in 
which it is (don't modify the readme or something else).
Thanks.

Grandia 2 and the relative names are trademarks of the
relative owners.

Mat
mattia.d.r@libero.it
http://www.matzone.altervista.org/
Member of SadNES cITy: http://www.sadnescity.it/
