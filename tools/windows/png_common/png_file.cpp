/***************************************************************************
 *
 *  Copyright (C) 2011 Brett Reid
 *
 *  File:png_file.cpp
 *  Purpose: To load/save png files from/to Windows bitmaps.
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
//#include "stdafx.h"
#include <windows.h>
#include <math.h>
#include <stdio.h>
#include "png.h"

extern HWND g_hWnd;

extern char zWndClassName[];
#define Message(str) MessageBox(NULL, str, zWndClassName, MB_OK | MB_ICONERROR);if (ShowCursor(TRUE) > 1) {ShowCursor(FALSE);}

HRESULT LoadPNG(TCHAR *filename,
                HBITMAP *ret_color, HPALETTE *ret_color_palette,
                HBITMAP *ret_mask, HPALETTE *ret_mask_palette,
                int *ret_width, int *ret_height)
{
	png_structp png_ptr;
	png_infop info_ptr;
	byte header[8];
	png_bytep *row_pointers;
	
	BOOL noerror = TRUE;
	
	HBITMAP hBitmap;
	HPALETTE hPalette, hOldPal;
	BITMAPINFO bi, biSrc;
	HDC hDC;
	
	png_byte color_type;
	png_byte bit_depth;
	int width, height;
	int y, number_of_passes;

	BOOL update = FALSE;
	
	// open the file and test it for being a png
	FILE *fp = fopen(filename, "rb");
	if (!fp)
	{
		//plog_fmt("Unable to open PNG file.");
		return (FALSE);
	}

	fread(header, 1, 8, fp);
	if (png_sig_cmp(header, 0, 8)) {
		//plog_fmt("Unable to open PNG file - not a PNG file.");
		fclose(fp);
		return (FALSE);
	}
	
	// Create the png structure
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr)
	{
		//plog_fmt("Unable to initialize PNG library");
		fclose(fp);
		return (FALSE);
	}
	
	// create the info structure
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		//plog_fmt("Failed to create PNG info structure.");
		return FALSE;
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
		update = TRUE;
	}

	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	{
		png_set_tRNS_to_alpha(png_ptr);
		update = TRUE;
	}

	if (bit_depth == 16)
	{
		png_set_strip_16(png_ptr);
		update = TRUE;
	}

	if (color_type == PNG_COLOR_TYPE_GRAY ||
		color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(png_ptr);
		update = TRUE;
	}

	if (update) {
		png_read_update_info(png_ptr, info_ptr);
		color_type = png_get_color_type(png_ptr, info_ptr);
		bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	}

	png_set_bgr(png_ptr);
	// after these requests, the data should always be RGB or ARGB

	/* initialize row_pointers */
	row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
	if (!row_pointers) return FALSE;
	for (y = 0; y < height; ++y) {
		row_pointers[y] = (png_bytep) malloc(png_get_rowbytes(png_ptr, info_ptr));
		if (!row_pointers[y]) return FALSE;
	}

	/* read the image data into row_pointers */
	png_read_image(png_ptr, row_pointers);

	/* we are done with the file pointer, so close it */
	fclose(fp);
	
	/* create the DIB */
	bi.bmiHeader.biWidth = (LONG)width;
	bi.bmiHeader.biHeight = -((LONG)height);
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biClrUsed = 0;
	bi.bmiHeader.biClrImportant = 0;
	bi.bmiHeader.biBitCount = 24;
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biSize = 40; // the size of the structure
	bi.bmiHeader.biXPelsPerMeter = 3424; // just a number I saw when testing this with a sample
	bi.bmiHeader.biYPelsPerMeter = 3424; // just a number I saw when testing this with a sample
	bi.bmiHeader.biSizeImage = width*height*3;
	
	biSrc.bmiHeader.biWidth = (LONG)width;
	biSrc.bmiHeader.biHeight = -((LONG)height);
	biSrc.bmiHeader.biPlanes = 1;
	biSrc.bmiHeader.biClrUsed = 0;
	biSrc.bmiHeader.biClrImportant = 0;
	biSrc.bmiHeader.biCompression = BI_RGB;
	biSrc.bmiHeader.biPlanes = 1;
	biSrc.bmiHeader.biSize = 40; // the size of the structure
	biSrc.bmiHeader.biXPelsPerMeter = 3424; // just a number I saw when testing this with a sample
	biSrc.bmiHeader.biYPelsPerMeter = 3424; // just a number I saw when testing this with a sample
	
	if (color_type == PNG_COLOR_TYPE_RGB_ALPHA) {
		biSrc.bmiHeader.biBitCount = 32;
		biSrc.bmiHeader.biSizeImage = width*height*4;
	} else {
		biSrc.bmiHeader.biBitCount = 24;
		biSrc.bmiHeader.biSizeImage = width*height*3;
	}
		
	//hDC = CreateCompatibleDC(NULL);
  hDC = GetDC(g_hWnd);
	
	hPalette = (HPALETTE) GetStockObject(DEFAULT_PALETTE);
	// Need to realize palette for converting DIB to bitmap. 
	hOldPal = SelectPalette(hDC, hPalette, TRUE);
	RealizePalette(hDC);

	// copy the data to the DIB
	hBitmap = CreateDIBitmap(hDC, &(bi.bmiHeader), 0, NULL,
							 &biSrc, DIB_RGB_COLORS);
		
	if (hBitmap)
	{
		for (y = 0; y < height; ++y)
		{
			if (SetDIBits(hDC, hBitmap, height-y-1, 1, row_pointers[y], &biSrc, DIB_RGB_COLORS) != 1)
			{
				//plog_fmt("Failed to alloc temporary memory for PNG data.");
				DeleteObject(hBitmap);
				hBitmap = NULL;
				noerror = FALSE;
				break;
			}
		}
	}
	SelectPalette(hDC, hOldPal, TRUE);
	RealizePalette(hDC);
	if (!hBitmap)
	{
		DeleteObject(hPalette);
		noerror = FALSE;
	}
	else
	{
    if (ret_color)
		  *ret_color = hBitmap;
    if (ret_color_palette)
		  *ret_color_palette = hPalette;
    if (ret_width)
		  *ret_width = width;
    if (ret_height)
		  *ret_height = height;
	}
	
	if (noerror && ret_mask && (color_type == PNG_COLOR_TYPE_RGB_ALPHA))
	{
		byte *pBits, v;
		int x;
		DWORD *srcrow;
		HBITMAP hBitmap2 = NULL;
		HPALETTE hPalette2 = (HPALETTE)GetStockObject(DEFAULT_PALETTE);
		BOOL have_alpha = FALSE;
		
		/* Need to realize palette for converting DIB to bitmap. */
		hOldPal = SelectPalette(hDC, hPalette2, TRUE);
		RealizePalette(hDC);
		
		/* allocate the storage space */
		pBits = (byte*)malloc(sizeof(byte)*width*height*3);
		if (!pBits)
		{
			noerror = FALSE;
		}

		if (noerror)
		{
			for (y = 0; y < height; ++y) {
				srcrow = (DWORD*)row_pointers[y];
				for (x = 0; x < width; ++x) {
					/* get the alpha byte from the source */
					v = (*((DWORD*)srcrow + x)>>24);
					v = 255 - v;
					if (v==255) 
					{
						have_alpha = TRUE;
					}
					/* write the alpha byte to the three colors of the storage space */
					*(pBits + (y*width*3) + (x*3)) = v;
					*(pBits + (y*width*3) + (x*3)+1) = v;
					*(pBits + (y*width*3) + (x*3)+2) = v;
				}
			}
			/* create the bitmap from the storage space */
			if (have_alpha)
			{
				hBitmap2 = CreateDIBitmap(hDC, &(bi.bmiHeader), CBM_INIT, pBits,
										  &bi, DIB_RGB_COLORS);
			}
			free(pBits);
		}
		SelectPalette(hDC, hOldPal, TRUE);
		RealizePalette(hDC);
		if (!hBitmap2)
		{
			DeleteObject(hPalette2);
			noerror = FALSE;
		}
		else
		{
      if (ret_mask)
		    *ret_mask = hBitmap2;
      if (ret_mask_palette)
		    *ret_mask_palette = hPalette2;
		}
	}

  // release the image memory
	for (y = 0; y < height; ++y)
	{
		free(row_pointers[y]);
	}
	free(row_pointers);
	
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
	
  //DeleteDC(hDC);
  ReleaseDC(g_hWnd,hDC);
	
	if (!noerror)
	{
		return (FALSE);
	}
	return (TRUE);

}

