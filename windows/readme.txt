The executable should be copied into the top level of Z+ or Angband. In other word it should
be placed into the same directory as the lib directory.

When the program first starts, it will look for the f_info.txt or terrain.txt file
in ./lib/edit. If it does not find it, it will ask you to pick a file from the edit
directory. It does not matter which file. This is only for the directory. If it does
not find the info files there, it will exit.

After the info files have loaded, and the list is shown, it is ready to work.

    Wrote a tile picker. can load the working tiles and pref file, also tiles and 
        pref file for 2 placeholder sources.
      matches hard coded strings in the tile filename "32x32", "16x16", 8x8" to 
        get the tile size. can also parse filename of format XxY.bmp. only looks
        for mask in XxYmsk.bmp.
      can pick a cell to assign an item from the working tiles. the top left cell is
        considdered to be not assigned.
      can mark an item as using placeholder artwork, so it is listed in a seperate
        list. "^" is the placeholder character and can appear anywhere in the
        comments for that item. either on the same line as the item, after the item
        or the whole line before the item. item must be assigned a cell to be
        considered to have placeholder artwork.
      can view the tiles and copy a tile and its mask from a source to the working
        set. from p1, pick, place, and mark that its a placeholder. From p2, just place.
      can swap tiles, including masks, with the one left of it, and save it in
        the working tiles. (Ctrl-P)
      can middle click on a tile to select what uses it in the main window.
        (left click to center the display on the click spot, right click to
        select a tile to be assigned or overwritten by a placeholder.)
      can save a 32bit png with transparency by clearing the mask name before
        the save as command, or by selecting "no, then "cancel" when it asks
        to confirm/pick the mask name during a "save as" command.
      *edit note is not working. notes are in parentheses after the comments
        on the same line or the previous one in the pref file. "# name (note)"
        notes are everything between the first open and the last close paren,
        *so need to rename items that have parentheses in their names,
        otherwise the part of the name in parens is taken to be the note, and
        keeps being added on repeated load-save-quit cycles. or change to brackets
      *need to be able to type in the coordinates.
      can load/save Angband info/pref files. "Angband" or "angband" must
        appear somewhere in the path of what is selected in the initial open
        dialog box. Possibly a dummy file in the edit directory.
      can load DaJAngband info/pref files. save not tested. "DaJAngband" must
        appear somewhere in the path of what is selected in the initial open
        dialog box. Possibly a dummy file in the edit directory.
      for the above two lines, it looks for ./lib/edit/terrain.txt. If found
        looks for the game name in the path, and does not present the initial
        open dialog box. Also, if you are working on pref files for one game,
        pref files for another should not be loaded, as only index numbers are
        used, names are only for the lists.
      loads flags from an ini file that tells how to load/save pref files, and
        which file names to use. loadflags = 0 for z+angband, 847 for angband.
        the ini file is written on program close. if the ini file does not
        exist, it uses flags determined by a game name found in the path.
      uses DierctX9 to load/save BMP files, load PNG files. Libpng is used to
        save PNG files.

To split a png file with transparency to seperate color and mask files: 
1. go to File -> Load Tiles to pick the image file to load
    (In the future if you already have a mask file, it will be loaded automatically
     if it has the right name.)
2. Wait for the file to load.  A message will pop up when it has finished loading.
    If it was not able to load a mask, it probably is a format I did not expect or 
    transparency is not present..
3. Add a mask filename. Select File -> Std Mask Name. This sets a default mask file
    name based on the loaded file name. (if the file name is [name].png, the default
    mask file name is [name]msk.png.)
4. Make sure the transparency is loaded. select an item from the list, it does not
    matter which one. Then press the "button" called center in the top right of the
    window. another window will come up showing the color portion of the loaded
    image. while this window is active, you can left clickto move around, and press
    Ctrl-T to switch to viewing the mask and back again. There will be a yellow or
    red box in the top left corner, this is the selction box, which you do not need
    to use for this.
5. Select File -> Save Tiles As. When the "save as" dialog box comes up, change the
    extension to bmp, maybe change the filename, then click OK.
6. A message will pop up asking if you want to save the mask to a standard mask
    name based on the filename you just selected. You probably want to click yes here.
    If you click no another "save as" dialog box will come up where you can pick the
    mask filename. If you click cancel in the "save as" dialog box, the mask file
    name will be cleared and the program will try to save both the color and mask
    in a 32 bit file with the step 4 filename.
7. Wait for the save to complete. A message will pop up saying if the save
    finished or failed.
8. Make sure the files were written correctly by checking them in another program.
9. Both the color and mask files are 24 bit. For the mask file, this wastes alot
    of space. In another program, convert the mask file to 1 bit or 256 bit grey
    scale, or whatever you prefer.
10. Done :)

Some tiles in the terrain tile build set (32x32_terain.png and
  32x32_terrain_overlay_door) are by David Gervais. The license for these tiles
  is below (copied from Angband's copying.txt):

 * David Gervais' (32x32) graphics may be redistributed, modified, and used
   only under the terms of the Creative Commons Attribution 3.0 licence:
   http://creativecommons.org/licenses/by/3.0/

The tile picker may be redistributed and used under the Angband license:

 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.

The source code is not released yet.  When it is, the license will be the dual
  licenses angband uses: the angband license or GPL v2.

