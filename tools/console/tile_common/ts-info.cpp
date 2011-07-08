/***************************************************************************
 *
 *  Copyright (C) 2011-2011 Brett Reid
 *  All Rights Reserved.
 *
 *  File:z-info.cpp
 *
 *  Created:   04/23/11 by Brett Reid
 *  Last Edit: 05/11/11 by Brett Reid
 *
 ***************************************************************************/
//#include "stdafx.h"
#include <windows.h>
#include <time.h>
#include "P:/utility/cstring.h"
#include "P:/utility/fileread.h"
//#include "menuid.h"
#include "bttnid.h"
#include "link.h"
#include "file.h"
#include "z-info.h"

extern char zWndClassName[];
#define Message(str) MessageBox(NULL, str, zWndClassName, MB_OK | MB_ICONERROR);if (ShowCursor(TRUE) > 1) {ShowCursor(FALSE);}

extern BOOL g_bCommandLine;
extern uint32 g_u32EntryCounts[10];
//extern LinkList<cString *> g_lFilesLoaded;
//bool appInitFunctions() {return false;}
extern int loadflags;
extern char *version;
zpaInfoEntry *g_pRecordListRoot,*g_pRecordListEnd;
zpaInfoEntry *g_pSelected;
char g_cCurrentList;

zpaTileSet g_tsWorking;
zpaTileSet g_tsPlaceholderSrc1;
zpaTileSet g_tsPlaceholderSrc2;
cString g_strInfoDir;
cString g_strWorkingPref;
cString g_strPlaceholder1Pref;
cString g_strPlaceholder2Pref;
sint32 g_s32NameStyle;

sint32 g_s32LineHeight;
sint32 g_s32TileHeight;
sint32 g_s32TileWidth;

#define CLASS_COUNT      11
#define RACE_COUNT       30
#define SPELL_COUNT      30

