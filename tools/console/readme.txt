There are three tools in this directory.
The first is tile_split.  This tool uses a pref file to split a tileset into individual components.

The usage of this tool is tile_split {-option} <pref_file> <tile_directory> <tileset_file>.  

option may be either "y" or "n".  If no option is given the tool will ask whether or not to write tile files for entries that use the same tile as earlier entries.  If the option is y, the tool will always write the files for duplicate entries.  If the option is anything else, the tool will not write a file.  

To compile this tool, use the main file in tile_split and all of the files in tile_common.


The second tool is tile_build, which does the opposite of the first tool.  This tool uses a pref file to build a tileset from individual components.  
The usage of this tool is tile_build {-option} <pref_file> <tile_directory> <tileset_file>.

option may be "w", "l", or "n".  These options determine what the tool will always do when there is a tile file for an entry that uses the same tile spot in the set as an earlier entry.  If "-w" is used, the tool will overwrite the spot with the file from the duplicate entry.  If "l" is used, the tool will pick an unused spot in the tile set to use for the duplicate entry.  If anything else is used for the option, no change will be made for the duplicate entries.  If no option is given, the tool will ask what to do for each duplicate entry.

To compile this tool, use the main file in tile_build and all of the files in tile_common.

If a previously unused spot is picked, the tool will show a message with the entriy's name and the coordinates of the new spot at the end of the program.


The third tool is png_premultiply_alpha.  This tool will multiply the foreground color by the transparency color.  A black color where a tile is fully transparent is needed by the masking routine for the windows platform.  If partial transparency is used, then the premultiplied color is needed by the windows platform and seems to be needed by the mac OSX platform.

The usage of this tool is png_premultiply_alpha {-b} <input_file> <output_file.  If -b is used, this tool will also change any pixel with partial transparency to no transparency.

To compile this tool, use the main file (png_premultiply_alpha.c) in png_premultiply_alpha and png_file.c in tile_common.

Libpng is needed for all three tools. Libpng may be obtained from the src/win ddirectory of the Angband source or from www.libpng.org.

A bit about the files in tile_common:
The following files are unchanged from the Angband source and came from the June 19th Nightly:
h-basic.h, z-form.c, z-form.h, z-type.h, z-util.c, z-util.h, z-virt.c, z-virt.h

Some files from the Angband source have minor changes:
z-type.c has a few includes that it doesn't need commented out.
x-char.c has a table that it needs that was copied from tables.c.
x-char.h has some function declarations copied from externs.h

Some files from the Angband source have a lot of changes and have "_b" in their file names:
z-term_b.h - everything was removed except the hook that x-char.c refers to
z-file_b.c and .h - all references to the ang_file structure were replaced by pointers to a FILE structure.
angband.h - changed to only include header files in the tile_common directory.

The other files in the tile_common directory are files I created for the tools and all use the typical Angband dual license. (so do the main files.)