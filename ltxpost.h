/*	postfilter
	$Copyright (C) 2002 Erich Fr�hst�ck
	A-3423 St.Andr�/W�rdern, S�dtirolergasse 17-21/5
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

extern int process_dvi (FILE *in, FILE *out);
extern int process_pdf (FILE *in, FILE *out);

#endif	/* dvipost.h */
