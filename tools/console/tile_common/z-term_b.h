/* File: z-term.h */

/*
 * Copyright (c) 1997 Ben Harrison
 * Modified 2011 by Brett Reid for tool programs outside of the game,
 * this file should not be mixed with or replace a file in the game source.
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

#ifndef INCLUDED_Z_TERM_H
#define INCLUDED_Z_TERM_H

#include "h-basic.h"

typedef struct term term;

struct term
{
	byte (*xchar_hook)(byte c);
};

extern term *Term;
#endif
