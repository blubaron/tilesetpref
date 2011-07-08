/***************************************************************************
 *
 *  Copyright (C) 2011 Brett Reid
 *
 *  File:tilebuild\ts-info_p.c
 *  Purpose: read the info from an Angband pref file for PC or flavor images.
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
#include "angband.h"
#include "ts-info.h"

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
sint32 tsbReadXtraFile(char *filename)
{
  int bytesread=0;
  char *pChar, *pEnd, *s;
  char line[160];
  char letter;
  sint32 x,y;
  sint32 id;
  tsbInfoEntry *entry;
  char *racename, *classname, *gender;

  FILE *fp = file_open(filename, MODE_READ, FTYPE_TEXT);
  if (!fp) {
    return -2;
  }
  while(file_getl(fp, line,160)) {
    bytesread = strlen(line);
    if (bytesread < 3) {
      continue;
    }
    pChar = line;
    if (*pChar == '#') {
      continue;
    }
    if (*(pChar+1) == ':') {
      letter = *pChar;
      if (letter == '?') {
        // build the id that we will use on the next line
        if (s = strstr(pChar, "RACE")) {
          s += 5;
          pEnd = strchr(s, ']');
          *pEnd = 0;
          racename = s;
        } else {
          racename = "Human";
        }
        if (s = strstr(pChar, "CLASS")) {
          s += 6;
          pEnd = strchr(s, ']');
          *pEnd = 0;
          classname = s;
        } else {
          classname = "Warrior";
        }
        if (s = strstr(pChar, "GENDER")) {
          s += 7;
          pEnd = strchr(s, ']');
          *pEnd = 0;
          gender = s;
        } else {
          gender = "Male";
        }
        entry = ZNEW(tsbInfoEntry);
        if (!entry) {
          file_close(fp);
          return -3;
        }
        entry->letter = 'P';
        entry->id = 0;
        entry->flags = 0;
        entry->bright_x = 0;
        entry->bright_y = 0;
        entry->dark_x = 0;
        entry->dark_y = 0;
        entry->x = 0;
        entry->y = 0;
      
        entry->filename_len = 0;
        entry->zFilename = NULL;

        entry->pNext = NULL;
        tsbAddInfoEntryS(entry, &g_pInfoRoot);

        entry->name_len = strlen(racename) + strlen(classname)+strlen(gender)+2;
        entry->zName = (char*) malloc(sizeof(char) * (entry->name_len+1));
        sprintf(entry->zName,"%s %s %s",racename, classname,gender);

      } else
      if (letter == '%') {
        continue;
      } else
      if (letter == 'R') {
        if (*(pChar+3) == 'x') {
          // id is in hex
          id = strtol(pChar+4,&pEnd, 16);
        } else {
          id = strtol(pChar+2,&pEnd, 10);
        }
        // read the image location
        //y = _tcstol(pEnd+1,&pChar, 16);
        //x = _tcstol(pChar+1,&pEnd, 16);
        if (*(pEnd+2) == 'x') {
          // id is in hex
          y = strtol(pEnd+3,&pChar, 16)-128;
        } else {
          y = strtol(pEnd+1,&pChar, 10);
        }
        if (*(pChar+2) == 'x') {
          // id is in hex
          x = strtol(pChar+3,&pEnd, 16)-128;
        } else {
          x = strtol(pChar+1,&pEnd, 10);
        }
        if (entry) {
          entry->y = y;
          entry->x = x;
        }
      }
    } // if (*(pChar+1) == ':')
  } // while 

  return 0;
}

sint32 tsbReadFlvrFile(char *filename)
{
  return tsbReadPrefFile(filename);
}
