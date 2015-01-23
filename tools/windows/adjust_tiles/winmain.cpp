/***************************************************************************
 *
 *  Copyright (C) 2011-2015 Brett Reid
 *
 *  File:png_premultiply_alpha/winmain.cpp
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
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
extern "C" {
#include "angband.h"
};
#include "png.h"
extern "C" {
int LoadPNG(char *filename, int *width, int *height,
                png_byte *color_type, png_byte *bitdepth,
                png_bytep **rows);
int SavePNG(char *filename, int width, int height,
                png_byte color_type, png_byte bitdepth,
                png_bytep *rows);
void FreePNG (png_bytep **prow_pointers, int height);
}
void RemoveFilepath(char *path);
void RemoveFilename(char *path);
char *util_string_append(char *s1, int s1_buff_size, const char *s2);

int MovePNGCell(int sx, int sy, int dx, int dy,
                    int cell_width, int cell_height,
                    int width, int height, 
                    png_byte color_type, png_byte bitdepth,
                    png_bytep *rows);
int ShiftUpPNGCell(int sx, int sy, int shift,
                int cell_width, int cell_height,
                int width, int height, 
                png_byte color_type, png_byte bitdepth,
                png_bytep *data);

HWND g_hWnd;
HINSTANCE g_hInst;
term *Term;

typedef int sint32;

char zWndClassName[] = "adjust_tiles";
#define Message(str) MessageBox(NULL, str, zWndClassName, MB_OK | MB_ICONERROR);if (ShowCursor(TRUE) > 1) {ShowCursor(FALSE);}

typedef struct _adjustment_entry {
  struct _adjustment_entry *pNext;
  int minx,maxx;
  int miny, maxy;
  int destx,desty;
} adjustment_entry;

void clear_adjustment_list(adjustment_entry *root)
{
  adjustment_entry *temp, *next;
  temp = root;
  while(temp) {
    next = temp->pNext;
    mem_free(temp);
    temp = next;
  }
}

//------------------------------------------------------------------
// 
// Function     : InitClass()
//
// Purpose      : Initialises and registers window class
//
//------------------------------------------------------------------

bool InitClass(HINSTANCE hInst)
{
  WNDCLASS    wndClass;
  
  // Fill out WNDCLASS info
  wndClass.style              = CS_HREDRAW | CS_VREDRAW;// | CS_CLASSDC | CS_OWNDC;
  wndClass.lpfnWndProc        = DefWindowProc;
  wndClass.cbClsExtra         = 0;
  wndClass.cbWndExtra         = 0;
  wndClass.hInstance          = hInst;
  wndClass.hIcon              = NULL;//LoadIcon(hInst, "VW");
  wndClass.hCursor            = LoadCursor(NULL, IDC_ARROW);
  wndClass.hbrBackground      = (HBRUSH)GetStockObject(BLACK_BRUSH);
  wndClass.lpszMenuName       = "main_menu";
  wndClass.lpszClassName      = zWndClassName;
  
  if (!RegisterClass(&wndClass)) {
    //VWRegError("Unable to create window class", "InitClass");
    return false;
  }
  
  // Everything's perfect
  return true;
}

//------------------------------------------------------------------
// 
// Function     : InitWindow()
//
// Purpose      : Initialises and creates the main window
//
//------------------------------------------------------------------

bool InitWindow(HINSTANCE hInst, int nCmdShow)
{
  
  // Create a window
  g_hWnd = CreateWindowEx(WS_EX_APPWINDOW,
                        zWndClassName, 
                        zWndClassName,
                        WS_OVERLAPPEDWINDOW,//WS_POPUP | WS_SYSMENU,
                        CW_USEDEFAULT, CW_USEDEFAULT,
                        200,200,
                        //GetSystemMetrics(SM_CXSCREEN),
                        //GetSystemMetrics(SM_CYSCREEN),
                        //0, 0, // full screen, so screen width, height can be 0
                        NULL,
                        NULL,
                        hInst,
                        NULL);
  // Return false if window creation failed
  if (!g_hWnd) {
    return false;
  }
  g_hInst = hInst;
  // Show the window
  ShowWindow(g_hWnd, SW_SHOWNORMAL);
  
  // Update the window
  UpdateWindow(g_hWnd);
  
  // Everything's perfect
  return true;
}

//------------------------------------------------------------------
// 
// Function     : WinMain()
//
// Purpose      : Entry point to application
//
//------------------------------------------------------------------

int FAR PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
  int len, coordtype = 0;
  TCHAR input[1024];
  TCHAR output[1024];
  TCHAR adjust[1024];
  TCHAR buf[1024];
  TCHAR line[256];

  // Set global handle
  if (!InitClass(hInst)) {
    Message("Class registration failed... Aborting");
    return E_FAIL;
  }
  
  // Initialise window
  if (!InitWindow(hInst, nCmdShow)) {
    Message("Window creation failed... Aborting");
    return E_FAIL;
  }
  
  input[0] = 0;
  output[0] = 0;
  adjust[0] = 0;
  line[0] = 0;

  // get the adjustment file name
  OPENFILENAME ofn;
  memset(&ofn,0,sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = NULL;
  ofn.lpstrFilter = "Text Image Files (*.txt)\0*.txt\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFile = adjust;
  ofn.nMaxFile = 1024;
  ofn.lpstrTitle = "Select the file to read adjustment ranges from";
  ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_NOREADONLYRETURN;
  ofn.lpstrInitialDir = ".";
  if (!GetOpenFileName(&ofn)) {
    Message("No adjustment file... Aborting");
    return E_ABORT;
  }

  // get the input file name
  memset(&ofn,0,sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = NULL;
  ofn.lpstrFilter = "PNG Image Files (*.png)\0*.png\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFile = input;
  ofn.nMaxFile = 1024;
  ofn.lpstrTitle = "Select the input file";
  ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_NOREADONLYRETURN;
  ofn.lpstrInitialDir = ".";
  if (!GetOpenFileName(&ofn)) {
    Message("No input file... Aborting");
    return E_ABORT;
  }
  len = _tcslen(input);
  if (len < 1024 - 14) {
    // create a default output name
    _tcsncpy(output,input,1024);
    _tcsncpy(output+len-4, "-adjusted.png", 14);
  }
  // get the output file name
  memset(&ofn,0,sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = NULL;
  ofn.lpstrFilter = "PNG Image Files (*.png)\0*.png\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFile = output;
  ofn.nMaxFile = 1024;
  ofn.lpstrTitle = "Enter a filename for the output";
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_NOREADONLYRETURN;
  ofn.lpstrInitialDir = ".";
  if (!GetOpenFileName(&ofn)) {
    Message("No output file... Aborting");
    return E_ABORT;
  }
  // load the adjustment ranges
  adjustment_entry *listroot, *listend, *temp;
  int linelen;
  //int x,y,nx,ny;
  TCHAR *start, *end;

  FILE *fp = file_open(adjust, MODE_READ, FTYPE_TEXT);
  if (!fp) {
    Message("Failed to load adjustment file... Aborting");
    return E_INVALIDARG;
  }
  listroot = NULL;
  listend = NULL;
  // read each line, and create an entry for it
  while(file_getl(fp, line,256)) {
    linelen = strlen(line);
    if (linelen < 2) {
      // skip empty lines
      continue;
    }
    if (line[0] == _T('#')) {
      // skip lines that are commented out
      continue;
    }
    if (line[0] == _T(';')) {
      // skip lines that are commented out
      continue;
    }
    if ((line[0] == _T('P')) && (line[1] == _T(':'))) {
      // we have a input/output filename pair
      // ignore it, since it's for pref files
      continue;
    }

    temp = (adjustment_entry*) mem_alloc(sizeof(adjustment_entry));
    if (!temp) {
      fclose(fp);
      clear_adjustment_list(listroot);
      Message("Unable to allocate adjustment range entry... Aborting");
      return E_OUTOFMEMORY;
    }
    start = line;
    temp->miny = _tcstol(start, &end, 0);
    temp->minx = _tcstol(end+1, &start, 0);
    temp->maxy = _tcstol(start+1, &end, 0);
    temp->maxx = _tcstol(end+1, &start, 0);
    temp->desty = _tcstol(start+1, &end, 0);
    temp->destx = _tcstol(end+1, &start, 0);
    temp->pNext = NULL;

    // add this entry to the list
    if (!listroot) listroot = temp;
    if (!listend) {
      listend = temp;
    } else {
      listend->pNext = temp;
      listend = temp;
    }
  }
  file_close(fp);
  fp = NULL;

  // load the input
  int wid,hgt;
  png_byte type, bits;

  png_bytep *ppData;
  int result;
  int black_partial = 0;
  int i,j;
  int sx, sy;
  //png_byte r,g,b,a;
  //png_bytep row;

  int cell_width, cell_height;

  // load the input
  result = LoadPNG(input, &wid, &hgt, &type, &bits, &ppData);
  if (FAILED(result)) {
    clear_adjustment_list(listroot);
    Message("Failed to load input file... Aborting");
    return result;
  }

  // get the cell width and height
  // remove the path from the input
  strncpy(buf,input,1024);
  RemoveFilepath(buf);

  // get the cell width from the filename, look for some thing like:
  // XxY, where X and Y are numbers
  cell_width = _tcstol(buf,&end,10);
  cell_height = _tcstol(end+1, NULL,10);
  if (!cell_width || !cell_height) {
    clear_adjustment_list(listroot);
    Message("Invalid tile dimensions in tileset filename... Aborting");
    FreePNG(&ppData, hgt);
    return E_INVALIDARG;
  }

  // XXX: an invalid line to prevent compiling, since this
  // program does not do anthing yet
//  cell_widt = cell_widt;


  // process the file
  temp = listroot;
  while (temp) {
  
	  // skip any entries with non tile values
	  if ((temp->minx < 128) || (temp->miny < 128)
	    || (temp->maxx < 128) || (temp->maxy < 128)
	    || (temp->destx < 128) || (temp->desty < 128))
	  {
		  temp = temp->pNext;
		  continue;
	  }

    sx = temp->destx - temp->minx;
    sy = temp->desty - temp->miny;

    if (temp->desty >= temp->miny) {
      if (temp->destx >= temp->minx) {
        for (j = temp->maxy; j >= temp->miny; --j) {
          for (i = temp->maxx; i >= temp->minx; --i) {
		        MovePNGCell(i-128, j-128, i+sx-128, j+sy-128, cell_width, cell_height, wid, hgt, type,bits, ppData);
          }
        }
      } else {
        for (j = temp->maxy; j >= temp->miny; --j) {
          for (i = temp->minx; i <= temp->maxx; ++i) {
		        MovePNGCell(i-128, j-128, i+sx-128, j+sy-128, cell_width, cell_height, wid, hgt, type,bits, ppData);
          }
        }
      }
    } else {
      if (temp->destx >= temp->minx) {
        for (j = temp->miny; j <= temp->maxy; ++j) {
          for (i = temp->maxx; i >= temp->minx; --i) {
		        MovePNGCell(i-128, j-128, i+sx-128, j+sy-128, cell_width, cell_height, wid, hgt, type,bits, ppData);
          }
        }
      } else {
        for (j = temp->miny; j <= temp->maxy; ++j) {
          for (i = temp->minx; i <= temp->maxx; ++i) {
		        MovePNGCell(i-128, j-128, i+sx-128, j+sy-128, cell_width, cell_height, wid, hgt, type,bits, ppData);
          }
        }
      }
    }
    temp = temp->pNext;
  }

  // See if we have any in place shifts
  temp = listroot;
  while (temp) {
  
	  // skip any entries with non tile values
	  if ((temp->minx < 128) || (temp->miny < 128)
	    || (temp->maxx < 128) || (temp->maxy < 128)
	    || (temp->destx < 128) || (temp->desty > 0))
	  {
		  temp = temp->pNext;
		  continue;
	  }
    for (j = temp->miny; j <= temp->maxy; ++j) {
      for (i = temp->minx; i <= temp->maxx; ++i) {
    		ShiftUpPNGCell(i-128, j-128, cell_height>>2, cell_width, cell_height, wid, hgt, type,bits, ppData);
      }
    }
    temp = temp->pNext;
  }
  // write the output
  result = SavePNG(output, wid,hgt, type,bits, ppData);
  if (result < 0) {
    Message("Failed to save output file... Aborting");
  } else {
    Message("Finished... Closing");
  }

  FreePNG(&ppData, hgt);

  clear_adjustment_list(listroot);
  listroot = NULL;
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

int MovePNGCell(int sx, int sy, int dx, int dy,
                int cell_width, int cell_height,
                int width, int height, 
                png_byte color_type, png_byte bitdepth,
                png_bytep *data)
{
	int j;
	int chan = 0; // number of channels per pixel
	int bpl = 0; // bytes per cell line
	int bpr = 0; // bytes per file row
	int mx = width / cell_width; // max cell column
	int my = height / cell_height; // max cell row
	
	if ((dx >= mx) || (dy >= my)) return -1;
	if ((sx >= mx) || (sy >= my)) return -1;
	
	if (color_type == PNG_COLOR_TYPE_RGB) {
		chan = 3;
	} else
	if (color_type == PNG_COLOR_TYPE_RGB_ALPHA) {
		chan = 4;
	} else
	if (color_type == PNG_COLOR_TYPE_GRAY) {
		chan = 1;
	} else
	if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		chan = 2;
	} else
	if (color_type == PNG_COLOR_TYPE_PALETTE) {
		chan = 1;
	} else
	{
		return -4;
	}
	bpl = chan*(bitdepth*cell_width)/8;
	bpr = chan*bitdepth*width/8;
	
	for (j = 0; j < cell_height; j++) {
		memcpy(*(data + ((dy*cell_height)+j)) + (dx*bpl), *(data + ((sy*cell_height)+j)) + (sx*bpl), bpl);
		//memset(data + (((sy*cell_height)+j)*bpr) + (sx*bpl), 0, bpl);
		memset(*(data + ((sy*cell_height)+j)) + (sx*bpl), 0, bpl);
	}
	return 0;
}

int ShiftUpPNGCell(int sx, int sy, int shift,
                int cell_width, int cell_height,
                int width, int height, 
                png_byte color_type, png_byte bitdepth,
                png_bytep *data)
{
	int j;
	int chan = 0; // number of channels per pixel
	int bpl = 0; // bytes per cell line
	int bpr = 0; // bytes per file row
	int mx = width / cell_width; // max cell column
	int my = height / cell_height; // max cell row
	
	if ((sx >= mx) || (sy >= my)) return -1;
	
	if (color_type == PNG_COLOR_TYPE_RGB) {
		chan = 3;
	} else
	if (color_type == PNG_COLOR_TYPE_RGB_ALPHA) {
		chan = 4;
	} else
	if (color_type == PNG_COLOR_TYPE_GRAY) {
		chan = 1;
	} else
	if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		chan = 2;
	} else
	if (color_type == PNG_COLOR_TYPE_PALETTE) {
		chan = 1;
	} else
	{
		return -4;
	}
	bpl = chan*(bitdepth*cell_width)/8;
	bpr = chan*bitdepth*width/8;
	
  if (shift > 0) {
	  for (j = shift; j < cell_height; j++) {
		  memcpy(*(data + ((sy*cell_height)+j)) + (sx*bpl),
        *(data + ((sy*cell_height)+j-shift)) + (sx*bpl), bpl);
	  }
	  for (j = cell_height-shift; j < cell_height; j++) {
  		memset(*(data + ((sy*cell_height)+j)) + (sx*bpl), 0, bpl);
	  }
  } else
  if (shift < 0) {
	  for (j = cell_height+shift; j >= 0; j--) {
		  memcpy(*(data + ((sy*cell_height)+j)) + (sx*bpl),
        *(data + ((sy*cell_height)+j-shift)) + (sx*bpl), bpl);
	  }
	  for (j = 0; j < shift; j++) {
  		memset(*(data + ((sy*cell_height)+j)) + (sx*bpl), 0, bpl);
	  }
  }
	return 0;
}