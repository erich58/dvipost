/*	dvi commands
	$Copyright (C) 2002 Erich Frühstück
	A-3423 St.Andrä/Wördern, Südtirolergasse 17-21/5
*/

#ifndef	dvi_h
#define	dvi_h	1

#include <stdio.h>
#include "dvicmd.h"

typedef struct {
	const char *name;
	FILE *file;
	unsigned pos;
	int ok;
	int trace;
	int num;	/* numerator of units fraction */
	int den;	/* denominator of units fraction */
	int mag;	/* magnification */
	double conv;	
	double true_conv;
	int last_page;	/* offset of last page */
	int depth;	/* stack depth */
	int mdepth;	/* maximum depth size */
	int npages;	/* number of pages */
} DviFile;

extern DviFile *df_init (DviFile *buf, const char *name, FILE *file);
extern void df_fatal (DviFile *df, const char *fmt, ...);
extern void df_trace (DviFile *df, const char *fmt, ...);

extern int din_byte (DviFile *df);
extern int din_signed (DviFile *df, unsigned len);
extern unsigned din_unsigned (DviFile *df, unsigned len);
extern char *din_string (DviFile *df, unsigned len);

extern void dout_byte (DviFile *df, int val);
extern void dout_signed (DviFile *df, int val, unsigned len);
extern void dout_unsigned (DviFile *df, unsigned val, unsigned len);
extern void dout_string (DviFile *df, char *buf, unsigned len);

/*	dvi - tokens
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
} DviFont;

extern DviFont *DviFontTab;
extern int DviFontDim;

extern DviFont *DviFont_get (int font);
extern void DviFont_add (DviToken *token);

#endif	/* dvi.h */
