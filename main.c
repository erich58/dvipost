/*	LaTeX - Postfilter

$Copyright (C) 2002 Erich Frühstück

Dvipost is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

Dvipost is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public
License along with pplatex; see the file COPYING.
If not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
*/

static char *version = "dvipost version 0.8\n\
Dvipost is copyright (C) 2002 Erich Fruehstueck.\n";

#include "ltxpost.h"
#include "dvi.h"
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

char **tex_argv = NULL;
int tex_argc = 0;

static int dvipost_help (char **argv, char *oarg);
static int pptex_help (char **argv, char *oarg);

static int pptex_arg (char **argv, char *oarg)
{
	tex_argv[tex_argc++] = argv[0];

	if	(oarg == NULL)
	{
		tex_argv[tex_argc++] = argv[1];
		return 2;
	}

	return 1;
}

static int pptex_noarg (char **argv, char *oarg)
{
	tex_argv[tex_argc++] = argv[0];
	return 1;
}

static int pptex_version (char **argv, char *oarg)
{
	puts(version);
	tex_argv[tex_argc++] = argv[0];
	return 1;
}

static int dvipost_debug (char **argv, char *oarg)
{
	verboselevel++;
	return 1;
}


typedef struct {
	char *name;
	char *desc;
	int (*eval) (char **argv, char *oarg);
} ODEF;

static ODEF pptex_odef[] = {
	{ "fmt", NULL, pptex_arg },
	{ "ini", NULL, pptex_noarg },
	{ "interaction", NULL, pptex_arg },
	{ "kpathsea-debug", NULL, pptex_arg },
	{ "mktex", NULL, pptex_arg },
	{ "mltex", NULL, pptex_noarg },
	{ "no-mktex", NULL, pptex_arg },
	{ "output-comment", NULL, pptex_arg },
	{ "progname", NULL, pptex_arg },
	{ "shell-escape", NULL, pptex_noarg },
	{ "translate-file", NULL, pptex_arg },
	{ "help", NULL, pptex_help },
	{ "version", NULL, pptex_version },
	{ "debug", "increase dvipost debug level", dvipost_debug },
	{ NULL, NULL, NULL },
};

static int dvipost_version (char **argv, char *oarg)
{
	puts(version);
	puts("There is NO warranty.  Redistribution of this software is");
	puts("covered by the terms of the GNU General Public License.");
	puts("For more information about these matters, see the files");
	puts("named COPYING and the TeX source.");
	exit(EXIT_SUCCESS);
	return 0;
}

static ODEF dvipost_odef[] = {
	{ "debug", "increase dvipost debug level", dvipost_debug },
	{ "version", "output version information and exit", dvipost_version },
	{ "help", "display this help and exit", dvipost_help },
	{ NULL, NULL, NULL },
};

static int pptex_help (char **argv, char *oarg)
{
	ODEF *odef;

	printf("Usage: %s [-debug] [TEXARGS]\n\n", pname);

	printf("Run %s and process DVI file with dvipost.\n",
		tex_argv[0]);
	printf("The following options are interpreted by %s:\n", pname);

	for (odef = pptex_odef; odef->name; odef++)
	{
		if	(odef->desc)
			printf("-%-12s %s\n", odef->name, odef->desc);
	}

	printf("\nAll other options and arguments are passed to %s.\n",
		tex_argv[0]);
	printf("\n");
	tex_argv[tex_argc++] = argv[0];
	return 1;
}

static int dvipost_help (char **argv, char *oarg)
{
	ODEF *odef;

	printf("Usage: %s [OPTION] infile [outfile]\n", pname);

	for (odef = dvipost_odef; odef->name; odef++)
	{
		if	(odef->desc)
			printf("-%-12s %s\n", odef->name, odef->desc);
	}

	return 1;
}

static int optend = 0;

static int is_arg (char *arg)
{
	return (arg[0] != '-' || arg[1] == 0);
}

static int is_optend (char *arg)
{
	return (arg[0] == '-' && arg[1] == '-' && arg[2] == 0);
}

