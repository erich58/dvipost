/*	message output
	$Copyright (C) 2002 Erich Frühstück
	A-3423 St.Andrä/Wördern, Südtirolergasse 17-21/5
*/

#include "dvipost.h"
#include <stdarg.h>

int verboselevel = NOTE;

void message (int level, const char *fmt, ...)
{
	if	(level <= verboselevel)
	{
		va_list list;

		va_start(list, fmt);
		vfprintf(stderr, fmt, list);
		va_end(list);
	}
}
