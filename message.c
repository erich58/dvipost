/*	message output
	$Copyright (C) 2002 Erich Fr�hst�ck
	A-3423 St.Andr�/W�rdern, S�dtirolergasse 17-21/5
*/

#include "dvipost.h"
#include <stdarg.h>

char *pname = "LaTeXPost";
int verboselevel = NOTE;

void message (int level, const char *fmt, ...)
{
	if	(level <= verboselevel)
	{
		va_list list;

		if	(*fmt == '$')
		{
			fmt++;

			if	(*fmt == '!')
			{
				fputs(pname, stderr);
				fmt++;
			}
		}

		va_start(list, fmt);
		vfprintf(stderr, fmt, list);
		va_end(list);
	}
}
