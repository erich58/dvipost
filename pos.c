/*	position table
	$Copyright (C) 2002 Erich Frühstück
	A-3423 St.Andrä/Wördern, Südtirolergasse 17-21/5
*/

#include "ltxpost.h"

#define	BSIZE	1000

static void pos_expand (PosTab *pos)
{
	if	(pos->size <= pos->dim)
	{
		POS *save = pos->tab;
		pos->size += BSIZE;
		pos->tab = xalloc(pos->size * sizeof(POS));
		memcpy(pos->tab, save, pos->dim * sizeof(POS));
		xfree(save);
	}
}

void pos_init (PosTab *pos)
{
	pos->dim = 0;
}

void pos_add (PosTab *pos, int beg, int end)
{
	if	(beg > end)
	{
		int x = beg;
		beg = end;
		end = x;
	}

	if	(pos->dim)
	{
		POS *last = pos->tab + pos->dim - 1;

		if	(beg <= last->end && end >= last->beg)
		{
			if	(last->end < end)	last->end = end;
			if	(last->beg > beg)	last->beg = beg;

			return;
		}
	}

	pos_expand(pos);
	pos->tab[pos->dim].beg = beg;
	pos->tab[pos->dim].end = end;
	pos->dim++;
}
