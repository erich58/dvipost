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
} DviFile;

extern DviFile *df_init (DviFile *buf, const char *name, FILE *file);
extern void df_fatal (DviFile *df, const char *fmt, ...);
extern void df_trace (DviFile *df, const char *fmt, ...);

extern int din_byte (DviFile *df);
extern int din_signed (DviFile *df, unsigned len);
extern unsigned din_unsigned (DviFile *df, unsigned len);
extern char *din_string (DviFile *df, char *buf, unsigned len);

extern void dout_byte (DviFile *df, int val);
extern void dout_signed (DviFile *df, int val, unsigned len);
extern void dout_unsigned (DviFile *df, unsigned val, unsigned len);
extern void dout_string (DviFile *df, char *buf, unsigned len);

typedef struct {
	int type;	/* command type */
	unsigned len;	/* extension length */
	unsigned size;	/* size of charakter buffer, must be 0 on start */
	char *buf;	/* character buffer */
} DviExtension;

extern DviExtension *din_extension (DviFile *df, DviExtension *ext, int type);
extern void dout_extension (DviFile *df, DviExtension *ext);

typedef struct {
	int type;	/* command type */
	int font;	/* font number */
	unsigned csum;	/* check sum in TFM file for font */
	int scale;	/* scale fsctor */
	int dsize;	/* design size */
	int dlen;	/* directory length */
	int nlen;	/* name length */
	char *name;	/* font name */
} DviFontDef;

extern DviFontDef *din_fontdef (DviFile *df, int type);
extern void dout_fontdef (DviFile *df, DviFontDef *fdef);
extern void dout_fonttab (DviFile *df);

#endif	/* dvi.h */