static int eval_opt (char **argv, ODEF *def)
{
	char *oname;
	int stat;

	if	(optend || argv[0][0] != '-')	return 0;

	oname = argv[0] + 1;

	switch (*oname)
	{
	case  0:	return 0;
	case '-':	oname++; break;
	default:	break;
	}

	if	(*oname == 0)
	{
		optend = 1;
		return 0;
	}

	for (stat = 0; def->name != NULL; def++)
	{
		char *p = def->name;
		char *arg = oname;

		while (*p && *p == *arg)
		{
			p++;
			arg++;
		}

		switch (*arg)
		{
		case '=':
			if	(stat == 0)
				stat = def->eval(argv, arg + 1);
			break;
		case 0:
			if	(stat == 0)
				stat = def->eval(argv, NULL);
			break;
		default:
			break;
		}
	}

	return stat;
}

static char *get_dvi_name (const char *arg)
{
	char *p, *dvi;
	
	p = strrchr(arg, '/');

	if	(p && p[1])
		arg = p + 1;

	dvi = strcpy(xalloc(strlen(arg) + 4), arg);
	p = strchr(dvi, '.');

	if	(p && strcmp(p, ".tex") == 0)
		*p = 0;

	strcat(dvi, ".dvi");
	return dvi;
}

static void unrecognized_option (const char *opt)
{
	message(ERR, "$!: unrecognized option: `%s'\n", opt);
	message(ERR, "Try %s --help for more information.\n", pname);
	exit(EXIT_FAILURE);
}

/*	the main programm
*/

int main (int argc, char **argv)
{
	char *dviname;
	int flag;
	int i, n;
	int status;
	pid_t pid;

	tex_argv = xalloc((1 + argc) * sizeof(char*));
	tex_argv[0] = argv[0];
	tex_argc = 1;

	pname = strrchr(argv[0], '/');

	if	(pname == NULL)	pname = argv[0];
	else			pname++;

	if	(pname[0] != 'p' || pname[1] != 'p')
	{
		for (i = 1; i < argc; )
		{
			if	(is_arg(argv[i]) || optend)
			{
				tex_argv[tex_argc++] = argv[i++];
			}
			else if	(is_optend(argv[i]))
			{
				tex_argv[tex_argc++] = argv[i++];
				optend = 1;
			}
			else if	((n = eval_opt(argv + i, dvipost_odef)) != 0)
			{
				i += n;
			}
			else	unrecognized_option (argv[i]);
		}

		tex_argv[tex_argc] = NULL;

		if	(tex_argv[1] == NULL || tex_argc > 3)
		{
			dvipost_help(NULL, NULL);
			return EXIT_SUCCESS;
		}

		return dvipost(tex_argv[1], tex_argv[2]);
	}

	tex_argv[0] = pname + 2;
	flag = 1;
	dviname = NULL;
	optend = 0;

	for (i = 1; i < argc; )
	{
		if	(is_arg(argv[i]) || optend)
		{
			if	(argv[i][0] == '\\')
			{
				tex_argv[tex_argc++] = argv[i++];
				flag = 0;
			}
			else if	(flag && argv[i][0] != '&')
			{
				dviname = get_dvi_name(argv[i]);
				tex_argv[tex_argc++] = argv[i++];
				flag = 0;
			}
			else	tex_argv[tex_argc++] = argv[i++];
		}
		else if	(is_optend(argv[i]))
		{
			tex_argv[tex_argc++] = argv[i++];
			optend = 1;
		}
		else if	((n = eval_opt(argv + i, pptex_odef)) != 0)
		{
			i += n;
		}
		else	unrecognized_option (argv[i]);
	}

	tex_argv[tex_argc] = NULL;
	fflush(stdout);

	if	(!dviname)
		execvp(tex_argv[0], tex_argv);

	pid = fork();

	if	(pid == -1)
	{
		perror(argv[0]);
		exit(EXIT_FAILURE);
	}

	if	(pid == 0)
		execvp(tex_argv[0], tex_argv);

	waitpid(pid, &status, 0);
	return dvipost(dviname, dviname);
}