sint32 zpaReadPrefFile(cString &file, char slot)
{
  // read 2 lines. If the second character of the second line is a colon, then
  // process this line as an assigned entry, and check the previous line for a
  // note in the comment and for the placeholder symbol. If the third character
  // of the second line is a colon, clear any assignment for the id, and check
  // the previous line for a note.
  int bytesread=0;
  TCHAR *pChar, *pEnd;
  sint32 id;
  cString line,prevline;
  line.SetMaxLength(255);
  sint32 x,y;
  bool valid;
  char letter;
  cString note;
  cString light;
  zpaInfoEntry *entry;
  FILE *fp = _tfopen(file.GetText(), _T("r"));
  if (!fp) {
    return -2;
  }
  g_u32EntryCounts[7] = 0;
  g_u32EntryCounts[8] = 0;
  light.SetMaxLength(24);
  // read each line, for every N: line, create an entry
  while((bytesread = FileReadLine(fp, line)) > 0) {
    if (bytesread > 2) {
      valid = false;
      pChar = line.GetText();
      if (*(pChar+1) == ':') {
        letter = *pChar;
        if (letter == '?') {
          prevline = line;
          continue;
        } else
        if (letter == '%') {
          cString str,name;
          str = file;
          name.SetText(pChar+2, 0);
          str.RemoveFilename();
          str.Append(name);
          if (_tcsstr(pChar,"flvr")) {
            zpaReadFlvrFile(str,slot);
          }
          if (_tcsstr(pChar,"xtra")) {
            zpaReadXtraFile(str,slot);
          }
          prevline = line;
          continue;
        } else
        if (*(pChar+3) == 'x') {
          // id is in hex
          id = _tcstol(pChar+4,&pEnd, 16);
        } else {
          id = _tcstol(pChar+2,&pEnd, 10);
        }
        if ((id == 0) && _istalpha(*(pChar+2))) {
          TCHAR *s;
          zpaEditorIDRedirection *rdentry = g_pRedirRoot;
          while (rdentry) {
            if (s=_tcsstr(pChar, rdentry->strName.GetText())){
              if (*(s+rdentry->strName.GetTextLength()) == ':') {
                id = rdentry->s32ID;
                pEnd = s+rdentry->strName.GetTextLength();
                break;
              }
            }
            rdentry = rdentry->pNext;
          }
          if (!rdentry) {
            prevline = line;
            continue;
          }
        }
        if ((loadflags & 256) && (letter == 'F')) {
          if (_istalpha(*(pEnd+1))) {
            light.SetText(pEnd+1,0);
            pEnd = _tcschr(pEnd+1,':');
          } else {
            light.SetText("all",3);
          }
        }
        // read the image location
        if (*(pEnd+2) == 'x') {
          // attr is in hex
          y = _tcstol(pEnd+3,&pChar, 16)-128;
        } else {
          y = _tcstol(pEnd+1,&pChar, 10);
        }
        if (*(pChar+2) == 'x') {
          // char is in hex
          x = _tcstol(pChar+3,&pEnd, 16)-128;
        } else {
          x = _tcstol(pChar+1,&pEnd, 10);
        }
        valid = true;
      } else
      if ((*(pChar) == '#') && (*(pChar+2) == ':')) {
        letter = *(pChar+1);
        id = _tcstol(pChar+3,&pEnd, 10);
        y = x = 0;
        valid = true;
      } else
      if ((*(pChar) == '#') && (*(pChar+2) == '#') && (*(pChar+5) == ':')) {
        // needed to check for a note?
        letter = *(pChar+4);
        id = _tcstol(pChar+6,&pEnd, 10);
        y = x = 0;
        valid = true;
      }

      if (valid) {
        TCHAR *place, *note;
        
        // find the entry
        entry = g_pRecordListRoot;
        while (entry) {
          if ((letter == entry->letter) && (entry->s16ID == id)) {
            break;
          }
          entry = entry->pNext;
        }
        if (!entry) continue; // ignore it if we do not already have an entry for it

        if (slot == 2) {
          //entry->s16P2Y = y;
          //entry->s16P2X = x;
          if ((letter == 'F') && (loadflags & 256)) {
            if (_tcsncmp(light.GetText(),"all", 3) == 0) {
              entry->s16P2Y = y;
              entry->s16P2X = x;
            } else
            if (_tcsncmp(light.GetText(),"lit", 3) == 0) {
              entry->s16P2Y = y;
              entry->s16P2X = x;
            }
          } else {
            entry->s16P2Y = y;
            entry->s16P2X = x;
          }
        } else
        if (slot == 1) {
          //entry->s16P1Y = y;
          //entry->s16P1X = x;
          if ((letter == 'F') && (loadflags & 256)) {
            if (_tcsncmp(light.GetText(),"all", 3) == 0) {
              entry->s16P1Y = y;
              entry->s16P1X = x;
            } else
            if (_tcsncmp(light.GetText(),"lit", 3) == 0) {
              entry->s16P1Y = y;
              entry->s16P1X = x;
            }
          } else {
            entry->s16P1Y = y;
            entry->s16P1X = x;
          }
        } else
        if (slot == 0) {
          //entry->s16Y = y;
          //entry->s16X = x;
          if ((letter == 'F') && (loadflags & 256)) {
            if (_tcsncmp(light.GetText(),"bright", 6) == 0) {
              entry->s16Yl = y;
              entry->s16Xl = x;
              entry->u16Flags |= FLAG_SHADE_OFF;
            } else
            if (_tcsncmp(light.GetText(),"dark", 4) == 0) {
              entry->s16Yd = y;
              entry->s16Xd = x;
              entry->u16Flags |= FLAG_SHADE_ON;
            } else
            if (_tcsncmp(light.GetText(),"lit", 3) == 0) {
              entry->s16Y = y;
              entry->s16X = x;
            } else
            {
              entry->u16Flags &= ~(FLAG_SHADE_ON|FLAG_SHADE_OFF);
              entry->s16Y = y;
              entry->s16X = x;
              entry->s16Yd = y;
              entry->s16Xd = x;
              entry->s16Yl = y;
              entry->s16Xl = x;
            }
          } else {
            entry->s16Y = y;
            entry->s16X = x;
          }
          if ((letter == 'F') && pEnd && (*pEnd == ':')) {// _tcschr(pEnd+1, ':')) {
            // see if there are additional coordinates on the line
            // read the image location
            if (*(pEnd+2) == 'x') {
              // id is in hex
              entry->s16Yd = _tcstol(pEnd+3,&pChar, 16)-128;
            } else {
              entry->s16Yd = _tcstol(pEnd+1,&pChar, 10);
            }
            if (*(pChar+2) == 'x') {
              // id is in hex
              entry->s16Xd = _tcstol(pChar+3,&pEnd, 16)-128;
            } else {
              entry->s16Xd = _tcstol(pChar+1,&pEnd, 10);
            }
            entry->u16Flags |= FLAG_SHADE_ON;
            if (pEnd && (*pEnd == ':')) { // _tcschr(pEnd+1, ':')) {
              // see if there are additional coordinates on the line
              // read the image location
              if (*(pEnd+2) == 'x') {
                // id is in hex
                entry->s16Yl = _tcstol(pEnd+3,&pChar, 16)-128;
              } else {
                entry->s16Yl = _tcstol(pEnd+1,&pChar, 10);
              }
              if (*(pChar+2) == 'x') {
                // id is in hex
                entry->s16Xl = _tcstol(pChar+3,&pEnd, 16)-128;
              } else {
                entry->s16Xl = _tcstol(pChar+1,&pEnd, 10);
              }
              entry->u16Flags |= FLAG_SHADE_OFF;
            } else {
              entry->u16Flags &= ~(FLAG_SHADE_OFF);
              entry->s16Yl = entry->s16Y;
              entry->s16Xl = entry->s16X;
            }
          } else if (!(loadflags & 256)) {
            entry->u16Flags &= ~(FLAG_SHADE_ON|FLAG_SHADE_OFF);
            entry->s16Yd = entry->s16Y;
            entry->s16Xd = entry->s16X;
            entry->s16Yl = entry->s16Y;
            entry->s16Xl = entry->s16X;
          }
          // see if the comments are at the end of the line
          place = _tcschr(pEnd, '#');
          if (place) {
            // if so scan for the placeholder symbol
            place = _tcschr(pEnd, PLC);
            if (place) {
              entry->u16Flags |= FLAG_PLACEHOLDER;
            }
            // and read the note
            note = strchr(pEnd, '(');
            if (note) {
              entry->strNote.SetText(note+1,0);
            } else {
              entry->strNote.Clear();
            }
          } else {
            // if not, scan for the placeholder and note on the previous line
            if (prevline.GetTextLength() > 3) {
              place = strchr(prevline.GetText(), PLC);
              if (place) {
                entry->u16Flags |= FLAG_PLACEHOLDER;
              }
              // and read the note
              note = strchr(prevline.GetText(), '(');
              if (note) {
                entry->strNote.SetText(note+1,0);
                entry->strNote.CountTextLength();
              } else {
                entry->strNote.Clear();
              }
            }
          }
          if (entry->strNote.GetTextLength()>1) {
            place = strrchr(entry->strNote.GetText(), ')');
            if (place && note) {
              entry->strNote.SetChar(place-entry->strNote.GetText()+1, 0);
              entry->strNote.CountTextLength();
            }
          }
        } // if (slot == 0)
      } // if (valid)
      prevline = line;
    } // if bytesread
  } // while
  // count the unassigned and placeholder entries
  entry = g_pRecordListRoot;
  while (entry) {
    if (entry->u16Flags & FLAG_PLACEHOLDER) {
      g_u32EntryCounts[8]++;
    }
    if ((entry->s16X <= 0) && (entry->s16Y <= 0)) {
      g_u32EntryCounts[7]++;
    }
    entry = entry->pNext;
  }
  if (slot == 1) {
    g_strPlaceholder1Pref = file;
  } else
  if (slot == 2) {
    g_strPlaceholder2Pref = file;
  } else
  {
    g_strWorkingPref = file;
  }
  prevline.Clear();
  line.Clear();
  return 0;
}


