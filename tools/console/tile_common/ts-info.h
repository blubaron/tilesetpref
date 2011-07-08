/***************************************************************************
 *
 *  Copyright (C) 2001-2003 Brett Reid, 
 *  All Rights Reserved.
 *
 *  File: z-info.h
 *
 *  Created:   04/27/11 by Brett Reid
 *  Last Edit: 04/27/11 by Brett Reid
 *
 ***************************************************************************/
#ifndef __Z_INFO_H_
#define __Z_INFO_H_

//#define FLAG_PLACEHOLDER      1
//#define FLAG_SHADE_ON         2
//#define FLAG_SHADE_OFF        4
//#define FLAG_OVERDRAW         FLAG_SHADE_ON
#define FLAG_DUPLICATE        8
#define FLAG_DUP_FIRST       16
#define FLAG_DUP_NOCHANGE    32
#define FLAG_DUP_WRITE       64
#define FLAG_DUP_DELAY      128

#define PLC '^'

typedef short sint16;
typedef int sint32;
typedef unsigned int uint32;

typedef struct tsb_tileset_entry {
  struct tsb_tileset_entry *pNext;
  sint16 id;
  char letter;
  char flags;
  sint16 x;
  sint16 y;
  sint16 bright_x;
  sint16 bright_y;
  sint16 dark_x;
  sint16 dark_y;
  sint16 name_len;
  sint16 filename_len;
  char *zName;
  char *zFilename;
} tsbInfoEntry;
extern tsbInfoEntry *g_pInfoRoot;


typedef struct _tsbEditorIDRedirection {
  struct _tsbEditorIDRedirection *pNext;
  char letter;
  sint16 id;
  sint16 name_len;
  char *zName;
} tsbEditorIDRedirection;
extern tsbEditorIDRedirection *g_pRedirRoot;

int tsbInitRedirEntriesFromFile(char *filename);
void tsbClearRedirEntries();

int tsbInitPlayerEntriesFromFiles();
void tsbClearPlayerEntries();

int tsbInitSpecialEntries();
int tsbInitSpecialEntriesAngbase();
int tsbInitSpecialEntriesAngband();

void tsbClearPrefs();
int tsbLoadInfoFile(char *filename, char letter);
int tsbLoadFlavorFile(char *filename, char letter);
int tsbLoadObjectFile(char *filename, char letter);

int tsbReadPrefFile(char *filename);
//int tsbSaveMainFile(char *filename);
int tsbReadXtraFile(char *filename);
//int tsbSaveXtraFile(char *filename);
int tsbReadFlvrFile(char *filename);
//int tsbSaveFlvrFile(char *filename);
void tsbClearEntries();
/*


extern zpaInfoEntry *g_pRecordListRoot,*g_pRecordListEnd;
extern zpaInfoEntry *g_pSelected;
extern char g_cCurrentList;
class zpaTileSet {
public:
  zpaInfoEntry *pNext,*pPrev;
	HBITMAP  hBitmap;
	HPALETTE hPalette;
	HBITMAP  hBitmapMask;
	HPALETTE hPaletteMask;
	sint16   CellWidth;
	sint16   CellHeight;
  sint16   s16ID;
  sint16   s16Height, s16Width;
  sint16   s16TopLeftX, s16TopLeftY;
  BYTE     bZoom;
  HWND     hWnd;
  HWND     hParent;
  cString  strName;
  cString  strMask;

  uint8  u8ViewMode;
  uint8  u8ClickType;
  sint8  s8SelectedX;
  sint8  s8SelectedY;
  sint16 s16Reserved;
  sint32 s32ClickX;
  sint32 s32ClickY;
  sint16 s16ViewHeight, s16ViewWidth;

  void Clear();
  HRESULT Load(cString &filename, int cellwidth, int cellheight, bool mask = false);
  HRESULT LoadFull(cString &filename, int cellwidth, int cellheight);
  
  HRESULT Save(cString &filename, bool mask = false);
  HRESULT SaveFull(cString &filename);
  HRESULT SavePNG(cString &filename, int mask);

  HRESULT SwapSelected(sint8 offsettoswap);
  HRESULT SwapSelected(sint16 swap_x, sint16 swap_y);
  HRESULT CopySelectedCell(zpaTileSet &src);
  HRESULT CopySelectedCellUnscaled(zpaTileSet &src);
  HRESULT CopySelectedCellOverlay(zpaTileSet &src);
  HRESULT AdjustColorByMask(int x, int y);

  zpaTileSet();
  ~zpaTileSet();
};
extern zpaTileSet g_tsWorking;
extern zpaTileSet g_tsPlaceholderSrc1;
extern zpaTileSet g_tsPlaceholderSrc2;

typedef struct _zpaEditorIDRedirection {
  struct _zpaEditorIDRedirection *pNext;
  char letter;
  sint32 s32ID;
  cString strName;
} zpaEditorIDRedirection;
extern zpaEditorIDRedirection *g_pRedirRoot;

extern cString g_strInfoDir;
extern cString g_strWorkingPref;
extern cString g_strPlaceholder1Pref;
extern cString g_strPlaceholder2Pref;
extern sint32 g_s32NameStyle;

extern sint32 g_s32LineHeight;
extern sint32 g_s32TileHeight;
extern sint32 g_s32TileWidth;

sint32 zpaInitPlayerEntries();
sint32 zpaInitPlayerEntriesFromFiles();
void zpaClearPlayerEntries();
sint32 zpaInitSpecialEntries();
sint32 zpaInitSpecialEntriesAngbase();
sint32 zpaInitSpecialEntriesAngband();

sint32 zpaInitRedirEntriesFromFile(cString &filename);
void zpaClearRedirEntries();

void zpaClearPrefs(char slot);
sint32 zpaLoadInfoFile(cString &file, char letter);
sint32 zpaLoadFlavorFile(cString &file, char letter);
sint32 zpaLoadObjectFile(cString &file, char letter);

sint32 zpaReadPrefFile(cString &file, char slot);
sint32 zpaSaveMainFile(cString &file);
sint32 zpaReadXtraFile(cString &file, char slot);
sint32 zpaSaveXtraFile(cString &file);
sint32 zpaReadFlvrFile(cString &file, char slot);
sint32 zpaSaveFlvrFile(cString &file);

void zpaCloseEntries();
*/

#endif