/***************************************************************************
 *
 *  Copyright (C) 2011 Brett Reid
 *
 *  File:png_premultiply_alpha/main.c
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
#include "png.h"

int LoadPNG(char *filename, int *width, int *height,
                png_byte *color_type, png_byte *bitdepth,
                png_bytep **rows);
int SavePNG(char *filename, int width, int height,
                png_byte color_type, png_byte bitdepth,
                png_bytep *rows);
void FreePNG (png_bytep **prow_pointers, int height);

char zWndClassName[] = "png_premultiply_alpha";


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
  int result;
  int black_partial = 0;
  int i,j;
  png_byte r,g,b,a;
  png_bytep row;

	// Process the command line
  if (argc < 3) {
		puts("Usage: png_premultiply_alpha {-b} <infile> <outfile>");
  	puts("  -b     Set a pixel with partial transparency to be fully opaque, so it looks like a black outline");
    return -1;
  }
  strncpy(input,argv[argc-2],1024);
  strncpy(output,argv[argc-1],1024);
  if (argc > 3) {
    if (strncmp(argv[argc-3],"-b",2) == 0) {
      black_partial = 1;
    }
  }

  // load the input
  result = LoadPNG(input, &wid,&hgt, &type,&bits, &ppData);
  if (result < 0) {
    puts("Failed to load input file... Aborting");
    return result;
  } else {
    puts("Load successful. Processing...");
  }
  if (type != PNG_COLOR_TYPE_RGB_ALPHA) {
    puts("Could not convert input file to RGBA... Aborting");
    FreePNG(&ppData, hgt);
    return -5;
  }

  // process the file
  for (j = 0; j < hgt; ++j) {
    row = ppData[j];
    for (i = 0; i < wid; ++i) {
      //b = *(row + i*4 + 0);
      //g = *(row + i*4 + 1);
      //r = *(row + i*4 + 2);
      a = *(row + i*4 + 3);
      if (a && (a != 255)) {
        float rf,gf,bf,af;
        //blend the color value based on this value
        r = *(row + i*4 + 0);
        g = *(row + i*4 + 1);
        b = *(row + i*4 + 2);

        rf = ((float)r) / 255.f;
        gf = ((float)g) / 255.f;
        bf = ((float)b) / 255.f;
        af = ((float)a) / 255.f;
        
        r = (png_byte)((rf/af)*255.f);
        g = (png_byte)((gf/af)*255.f);
        b = (png_byte)((bf/af)*255.f);
        if (r > 255) r = 255;
        if (g > 255) r = 255;
        if (b > 255) r = 255;
        
        *(row + i*4 + 0) = r;
        *(row + i*4 + 1) = g;
        *(row + i*4 + 2) = b;
      }
    }
  }

  // write the output
  result = SavePNG(output, wid,hgt, type,bits, ppData);
  if (result < 0) {
    puts("Failed to save output file... Aborting");
  } else {
    puts("Finished... Closing");
  }

  FreePNG(&ppData, hgt);
  return result;
}
