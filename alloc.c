/*	memmory allocation
	$Copyright (C) 2002 Erich Frühstück
	A-3423 St.Andrä/Wördern, Südtirolergasse 17-21/5
*/

#include "ltxpost.h"

extern void *xalloc (size_t size)
{
	void *p;
	
	if	(size == 0)	return NULL;

	p = malloc(size);

	if	(p == NULL)
	{
		message(ERR, "malloc(%lu) failed.\n", (unsigned long) size);
		exit(EXIT_FAILURE);
	}

	return p;
}

extern void xfree (void *p)
{
	if	(p)	free(p);
}
