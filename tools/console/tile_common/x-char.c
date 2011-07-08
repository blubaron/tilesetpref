/*
 * File: x-char.c
 * Purpose: Enable extended character sets for the Operating Systems that
 * can handle it without affecting the ASCII-only OSs.
 *
 * Copyright (c) 2007 Leon Marrick, Hugo Cornelius, Diego Gonzalez, Jeff Greene
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
 */
#include "angband.h"
#include "z-util.h"
//#include "z-term.h"
#include "x-char.h"

/*
 * Translate from encodes to extended 8-bit characters and back again.
 */
static const xchar_type latin1_encode[] =
{
    { "`A", 192 },  { "'A", 193 },  { "^A", 194 },  { "~A", 195 },
    { "\"A", 196 },  { "*A", 197 },  { ",C", 199 },  { "`E", 200 },
    { "'E", 201 },  { "^E", 202 }, { "\"E", 203 },  { "`I", 204 },
    { "'I", 205 },  { "^I", 206 }, { "\"I", 207 },  { "~N", 209 },
    { "`O", 210 },  { "'O", 211 },  { "^O", 212 },  { "~O", 213 },
	{ "\"O", 214 },  { "/O", 216 },  { "`U", 217 },  { "'U", 218 },
    { "^U", 219 }, { "\"U", 220 },  { "'Y", 221 },  { "`a", 224 },
    { "'a", 225 },  { "^a", 226 },  { "~a", 227 }, { "\"a", 228 },
    { "*a", 229 },  { ",c", 231 },  { "`e", 232 },  { "'e", 233 },
    { "^e", 234 }, { "\"e", 235 },  { "`i", 236 },  { "'i", 237 },
    { "^i", 238 }, { "\"i", 239 },  { "~n", 241 },  { "`o", 242 },
    { "'o", 243 },  { "^o", 244 },  { "~o", 245 }, { "\"o", 246 },
    { "/o", 248 },  { "`u", 249 },  { "'u", 250 },  { "^u", 251 },
    { "\"u", 252 },  { "'y", 253 }, { "\"y", 255 },

    { "iexcl", 161 }, { "euro", 162 }, { "pound", 163 }, { "curren", 164 },
    { "yen", 165 },   { "brvbar", 166 }, { "sect", 167 }, { "Agrave", 192 },
    { "Aacute", 193 }, { "Acirc", 194 }, { "Atilde", 195 }, { "Auml", 196 },
    { "Aring", 197 }, { "Aelig", 198 }, { "Ccedil", 199 }, { "Egrave", 200 },
    { "Eacute", 201 }, { "Ecirc", 202 }, { "Euml", 203 }, { "Igrave", 204 },
    { "Iacute", 205 }, { "Icirc", 206 }, { "Iuml", 207 }, { "ETH", 208 },
    { "Ntilde", 209 }, { "Ograve", 210 }, { "Oacute", 211 }, { "Ocirc", 212 },
    { "Otilde", 213 }, { "Ouml", 214 }, { "Oslash", 216 }, { "Ugrave", 217 },
    { "Uacute", 218 }, { "Ucirc", 219 }, { "Uuml", 220 }, { "Yacute", 221 },
    { "THORN", 222 }, { "szlig", 223 }, { "agrave", 224 }, { "aacute", 225 },
    { "acirc", 226 }, { "atilde", 227 }, { "auml", 228 }, { "aring", 229 },
    { "aelig", 230 }, { "ccedil", 231 }, { "egrave", 232 }, { "eacute", 233 },
    { "ecirc", 234 }, { "euml", 235 }, { "igrave", 236 }, { "iacute", 237 },
    { "icirc", 238 }, { "iuml", 239 }, { "eth", 240 },   { "ntilde", 241 },
    { "ograve", 242 }, { "oacute", 243 }, { "ocirc", 244 }, { "otilde", 245 },
    { "ouml", 246 }, { "oslash", 248 }, { "ugrave", 249 }, { "uacute", 250 },
    { "ucirc", 251 }, { "uuml", 252 }, { "yacute", 253 }, { "thorn", 254 },
    { "yuml", 255 },   { "\0", 0 }
};

/*
 * Translate from ISO Latin-1 characters 128+ to 7-bit ASCII.
 *
 * We use this table to maintain compatibility with systems that cannot
 * display 8-bit characters.  We also use it whenever we wish to suppress
 * accents or ensure that a character is 7-bit.
 */