HRESULT SavePNG(TCHAR *filename,
                HBITMAP color, HPALETTE color_palette,
                HBITMAP mask, HPALETTE mask_palette,
                int width, int height)
{
  png_structp png_ptr;
  png_infop info_ptr;
  png_bytep *row_pointers;

  HRESULT result;
  BOOL noerror = TRUE;
  BOOL masksaved = FALSE;

  HBITMAP hbmOld, hbmOldMask;
	HDC dcColor, dcMask;

  png_byte color_type;
  png_byte bit_depth;
  png_byte channels;

  int x,y;

  if (mask) {
    color_type = PNG_COLOR_TYPE_RGB_ALPHA;
    bit_depth = 8;
    channels = 4;
  } else
  {
    color_type = PNG_COLOR_TYPE_RGB;
    bit_depth = 8;
    channels = 3;
  }

  // open the file and test it for being a png
  FILE *fp = fopen(filename, "wb");
  if (!fp)
  {
    //plog_fmt("Unable to open PNG file.");
    return E_FAIL;
  }

  // Create the png structure
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(!png_ptr) {
    //plog_fmt("Unable to initialize PNG library");
    fclose(fp);
    return E_FAIL;
  }

  // create the info structure
  if (noerror) {
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
      png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
			//plog_fmt("Failed to create PNG info structure.");
      noerror = FALSE;
      result = E_FAIL;
    }
  }

  if (noerror) {
    // setup error handling for init
    png_init_io(png_ptr, fp);

    png_set_IHDR(png_ptr, info_ptr, width, height,
      bit_depth, color_type, PNG_INTERLACE_NONE,
      PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    if (bit_depth < 8) {
      png_set_packing(png_ptr);
    }
    png_write_info(png_ptr, info_ptr);
  }

  // copy the allocate the memory libpng can access
  if (noerror) {
    // setup error handling for read
    row_pointers = (png_bytep*) malloc(sizeof(png_bytep)*height);
    if (!row_pointers)
    {
			//plog_fmt("Failed to alloc temporary memory for PNG data.");
      noerror = FALSE;
      result = E_OUTOFMEMORY;
    }
    if (noerror) {
      for (y = 0; y < height; ++y)
      {
        row_pointers[y] = (png_bytep) malloc(sizeof(png_bytep)*width*channels);
        if (!row_pointers[y])
        {
			    //plog_fmt("Failed to alloc temporary memory for PNG data.");
          noerror = FALSE;
          result = E_OUTOFMEMORY;
          break;
        }
      }
    }
  }

  // copy the data to it
  if (noerror) {
    COLORREF bgr, a;
	  dcColor = CreateCompatibleDC(NULL);
    if (!dcColor) {
    }
    hbmOld = (HBITMAP)SelectObject(dcColor, color);
    
    if (color_type == PNG_COLOR_TYPE_GRAY) {
      // copy just the mask bitmap data
      byte *data;

      dcMask = CreateCompatibleDC(NULL);
      if (!dcMask) {
      }
      hbmOldMask = (HBITMAP)SelectObject(dcMask, mask);
      for (y = 0; y < height; ++y) {
        data = row_pointers[y];
        for (x = 0; x < width; x += 8) {
          a = GetPixel(dcMask, x,y);
          data[x] = ((a&0x0000FF00) >> 8);
        }
      }
      png_set_packing(png_ptr);
      masksaved = TRUE;

      SelectObject(dcMask, hbmOldMask);
      DeleteDC(dcMask);
    } else
    if (color_type == PNG_COLOR_TYPE_RGB_ALPHA) {
      // copy both sets of data
      DWORD  argb, *data;

      dcMask = CreateCompatibleDC(NULL);
      if (!dcMask) {
      }
      hbmOldMask = (HBITMAP)SelectObject(dcMask, mask);

      for (y = 0; y < height; ++y) {
        data = (DWORD*) row_pointers[y];
        for (x = 0; x < width; ++x) {
          bgr = GetPixel(dcColor, x,y);
          a = GetPixel(dcMask, x,y);
          //argb = ((bgr&0x00FF0000)>>16) | ((bgr&0x0000FF00)) | ((bgr&0x000000FF)<<16) | (0xFF000000 - ((a&0x0000FF00)<<16));
          argb = (bgr&0x00FFFFFF) | (0xFF000000 - ((a&0x0000FF00)<<16));
          *(data+x) = argb;
        }
      }
      masksaved = TRUE;

      SelectObject(dcMask, hbmOldMask);
      DeleteDC(dcMask);
    } else
    {
      // copy just the color data
      byte b[3], *data;
      for (y = 0; y < height; ++y) {
        data = row_pointers[y];
        for (x = 0; x < width; ++x) {
          bgr = GetPixel(dcColor, x,y);
          b[0] = ((bgr&0x000000FF));
          b[1] = ((bgr&0x0000FF00)>>8);
          b[2] = ((bgr&0x00FF0000)>>16);
          *(data++) = b[2];
          *(data++) = b[1];
          *(data++) = b[0];
        }
      }
    }

    SelectObject(dcColor, hbmOld);
    DeleteDC(dcColor);
  }  

  // write the file
  if (noerror)
  {
    //png_set_bgr(png_ptr);
    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, NULL);
  }

  // we are done with the file pointer, so
  // release all the the PNG Structures
  if (info_ptr) {
    png_destroy_write_struct(&png_ptr, &info_ptr);//, (png_infopp)NULL);
    info_ptr = NULL;
    png_ptr = NULL;
  }
  else if (png_ptr) {
    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);//, (png_infopp)NULL);
    png_ptr = NULL;
  }
  
  // we are done with the file pointer, so close it
  if (fp) {
    fclose(fp);
    fp = NULL;
  }

  if (!noerror)
  {
    return result;
  }
  // return S_OK if both were saved, S_FALSE if just color bitmap
  // S_OK also if just mask was saved and that's all we wanted
  if (masksaved) {
    return S_OK;
  } else {
    return S_FALSE;
  }
}