sint32 zpaSaveMainFile(cString &file)
{
  zpaInfoEntry *entry;
  FILE *fp = _tfopen(file.GetText(), _T("wb"));
  if (!fp) {
    return -2;
  }
  // write the header
  cString str;
  str = file;
  if (str.GetMaxLength() < 9) str.SetMaxLength(15);
  str.RemoveFilepath();
  fprintf(fp, "# File: %s\r\n", str.GetText());
  fprintf(fp, "#\r\n");
  fprintf(fp, "#\r\n");
  fprintf(fp, "# This file defines special attr/char mappings for use in a \"graphics\" mode\r\n");
  fprintf(fp, "#\r\n");
  fprintf(fp, "# Initially used with David Gervais's 32x32 tiles.\r\n");
  fprintf(fp, "# Converted by Mogami from the dg32-assign of AngbandTK\r\n");
  fprintf(fp, "# Z+Angband graphic mapping by Buzzkill, assisted by Mangojuice.\r\n");
  fprintf(fp, "# Last updated November 2008\r\n");
  fprintf(fp, "#\r\n");
  fprintf(fp, "# This file generated by Blue Baron's Tileset Editor v.%s on %s.\r\n",version, _tstrdate(str.GetTextBuffer()));
  fprintf(fp, "#\r\n");
  fprintf(fp, "# See \"lib/help/command.txt\" and \"src/files.c\" for more information.\r\n");
  fprintf(fp, "#\r\n");

  // write the T: entries
  fprintf(fp, "\r\n##### thaumaturgical fields ##### t_info.txt\r\n\r\n");
  entry = g_pRecordListRoot;
  while (entry) {
    if (entry->letter == 'T') {
      if (entry->s16Y || entry->s16X) {
        if (entry->strNote.GetTextLength() > 3)  {
          if (entry->u16Flags & FLAG_SHADE_ON)  {
            fprintf(fp, "T:%d:0x%.2X:0x%.2X:L # %s %c (%s)\r\n", entry->s16ID,
              entry->s16Y+128, entry->s16X+128, entry->strName.GetText(),
              //entry->s16Y|0x80, entry->s16X|0x80, entry->strName.GetText(),
              ((entry->u16Flags & FLAG_PLACEHOLDER) ? PLC : ' '), entry->strNote.GetText());
          } else if (entry->u16Flags & FLAG_SHADE_OFF)  {
            fprintf(fp, "T:%d:0x%.2X:0x%.2X:l # %s %c (%s)\r\n", entry->s16ID,
              entry->s16Y+128, entry->s16X+128, entry->strName.GetText(),
              //entry->s16Y|0x80, entry->s16X|0x80, entry->strName.GetText(),
              ((entry->u16Flags & FLAG_PLACEHOLDER) ? PLC : ' '), entry->strNote.GetText());
          } else {
            fprintf(fp, "T:%d:0x%.2X:0x%.2X # %s %c (%s)\r\n", entry->s16ID,
              entry->s16Y+128, entry->s16X+128, entry->strName.GetText(),
              //entry->s16Y|0x80, entry->s16X|0x80, entry->strName.GetText(),
              ((entry->u16Flags & FLAG_PLACEHOLDER) ? PLC : ' '), entry->strNote.GetText());
          }
        } else {
          if (entry->u16Flags & FLAG_SHADE_ON)  {
            fprintf(fp, "T:%d:0x%.2X:0x%.2X:L # %s %c\r\n", entry->s16ID,
              entry->s16Y+128, entry->s16X+128, entry->strName.GetText(),
              //entry->s16Y|0x80, entry->s16X|0x80, entry->strName.GetText(),
              ((entry->u16Flags & FLAG_PLACEHOLDER) ? PLC : ' '));
          } else if (entry->u16Flags & FLAG_SHADE_OFF)  {
            fprintf(fp, "T:%d:0x%.2X:0x%.2X:l # %s %c\r\n", entry->s16ID,
              entry->s16Y+128, entry->s16X+128, entry->strName.GetText(),
              //entry->s16Y|0x80, entry->s16X|0x80, entry->strName.GetText(),
              ((entry->u16Flags & FLAG_PLACEHOLDER) ? PLC : ' '));
          } else {
            fprintf(fp, "T:%d:0x%.2X:0x%.2X # %s %c\r\n", entry->s16ID,
              entry->s16Y+128, entry->s16X+128, entry->strName.GetText(),
              //entry->s16Y|0x80, entry->s16X|0x80, entry->strName.GetText(),
              ((entry->u16Flags & FLAG_PLACEHOLDER) ? PLC : ' '));
          }
        }
      } else {
        fprintf(fp, "### T:%d:          # %s\r\n", entry->s16ID,
          entry->strName.GetText());
      }
    }
    entry = entry->pNext;
  }
  // write the F: entries
  fprintf(fp, "\r\n##### Feature attr/char definitions ##### f_info.txt\r\n\r\n");
  entry = g_pRecordListRoot;
  while (entry) {
    if (entry->letter == 'F') {
      if (entry->s16Y || entry->s16X) {
        if (entry->strNote.GetTextLength() > 3)  {
          if (loadflags & 256) {
            if (entry->u16Flags & (FLAG_SHADE_ON|FLAG_SHADE_OFF)) {
              fprintf(fp, "F:%d:bright:0x%.2X:0x%.2X\r\n", entry->s16ID,
                entry->s16Yl+128, entry->s16Xl+128);
              fprintf(fp, "F:%d:lit:0x%.2X:0x%.2X # %s %c (%s)\r\n", entry->s16ID,
                entry->s16Y+128, entry->s16X+128, 
                entry->strName.GetText(),
                ((entry->u16Flags & FLAG_PLACEHOLDER) ? PLC : ' '), entry->strNote.GetText());
              fprintf(fp, "F:%d:dark:0x%.2X:0x%.2X\r\n", entry->s16ID,
                entry->s16Yd+128, entry->s16Xd+128);
            } else {
              fprintf(fp, "F:%d:all:0x%.2X:0x%.2X # %s %c (%s)\r\n", entry->s16ID,
                entry->s16Y+128, entry->s16X+128, 
                entry->strName.GetText(),
                //entry->s16Y|0x80, entry->s16X|0x80, entry->strName.GetText(),
                ((entry->u16Flags & FLAG_PLACEHOLDER) ? PLC : ' '), entry->strNote.GetText());
            }
          } else
          if ((entry->s16Xl != entry->s16X) || (entry->s16Yl != entry->s16Y)) {
            if ((entry->s16Xd != entry->s16X) || (entry->s16Yd != entry->s16Y)) {
              fprintf(fp, "F:%d:0x%.2X:0x%.2X:0x%.2X:0x%.2X:0x%.2X:0x%.2X # %s %c (%s)\r\n", entry->s16ID,
                entry->s16Y+128, entry->s16X+128, 
                entry->s16Yd+128, entry->s16Xd+128,
                entry->s16Yl+128, entry->s16Xl+128,
                entry->strName.GetText(),
                //entry->s16Y|0x80, entry->s16X|0x80, entry->strName.GetText(),
                ((entry->u16Flags & FLAG_PLACEHOLDER) ? PLC : ' '), entry->strNote.GetText());
            } else {
              fprintf(fp, "F:%d:0x%.2X:0x%.2X:0x%.2X:0x%.2X:0x%.2X:0x%.2X # %s %c (%s)\r\n", entry->s16ID,
                entry->s16Y+128, entry->s16X+128, 
                entry->s16Y+128, entry->s16X+128,
                entry->s16Yl+128, entry->s16Xl+128,
                entry->strName.GetText(),
                //entry->s16Y|0x80, entry->s16X|0x80, entry->strName.GetText(),
                ((entry->u16Flags & FLAG_PLACEHOLDER) ? PLC : ' '), entry->strNote.GetText());
            }
          } else
          if ((entry->s16Xd != entry->s16X) || (entry->s16Yd != entry->s16Y)) {
            fprintf(fp, "F:%d:0x%.2X:0x%.2X:0x%.2X:0x%.2X # %s %c (%s)\r\n", entry->s16ID,
              entry->s16Y+128, entry->s16X+128, 
              entry->s16Yd+128, entry->s16Xd+128, entry->strName.GetText(),
              //entry->s16Y|0x80, entry->s16X|0x80, entry->strName.GetText(),
              ((entry->u16Flags & FLAG_PLACEHOLDER) ? PLC : ' '), entry->strNote.GetText());
          } else
          {
            fprintf(fp, "F:%d:0x%.2X:0x%.2X # %s %c (%s)\r\n", entry->s16ID,
              entry->s16Y+128, entry->s16X+128, entry->strName.GetText(),
              //entry->s16Y|0x80, entry->s16X|0x80, entry->strName.GetText(),
              ((entry->u16Flags & FLAG_PLACEHOLDER) ? PLC : ' '), entry->strNote.GetText());
          }
          /*if (entry->u16Flags & FLAG_SHADE_ON)  {
            fprintf(fp, "F:%d:0x%.2X:0x%.2X:L # %s %c (%s)\r\n", entry->s16ID,
              entry->s16Y+128, entry->s16X+128, entry->strName.GetText(),
              //entry->s16Y|0x80, entry->s16X|0x80, entry->strName.GetText(),
              ((entry->u16Flags & FLAG_PLACEHOLDER) ? PLC : ' '), entry->strNote.GetText());
          } else if (entry->u16Flags & FLAG_SHADE_OFF)  {
            fprintf(fp, "F:%d:0x%.2X:0x%.2X:l # %s %c (%s)\r\n", entry->s16ID,
              entry->s16Y+128, entry->s16X+128, entry->strName.GetText(),
              //entry->s16Y|0x80, entry->s16X|0x80, entry->strName.GetText(),
              ((entry->u16Flags & FLAG_PLACEHOLDER) ? PLC : ' '), entry->strNote.GetText());
          } else {
            fprintf(fp, "F:%d:0x%.2X:0x%.2X # %s %c (%s)\r\n", entry->s16ID,
              entry->s16Y+128, entry->s16X+128, entry->strName.GetText(),
              //entry->s16Y|0x80, entry->s16X|0x80, entry->strName.GetText(),
              ((entry->u16Flags & FLAG_PLACEHOLDER) ? PLC : ' '), entry->strNote.GetText());
          }*/
        } else {
          if (loadflags & 256) {
            if (entry->u16Flags & (FLAG_SHADE_ON|FLAG_SHADE_OFF)) {
              fprintf(fp, "F:%d:bright:0x%.2X:0x%.2X\r\n", entry->s16ID,
                entry->s16Yl+128, entry->s16Xl+128);
              fprintf(fp, "F:%d:lit:0x%.2X:0x%.2X # %s %c\r\n", entry->s16ID,
                entry->s16Y+128, entry->s16X+128, 
                entry->strName.GetText(),
                ((entry->u16Flags & FLAG_PLACEHOLDER) ? PLC : ' '));
              fprintf(fp, "F:%d:dark:0x%.2X:0x%.2X\r\n", entry->s16ID,
                entry->s16Yd+128, entry->s16Xd+128);
            } else {
              fprintf(fp, "F:%d:all:0x%.2X:0x%.2X # %s %c\r\n", entry->s16ID,
                entry->s16Y+128, entry->s16X+128, 
                entry->strName.GetText(),
                //entry->s16Y|0x80, entry->s16X|0x80, entry->strName.GetText(),
                ((entry->u16Flags & FLAG_PLACEHOLDER) ? PLC : ' '));
            }
          } else
          if ((entry->s16Xl != entry->s16X) || (entry->s16Yl != entry->s16Y)) {
            if ((entry->s16Xd != entry->s16X) || (entry->s16Yd != entry->s16Y)) {
              fprintf(fp, "F:%d:0x%.2X:0x%.2X:0x%.2X:0x%.2X:0x%.2X:0x%.2X # %s %c\r\n", entry->s16ID,
                entry->s16Y+128, entry->s16X+128, 
                entry->s16Yd+128, entry->s16Xd+128,
                entry->s16Yl+128, entry->s16Xl+128,
                entry->strName.GetText(),
                //entry->s16Y|0x80, entry->s16X|0x80, entry->strName.GetText(),
                ((entry->u16Flags & FLAG_PLACEHOLDER) ? PLC : ' '));
            } else {
              fprintf(fp, "F:%d:0x%.2X:0x%.2X:0x%.2X:0x%.2X:0x%.2X:0x%.2X # %s %c\r\n", entry->s16ID,
                entry->s16Y+128, entry->s16X+128, 
                entry->s16Y+128, entry->s16X+128, 
                entry->s16Yl+128, entry->s16Xl+128, entry->strName.GetText(),
                //entry->s16Y|0x80, entry->s16X|0x80, entry->strName.GetText(),
                ((entry->u16Flags & FLAG_PLACEHOLDER) ? PLC : ' '));
            }
          } else
          if ((entry->s16Xd != entry->s16X) || (entry->s16Yd != entry->s16Y)) {
            fprintf(fp, "F:%d:0x%.2X:0x%.2X:0x%.2X:0x%.2X # %s %c\r\n", entry->s16ID,
              entry->s16Y+128, entry->s16X+128, 
              entry->s16Yd+128, entry->s16Xd+128, entry->strName.GetText(),
              //entry->s16Y|0x80, entry->s16X|0x80, entry->strName.GetText(),
              ((entry->u16Flags & FLAG_PLACEHOLDER) ? PLC : ' '));
          } else
          {
            fprintf(fp, "F:%d:0x%.2X:0x%.2X # %s %c\r\n", entry->s16ID,
              entry->s16Y+128, entry->s16X+128, entry->strName.GetText(),
              //entry->s16Y|0x80, entry->s16X|0x80, entry->strName.GetText(),
              ((entry->u16Flags & FLAG_PLACEHOLDER) ? PLC : ' '));
          }
          /*if (entry->u16Flags & FLAG_SHADE_ON)  {
            fprintf(fp, "F:%d:0x%.2X:0x%.2X:L # %s %c\r\n", entry->s16ID,
              entry->s16Y+128, entry->s16X+128, entry->strName.GetText(),
              //entry->s16Y|0x80, entry->s16X|0x80, entry->strName.GetText(),
              ((entry->u16Flags & FLAG_PLACEHOLDER) ? PLC : ' '));
          } else if (entry->u16Flags & FLAG_SHADE_OFF)  {
            fprintf(fp, "F:%d:0x%.2X:0x%.2X:l # %s %c\r\n", entry->s16ID,
              entry->s16Y+128, entry->s16X+128, entry->strName.GetText(),
              //entry->s16Y|0x80, entry->s16X|0x80, entry->strName.GetText(),
              ((entry->u16Flags & FLAG_PLACEHOLDER) ? PLC : ' '));
          } else {
            fprintf(fp, "F:%d:0x%.2X:0x%.2X # %s %c\r\n", entry->s16ID,
              entry->s16Y+128, entry->s16X+128, entry->strName.GetText(),
              //entry->s16Y|0x80, entry->s16X|0x80, entry->strName.GetText(),
              ((entry->u16Flags & FLAG_PLACEHOLDER) ? PLC : ' '));
          }*/
        }
      } else {
        fprintf(fp, "### F:%d:          # %s\r\n", entry->s16ID,
          entry->strName.GetText());
      }
    }
    entry = entry->pNext;
  }
  // write the R: entries
  fprintf(fp, "\r\n##### Monster attr/char definitions ##### r_info.txt\r\n\r\n");
  entry = g_pRecordListRoot;
  while (entry) {
    if (entry->letter == 'R') {
      if (entry->s16Y || entry->s16X) {
        if (entry->strNote.GetTextLength() > 3)  {
          fprintf(fp, "R:%d:0x%.2X:0x%.2X%s # %s %c (%s)\r\n", entry->s16ID,
            entry->s16Y+128, entry->s16X+128,
              //entry->s16Y|0x80, entry->s16X|0x80,
            ((entry->u16Flags & FLAG_OVERDRAW) ? ":O" : " "),
            entry->strName.GetText(),
            ((entry->u16Flags & FLAG_PLACEHOLDER) ? PLC : ' '),
            entry->strNote.GetText());
        } else {
          fprintf(fp, "R:%d:0x%.2X:0x%.2X%s # %s %c\r\n", entry->s16ID,
            entry->s16Y+128, entry->s16X+128,
              //entry->s16Y|0x80, entry->s16X|0x80,
            ((entry->u16Flags & FLAG_OVERDRAW) ? ":O" : " "),
            entry->strName.GetText(),
            ((entry->u16Flags & FLAG_PLACEHOLDER) ? PLC : ' '));
        }
      } else {
        if (entry->strNote.GetTextLength() > 3)  {
          fprintf(fp, "### R:%d:          # %s (%s)\r\n", entry->s16ID,
            entry->strName.GetText(), entry->strNote.GetText());
        } else {
          fprintf(fp, "### R:%d:          # %s\r\n", entry->s16ID,
            entry->strName.GetText());
        }
      }
    }
    entry = entry->pNext;
  }
  // write the K: entries
  fprintf(fp, "\r\n##### Object attr/char definitions ##### k_info.txt\r\n\r\n");
  entry = g_pRecordListRoot;
  while (entry) {
    if (entry->letter == 'K') {
      zpaEditorIDRedirection *rdentry = g_pRedirRoot;
      while (rdentry) {
        if ((rdentry->letter == 'K') && (rdentry->s32ID == entry->s16ID)) {
          break;
        }
        rdentry = rdentry->pNext;
      }
      if (entry->s16Y || entry->s16X) {
        if (entry->strNote.GetTextLength() > 3)  {
          if (rdentry) {
            fprintf(fp, "%s:0x%.2X:0x%.2X # %s %c (%s)\r\n", rdentry->strName.GetText(),
              entry->s16Y+128, entry->s16X+128, entry->strName.GetText(),
              ((entry->u16Flags & FLAG_PLACEHOLDER) ? PLC : ' '), entry->strNote.GetText());
          } else {
            fprintf(fp, "K:%d:0x%.2X:0x%.2X # %s %c (%s)\r\n", entry->s16ID,
              entry->s16Y+128, entry->s16X+128, entry->strName.GetText(),
                //entry->s16Y|0x80, entry->s16X|0x80, entry->strName.GetText(),
              ((entry->u16Flags & FLAG_PLACEHOLDER) ? PLC : ' '), entry->strNote.GetText());
          }
        } else {
          if (rdentry) {
            fprintf(fp, "%s:0x%.2X:0x%.2X # %s %c\r\n", rdentry->strName.GetText(),
              entry->s16Y+128, entry->s16X+128, entry->strName.GetText(),
              ((entry->u16Flags & FLAG_PLACEHOLDER) ? PLC : ' '));
          } else {
            fprintf(fp, "K:%d:0x%.2X:0x%.2X # %s %c\r\n", entry->s16ID,
              entry->s16Y+128, entry->s16X+128, entry->strName.GetText(),
                //entry->s16Y|0x80, entry->s16X|0x80, entry->strName.GetText(),
              ((entry->u16Flags & FLAG_PLACEHOLDER) ? PLC : ' '));
          }
        }
      } else {
        if (entry->strNote.GetTextLength() > 3)  {
          if (rdentry) {
            fprintf(fp, "### %s:          # %s (%s)\r\n", rdentry->strName.GetText(),
              entry->strName.GetText(), entry->strNote.GetText());
          } else {
            fprintf(fp, "### K:%d:          # %s (%s)\r\n", entry->s16ID,
              entry->strName.GetText(), entry->strNote.GetText());
          }
        } else {
          if (rdentry) {
            fprintf(fp, "### %s:          # %s\r\n", rdentry->strName.GetText(),
              entry->strName.GetText());
          } else {
            fprintf(fp, "### K:%d:          # %s\r\n", entry->s16ID,
              entry->strName.GetText());
          }
        }
      }
    }
    entry = entry->pNext;
  }

  // write the trailer
  cString str2;
  str = file;
  str2 = file;
  str.RemoveFilepath();
  fprintf(fp, "\r\n# Load the flavor pictures\r\n");
  TCHAR *c = _tcsstr(str.GetText(), "graf");
  if (c) {
    c[0] = 'f';
    c[1] = 'l';
    c[2] = 'v';
    c[3] = 'r';
    fprintf(fp, "%%:%s\r\n", str.GetText());
    str2.RemoveFilename();
    str2.Append(str);
    zpaSaveFlvrFile(str2);
    
    fprintf(fp, "\r\n# Load the special player pictures\r\n");
    c[0] = 'x';
    c[1] = 't';
    c[2] = 'r';
    c[3] = 'a';
    fprintf(fp, "%%:%s\r\n", str.GetText());
    str2.RemoveFilename();
    str2.Append(str);
    zpaSaveXtraFile(str2);
  }
  /*fprintf(fp, "\r\n# Load the special player pictures\r\n");
  if (file.GetTextLength() > 11) {
    file.SetChar(g_s32NameStyle,'x');
    file.SetChar(g_s32NameStyle+1,'t');
    file.SetChar(g_s32NameStyle+2,'r');
    file.SetChar(g_s32NameStyle+3,'a');
    fprintf(fp, "%%:%s.prf\r\n", file.GetText());
    zpaSaveXtraFile(file);
  }*/
  fclose(fp);
  return 0;
}
void zpaClearPrefs(char slot)
{
  zpaInfoEntry *entry;
  // find the entry
  if (slot == 2) {
    entry = g_pRecordListRoot;
    while (entry) {
      entry->s16P2X = 0;
      entry->s16P2Y = 0;
      entry = entry->pNext;
    }
  } else
  if (slot == 1) {
    entry = g_pRecordListRoot;
    while (entry) {
      entry->s16P1X = 0;
      entry->s16P1Y = 0;
      entry = entry->pNext;
    }
  } else
  {
    entry = g_pRecordListRoot;
    while (entry) {
      entry->s16X = 0;
      entry->s16Y = 0;
      entry = entry->pNext;
    }
  }
}
sint32 zpaLoadInfoFile(cString &file, char letter)
{
  int bytesread=0;
  TCHAR *pChar, *pEnd;
  //int res = 0;
  //sint32 len;
  cString line;
  line.SetMaxLength(255);

  FILE *fp = _tfopen(file.GetText(), _T("rb"));
  if (!fp) {
    return -2;
  }
  // read each line, for every N: line, create an entry
  while((bytesread = FileReadLine(fp, line)) > 0) {
    if (bytesread > 1) {
      //pChar = line.GetWord(1, NULL, temp);
      pChar = line.GetText();
      if (pChar == NULL)
        return E_INVALIDARG;

      // test valid commands to see if the command is valid
      if (*pChar == '#') {
        // we have a comment return
        continue;
      }
      if (*pChar == 'N') {
        // parse the line
        sint32 id;
        cString str;
        id = _tcstol(pChar+2,&pEnd, 10);
        str.SetText(pEnd+1,0);
        if (str.GetTextLength() < 2) {
          continue;
        }
        // create an entry
        zpaInfoEntry *entry = new zpaInfoEntry();
        if (!entry) {
          fclose(fp);
          return -1;
        }
        entry->s16ID = id;
        entry->strName = str;
        entry->letter = letter;
        AddEntry<zpaInfoEntry>(entry, &g_pRecordListRoot,&g_pRecordListEnd);
        continue;
      }
      // Ingore the other lines
    }
  }
  fclose(fp);
  return 0;
}
void zpaCloseEntries()
{
  DeleteEntries<zpaInfoEntry>(&g_pRecordListRoot,&g_pRecordListEnd);
  g_pSelected = NULL;
}
sint32 zpaLoadFlavorFile(cString &file, char letter)
{
  int bytesread=0;
  TCHAR *pChar, *pEnd;
  //int res = 0;
  //sint32 len;
  cString line;
  line.SetMaxLength(255);
  zpaInfoEntry *entry = NULL;
  FILE *fp = _tfopen(file.GetText(), _T("rb"));
  if (!fp) {
    return -2;
  }
  // read each line, for every N: line, create an entry
  while((bytesread = FileReadLine(fp, line)) > 0) {
    if (bytesread > 1) {
      //pChar = line.GetWord(1, NULL, temp);
      pChar = line.GetText();
      if (pChar == NULL)
        return E_INVALIDARG;

      // test valid commands to see if the command is valid
      if (*pChar == '#') {
        // we have a comment return
        continue;
      }
      if (*pChar == 'N') {
        // parse the line
        sint32 id;
        cString str;
        id = _tcstol(pChar+2,&pEnd, 10);
        str.SetText(pEnd+1,0);
        if (str.GetTextLength() < 1) {
          continue;
        }
        // create an entry
        entry = new zpaInfoEntry();
        if (!entry) {
          fclose(fp);
          return -1;
        }
        entry->s16ID = id;
        entry->strName = str;
        entry->letter = letter;
        AddEntry<zpaInfoEntry>(entry, &g_pRecordListRoot,&g_pRecordListEnd);
        continue;
      }
      if ((*pChar == 'D') && (letter = 'L') && entry) {
        cString str;
        str.SetText(pChar+2,0);
        entry->strName = str;
      }
      // Ingore the other lines
    }
  }
  fclose(fp);
  return 0;
}
sint32 zpaLoadObjectFile(cString &file, char letter)
{
  int bytesread=0;
  TCHAR *pChar, *pEnd;
  //int res = 0;
  //sint32 len;
  cString line;
  line.SetMaxLength(255);
  zpaInfoEntry *entry = NULL;
  FILE *fp = _tfopen(file.GetText(), _T("rb"));
  if (!fp) {
    return -2;
  }
  // read each line, for every N: line, create an entry
  while((bytesread = FileReadLine(fp, line)) > 0) {
    if (bytesread > 1) {
      //pChar = line.GetWord(1, NULL, temp);
      pChar = line.GetText();
      if (pChar == NULL)
        return E_INVALIDARG;

      // test valid commands to see if the command is valid
      if (*pChar == '#') {
        // we have a comment return
        continue;
      }
      if (*pChar == 'N') {
        // parse the line
        sint32 id;
        cString str;
        id = _tcstol(pChar+2,&pEnd, 10);
        str.SetText(pEnd+1,0);
        if (str.GetTextLength() < 1) {
          continue;
        }
        // create an entry
        entry = new zpaInfoEntry();
        if (!entry) {
          fclose(fp);
          return -1;
        }
        entry->s16ID = id;
        entry->strName = str;
        entry->letter = letter;
        AddEntry<zpaInfoEntry>(entry, &g_pRecordListRoot,&g_pRecordListEnd);
        continue;
      }
      if ((*pChar == 'D') && (letter = 'L') && entry) {
        cString str;
        str.SetText(pChar+2,0);
        entry->strName = str;
      }
      // Ingore the other lines
    }
  }
  fclose(fp);
  return 0;
}

zpaInfoEntry::zpaInfoEntry()
{
  s16ID = 0;
  pNext = NULL;
  pPrev = NULL;
  s16P1X = 0;
  s16P1Y = 0;
  s16P2X = 0;
  s16P2Y = 0;
  s16X = 0;
  s16Y = 0;
  u16Flags = 0;
  letter = 0;
  //entry->strName = str;
  //strNote = 0;
}
zpaInfoEntry::~zpaInfoEntry()
{
  strName.Clear();
  strNote.Clear();
}
