/*	dvi commands

$Copyright (C) 2002 Erich Fruehstueck

Dvipost is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

Dvipost is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public
License along with dvipost; see the file COPYING.
If not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef	dvi_h
#define	dvi_h	1

#include <stdio.h>
#include "dvicmd.h"

typedef struct {
	const char *name; /* file name */
	FILE *file;	/* file pointer */
	unsigned pos;	/* file position */
	int ok;		/* status flag */
	int last_page;	/* offset of last page */
	int depth;	/* stack depth */
	int mdepth;	/* maximum depth size */
	int npages;	/* number of pages */
} DviFile;

extern DviFile *df_init (DviFile *buf, const char *name, FILE *file);
extern void df_fatal (DviFile *df, const char *fmt, ...);

extern int din_byte (DviFile *df);
extern int din_signed (DviFile *df, unsigned len);
extern unsigned din_unsigned (DviFile *df, unsigned len);
extern char *din_string (DviFile *df, unsigned len);

extern void dout_byte (DviFile *df, int val);
extern void dout_signed (DviFile *df, int val, unsigned len);
extern void dout_unsigned (DviFile *df, unsigned val, unsigned len);
extern void dout_string (DviFile *df, const char *buf, unsigned len);
extern void dout_special (DviFile *df, const char *arg);
extern void dout_right (DviFile *df, int val);
extern void dout_down (DviFile *df, int val);
extern void dout_setrule (DviFile *df, int h, int w);
extern void dout_putrule (DviFile *df, int h, int w);

/*	token
*/

typedef struct {
	int type;	/* token type */
	int par[10];	/* int parameters */
	char *str;	/* string parameter */
} DviToken;

DviToken *din_token (DviFile *df);
void dout_token (DviFile *df, DviToken *token);

/*	font table
*/

typedef struct {
	DviToken token;	/* font def token */
	int width[256]; /* character width */
	int height[256]; /* character height */
	int depth[256]; /* character depth */
	int offset;	/* overstrike offset */
} DviFont;

#define	DviFont_num(x)		(x)->token.par[0]
#define	DviFont_csum(x)		(x)->token.par[1]
#define	DviFont_scale(x)	(x)->token.par[2]
#define	DviFont_dsize(x)	(x)->token.par[3]
#define	DviFont_name(x)		(x)->token.str

extern DviFont *DviFontTab;
extern int DviFontDim;

extern DviFont *DviFont_get (int font);
extern void DviFont_add (DviToken *token);

extern int tfm_load (int *width, const char *name, int size);
extern int DviFont_tfm (DviFont *font);

extern DviFont *dvi_font;

/*	stack
*/

typedef struct DviStatStruct DviStat;;

struct DviStatStruct {
	DviStat *next;
	int h;	/* horizontal coordinate */
	int v;	/* vertical coordinate */
	int w;	/* horizontal spacing */
	int x;	/* horizontal spacing */
	int y;	/* vertical spacing */
	int z;	/* vertical spacing */
};

extern DviStat dvi_stat;

/*	process state
*/

typedef struct {
	int num;	/* numerator of units fraction */
	int den;	/* denominator of units fraction */
	int mag;	/* magnification */
	double true_conv;
	double conv;	
} DviUnit;

extern double tfm_conv;
extern DviUnit dvi_unit;

#endif	/* dvi.h */
