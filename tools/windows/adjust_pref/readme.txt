This tool will read some ranges and destinations from a text file and adjust pref files for graphical tilesets.

Usage:
The text file should have one range and destination or one P: per line. Each range line should have the format:
0x80:0x81 0x80:0xFD 0x80:0x83
The seperators can be any character, since only that the numbers are seperated by a single character matters.
P: lines should have the format: P:<input>:<output>, with no spaces. On these lines, the seperator must be a colon (:).
Any line that starts with a '#' or a ';' will be ignored.
There should not be any whitespace in front of the first character of the line.
With the above sample, pref file entries that uses a tile in the range would use a tile two tiles to the right of the original. The range is the second tile from the left to the third tile from the end, on the first row.

After the program start, it check if adjust.txt exists. If it does, it asks to make sure that you want to use it. If no is selected, or if adjust.txt does not exist, an open file dialog box will come up to select the file with the adjustment ranges and destinations. If the adjustment file contains P: lines (input/output file pairs), they will be used. Multiple p: lines may be used. If no P: lines were found, two dialog boxes will come up. The first is used to select the pref file to adjust. The second dialog box selects a filename to write the adjusted file to. The output file name should be different than the input file name.

As the program writes the output file it will also write a log with which pref file entries are changed. The name of the log file is based on the output file name.

Compiling:
If you want to compile this program, then you will need some files from tilesetpref/tools/console/tile_common in addition to the winmain.cpp from this directory. You will need angband.h, h-basic.h, and all of the files that start with x or z.  You should be able to build the program with whatever IDE or build system you are comfortable with.
