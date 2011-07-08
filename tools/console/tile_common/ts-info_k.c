/***************************************************************************
 *
 *  Copyright (C) 2011 Brett Reid
 *
 *  File:tilebuild\ts-info_f.c
 *  Purpose: read the info from a redirection file, to replace the strings
 *    used in vanilla pref files with numeric ids, and back.
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

//extern char zWndClassName[];
//#define Message(str) MessageBox(NULL, str, zWndClassName, MB_OK | MB_ICONERROR);if (ShowCursor(TRUE) > 1) {ShowCursor(FALSE);}

//extern LinkList<cString *> g_lFilesLoaded;
//bool appInitFunctions() {return false;}
extern int loadflags;

tsbEditorIDRedirection *g_pRedirRoot;

static void tsbAddRedirEntryS(tsbEditorIDRedirection *entry, tsbEditorIDRedirection **root) {
  if (entry == NULL) return;
  if (root) {
    tsbEditorIDRedirection* temp = *root;
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

int tsbInitRedirEntriesFromFile(char *filename)
{
  tsbEditorIDRedirection *entry;
  char str[128];
  FILE *fp;
  int bytesread=0;
  char *pChar, *pEnd;
  char line[256];

  int len;
  // read the redirection info
  fp = file_open(filename, MODE_READ, FTYPE_TEXT);
  if (!fp) {
    return -2;
  }
  // read each line, for every N: line, create an entry
  while(file_getl(fp, line,160)) {
    bytesread = strlen(line);
    if (bytesread < 1) {
      continue;
    }
    //pChar = line.GetWord(1, NULL, temp);
    pChar = line;
    if (pChar == NULL)
      return -4;

    // test valid commands to see if the command is valid
    if (*pChar == '#') {
      // we have a comment return
      continue;
    }
    if (my_isalpha(*pChar)) {
      // parse the line
      int id;
      id = strtol(pChar+2,&pEnd, 10);
      len = strlen(pEnd);
      if (len > 128) {
        strncpy(str,pEnd+1,128);
      } else {
        strncpy(str,pEnd+1,len);
      }
      str[127] = 0;
      len = strlen(str);
      //str.SetText(pEnd+1,0);
      if (len < 3) {
        continue;
      }
      // create an entry
      entry = ZNEW(tsbEditorIDRedirection);
      if (!entry) {
        file_close(fp);
        return -3;
      }
      entry->letter = *pChar;
      entry->id = id;
      entry->zName = string_make(str);
      entry->name_len = len;
      entry->pNext = NULL;
      tsbAddRedirEntryS(entry, &g_pRedirRoot);
      continue;
    }
    // Ingore the other lines
  }
  file_close(fp);

  return 0;
}
void tsbClearRedirEntries()
{
  tsbEditorIDRedirection *entry = g_pRedirRoot,*next;
  while (entry) {
    next = entry->pNext;
    string_free(entry->zName);
    FREE(entry);
    entry = next;
  }
  g_pRedirRoot = NULL;
}


