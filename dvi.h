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
} DviFile;

extern DviFile *df_init (DviFile *buf, const char *name, FILE *file);
extern void df_fatal (DviFile *df, const char *fmt, ...);

extern int din_byte (DviFile *df);
extern int din_signed (DviFile *df, unsigned len);
extern int din_unsigned (DviFile *df, unsigned len);
extern char *din_string (DviFile *df, char *buf, unsigned len);
extern void din_trace (DviFile *df, const char *fmt, ...);

extern void dout_byte (DviFile *df, int val);
extern void dout_signed (DviFile *df, int val, unsigned len);
extern void dout_unsigned (DviFile *df, int val, unsigned len);
extern void dout_string (DviFile *df, char *buf, unsigned len);

#endif	/* dvi.h */