static const char seven_bit_translation[128] =
{
 	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
 	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
 	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
 	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
 	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
 	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
 	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
 	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
 	'A', 'A', 'A', 'A', 'A', 'A', ' ', 'C',
 	'E', 'E', 'E', 'E', 'I', 'I', 'I', 'I',
 	'D', 'N', 'O', 'O', 'O', 'O', 'O', ' ',
 	'O', 'U', 'U', 'U', 'U', 'Y', ' ', ' ',
 	'a', 'a', 'a', 'a', 'a', 'a', ' ', 'c',
 	'e', 'e', 'e', 'e', 'i', 'i', 'i', 'i',
 	'o', 'n', 'o', 'o', 'o', 'o', 'o', ' ',
	'o', 'u', 'u', 'u', 'u', 'y', ' ', 'y'
};

/*
 * Link to the xchar_trans function.
 */
static void xchar_trans_hook(char *s, int encoding)
{
 	/* Option to translate into ASCII */
 	if (encoding == ASCII)
 	{
 		if (*s < 0) *s = seven_bit_translation[128 + *s];
 	}

 	/* Option to translate into system-specific character set */
 	else if (encoding == SYSTEM_SPECIFIC)
 	{
 		if (*s < 0) *s = xchar_trans(*s);
	}
}


/*
 * Given what we think is an encode, return a Latin-1 character position.
 */
static byte encode_to_xchar(char *encode)
{
 	int i;

 	/* Scan until we hit the end-of-table marker */
 	for (i = 0; latin1_encode[i].c; i++)
 	{
 		/* We found the encode; return the character */
 		if (streq(encode, latin1_encode[i].tag))
 			return (latin1_encode[i].c);
 	}

 	/* This encode is not recognized */
 	return (0);
}

/*
 * Read an encode.  Return the Latin-1 character position if successful.
 */
bool get_encode(char *str, char *c)
{
 	int n = 0;
 	char *s;
 	char encode[80];

 	/* Assume empty char */
 	*c = '\0';

 	/* An encode must start with a '[' */
 	if (str[0] != '[') return (FALSE);

 	/* Copy the encode (between brackets) */
 	for (s = str + 1; ((n < 80) && (*s) && (*s != ']')); n++)
 	{
 		encode[n] = *s++;
 	}

 	/* End the encode */
 	encode[n] = '\0';

 	/* We have a trailing bracket */
 	if (*s == ']')
 	{
 		/* Look up extended character */
 		*c = (char)encode_to_xchar(encode);

 		/* Encode is legal -- return the char */
 		if (*c) return (TRUE);
 	}

 	/* Illegal encode */
 	return (FALSE);
}



/*
 * Take a 7-bit ASCII string and use any encodes in it to insert 8-bit
 * characters.  Use the Latin-1 (ISO) standard by default.  -LM-
 *
 * Optionally, translate into 7-bit ASCII or a system-specific character set.
 *
 * The input string must be '\0'-terminated, and should not be greater than
 * 1024 character in length (we check this).
 */
void xstr_trans(char *str, int encoding)
{
 	/* Assume no encodes in this string */
 	bool flag = FALSE;

 	int n, c;

 	char *s, *b;
 	char buf[1024];
 	char encode[80];

 	/* Require a string */
 	if (!str) return;

 	/* Start at the beginning */
 	s = str;
 	b = buf;

 	/* Scan the string */
 	for (; *s;)
 	{
 		/* Paranoia -- check bounds */
 		if (b - buf > 1022) break;

 		/* Character is a [ */
 		if (*s == '[')
 		{
 			/* Remember where we are */
 			char *s_old = s;

 			/* Assume illegal */
 			c = 0;

 			/* Skip past the open bracket */
 			s += 1;

 			/* Copy the encode (between brackets) */
 			for (n = 0; ((n < 79) && (*s) && (*s != ']')); n++)
 			{
 				encode[n] = *s++;
 			}

 			/* End the encode */
 			encode[n] = '\0';

 			/* We have a trailing bracket */
 			if (*s == ']')
 			{
 				/* Go to next character */
 				s++;

 				/* Look up extended character */
 				c = (char)encode_to_xchar(encode);
 			}

 			/* Encode is legal */
 			if (c)
 			{
 				/* Note the encode */
 				flag = TRUE;

				/* Save it */
 				*b++ = c;
 			}

 			/* Encode is illegal */
 			else
 			{
 				/* Return to start of encode */
 				s = s_old;

 				/* Copy the '[', go to the next character */
 				*b++ = *s++;
 			}
 		}

 		/* No encoding recognized */
 		else
 		{
 			/* Copy the character */
 			*b++ = *s++;
 		}
	}

 	/* End the string */
 	*b = '\0';

 	/* Copy the edited string back, if we've found encodes */
 	if (flag) strcpy(str, buf);

	/* Translate the string if we don't want standard Latin-1 */
	if (encoding != LATIN1)
	{
		for (s = str; *s; s++) xchar_trans_hook(s, encoding);
	}
}

