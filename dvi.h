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

extern DviInput *df_init (DviInput *buf, const char *name, FILE *file);
extern void df_fatal (DviInput *df, const char *fmt, ...);
extern int df_byte (DviInput *df);
extern int df_signed (DviInput *df, unsigned len);
extern int df_unsigned (DviInput *df, unsigned len);
extern char *df_string (DviInput *df, char *buf, unsigned len);
extern void df_trace (DviInput *df, const char *fmt, ...);

#endif	/* dvi.h */
