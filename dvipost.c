/*	dvi file post processing

$Copyright (C) 2002 Erich Frühstück
This file is part of pplatex.

pplatex is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

pplatex is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public
License along with pplatex; see the file COPYING.
If not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
*/


#include "dvipost.h"
#include "dvi.h"
#include <unistd.h>

int dvipost (const char *iname, const char *oname)
{
	FILE *input, *tmp, *output;
	int c;
	int stat;

/*	open temporary file
*/
	tmp = tmpfile();

	if	(!tmp)
	{
		perror(pname);
		exit(EXIT_FAILURE);
	}

/*	open input file
*/
	if	(strcmp(iname, "-") != 0)
	{
		input = fopen(iname, "rb");

		if	(!input)
		{
			fprintf(stderr, "%s: ", iname);
			perror(iname);
			fclose(tmp);
			return EXIT_FAILURE;
		}
	}
	else
	{
		iname = "<stdin>";
		input = stdin;
	}

/*	check magic and process input
*/
	message(NOTE, "$!: Process input file %s\n", iname);
	c = getc(input);
	ungetc(c, input);

	switch (c)
	{
	case DVI_PRE:
		stat = process_dvi(iname, input, tmp);
		break;
	default:
		message(ERR, "$!: Bad magic: %s is not a dvi file.\n", iname);
		stat = 1;
		break;
	}

	fclose(input);

	if	(stat)
	{
		fclose(tmp);
		return EXIT_FAILURE;
	}

	if	(oname == NULL)
	{
		fclose(tmp);
		return EXIT_SUCCESS;
	}

/*	copy file to output
*/
	rewind(tmp);

	if	(oname && strcmp(oname, "-") != 0)
	{
		output = fopen(oname, "wb");

		if	(!output)
		{
			fprintf(stderr, "%s: ", pname);
			perror(oname);
			fclose(tmp);
			return EXIT_FAILURE;
		}
	}
	else
	{
		oname = "<stdout>";
		output = stdout;
	}

	message(NOTE, "$!: Copy data to %s\n", oname);

	while ((c = getc(tmp)) != EOF)
		putc(c, output);

	fclose(tmp);

	if	(ferror(output))
	{
		fprintf(stderr, "%s: ", pname);
		perror(oname);
		stat = EXIT_FAILURE;
	}
	else	stat = EXIT_SUCCESS;

	fclose(output);
	return stat;
}