/*
 * Given a position in the ISO Latin-1 character set (which Angband uses
 * internally), return the correct display character on this system.
 * Assume ASCII-only if no special hook is available.  -LM-
 */
char xchar_trans(byte c)
{
 	char s;

 	/* Use the hook, if available */
 	if (Term->xchar_hook) return ((char)(Term->xchar_hook)(c));

 	/* 7-bit characters are not translated */
 	if (c < 128) return (c);

 	/* Translate to 7-bit (strip accent or convert to space) */
 	s = seven_bit_translation[c - 128];

 	if (s == 0) return (c);
 	else return (s);
}

/*
 * Character translations and definitions.  -JG-
 *
 * Upper case and lower case equivalents of a given ISO Latin-1 character.
 * A character's general "type"; types may be combined.
 *
 * Note that this table assumes use of the standard Angband extended fonts.
 *
 * Notice the accented characters in positions 191+.  If they don't appear
 * correct to you, then you are viewing this table in a non-Latin-1 font.
 * Individual ports are responsible for translations from the game's
 * internal Latin-1 to their system's character encoding(s) (unless ASCII).
 */
const byte char_tables[256][CHAR_TABLE_SLOTS] =
{
/* TO_UPPER, TO_LOWER, CHAR TYPE */
    {   0,     0,           0L },               /*        Empty      */
    {   1,     1,  CHAR_SYMBOL },               /*        Solid      */
    {   2,     2,  CHAR_SYMBOL },               /* Mostly solid      */
    {   3,     3,  CHAR_SYMBOL },               /* Wall pattern      */
    {   4,     4,  CHAR_SYMBOL },               /*    Many dots      */
    {   5,     5,  CHAR_SYMBOL },               /*  Medium dots      */
    {   6,     6,  CHAR_SYMBOL },               /*     Few dots      */
    {   7,     7,  CHAR_SYMBOL },               /*     Tiny dot      */
    {   8,     8,  CHAR_SYMBOL },               /*    Small dot      */
    {   9,     9,  CHAR_SYMBOL },               /*   Medium dot      */
    {  10,    10,  CHAR_SYMBOL },               /*    Large dot      */
    {  11,    11,  CHAR_SYMBOL },               /*       Rubble      */
    {  12,    12,  CHAR_SYMBOL },               /*     Treasure      */
    {  13,    13,  CHAR_SYMBOL },               /*  Closed door      */
    {  14,    14,  CHAR_SYMBOL },               /*    Open Door      */
    {  15,    15,  CHAR_SYMBOL },               /*  Broken door      */
    {  16,    16,  CHAR_SYMBOL },               /*       Pillar      */
    {  17,    17,  CHAR_SYMBOL },               /*        Water      */
    {  18,    18,  CHAR_SYMBOL },               /*         Tree      */
    {  19,    19,  CHAR_SYMBOL },               /*    Fire/lava      */
    {  20,    20,  CHAR_SYMBOL },               /*   Pit/portal      */
    {  22,    22,           0L },               /*       Unused      */
    {  22,    22,           0L },               /*       Unused      */
    {  23,    23,           0L },               /*       Unused      */
    {  24,    24,           0L },               /*       Unused      */
    {  25,    25,           0L },               /*       Unused      */
    {  26,    26,           0L },               /*       Unused      */
    {  27,    27,           0L },               /*       Unused      */
    {  28,    28,           0L },               /*       Unused      */
    {  29,    29,           0L },               /*       Unused      */
    {  30,    30,           0L },               /*       Unused      */
    {  31,    31,           0L },               /*       Unused      */

    {  32,    32,   CHAR_BLANK },               /*        Space      */
    {  33,    33,   CHAR_PUNCT },               /*            !      */
    {  34,    34,   CHAR_PUNCT },               /*            "      */
    {  35,    35,   CHAR_PUNCT },               /*            #      */
    {  36,    36,   CHAR_PUNCT },               /*            $      */
    {  37,    37,   CHAR_PUNCT },               /*            %      */
    {  38,    38,   CHAR_PUNCT },               /*            &      */
    {  39,    39,   CHAR_PUNCT },               /*            '      */
    {  40,    40,   CHAR_PUNCT },               /*            (      */
    {  41,    41,   CHAR_PUNCT },               /*            )      */
    {  42,    42,   CHAR_PUNCT },               /*            *      */
    {  43,    43,   CHAR_PUNCT },               /*            +      */
    {  44,    44,   CHAR_PUNCT },               /*            ,      */
    {  45,    45,   CHAR_PUNCT },               /*            -      */
    {  46,    46,   CHAR_PUNCT },               /*            .      */
    {  47,    47,   CHAR_PUNCT },               /*            /      */

    {  48,    48,   CHAR_DIGIT },               /*            0      */
    {  49,    49,   CHAR_DIGIT },               /*            1      */
    {  50,    50,   CHAR_DIGIT },               /*            2      */
    {  51,    51,   CHAR_DIGIT },               /*            3      */
    {  52,    52,   CHAR_DIGIT },               /*            4      */
    {  53,    53,   CHAR_DIGIT },               /*            5      */
    {  54,    54,   CHAR_DIGIT },               /*            6      */
    {  55,    55,   CHAR_DIGIT },               /*            7      */
    {  56,    56,   CHAR_DIGIT },               /*            8      */
    {  57,    57,   CHAR_DIGIT },               /*            9      */
    {  58,    58,   CHAR_DIGIT },               /*            :      */
    {  59,    59,   CHAR_DIGIT },               /*            ;      */

    {  60,    60,   CHAR_PUNCT },               /*            <      */
    {  61,    61,   CHAR_PUNCT },               /*            =      */
    {  62,    62,   CHAR_PUNCT },               /*            >      */
    {  63,    63,   CHAR_PUNCT },               /*            ?      */
    {  64,    64,   CHAR_PUNCT },               /*            @      */
    {  65,    97,   CHAR_UPPER | CHAR_VOWEL },  /*            A      */
    {  66,    98,   CHAR_UPPER },               /*            B      */
    {  67,    99,   CHAR_UPPER },               /*            C      */
    {  68,   100,   CHAR_UPPER },               /*            D      */
    {  69,   101,   CHAR_UPPER | CHAR_VOWEL },  /*            E      */
    {  70,   102,   CHAR_UPPER },               /*            F      */
    {  71,   103,   CHAR_UPPER },               /*            G      */
    {  72,   104,   CHAR_UPPER },               /*            H      */
    {  73,   105,   CHAR_UPPER | CHAR_VOWEL },  /*            I      */
    {  74,   106,   CHAR_UPPER },               /*            J      */
    {  75,   107,   CHAR_UPPER },               /*            K      */
    {  76,   108,   CHAR_UPPER },               /*            L      */
    {  77,   109,   CHAR_UPPER },               /*            M      */
    {  78,   110,   CHAR_UPPER },               /*            N      */
    {  79,   111,   CHAR_UPPER | CHAR_VOWEL },  /*            O      */
    {  80,   112,   CHAR_UPPER },               /*            P      */
    {  81,   113,   CHAR_UPPER },               /*            Q      */
    {  82,   114,   CHAR_UPPER },               /*            R      */
    {  83,   115,   CHAR_UPPER },               /*            S      */
    {  84,   116,   CHAR_UPPER },               /*            T      */
    {  85,   117,   CHAR_UPPER | CHAR_VOWEL },  /*            U      */
    {  86,   118,   CHAR_UPPER },               /*            V      */
    {  87,   119,   CHAR_UPPER },               /*            W      */
    {  88,   120,   CHAR_UPPER },               /*            X      */
    {  89,   121,   CHAR_UPPER },               /*            Y      */
    {  90,   122,   CHAR_UPPER },               /*            Z      */

    {  91,    91,   CHAR_PUNCT },               /*            [      */
    {  92,    92,   CHAR_PUNCT },               /*            \      */
    {  93,    93,   CHAR_PUNCT },               /*            ]      */
    {  94,    94,   CHAR_PUNCT },               /*            ^      */
    {  95,    95,   CHAR_PUNCT },               /*            _      */
    {  96,    96,   CHAR_PUNCT },               /*            `      */
    {  65,    97,   CHAR_LOWER | CHAR_VOWEL },  /*            a      */
    {  66,    98,   CHAR_LOWER },               /*            b      */
    {  67,    99,   CHAR_LOWER },               /*            c      */
    {  68,   100,   CHAR_LOWER },               /*            d      */
    {  69,   101,   CHAR_LOWER | CHAR_VOWEL },  /*            e      */
    {  70,   102,   CHAR_LOWER },               /*            f      */
    {  71,   103,   CHAR_LOWER },               /*            g      */
    {  72,   104,   CHAR_LOWER },               /*            h      */
    {  73,   105,   CHAR_LOWER | CHAR_VOWEL },  /*            i      */
    {  74,   106,   CHAR_LOWER },               /*            j      */
    {  75,   107,   CHAR_LOWER },               /*            k      */
    {  76,   108,   CHAR_LOWER },               /*            l      */
    {  77,   109,   CHAR_LOWER },               /*            m      */
    {  78,   110,   CHAR_LOWER },               /*            n      */
    {  79,   111,   CHAR_LOWER | CHAR_VOWEL },  /*            o      */
    {  80,   112,   CHAR_LOWER },               /*            p      */
    {  81,   113,   CHAR_LOWER },               /*            q      */
    {  82,   114,   CHAR_LOWER },               /*            r      */
    {  83,   115,   CHAR_LOWER },               /*            s      */
    {  84,   116,   CHAR_LOWER },               /*            t      */
    {  85,   117,   CHAR_LOWER | CHAR_VOWEL },  /*            u      */
    {  86,   118,   CHAR_LOWER },               /*            v      */
    {  87,   119,   CHAR_LOWER },               /*            w      */
    {  88,   120,   CHAR_LOWER },               /*            x      */
    {  89,   121,   CHAR_LOWER },               /*            y      */
    {  90,   122,   CHAR_LOWER },               /*            z      */
    { 123,   123,   CHAR_PUNCT },               /*            {    */
    { 124,   124,   CHAR_PUNCT },               /*            |      */
    { 125,   125,   CHAR_PUNCT },               /*            }      */
    { 126,   126,   CHAR_PUNCT },               /*            ~      */
    { 127,   127,  CHAR_SYMBOL },               /* Wall pattern      */

    { 128,   128,           0L },               /*       Unused      */
    { 129,   129,           0L },               /*       Unused      */
    { 130,   130,           0L },               /*       Unused      */
    { 131,   131,           0L },               /*       Unused      */
    { 132,   132,           0L },               /*       Unused      */
    { 133,   133,           0L },               /*       Unused      */
    { 134,   134,           0L },               /*       Unused      */
    { 135,   135,           0L },               /*       Unused      */
    { 136,   136,           0L },               /*       Unused      */
    { 137,   137,           0L },               /*       Unused      */
    { 138,   138,           0L },               /*       Unused      */
    { 139,   139,           0L },               /*       Unused      */
    { 140,   140,           0L },               /*       Unused      */
    { 141,   141,           0L },               /*       Unused      */
    { 142,   142,           0L },               /*       Unused      */
    { 143,   143,           0L },               /*       Unused      */
    { 144,   144,           0L },               /*       Unused      */
    { 145,   145,           0L },               /*       Unused      */
    { 146,   146,           0L },               /*       Unused      */
    { 147,   147,           0L },               /*       Unused      */
    { 148,   148,           0L },               /*       Unused      */
    { 149,   149,           0L },               /*       Unused      */
    { 150,   150,           0L },               /*       Unused      */
    { 151,   151,           0L },               /*       Unused      */
    { 152,   152,           0L },               /*       Unused      */
    { 153,   153,           0L },               /*       Unused      */
    { 154,   154,           0L },               /*       Unused      */
    { 155,   155,           0L },               /*       Unused      */
    { 156,   156,           0L },               /*       Unused      */
    { 157,   157,           0L },               /*       Unused      */
    { 158,   158,           0L },               /*       Unused      */
    { 159,   159,           0L },               /*       Unused      */
    { 160,   160,           0L },               /*       Unused      */

    { 161,   161,   CHAR_PUNCT },               /*       iexcl   ¡   */
    { 162,   162,   CHAR_PUNCT },               /*        euro   ¢   */
    { 163,   163,   CHAR_PUNCT },               /*       pound   £   */
    { 164,   164,   CHAR_PUNCT },               /*      curren   ¤   */
    { 165,   165,   CHAR_PUNCT },               /*         yen   ¥   */
    { 166,   166,   CHAR_PUNCT },               /*      brvbar   ¦   */
    { 167,   167,   CHAR_PUNCT },               /*        sect   §   */
    { 168,   168,  CHAR_SYMBOL },               /*  Bolt - vert      */
    { 169,   169,  CHAR_SYMBOL },               /*  Bolt - horz      */
    { 170,   170,  CHAR_SYMBOL },               /*  Bolt -rdiag      */
    { 171,   171,  CHAR_SYMBOL },               /*  Bolt -ldiag      */
    { 172,   172,  CHAR_SYMBOL },               /*  Spell-cloud      */
    { 173,   173,  CHAR_SYMBOL },               /*   Spell-elec      */
    { 174,   174,  CHAR_SYMBOL },               /*  Spell-explo      */

    { 175,   175,           0L },               /*       Unused      */
    { 176,   176,           0L },               /*       Unused      */
    { 177,   177,           0L },               /*       Unused      */
    { 178,   178,           0L },               /*       Unused      */
    { 179,   179,           0L },               /*       Unused      */
    { 180,   180,           0L },               /*       Unused      */
    { 181,   181,           0L },               /*       Unused      */
    { 182,   182,           0L },               /*       Unused      */
    { 183,   183,           0L },               /*       Unused      */
    { 184,   184,           0L },               /*       Unused      */
    { 185,   185,           0L },               /*       Unused      */
    { 186,   186,           0L },               /*       Unused      */
    { 187,   187,           0L },               /*       Unused      */
    { 188,   188,           0L },               /*       Unused      */
    { 189,   189,           0L },               /*       Unused      */
    { 190,   190,           0L },               /*       Unused      */

    { 191,   191,   CHAR_PUNCT },               /*      iquest   ¿   */
    { 192,   224,   CHAR_UPPER | CHAR_VOWEL },  /*      Agrave   À   */
    { 193,   225,   CHAR_UPPER | CHAR_VOWEL },  /*      Aacute   Á   */
    { 194,   226,   CHAR_UPPER | CHAR_VOWEL },  /*       Acirc   Â   */
    { 195,   227,   CHAR_UPPER | CHAR_VOWEL },  /*      Atilde   Ã   */
    { 196,   228,   CHAR_UPPER | CHAR_VOWEL },  /*        Auml   Ä   */
    { 197,   229,   CHAR_UPPER | CHAR_VOWEL },  /*       Aring   Å   */
    { 198,   230,   CHAR_UPPER | CHAR_VOWEL },  /*       Aelig   Æ   */
    { 199,   231,   CHAR_UPPER },               /*      Ccedil   Ç   */
    { 200,   232,   CHAR_UPPER | CHAR_VOWEL },  /*      Egrave   È   */
    { 201,   233,   CHAR_UPPER | CHAR_VOWEL },  /*      Eacute   É   */
    { 202,   234,   CHAR_UPPER | CHAR_VOWEL },  /*       Ecirc   Ê   */
    { 203,   235,   CHAR_UPPER | CHAR_VOWEL },  /*        Euml   Ë   */
    { 204,   236,   CHAR_UPPER | CHAR_VOWEL },  /*      Igrave   Ì   */
    { 205,   237,   CHAR_UPPER | CHAR_VOWEL },  /*      Iacute   Í   */
    { 206,   238,   CHAR_UPPER | CHAR_VOWEL },  /*       Icirc   Î   */
    { 207,   239,   CHAR_UPPER | CHAR_VOWEL },  /*        Iuml   Ï   */
    { 208,   240,   CHAR_UPPER },               /*         ETH   Ð   */
    { 209,   241,   CHAR_UPPER },               /*      Ntilde   Ñ   */
    { 210,   242,   CHAR_UPPER | CHAR_VOWEL },  /*      Ograve   Ò   */
    { 211,   243,   CHAR_UPPER | CHAR_VOWEL },  /*      Oacute   Ó   */
    { 212,   244,   CHAR_UPPER | CHAR_VOWEL },  /*       Ocirc   Ô   */
    { 213,   245,   CHAR_UPPER | CHAR_VOWEL },  /*      Otilde   Õ   */
    { 214,   246,   CHAR_UPPER | CHAR_VOWEL },  /*        Ouml   Ö   */
    { 215,   215,           0L },               /*       Unused      */
    { 216,   248,   CHAR_UPPER | CHAR_VOWEL },  /*      Oslash   Ø   */
    { 217,   249,   CHAR_UPPER | CHAR_VOWEL },  /*      Ugrave   Ù   */
    { 218,   250,   CHAR_UPPER | CHAR_VOWEL },  /*      Uacute   Ú   */
    { 219,   251,   CHAR_UPPER | CHAR_VOWEL },  /*       Ucirc   Û   */
    { 220,   252,   CHAR_UPPER | CHAR_VOWEL },  /*        Uuml   Ü   */
    { 221,   253,   CHAR_UPPER },               /*      Yacute   Ý   */
    { 222,   254,   CHAR_UPPER },               /*       THORN   Þ   */
    { 223,   223,   CHAR_LOWER },               /*       szlig   ß   */

    { 192,   224,   CHAR_LOWER | CHAR_VOWEL },  /*      agrave   à   */
    { 193,   225,   CHAR_LOWER | CHAR_VOWEL },  /*      aacute   á   */
    { 194,   226,   CHAR_LOWER | CHAR_VOWEL },  /*       acirc   â   */
    { 195,   227,   CHAR_LOWER | CHAR_VOWEL },  /*      atilde   ã   */
    { 196,   228,   CHAR_LOWER | CHAR_VOWEL },  /*        auml   ä   */
    { 197,   229,   CHAR_LOWER | CHAR_VOWEL },  /*       aring   å   */
    { 198,   230,   CHAR_LOWER | CHAR_VOWEL },  /*       aelig   æ   */
    { 199,   231,   CHAR_LOWER },               /*      ccedil   ç   */
    { 200,   232,   CHAR_LOWER | CHAR_VOWEL },  /*      egrave   è   */
    { 201,   233,   CHAR_LOWER | CHAR_VOWEL },  /*      eacute   é   */
    { 202,   234,   CHAR_LOWER | CHAR_VOWEL },  /*       ecirc   ê   */
    { 203,   235,   CHAR_LOWER | CHAR_VOWEL },  /*        euml   ë   */
    { 204,   236,   CHAR_LOWER | CHAR_VOWEL },  /*      igrave   ì   */
    { 205,   237,   CHAR_LOWER | CHAR_VOWEL },  /*      iacute   í   */
    { 206,   238,   CHAR_LOWER | CHAR_VOWEL },  /*       icirc   î   */
    { 207,   239,   CHAR_LOWER | CHAR_VOWEL },  /*        iuml   ï   */
    { 208,   240,   CHAR_LOWER },               /*         eth   ð   */
    { 209,   241,   CHAR_LOWER },               /*      ntilde   ñ   */
    { 210,   242,   CHAR_LOWER | CHAR_VOWEL },  /*      ograve   ò   */
    { 211,   243,   CHAR_LOWER | CHAR_VOWEL },  /*      oacute   ó   */
    { 212,   244,   CHAR_LOWER | CHAR_VOWEL },  /*       ocirc   ô   */
    { 213,   245,   CHAR_LOWER | CHAR_VOWEL },  /*      otilde   õ   */
    { 214,   246,   CHAR_LOWER | CHAR_VOWEL },  /*        ouml   ö   */
    { 247,   247,           0L },               /*       Unused      */
    { 216,   248,   CHAR_LOWER | CHAR_VOWEL },  /*      oslash   ø   */
    { 217,   249,   CHAR_LOWER | CHAR_VOWEL },  /*      ugrave   ù   */
    { 218,   250,   CHAR_LOWER | CHAR_VOWEL },  /*      uacute   ú   */
    { 219,   251,   CHAR_LOWER | CHAR_VOWEL },  /*       ucirc   û   */
    { 220,   252,   CHAR_LOWER | CHAR_VOWEL },  /*        uuml   ü   */
    { 221,   253,   CHAR_LOWER },               /*      yacute   ý   */
    { 222,   254,   CHAR_LOWER },               /*       thorn   þ   */
    { 121,   255,   CHAR_LOWER },               /*        yuml   ÿ   */
};

