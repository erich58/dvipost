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
} DVIFile;

extern DVIFile *df_init (DVIFile *buf, const char *name, FILE *file);
extern void df_fatal (DVIFile *df, const char *fmt, ...);
extern int df_byte (DVIFile *df);
extern int df_signed (DVIFile *df, unsigned len);
extern int df_unsigned (DVIFile *df, unsigned len);
extern char *df_string (DVIFile *df, char *buf, unsigned len);
extern void df_trace (DVIFile *df, const char *fmt, ...);

#endif	/* dvi.h */
