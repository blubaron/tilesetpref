/***************************************************************************
 *
 *  Copyright (C) 2011 Brett Reid
 *
 *  File:tile_split/main.c
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

char zWndClassName[] = "tile_split";

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

  png_bytep *ppData;
  char input[1024];
  char output[1024];
  char pref[1024];
  char temp[1024];
  char *pEnd;
  int result;
  int image_row_bytes = 0;
  int i,j;
  tsbInfoEntry *entry, *dup;
  int cell_width, cell_height;
  int rows,columns;
  png_bytep *ppOut;
  png_bytep pInRow;
  png_bytep pOutRow;
  int default_action = 0;

  Term = ZNEW(term);
  Term->xchar_hook = NULL;

	// Process the command line
  if (argc < 4) {
		puts("Usage: tilesplit {-option} <pref file> <tile dir> <in file>");
    puts(" Options:");
  	puts("  -y     Write a seperate file for any entry that uses a duplicate spot.");
  	puts("  -n     Make no change for any entry that uses a duplicate spot.");
  	puts("  if any option is used no prompt will appear for any entry that uses a duplicate spot.");
    return -1;
  }
  if ((argc == 5) && (*argv[1] == '-')) {
    if (argv[1][1] == 'y') {
      default_action = 'y';
    } else {
      default_action = 'n';
    }
    i = 1;
  } else {
    // no optional parameters or flags
    if ((*argv[1] == '-') || (*argv[2] == '-') || (*argv[3] == '-')) {
		  puts("Usage: tilesplit {-option} <pref file> <tile dir> <in file>");
      puts(" Options:");
  	  puts("  -y     Write a seperate file for any entry that uses a duplicate spot.");
  	  puts("  -n     Make no change for any entry that uses a duplicate spot.");
  	  puts("  if any option is used no prompt will appear for any entry that uses a duplicate spot.");
      return -1;
    }
    i = 0;
  }
  strncpy(pref,argv[i+1],1024);
  strncpy(input,argv[i+3],1024);
  strncpy(output,argv[i+2],1024);

  if (strlen(output) > 256) {
		puts("The output tile directory name is too long. It may be at most 256 characters.");
    return -1;
  }

  if (!dir_exists(output)) {
    if (!dir_create(output)) {
		  puts("Unable to create tile directory");
      return -1;
    }
  }

  // add a trailing slash to the output name if it does not already have one
  j = strlen(output);
  if ((output[j-1] != '/') && (output[j-1] != '\\') && (j < 1000)) {
#if (defined(WINDOWS))
    output[j] = '\\';
#else
    output[j] = '/';
#endif
  }

  // TODO? try creating a file in the directory to make sure we have write permissions

  // read the pref file
  puts("Reading Pref File...");
  result = tsbReadPrefFile(pref);
  if (result < 0) {
    puts("Could not read the pref file... Aborting");
    tsbClearEntries();
    return -1;
  }

  // build the file name for each entry
  puts("Building File Names and marking duplicate locations...");
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
        }
        dup = dup->pNext;
      }
    }
    entry = entry->pNext;
  }

  // load the tileset
  puts("Loading Input File...");
  result = LoadPNG(input, &wid,&hgt, &type,&bits, &ppData);
  if (result < 0) {
    puts("Failed to load input file... Aborting");
    tsbClearEntries();
    return result;
  } else {
    puts("Load successful. Processing...");
  }
  if (type != PNG_COLOR_TYPE_RGB_ALPHA) {
    puts("Could not convert input file to RGBA... Aborting");
    FreePNG(&ppData, hgt);
    tsbClearEntries();
    return -5;
  }
  // remove the path from the input
  strncpy(temp,input,1024);
  RemoveFilepath(temp);
  // get the cell width from the filename, look for some thing like:
  // XxY, where X and Y are numbers
  cell_width = strtol(temp,&pEnd,10);
  cell_height = strtol(pEnd+1, NULL,10);
  if (!cell_width || !cell_height) {
    puts("Invalid tile dimensions in tileset filename... Aborting");
    FreePNG(&ppData, hgt);
    tsbClearEntries();
    return -3;
  }

  // get the number of rows and columns in the file
  columns = wid/cell_width;
  rows = hgt/cell_height;
  image_row_bytes = cell_width*4;

  // allocate the tile storage
	ppOut = (png_bytep*) malloc(sizeof(png_bytep) * cell_height);
  if (!ppOut) {
    puts("Could not allocate output storage... Aborting");
    FreePNG(&ppData, hgt);
    tsbClearEntries();
    return -3;
  }
	for (j = 0; j < cell_height; ++j) {
		ppOut[j] = (png_bytep) malloc(sizeof(png_bytep) * cell_width * 4);
    if (!ppOut[j]) {
      puts("Could not allocate output storage... Aborting");
      FreePNG(&ppOut, cell_height);
      FreePNG(&ppData, hgt);
      tsbClearEntries();
      return -3;
    }
	}

  puts("Saving Output Files...");
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
          printf("uses the same entry as %s. Do you want to write\n\
            a seperate file for this entry? (y/n)", dup->zName);
          ch = getche();
          printf("\n");
          if ((ch == 'y') || (ch != 'Y')) {
            ch = 'y'; 
          } else
          if ((ch == 'w') || (ch != 'W')) {
            ch = 'y'; 
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
      }
      if (ch != 'y') {
        // do not write anything for this entry
        entry = entry->pNext;
        continue;
      }
    }
    printf("\n",entry->zName);
    // copy the tile for the entry
    for (j = 0; j < cell_height; ++j) {
      pInRow = ppData[j+entry->y*cell_height];
      pOutRow = ppOut[j];
      for (i = 0; i < image_row_bytes; ++i) {
        pOutRow[i] = pInRow[i+entry->x*image_row_bytes];
      }
    }
    strncpy(temp,output,1024);
    util_string_append(temp, 1024, entry->zFilename);
    util_string_append(temp, 1024, ".png");
    // save the tile
    result = SavePNG(temp, cell_width, cell_height, type,bits, ppOut);
    if (result < 0) {
      puts("Failed to save output file.");
    }
    entry = entry->pNext;
  }


  FreePNG(&ppOut, cell_height);
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
