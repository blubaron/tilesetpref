/***************************************************************************
 *
 *  Copyright (C) 2011 Brett Reid
 *
 *  File:tilebuild\readpref.c
 *  Purpose: read the info from an Angband pref file.
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
#include <stdio.h>
#include "angband.h"
#include "ts-info.h"

//tsbInfoEntry *g_pTSEntryRoot;
tsbInfoEntry *g_pInfoRoot;

static void tsbAddInfoEntryS(tsbInfoEntry *entry, tsbInfoEntry **root)
{
  if (entry == NULL) return;
  if (root) {
    tsbInfoEntry* temp = *root;
    while (temp) {
      if (temp->pNext == NULL) {
        break;
      }
      temp = temp->pNext;
    }
    if (temp) {
      // we have found the end of the list
      temp->pNext = entry;
    } else {
      // put it at the beginning of the list;
      entry->pNext = *root;
      *root = entry;
    }
  }
  if (root) {
    if (*root == NULL) *root = entry;
  }
}

sint32 tsbReadPrefFile(char *filename)
{
  // read 2 lines. If the second character of the second line is a colon, then
  // process this line as an assigned entry, and check the previous line for a
  // note in the comment and for the placeholder symbol. If the third character
  // of the second line is a colon, clear any assignment for the id, and check
  // the previous line for a note.
  int bytesread=0, i;
  char *pChar, *pEnd;
  sint32 id = 0;
  char line[256];
  sint32 x,y;
  char letter;
  int idsegments;

  tsbInfoEntry *entry;
  FILE *fp = file_open(filename, MODE_READ, FTYPE_TEXT);
  if (!fp) {
    return -2;
  }
  // read each line, and see if we have an entry for it
  while(file_getl(fp, line,160)) {
    bytesread = strlen(line);
    if (bytesread < 2) {
      continue;
    }
    idsegments = 2;
    pChar = line;
    if ((line[1] == ':') || (strncmp(pChar,"GF:",3) == 0)) {
      letter = line[0];
      if (letter == '?') {
        continue;
      } else
      if (letter == '#') {
        continue;
      } else
      if (letter == '%') {
        // build the file path/name that this will use
        char *n,*p,*file;
        int len = strlen(filename);
        file = (char*) malloc(sizeof(char)*(len+1));
        strncpy(file, filename, len);
        file[len] = 0;
        p = NULL;
        n = file;
        while (n = strstr(n+1,"graf")) {
          p = n;
        }
        if (strstr(pChar,"flvr")) {
          if (p) {
            strncpy(p,"flvr",4);
            tsbReadFlvrFile(file);
          } else {
            tsbReadFlvrFile(pChar+2);
          }
        }
        if (strstr(pChar,"xtra")) {
          if (p) {
            strncpy(p,"xtra",4);
            tsbReadXtraFile(file);
          } else {
            tsbReadXtraFile(pChar+2);
          }
        }
        continue;
      } else
      if (letter == 'K') {
        idsegments = 3;
      } else
      if (strncmp(pChar,"GF:",3) == 0) {
        idsegments = 3;
      } else
      if (letter == 'F') {
        idsegments = 3;
      }
      entry = ZNEW(tsbInfoEntry);
      if (!entry) {
        file_close(fp);
        return -3;
      }
      entry->letter = letter;
      entry->id = id;
      entry->flags = 0;
      entry->bright_x = 0;
      entry->bright_y = 0;
      entry->dark_x = 0;
      entry->dark_y = 0;
      entry->x = 0;
      entry->y = 0;
      
      entry->name_len = 0;
      entry->zName = 0;
      entry->filename_len = 0;
      entry->zFilename = NULL;

      entry->pNext = NULL;
      tsbAddInfoEntryS(entry, &g_pInfoRoot);

      for (i=0; i < idsegments; ++i) {
        pEnd = strchr(pChar+1,':');
        pChar = pEnd;
      }
      entry->name_len = pEnd - line;
      entry->zName = (char*) malloc(sizeof(char) * (entry->name_len+1));
      if (!entry->zName) {
        file_close(fp);
        return -3;
      }
      strncpy(entry->zName,line,entry->name_len);
      entry->zName[entry->name_len] = 0;
      pChar = line;

      if (*(pChar+3) == 'x') {
        // id is in hex
        id = strtol(pChar+4,NULL, 16);
      } else {
        id = strtol(pChar+2,NULL, 10);
      }
      // read the image location
      if (*(pEnd+2) == 'x') {
        // attr is in hex
        y = strtol(pEnd+3,&pChar, 16)-128;
      } else {
        y = strtol(pEnd+1,&pChar, 10)-128;
        // subtract 128 here because all of this is for vanilla
        // if for eg SAngband, which uses decimal numbers without the high bit,
        // you would want to remove this -128
      }
      if (*(pChar+2) == 'x') {
        // char is in hex
        x = strtol(pChar+3,&pEnd, 16)-128;
      } else {
        x = strtol(pChar+1,&pEnd, 10)-128;
        // subtract 128 here because all of this is for vanilla
        // if for eg SAngband, which uses decimal numbers without the high bit,
        // you would want to remove this -128
      }

      entry->y = y;
      entry->x = x;
      if ((letter == 'F') && pEnd && (*pEnd == ':')) {// _tcschr(pEnd+1, ':')) {
        // see if there are additional coordinates on the line
        // read the image location
        if (*(pEnd+2) == 'x') {
          // id is in hex
          entry->dark_y = strtol(pEnd+3,&pChar, 16)-128;
        } else {
          entry->dark_y = strtol(pEnd+1,&pChar, 10)-128;
        }
        if (*(pChar+2) == 'x') {
          // id is in hex
          entry->dark_x = strtol(pChar+3,&pEnd, 16)-128;
        } else {
          entry->dark_x = strtol(pChar+1,&pEnd, 10)-128;
        }
        //entry->u16Flags |= FLAG_SHADE_ON;
        if (pEnd && (*pEnd == ':')) { // _tcschr(pEnd+1, ':')) {
          // see if there are additional coordinates on the line
          // read the image location
          if (*(pEnd+2) == 'x') {
            // id is in hex
            entry->bright_y = strtol(pEnd+3,&pChar, 16)-128;
          } else {
            entry->bright_y = strtol(pEnd+1,&pChar, 10)-128;
          }
          if (*(pChar+2) == 'x') {
            // id is in hex
            entry->bright_x = strtol(pChar+3,&pEnd, 16)-128;
          } else {
            entry->bright_x = strtol(pChar+1,&pEnd, 10)-128;
          }
        } else {
          entry->bright_y = entry->y;
          entry->bright_x = entry->x;
        }
      } else {
        //entry->u16Flags &= ~(FLAG_SHADE_ON|FLAG_SHADE_OFF);
        entry->dark_y = entry->y;
        entry->dark_x = entry->x;
        entry->bright_y = entry->y;
        entry->bright_x = entry->x;
      }
    }
  } // while
  
  file_close(fp);
  return 0;
}

void tsbClearPrefs()
{
  tsbInfoEntry *entry;
  entry = g_pInfoRoot;
  while (entry) {
    entry->x = -1;
    entry->y = -1;
    entry->bright_x = -1;
    entry->bright_y = -1;
    entry->dark_x = -1;
    entry->dark_y = -1;
    entry = entry->pNext;
  }
}

void tsbClearEntries()
{
  tsbInfoEntry *entry, *next;
  entry = g_pInfoRoot;
  while (entry) {
    next = entry->pNext;
    if (entry->zFilename) {
      free(entry->zFilename);
      entry->zFilename = NULL;
    }
    if (entry->zName) {
      free(entry->zName);
      entry->zName = NULL;
    }
    FREE(entry);
    entry = next;
  }
  g_pInfoRoot = NULL;
}

