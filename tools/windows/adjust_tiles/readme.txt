This tool will read some ranges and destinations from a text file and adjust the cells in a graphical tileset.

Usage:
The text file should have one range and destination per line. Each line should have the format:
0x80:0x81 0x80:0xFD 0x80:0x83
The seperators can be any character, since the actual character is not checked, only offset from read numbers.
Any line that starts with a '#' or a ';' will be ignored.
There should not be any whitespace in front of the first character of the line.
The above sample means move the pref file coordinates for any entry that uses a tile between the 2nd and 3rd to last tiles two tiles to the right.

If the fifth number of a line is less than 128, then the program will shift the cells in the range up or down by the given number of pixels. This shifting happens after any cells are moved with the main syntax.


After the program start, three open file dialog boxes will come up. The first is used to select the text file with the adjustment ranges and destinations. The second is used to select the pref file to adjust. The third dialog box selects a filename to write the adjusted file to. The output file name should be different than the input file name.

This program uses the same format as the adjust_pref program and can use the same file. If the adjust file has P: lines, they will be ignored.

Compiling:
If you want to compile this program, then you will need some files from tilesetpref/tools/console/tile_common in addition to the winmain.cpp from this directory. You will need angband.h, h-basic.h, and all of the files that start with x or z.  You should be able to build the program with whatever IDE or build system you are comfortable with.
