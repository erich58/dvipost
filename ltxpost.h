/*	postfilter
	$Copyright (C) 2002 Erich Frühstück
	A-3423 St.Andrä/Wördern, Südtirolergasse 17-21/5
*/

#ifndef	dvipost_h
#define	dvipost_h	1

#include <stdio.h>
#include <stdlib.h>

#define	ERR	0
#define	NOTE	1
#define	STAT	2
#define	DBG	3

extern char *pname;
extern int verboselevel;
extern void message (int level, const char *fmt, ...);

extern void *xalloc (size_t size);
extern void xfree (void *data);

extern int process_dvi (const char *id, FILE *in, FILE *out);
extern int process_pdf (const char *id, FILE *in, FILE *out);

#endif	/* dvipost.h */
