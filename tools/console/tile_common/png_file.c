/***************************************************************************
 *
 *  Copyright (C) 2011 Brett Reid
 *
 *  File:png_file.cpp
 *  Purpose: To load/save png files from/to general memory.
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
#include <stdlib.h>
#include <stdio.h>
#include "png.h"


extern char zWndClassName[];
void FreePNG (png_bytep **prow_pointers, int height);

int LoadPNG(char *filename, int *wid, int *hgt,
                png_byte *ct, png_byte *bd,
                png_bytep **rows)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_byte header[8];
	png_bytep *row_pointers;
	
	png_byte color_type;
	png_byte bit_depth;
	int width, height;
	int y, number_of_passes;

	png_byte update = 0;
	
	// open the file and test it for being a png
	FILE *fp = fopen(filename, "rb");
	if (!fp)
	{
		//plog_fmt("Unable to open PNG file.");
		return -1;
	}

	fread(header, 1, 8, fp);
	if (png_sig_cmp(header, 0, 8)) {
		//plog_fmt("Unable to open PNG file - not a PNG file.");
		fclose(fp);
		return -1;
	}
	
	// Create the png structure
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr)
	{
		//plog_fmt("Unable to initialize PNG library");
		fclose(fp);
		return -1;
	}
	
	// create the info structure
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		//plog_fmt("Failed to create PNG info structure.");
		fclose(fp);
		return -1;
	}
	
	// setup error handling for init
	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);
	
	png_read_info(png_ptr, info_ptr);
	
	width = png_get_image_width(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);
	bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	
	number_of_passes = png_set_interlace_handling(png_ptr);
	if (color_type == PNG_COLOR_TYPE_PALETTE)
	{
		png_set_palette_to_rgb(png_ptr);
		update = 1;
	}

	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	{
		png_set_tRNS_to_alpha(png_ptr);
		update = 1;
	}

	if (bit_depth == 16)
	{
		png_set_strip_16(png_ptr);
		update = 1;
	}

	if (color_type == PNG_COLOR_TYPE_GRAY ||
		color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(png_ptr);
		update = 1;
	}

	if (update) {
		png_read_update_info(png_ptr, info_ptr);
		color_type = png_get_color_type(png_ptr, info_ptr);
		bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	}

	//png_set_bgr(png_ptr);
	// after these requests, the data should always be RGB or ARGB

	/* initialize row_pointers */
	row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
  if (!row_pointers) {
	  if (info_ptr) {
		  png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		  info_ptr = NULL;
		  png_ptr = NULL;
	  }
	  else if (png_ptr) {
		  png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		  png_ptr = NULL;
	  }
		fclose(fp);
    return -3;
  }
	for (y = 0; y < height; ++y) {
		row_pointers[y] = (png_bytep) malloc(png_get_rowbytes(png_ptr, info_ptr));
    if (!row_pointers[y]) {
	    if (info_ptr) {
		    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		    info_ptr = NULL;
		    png_ptr = NULL;
	    }
	    else if (png_ptr) {
		    png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		    png_ptr = NULL;
	    }
	  	fclose(fp);
      return -3;
    }
	}

	/* read the image data into row_pointers */
	png_read_image(png_ptr, row_pointers);

	// release all the the PNG Structures
	if (info_ptr) {
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		info_ptr = NULL;
		png_ptr = NULL;
	}
	else if (png_ptr) {
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		png_ptr = NULL;
	}
  fclose(fp);

  if (wid) *wid = width;
  if (hgt) *hgt = height;
  if (ct) *ct = color_type;
  if (bd) *bd = bit_depth;
  if (rows)
  {
    *rows = row_pointers;
  } else {
    FreePNG(&row_pointers, height);
  }
	
	return 0;

}

int SavePNG(char *filename, int width, int height,
                png_byte color_type, png_byte bit_depth,
                png_bytep *row_pointers)
{
  png_structp png_ptr;
  png_infop info_ptr;

  int result = 0;

  // open the file and test it for being a png
  FILE *fp = fopen(filename, "wb");
  if (!fp)
  {
    puts("Unable to open PNG file.");
    return -1;
  }

  // Create the png structure
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(!png_ptr) {
    puts("Unable to initialize PNG library");
    fclose(fp);
    return -3;
  }

  // create the info structure
  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
  {
    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		puts("Failed to create PNG info structure.");
    fclose(fp);
    result = -3;
  }

  // setup error handling for init
  png_init_io(png_ptr, fp);

  png_set_IHDR(png_ptr, info_ptr, width, height,
    bit_depth, color_type, PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  if (bit_depth < 8) {
    png_set_packing(png_ptr);
  }
  png_write_info(png_ptr, info_ptr);

  // write the file
  //png_set_bgr(png_ptr);
  png_write_image(png_ptr, row_pointers);
  png_write_end(png_ptr, NULL);

  // we are done with the file pointer, so
  // release all the the PNG Structures
  if (info_ptr) {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    info_ptr = NULL;
    png_ptr = NULL;
  } else
  if (png_ptr) {
    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    png_ptr = NULL;
  }
  
  // we are done with the file pointer, so close it
  fclose(fp);

  return result;
}

void FreePNG (png_bytep **prow_pointers, int height)
{
  // release the image memory
  if (prow_pointers) {
    int y;
	  for (y = 0; y < height; ++y) {
		  free((*prow_pointers)[y]);
	  }
	  free(*prow_pointers);
  }
}
