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
} DviInput;

extern DviInput *din_init (DviInput *buf, const char *name, FILE *file);
extern void din_fatal (DviInput *df, const char *fmt, ...);
extern int din_byte (DviInput *df);
extern int din_signed (DviInput *df, unsigned len);
extern int din_unsigned (DviInput *df, unsigned len);
extern char *din_string (DviInput *df, char *buf, unsigned len);
extern void din_trace (DviInput *df, const char *fmt, ...);

#endif	/* dvi.h */
