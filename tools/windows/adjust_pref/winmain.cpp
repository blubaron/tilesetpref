/***************************************************************************
 *
 *  Copyright (C) 2011 Brett Reid
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
HWND g_hWnd;
HINSTANCE g_hInst;
term *Term;

typedef int sint32;

char zWndClassName[] = "adjust_pref_file";
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
  //TCHAR temp[1024];
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
  ofn.lpstrFilter = "Angband PRF Files (*.prf)\0*.prf\0";
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
  if (len < 1024 - 12) {
    // create a default output name
    _tcsncpy(output,input,1024);
    _tcsncpy(output+len-4, "-result.prf", 12);
  }
  // get the output file name
  memset(&ofn,0,sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = NULL;
  ofn.lpstrFilter = "Angband PRF Files (*.prf)\0*.prf\0";
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
  
  // make sure the input and output files are not thte same
  if (_tcsncmp(input, output, 1024) == 0) {
    Message("Input and output file names cannot be the same... Aborting");
    return E_ABORT;
  }

  // load the adjustment ranges
  adjustment_entry *listroot, *listend, *temp;
  int linelen;
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
  FILE *in = file_open(input, MODE_READ, FTYPE_TEXT);
  if (!in) {
    clear_adjustment_list(listroot);
    Message("Failed to load input file... Aborting");
    return E_INVALIDARG;
  }
  FILE *out = file_open(output, MODE_WRITE, FTYPE_TEXT);
  if (!out) {
    file_close(in);
    clear_adjustment_list(listroot);
    Message("Failed to open output file... Aborting");
    return E_INVALIDARG;
  }
  int x,y,nx,ny;
  TCHAR *a,*b;

  // start logging
  len = strlen(output);
  if (len < 1024 - 9) {
    _tcsncpy(output + len, ".log.txt", 8);
    fp = file_open(output, MODE_WRITE, FTYPE_TEXT);
  }
  // read each line, and write it to the output file
  while(file_getl(in, line,256)) {
    linelen = strlen(line);

    // see if this line could have some coordinates
    if ((line[1] == _T(':')) || (line[2] == _T(':'))) {
      b = _tcsrchr(line, _T(':'));
      if ((b > line+linelen-2) || _istalpha(*(b+1)) || _istspace(*(b+1))) {
        // b is just before a flag and needs to be set back more
        a = b-1;
        // move the pointer to a previous colon
        while((*(a--) != _T(':')) && (a > line));
        b = a+1;
      }
      a = _tcschr(b, _T('/'));
      if (a && (a-line < linelen)) {
        // the coordinate pair is seperated by a slash and b is
        // the start of the coordinate
        start = b+1;
      } else {
        // the coordinate pair is seperated by a colon and b is
        // the middle of the coordinate
        start = b;
        while(_istalnum(*(--start)));
        start += 1;
      }
      y = _tcstol(start, &a, 0);
      x = _tcstol(a+1, &end, 0);
      if (coordtype == 0) {
        if (start[0] == '0') {
          if ((start[1] == _T('x')) || (start[1] == _T('X'))) {
            coordtype = 1;
          } else {
            coordtype = 3;
          }
        } else
        if ((start[0] == _T('+')) || (a[2]) == _T('+')) {
          coordtype = 4;
        } else
        {
          coordtype = 2;
        }
      }
      if (coordtype == 4) {
        x += 128;
        y += 128;
      }
      nx = 0; ny = 0;
      temp = listroot;
      while (temp) {
        if ((y >= temp->miny) && (y <= temp->maxy)) {
          if ((x >= temp->minx) && (x <= temp->maxx)) {
            ny = temp->desty + (y - temp->miny);
            nx = temp->destx + (x - temp->minx);
            break;
          }
        }
        temp = temp->pNext;
      }
      if (nx || ny) {
        // the coordinates were shifted
        // write the first part of the line
        fwrite(line,sizeof(TCHAR),start-line,out);
        // write the coordinates
        if (coordtype == 4) {
          if (ny >= 128) {
            if (nx >= 128) {
              fprintf(out, "+%d:+%d", ny-128,nx-128);
            } else {
              fprintf(out, "+%d:%d", ny-128,nx-128);
            }
          } else {
            if (nx >= 128) {
              fprintf(out, "%d:+%d", ny-128,nx-128);
            } else {
              fprintf(out, "%d:%d", ny-128,nx-128);
            }
          }
        } else
        if (coordtype == 3) {
          fprintf(out, "0%o:0%o", ny,nx);
        } else
        if (coordtype == 2) {
          fprintf(out, "%d:%d", ny,nx);
        } else
        {
          fprintf(out, "0x%.2X:0x%.2X", ny,nx);
        }
        // write the end of the line
        fwrite(end,sizeof(TCHAR),linelen-(end-line),out);
        
        if (fp) {
          if (coordtype == 4) {
            if (ny >= 128) {
              if (nx >= 128) {
                fprintf(fp, "Adjusted %s to +%d:+%d\r\n", line, ny-128,nx-128);
              } else {
                fprintf(fp, "Adjusted %s to +%d:%d\r\n", line, ny-128,nx-128);
              }
            } else {
              if (nx >= 128) {
                fprintf(fp, "Adjusted %s to %d:+%d\r\n", line, ny-128,nx-128);
              } else {
                fprintf(fp, "Adjusted %s to %d:%d\r\n", line, ny-128,nx-128);
              }
            }
          } else
          if (coordtype == 3) {
            fprintf(fp, "Adjusted %s to 0%o:0%o\r\n", line, ny,nx);
          } else
          if (coordtype == 2) {
            fprintf(fp, "Adjusted %s to %d:%d\r\n", line, ny,nx);
          } else
          {
            fprintf(fp, "Adjusted %s to 0x%.2X:0x%.2X\r\n", line, ny,nx);
          }
        }
      } else {
        // coordinates were not changed, just copy the line
        fwrite(line,sizeof(TCHAR),linelen,out);
      }
    } else {
      // if no coordinates, just copy the line
      fwrite(line,sizeof(TCHAR),linelen,out);
    }
    fwrite(_T("\r\n"),sizeof(TCHAR),2,out);
    // clear the front of the line  to prevent false positives
    memset(line,0,8);
  }

  file_close(out);
  file_close(in);
  clear_adjustment_list(listroot);
  listroot = NULL;

  if (fp) {
    fprintf(fp, "Finished... Closing\r\n");
    file_close(fp);
  }

  Message("Finished... Closing");
  return S_OK;
}
