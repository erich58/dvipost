/*	postfilter

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

#ifndef	dvipost_h
#define	dvipost_h	1

#define	_XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
extern void dvipost_search (time_t stamp);

#endif	/* dvipost.h */
