/*	postfilter
	$Copyright (C) 2002 Erich Frühstück
	A-3423 St.Andrä/Wördern, Südtirolergasse 17-21/5
*/

#ifndef	ltxpost_h
#define	ltxpost_h	1

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
extern char *xcopy (const char *str);

typedef struct {
	int beg;
	int end;
} POS;

typedef struct {
	POS *tab;
	size_t dim;
	size_t size;
} PosTab;

void pos_init (PosTab *pos);
void pos_add (PosTab *pos, int beg, int end);

extern int process_dvi (const char *id, FILE *in, FILE *out);
extern int dvipost (const char *iname, const char *oname);

#endif	/* ltxpost.h */
