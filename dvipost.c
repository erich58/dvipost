/*	postfilter
	$Copyright (C) 2002 Erich Frühstück
	A-3423 St.Andrä/Wördern, Südtirolergasse 17-21/5
*/


#include "dvipost.h"
#include "dvi.h"
#include <unistd.h>
#include <getopt.h>

/*	print usage information
*/

static void usage (int flag)
{
	fprintf(stderr, "usage: %s [-hqv] input [output]\n", pname);

	if	(flag > 0)
	{
		fputs("\t-h this output.\n", stderr);
		fputs("\t-q be quiet, report only errors.\n", stderr);
		fputs("\t-v increase verbose level.\n", stderr);
	}

	exit(flag >= 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}

/*	the main programm
*/

int main (int argc, char **argv)
{
	extern int optind;
	extern char *optarg;
	char *iname, *oname;
	FILE *input, *tmp, *output;
	int opt;
	int c;
	int stat;

/*	parse optionms and args
*/
	pname = argv[0];

	while ((opt = getopt(argc, argv, "hqv")) != -1)
	{
		switch (opt)
		{
		case 'h':
			usage(1);
			break;
		case 'q':
			verboselevel = 0;
			break;
		case 'v':
			verboselevel++;
			break;
		default:
			usage(-1);
			break;
		}
	}

	if	(optind == argc)
		usage(-1);

	iname = (optind < argc) ? argv[optind++] : NULL;
	oname = (optind < argc) ? argv[optind++] : NULL;

	if	(optind < argc)
		usage(0);

	message(NOTE, "This is DVIPost, Version 0.1, $Revision$.\n");

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
			fprintf(stderr, "%s: ", pname);
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

	switch (c)
	{
	case DVI_PRE:
		stat = process_dvi(input, tmp);
		break;
	case '%':
		stat = process_pdf(input, tmp);
		break;
	default:
		message(ERR, "$!: Bad magic: "
			"%s is neither dvi nor pdf file.\n",
			iname);
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
