/***************************************************************************
 *
 *  Copyright (C) 2011 Brett Reid
 *
 *  File:png_black_bg/winmain.cpp
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
#include "png.h"

HWND g_hWnd;
HINSTANCE g_hInst;

typedef int sint32;

HRESULT LoadPNG(TCHAR *filename,
                HBITMAP *color, HPALETTE *color_palette,
                HBITMAP *mask, HPALETTE *mask_palette,
                int *width, int *height);
HRESULT SavePNG(TCHAR *filename,
                HBITMAP color, HPALETTE color_palette,
                HBITMAP mask, HPALETTE mask_palette,
                int width, int height);

char zWndClassName[] = "png_premultiply_alpha";
#define Message(str) MessageBox(NULL, str, zWndClassName, MB_OK | MB_ICONERROR);if (ShowCursor(TRUE) > 1) {ShowCursor(FALSE);}

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
	HBITMAP  hBitmap;
	HPALETTE hPalette;
	HBITMAP  hBitmapMask;
	HPALETTE hPaletteMask;
  int wid,hgt;
  TCHAR input[1024];
  TCHAR output[1024];
  //TCHAR temp[1024];
  HRESULT result;

  // Set global handle
  if (!InitClass(hInst)) {
    Message("Class registration failed... Aborting");
    return 1;
  }
  
  // Initialise window
  if (!InitWindow(hInst, nCmdShow)) {
    Message("Window creation failed... Aborting");
    return 1;
  }
  input[0] = 0;
  output[0] = 0;
  // get the input file name
  OPENFILENAME ofn;
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
    return 2;
  }
  strncpy(output,input,1024);
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
    return 3;
  }
  // load the input
  result = LoadPNG(input, &hBitmap, &hPalette, &hBitmapMask, &hPaletteMask, &wid,&hgt);
  if (FAILED(result)) {
    Message("Failed to load input file... Aborting");
    return result;
  }

  // process the file
  sint32 x1 = 0;
  sint32 y1 = 0;
  sint32 x2 = wid;
  sint32 y2 = hgt;
  HDC image, temp, buffer;
  HBITMAP hbmOld, hbmOldTemp;
  COLORREF rgb,al,black,white;
  byte r,g,b,a;
  float rf,gf,bf,af;
  if (g_hWnd) {
    //image = CreateCompatibleDC(NULL);
    image = GetDC(g_hWnd);

    buffer = CreateCompatibleDC(image);
    temp = CreateCompatibleDC(image);
    
    hbmOld = (HBITMAP)SelectObject(buffer, hBitmap);
    hbmOldTemp = (HBITMAP)SelectObject(temp, hBitmapMask);

    int i,j;

    // we have a mask, for every mask pixel that is true white, make the
    // corresponding color pixel true black
    black = RGB(0,0,0);
    white = RGB(255,255,255);
    for (j = y1; j < y2; ++j) {
      for (i = x1; i < x2; ++i) {
        al = GetPixel(temp, i,j);
          rgb = GetPixel(buffer, i,j);
        if (al == white) {
          SetPixel(buffer,i,j,black);
        } else
        if (al != black) {
          //blend the color value based on this value?
          rgb = GetPixel(buffer, i,j);
          rf = ((float)(GetRValue(rgb))) / 255.f;
          gf = ((float)(GetGValue(rgb))) / 255.f;
          bf = ((float)(GetBValue(rgb))) / 255.f;
          af = (255.f-(float)(GetGValue(al))) / 255.f;
          r = (byte)(rf*af*255.f);
          g = (byte)(gf*af*255.f);
          b = (byte)(bf*af*255.f);
          rgb = RGB(r,g,b);
          SetPixel(buffer,i,j,rgb);
          SetPixel(temp,i,j,black);
        }
      }
    }

 	  SelectObject(buffer, hbmOld);
 	  SelectObject(temp, hbmOldTemp);
	  DeleteDC(temp);
	  DeleteDC(buffer);

    ReleaseDC(g_hWnd,image);
  }

  // write the output
  result = SavePNG(output, hBitmap, hPalette, hBitmapMask, hPaletteMask, wid,hgt);
  if (FAILED(result)) {
    Message("Failed to save output file... Aborting");
  }

  DeleteObject(hBitmap);
  DeleteObject(hPalette);
  DeleteObject(hBitmapMask);
  DeleteObject(hPaletteMask);

  if (FAILED(result)) {
    return result;
  }

  Message("Finished... Closing");
  return 0;
}
