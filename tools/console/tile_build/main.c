/***************************************************************************
 *
 *  Copyright (C) 2011 Brett Reid
 *
 *  File:tile_build/main.c
 *  Purpose: Entry point and main function of program.
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 *
 ***************************************************************************/

// Includes....
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "png.h"
#include "../tile_common/ts-info.h"
#include "../tile_common/angband.h"

int LoadPNG(char *filename, int *width, int *height,
                png_byte *color_type, png_byte *bitdepth,
                png_bytep **rows);
int SavePNG(char *filename, int width, int height,
                png_byte color_type, png_byte bitdepth,
                png_bytep *rows);
void FreePNG (png_bytep **prow_pointers, int height);

char zWndClassName[] = "tile_build";

term *Term;
int loadflags = 847;

void RemoveFilepath(char *path);
void RemoveFilename(char *path);
char *util_string_append(char *s1, int s1_buff_size, const char *s2);
//------------------------------------------------------------------
// 
// Function     : main()
//
// Purpose      : Entry point to application
//
//------------------------------------------------------------------

int main(int argc, char *argv[])
{
  int wid,hgt;
  png_byte type, bits;
  int cwid,chgt;
  png_byte ctype, cbits;

  png_bytep *ppData;
  char file[1024];
  char dir[1024];
  char pref[1024];
  char temp[1024];
  char *pEnd;
  int result;
  int image_row_bytes = 0;
  int i,j;
  tsbInfoEntry *entry, *dup;
  int cell_width, cell_height;
  int rows,columns;
  png_bytep *ppIn;
  png_bytep pInRow;
  png_bytep pOutRow;
  int default_action = 0;
  int duplicates = 0;
  int *x = NULL;
  int *y = NULL;

	// Process the command line
  if (argc < 4) {
		puts("Usage: tile_build {-option} <pref file> <tile dir> <out file>");
    puts(" Options:");
  	puts("  -w     If the file exists, overwrite the tile for any entry that uses a duplicate spot.");
  	puts("  -l     If the file exists, any entry that uses a duplicate spot will use a previously unused spot.");
  	puts("  -n     Make no change for any entry that uses a duplicate spot");
  	puts("  if any option is used no prompt will appear for any entry that uses a duplicate spot");
    return -1;
  }
  if ((argc == 5) && (*argv[1] == '-')) {
    if (argv[1][1] == 'w') {
      default_action = 'w';
    } else
    if (argv[1][1] == 'l') {
      default_action = 'l';
    } else {
      default_action = 'n';
    }
    i = 1;
  } else {
    // no optional parameters ofr flags
    if ((*argv[1] == '-') || (*argv[2] == '-') || (*argv[3] == '-')) {
		  puts("Usage: tile_build {-option} <pref file> <tile dir> <out file>");
      puts(" Options:");
  	  puts("  -w     If the file exists, overwrite the tile for any entry that uses a duplicate spot.");
  	  puts("  -l     If the file exists, any entry that uses a duplicate spot will use a previously unused spot.");
  	  puts("  -n     Make no change for any entry that uses a duplicate spot");
  	  puts("  if any option is used no prompt will appear for any entry that uses a duplicate spot");
      return -1;
    }
    i = 0;
  }
  strncpy(pref,argv[i+1],1024);
  strncpy(dir,argv[i+2],1024);
  strncpy(file,argv[i+3],1024);

  if (strlen(dir) > 256) {
		puts("The input tile directory name is too long. It may be at most 256 characters.");
    return -1;
  }

  Term = ZNEW(term);
  Term->xchar_hook = NULL;

  if (!dir_exists(dir)) {
    if (!dir_create(dir)) {
		  puts("Unable to create tile directory");
      FREE(Term);
      return -1;
    }
  }

  // add a trailing slash to the output name if it does not already have one
  j = strlen(dir);
  if ((dir[j-1] != '/') && (dir[j-1] != '\\') && (j < 960)) {
#if (defined(WINDOWS))
    dir[j] = '\\';
#else
    dir[j] = '/';
#endif
  }

  // remove the path from the input
  strncpy(temp,file,1024);
  RemoveFilepath(temp);
  
  // get the cell width from the filename, look for some thing like:
  // XxY, where X and Y are numbers
  cell_width = strtol(temp,&pEnd,10);
  cell_height = strtol(pEnd+1, NULL,10);
  if (!cell_width || !cell_height) {
    puts("Invalid tile dimensions in tileset filename... Aborting");
    FREE(Term);
    return -2;
  }

  // read the pref file
  puts("Reading Pref File...");
  result = tsbReadPrefFile(pref);
  if (result < 0) {
    puts("Could not read the pref file... Aborting");
    tsbClearEntries();
    FREE(Term);
    return -1;
  }

  // build the file name for each entry
  puts("Building file names and marking duplicate locations...");
  entry = g_pInfoRoot;
  while (entry) {
    entry->filename_len = entry->name_len;
    entry->zFilename = (char*) malloc(sizeof(char) * (entry->filename_len+1));
    // need an out of memory check here
    j = 0;
    for (i=0; i < entry->name_len; ++i) {
      if (isalnum(entry->zName[i])) {
        entry->zFilename[j++] = tolower(entry->zName[i]);
      } else
      if (entry->zName[i] == '~') {
        // skip this one
      } else
      if (isspace(entry->zName[i])) {
        // just becomes an underscore
        // check if this is a gf seperator, and only use one underscore
        while (isspace(entry->zName[i]) || (entry->zName[i] == '|')) {
          i++;
        }
        i--;
        entry->zFilename[j++] = '_';
      } else
      {
        // any other character that is not an alphanumeric
        // just becomes an underscore
        entry->zFilename[j++] = '_';
      }
    }
    entry->zFilename[j] = 0;
    entry->filename_len = strlen(entry->zFilename);

    if (!(entry->flags & FLAG_DUPLICATE)) {
      dup = entry->pNext;
      while (dup) {
        if ((dup->y == entry->y) && (dup->x == entry->x)) {
          entry->flags |= (FLAG_DUPLICATE|FLAG_DUP_FIRST);
          dup->flags |= FLAG_DUPLICATE;
          ++duplicates;
        }
        dup = dup->pNext;
      }
    }
    entry = entry->pNext;
  }

  // load the tileset
  puts("Attempting to loading tile set file...");
  result = LoadPNG(file, &wid,&hgt, &type,&bits, &ppData);
  if (result < 0) {
    int maxx=0,maxy=0;
    puts("Load failed, using new file.");
    // the file does not already exist, scan the entries to
    // get what dimensions the file should be
    entry = g_pInfoRoot;
    while (entry) {
      if (entry->y > 128) {
        entry->y -= 128;
      }
      if (entry->x > 128) {
        entry->x -= 128;
      }
      if (entry->x > maxx)
        maxx = entry->x;
      if (entry->y > maxy)
        maxy = entry->y;
      entry = entry->pNext;
    }

    // if possible, add some extra rows and columns for future use by people
    // that might not use this program
    if (maxx > 123)
      maxx = 127;
    else 
      maxx += 4;
    if (maxy > 125)
      maxy = 127;
    else 
      maxy += 2; 

    hgt = (maxy+1)*cell_height;
    wid = (maxx+1)*cell_width;
    type = PNG_COLOR_TYPE_RGB_ALPHA;
    bits = 8;

    // allocate the tile storage
	  ppData = (png_bytep*) malloc(sizeof(png_bytep) * hgt);
    if (!ppData) {
      puts("Could not allocate output storage... Aborting");
      tsbClearEntries();
      FREE(Term);
      return -3;
    }
	  for (j = 0; j < hgt; ++j) {
		  ppData[j] = (png_bytep) malloc(sizeof(png_bytep) * wid * 4);
      if (!ppData[j]) {
        puts("Could not allocate output storage... Aborting");
        FreePNG(&ppData, hgt);
        tsbClearEntries();
        FREE(Term);
        return -3;
      }
      memset(ppData[j],0,sizeof(png_bytep) * wid * 4);
	  }
  } else {
    puts("Load successful. Processing...");
  }
  if (type != PNG_COLOR_TYPE_RGB_ALPHA) {
    puts("Could not convert input file to RGBA... Aborting");
    FreePNG(&ppData, hgt);
    tsbClearEntries();
    FREE(Term);
    return -5;
  }

  // get the number of rows and columns in the file
  columns = wid/cell_width;
  rows = hgt/cell_height;
  image_row_bytes = cell_width*4;

  // if there is at least a possibility of using unused locations,
  // lets go ahead and generate them now
  if (duplicates && (default_action != 'w') && (default_action != 'n')) {
    int index = 0;
    x = (int*) malloc(sizeof(int) * (duplicates+1));
    if (!x) {
      puts("Could not allocate duplicate location storage... Aborting");
      FreePNG(&ppData, hgt);
      tsbClearEntries();
      FREE(Term);
      return -3;
    }
    y = (int*) malloc(sizeof(int) * (duplicates+1));
    if (!y) {
      puts("Could not allocate duplicate location storage... Aborting");
      free(x);
      FreePNG(&ppData, hgt);
      tsbClearEntries();
       FREE(Term);
      return -3;
    }
    puts("Testing unused spots...");
    for (j = 1; j < rows; ++j) {
      if (index > duplicates) {
        break;
      }
      for (i = 0; i < columns; ++i) {
        if (index > duplicates) {
          break;
        }
        entry = g_pInfoRoot;
        while (entry) {
          if ((entry->y == j) && (entry->x == i)) {
            break;
          }
          entry = entry->pNext;
        }
        if (!entry) {
          // this is an unused spot, store these coordinates
          x[index] = i;
          y[index++] = j;
        }
      }
    }
  }

  puts("Loading Tile Files...");
  // process each entry
  entry = g_pInfoRoot;
  while (entry) {
    printf("%s ",entry->zName);
    if ((entry->x < 0) || (entry->y < 0)) {
      //if a coordinate is less than 0, give a warning
      puts("is not using graphics... Skipping");
      entry = entry->pNext;
      continue;
    }
    if (entry->x >= columns) {
      // a coordinate is too high give a warning
      puts("Column is off image... Skipping");
      entry = entry->pNext;
      continue;
    }
    if (entry->y >= rows) {
      // a coordinate is too high give a warning
      puts("Row is off image... Skipping");
      entry = entry->pNext;
      continue;
    }

    strncpy(temp,dir,1024);
    util_string_append(temp, 1024, entry->zFilename);
    util_string_append(temp, 1024, ".png");

    result = LoadPNG(temp, &cwid,&chgt, &ctype,&cbits, &ppIn);
    if (result < 0) {
      printf("... load failed... Skipping\n");
      entry = entry->pNext;
      continue;
    }
    // TODO give a warning if this file and the main one are not
    // the same type and bitdepth

    // if this entry is a duplicate, give a warning and a prompt
    if ((entry->flags & FLAG_DUPLICATE) && !(entry->flags & FLAG_DUP_FIRST)) {
      int ch;
      dup = g_pInfoRoot;
      while (dup) {
        if ((dup->flags & FLAG_DUP_FIRST)
          && (dup->x == entry->x) && (dup->y == entry->y))  {
          break;
        }
        dup = dup->pNext;
      }
      if (!default_action) {
        if (dup) {
          printf("uses the same entry as %s. Do you want to overwrite\n\
            the tileset location, use the first unused location, or make\n\
            no changes? (w/l/n)", dup->zName);
          ch = getche();
          printf("\n");
          if ((ch == 'l') || (ch != 'L')) {
            ch = 'l'; 
          } else
          if ((ch == 'w') || (ch != 'W')) {
            ch = 'w'; 
          } else
          {
            ch = 'n';
          }
        } else {
          ch = 'n';
        }
      } else {
        ch = default_action;
        if ((ch == 'n') && dup) {
          printf("uses the same entry as %s. No changes made.\n", dup->zName);
        }
        //if ((ch == 'l') && dup) {
          // until the code to use an unused spot is written, just give
          // a warning and skip.
        //  printf("uses the same entry as %s. Not yet using empty spots. No changes made.\n", dup->zName);
        //}
      }
      if (ch == 'l') {
        // assign an unused spot
        for(j=0;j<duplicates;j++) {
          if (y[j] != 0) {
            entry->y = y[j];
            entry->x = x[j];
            // mark that we want to write the new spot for this entry to the pref file
            entry->flags |= FLAG_DUP_WRITE;
            y[j] = 0;
            break;
          }
        }
      } else
      if (ch != 'w') {
        // do not write anything for this entry
        entry = entry->pNext;
        continue;
      }
    }
    printf("\n",entry->zName);

    // copy the tile for the entry
    if ((chgt > cell_height) && (entry->y > 0)) {
      int shift = chgt-cell_height;
      for (j = 0; j < chgt; ++j) {
        pInRow = ppIn[j];
        pOutRow = ppData[j+(entry->y*cell_height)- shift];
        for (i = 0; i < image_row_bytes; ++i) {
          pOutRow[i] = pInRow[i+entry->x*image_row_bytes];
        }
      }
    } else {
      for (j = 0; j < cell_height; ++j) {
        pInRow = ppIn[j];
        pOutRow = ppData[j+(entry->y*cell_height)];
        for (i = 0; i < image_row_bytes; ++i) {
          pOutRow[i+entry->x*image_row_bytes] = pInRow[i];
        }
      }
    }
    FreePNG(&ppIn, chgt);

    entry = entry->pNext;
  }
  puts("Saving Output File...");
  // save the tile
  result = SavePNG(file, wid, hgt, type,bits, ppData);
  if (result < 0) {
    puts("Failed to save output file.");
  }

  if (x && y && (y[0] == 0)) {
    // a previously unused location was used
    // if we were to write a pref file here, most of the comments in the 
    // pref file would be lost. I do not like overwriting parts an already
    // existing file so for now, just show a message of the change and let
    // the user change the pref file manually.
    // maybe copy the pref files to new names, and adjust that? if so, change
    // the below to a prompt and ask the user what they want to do.
    puts("WARNING: The coordinates of some duplicate entries were changed to");
    puts("previously unused spots in the tile set. Please change the entries");
    puts("in the pref file for the following to their new coordinates:");

    entry = g_pInfoRoot;
    while (entry) {
      if (entry->flags & FLAG_DUP_WRITE) {
        printf("%s: please change to 0x%.2X:0x%.2X in the pref file\n",
          entry->zName, entry->y + 128, entry->x + 128);
      }
      entry = entry->pNext;
    }
  }



  if(x) {
    free(x);
  }
  if(y) {
    free(y);
  }
  FreePNG(&ppData, hgt);
  tsbClearEntries();
  FREE(Term);

  //if (default_action) {
  //  puts("Finished, press any key to exit.");
  //  i = getch();
  //  i=i;
  //}
  puts("Finished. Exiting.");
  return result;
}

void RemoveFilepath(char *path)
{
  char *forward, *back;
  forward = strrchr(path, '/');
  back = strrchr(path, '\\');
  if (forward || back) {
    if (back > forward) {
      // the back slash is after the forward slash, break the filename on it
      strcpy(path,(back+1));
    } else {
      // the forward slash is after the back slash, break the filename on it
      strcpy(path,(forward+1));
    }
  }
}
void RemoveFilename(char *path)
{
  char *forward, *back;
  forward = strrchr(path, '/');
  back = strrchr(path, '\\');
  if (forward || back) {
    int end;
    if (back > forward) {
      // the back slash is after the forward slash, break the filename on it
      end = (back+1) - path + 1;
      path[end] = 0;
    } else {
      // the forward slash is after the back slash, break the filename on it
      end = (forward+1) - path + 1;
      path[end] = 0;
    }
  }
}

char *util_string_append(char *s1, int s1_buff_size, const char *s2)
{
  int len1 = strlen(s1);
  int len2 = strlen(s2);
  if (s1_buff_size < len1+len2) {
    return NULL;
  }
  strncpy(s1+len1,s2, len2);
  return s1;
}
