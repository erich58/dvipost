/*	postfilter
	$Copyright (C) 2002 Erich Frühstück
	A-3423 St.Andrä/Wördern, Südtirolergasse 17-21/5
*/


#include "dvipost.h"
#include <unistd.h>
#include <getopt.h>

extern int optind;
extern char *optarg;

int verbose = 0;

static void usage (const char *name)
{
	fprintf(stderr, "usage: %s [-v] input [output]\n", name);
}

int main (int argc, char **argv)
{
	FILE *input, *output;
	int opt;
	int c;

	while ((opt = getopt(argc, argv, "v")) != -1)
	{
		switch (opt)
		{
		case 'v':
			verbose++;
			break;
		default:
			return EXIT_FAILURE;
		}
	}

	if	(optind == argc)
	{
		usage(argv[0]);
		return EXIT_SUCCESS;
	}

	if	(optind < argc)
	{
		if	(strcmp(argv[optind], "-") != 0)
		{
			input = fopen(argv[optind], "rb");

			if	(!input)
			{
				fprintf(stderr, "%s: file %s not found.\n",
					argv[0], argv[optind]);
				return EXIT_FAILURE;
			}
		}
		else	input = stdin;

		optind++;
	}

	if	(optind < argc)
	{
		if	(strcmp(argv[optind], "-") != 0)
		{
			output = fopen(argv[optind], "wb");

			if	(!output)
			{
				fprintf(stderr, "%s: file %s not found.\n",
					argv[0], argv[optind]);
				fclose(input);
				return EXIT_FAILURE;
			}
		}
		else	output = stdout;

		optind++;
	}
	else	output = stdout;

	if	(optind < argc)
	{
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	while ((c = getc(input)) != EOF)
		putc(c, output);

	return EXIT_SUCCESS;
}
