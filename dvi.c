/*	process dvi file

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

#include "dvipost.h"
#include "dvi.h"
#include <ctype.h>

#define	DEBUG		1	/* Allow command debugging */

#define	RESOLUTION	300.

#define	PAGE_HEAD	0
#define	PAGE_BODY	1
#define	PAGE_NOTE	2
#define	PAGE_FOOT	3
#define	PAGE_FLOAT	4
#define	PAGE_NSTAT	5

static int page_num = 0;
static int page_stat = 0;

static char *magic = "dvipost";

static char *checkname (const char *base, char *name);

DviStat dvi_stat = { NULL, 0, 0, 0, 0, 0, 0 };
DviUnit dvi_unit = { 0 };
DviFont *dvi_font = NULL;
PosTab pos_changed = { NULL, 0, 0 };

/*	status stack
*/

static void dvi_push (void)
{
	DviStat *ptr = xalloc(sizeof(DviStat));
	*ptr = dvi_stat;
	dvi_stat.next = ptr;
}

static void dvi_pop (void)
{
	DviStat *ptr = dvi_stat.next;
	dvi_stat = *ptr;
	xfree(ptr);
}

/*	meassurement units
*/

static double pt_to_dvi = 0.;
static double in_to_dvi = 0.;
static double mm_to_dvi = 0.;
static double cm_to_dvi = 0.;

static void set_unit (DviToken *token)
{
	dvi_unit.num = token->par[0];
	dvi_unit.den = token->par[1];
	dvi_unit.mag = token->par[2];
	dvi_unit.true_conv = (dvi_unit.num / 254000.0);
	dvi_unit.true_conv *= (RESOLUTION / dvi_unit.den);
	dvi_unit.conv = dvi_unit.true_conv * (dvi_unit.mag / 1000.);

	pt_to_dvi = (254000. / dvi_unit.num) * (dvi_unit.den / 72.27);
	in_to_dvi = (254000. / dvi_unit.num) * (double) dvi_unit.den;
	cm_to_dvi = (100000. / dvi_unit.num) * (double) dvi_unit.den;
	mm_to_dvi = (10000. / dvi_unit.num) * (double) dvi_unit.den;
	/*
	message(NOTE, "1pt = %.0f dvi units.\n", pt_to_dvi);
	message(NOTE, "1in = %.0f dvi units.\n", in_to_dvi);
	message(NOTE, "1mm = %.0f dvi units.\n", mm_to_dvi);
	message(NOTE, "1cm = %.0f dvi units.\n", cm_to_dvi);
	*/
}

/*	debugging functions and macros
*/

#if	DEBUG
static int dbg_pos = 0;

static void dbg_printf (const char *fmt, ...)
{
	if	(dbg_pos)
	{
		va_list list;
		va_start(list, fmt);
		vfprintf(stderr, fmt, list);
		va_end(list);
	}
}

static void dbg_end (void)
{
	if	(dbg_pos)
	{
		putc('\n', stderr);
		dbg_pos = 0;
	}
}

static void dbg_beg (DviFile *df)
{
	if	(dbg_pos)
		putc('\n', stderr);

	dbg_pos = (verboselevel >= STAT) ? df->pos : 0;
}


static char *tab1[] = {
	"set1 ", "set2 ", "set3 ", "set4 ", "setrule ",
	"put1 ", "put2 ", "put3 ", "put4 ", "putrule ",
	"nop ", "beginning of page ", "eop ", "push ", "pop ",
	"right1 ", "right2 ","right3 ", "right4 ",
	"w0 ", "w1 ", "w2 ", "w3 ", "w4 ",
	"x0 ", "x1 ", "x2 ", "x3 ", "x4 ",
	"down1 ", "down2 ", "down3 ", "down4 ",
	"y0 ", "y1 ", "y2 ", "y3 ", "y4 ",
	"z0 ", "z1 ", "z2 ", "z3 ", "z4 "
};

static char *tab2[] = {
	"fnt1 ", "fnt2 ", "fnt3 ", "fnt4 ",
	"xxx ", "xxx ", "xxx ", "xxx ",
	"fntdef1 ", "fntdef2 ", "fntdef3 ", "fntdef4 ",
	"pre ", "post ", "post ",
	"??? ", "??? ", "??? ", "??? ", "??? ", "???"
};


static void dbg_type (DviToken *token)
{
	if	(dbg_pos == 0)
		return;

	fprintf(stderr, "%d: ", dbg_pos);

	if	(token->type >= DVI_FNT1)
		fputs(tab2[token->type - DVI_FNT1], stderr);
	else if	(token->type >= DVI_FONT_00)
		fprintf(stderr, "fntnum%d ", token->type - DVI_FONT_00);
	else if	(token->type >= DVI_SET1)
		fputs(tab1[token->type - DVI_SET1], stderr);
	else	fprintf(stderr, "setchar%d ", token->type);
}

#define	dbg(args)	dbg_printf args

#define	dbg_level(level)	\
dbg_printf("\nlevel %d:(h=%d,v=%d,w=%d,x=%d,y=%d,z=%d) ", level, \
	dvi_stat.h, dvi_stat.v, dvi_stat.w, dvi_stat.x,	dvi_stat.y, dvi_stat.z)

#define	dbg_font(font)	\
dbg_printf("current font is %s ", font ? font->token.str : "<undef>");

#else

#define	dbg_beg(df)
#define	dbg_end()
#define	dbg(args)
#define	dbg_type(token)
#define	dbg_level(level)
#define	dbg_font(font)

#endif

/*	post filter status
*/

typedef struct {
	char *name;
	void *par;
	void (*eval) (void *par, const char *arg);
} PostCmd;

static int osrule = 0;
static int cbrule = 0;
static int cbsep = 0;
static int cbexp = 0;
static int floatdepth = 0;
static int cbmode[PAGE_NSTAT] = { 0 };
static int cbstat = 0;
static int osmode[PAGE_NSTAT] = { 0 };
static int osstat = 0;
static int textwidth = 0;
static int textheight = 0;
static int evensidemargin = 0;
static int oddsidemargin = 0;
static int hoffset = 0;
static int voffset = 0;
static int topmargin = 0;
static int headheight = 0;
static int headsep = 0;
static int footskip = 0;
static int marginparsep = 0;
static int marginparwidth = 0;
static int layout = 0;
static int twoside = 0;
static int cbframe = 0;
static char *pre = NULL;
static char *post = NULL;
static char *osstart = NULL;
static char *osend = NULL;
static char *cbstart = NULL;
static char *cbend = NULL;

static void set_length (void *par, const char *arg)
{
	int *length = par;
	double x;
	char *p;

	x = strtod(arg, &p);

	if	(!p)
	{
		*length = 0;
		return;
	}

	while (isspace(*p))
		p++;

	if	(checkname("pt", p))	*length = x * pt_to_dvi;
	else if	(checkname("mm", p))	*length = x * mm_to_dvi;
	else if	(checkname("cm", p))	*length = x * cm_to_dvi;
	else if	(checkname("in", p))	*length = x * in_to_dvi;
	else			
	{
		message(ERR, "$!: unknown unit %s on page %d.\n",
			p, page_num);
		*length = 0;
	}
}

static void set_string (void *par, const char *arg)
{
	char **ptr = par;
	xfree(*ptr);
	*ptr = xcopy(arg);
}

static void set_flag (void *par, const char *arg)
{
	int *flag = par;

	*flag =	atoi(arg);
}

static void set_depth (void *par, const char *arg)
{
	int *depth = par;

	if	(atoi(arg))		depth[page_stat]++;
	else if	(depth[page_stat])	depth[page_stat]--;
}

static int save_stat = 0;

static void set_float (void *par, const char *arg)
{
	if	(atoi(arg))
	{
		if	(floatdepth == 0)
		{
			save_stat = page_stat;
			page_stat = PAGE_FLOAT;
		}

		floatdepth++;
	}
	else if	(floatdepth)
	{
		floatdepth--;

		if	(floatdepth == 0)
			page_stat = save_stat;
	}
}

static void set_page_stat (void *par, const char *arg)
{
	page_stat = (int) par;
}

static PostCmd cmd_tab[] = {
	{ "textwidth", &textwidth, set_length },
	{ "textheight", &textheight, set_length },
	{ "oddsidemargin", &oddsidemargin, set_length },
	{ "evensidemargin", &evensidemargin, set_length },
	{ "hoffset", &hoffset, set_length },
	{ "voffset", &voffset, set_length },
	{ "topmargin", &topmargin, set_length },
	{ "headheight", &headheight, set_length },
	{ "headsep", &headsep, set_length },
	{ "footskip", &footskip, set_length },
	{ "marginparsep", &marginparsep, set_length },
	{ "marginparwidth", &marginparwidth, set_length },
	{ "twoside", &twoside, set_flag },
	{ "layout", &layout, set_length },
	{ "cbframe", &cbframe, set_flag },
	{ "osrule", &osrule, set_length },
	{ "cbrule", &cbrule, set_length },
	{ "cbsep", &cbsep, set_length },
	{ "cbexp", &cbexp, set_length },
	{ "cbmode", cbmode, set_depth },
	{ "osmode", osmode, set_depth },
	{ "pre", &pre, set_string },
	{ "post", &post, set_string },
	{ "osstart", &osstart, set_string },
	{ "osend", &osend, set_string },
	{ "cbstart", &cbstart, set_string },
	{ "cbend", &cbend, set_string },
	{ "top", (void *) PAGE_BODY, set_page_stat },
	{ "footnote", (void *) PAGE_NOTE, set_page_stat },
	{ "bottom", (void *) PAGE_FOOT, set_page_stat },
	{ "float", NULL, set_float },
};

#define	cmd_dim	(sizeof(cmd_tab) / sizeof(cmd_tab[0]))

static char *checkname (const char *base, char *name)
{
	int i;

	for (i = 0; name[i] != 0; i++)
	{
		if	(base[i] == 0)	break;
		if	(base[i] != name[i])	return NULL;
	}

	if	(base[i] != 0)	return NULL;

	name += i;

	switch (*name)
	{
	case ':':
	case '=':
	case ' ':
		do	name++;
		while	(*name == '=' || *name == ':' || *name == ' ');

		break;
	case 0:	
		break;
	default:
		return NULL;
	}

	return name;
}

static int postcmd (char *cmd)
{
	char *arg;
	int i;

	cmd = checkname(magic, cmd);

	if	(!cmd)	return 0;

	while (isspace(*cmd))
		cmd++;

	dbg_end();

	for (i = 0; i < cmd_dim; i++)
	{
		if	((arg = checkname(cmd_tab[i].name, cmd)))
		{
			cmd_tab[i].eval(cmd_tab[i].par, arg);
			return 1;
		}
	}

	message(NOTE, "$!: unknown command <%s>\n", cmd);
	return 1;
}

DviToken tok_buf;

static void cmd_goto (DviFile *out, int v, int h)
{
	dout_down(out, v - dvi_stat.v);
	dvi_stat.v = v;
	dout_right(out, h - dvi_stat.h);
	dvi_stat.h = h;
}

static void cmd_move (DviFile *out, int v, int h)
{
	dout_down(out, v);
	dvi_stat.v += v;
	dout_right(out, h);
	dvi_stat.h += h;
}

static void endpage (DviFile *out)
{
	int i;
	int margin, cbcol;

	cbcol = marginparsep + marginparwidth + cbsep;

	if	(twoside && page_num % 2 == 0)
	{
		margin = evensidemargin;
		cbcol = hoffset + margin - cbcol - cbrule;
	}
	else
	{
		margin = oddsidemargin;
		cbcol = hoffset + margin + textwidth + cbcol;
	}
	
	if	(layout)
	{
		int footheight;
		int x0, y0;
		int n;

		y0 = voffset;
		x0 = hoffset;
		n = 10. * pt_to_dvi;
		cmd_goto(out, y0 + 0.5 * layout, x0 - n - 0.5 * layout);
		dout_putrule(out, layout, 2 * n);
		cmd_goto(out, y0 + n, x0 - 0.5 * layout);
		dout_putrule(out, 2 * n, layout);

		x0 += margin;
		y0 += topmargin;
		cmd_goto(out, y0, x0);
		dout_putrule(out, layout, textwidth);
		cmd_move(out, headheight, 0);
		dout_putrule(out, headheight, layout);
		dout_setrule(out, layout, textwidth - layout);
		dvi_stat.h += textwidth - layout;
		dout_putrule(out, headheight, layout);

		y0 += headheight + headsep;
		cmd_goto(out, y0, x0);
		dout_putrule(out, layout, textwidth);
		cmd_move(out, textheight, 0);
		dout_putrule(out, textheight, layout);
		dout_setrule(out, layout, textwidth - layout);
		dvi_stat.h += textwidth - layout;
		dout_putrule(out, textheight, layout);

		if	(twoside && page_num % 2 == 0)
			cmd_goto(out, y0, x0 - marginparsep -
				marginparwidth);
		else	cmd_goto(out, y0, x0 + textwidth + marginparsep);

		dout_putrule(out, layout, marginparwidth);
		cmd_move(out, textheight, 0);
		dout_putrule(out, textheight, layout);
		dout_setrule(out, layout, marginparwidth - layout);
		dvi_stat.h += marginparwidth - layout;
		dout_putrule(out, textheight, layout);

		y0 += textheight + footskip - footheight;
		cmd_goto(out, y0, x0);
		footheight = headheight;

		if	(footheight > footskip)
			footheight = footskip;

		dout_putrule(out, layout, textwidth);
		cmd_move(out, footheight, 0);
		dout_putrule(out, footheight, layout);
		dout_setrule(out, layout, textwidth - layout);
		dvi_stat.h += textwidth - layout;
		dout_putrule(out, footheight, layout);
	}

	if	(pos_changed.dim)
	{
		dout_special(out, cbstart);

		for (i = 0; i < pos_changed.dim; i++)
		{
			POS *p = pos_changed.tab + i;
			cmd_goto(out, p->end, cbcol);
			dout_putrule(out, p->end - p->beg, cbrule);
		}

		dout_special(out, cbend);
	}
}

/*	functions
*/

static void mv_right(int val)
{
	dbg(("h:=%d%+d", dvi_stat.h, val));
	dvi_stat.h += val;
	dbg(("=%d ", dvi_stat.h));
}

static void mv_down(int val)
{
	dbg(("v:=%d%+d", dvi_stat.v, val));
	dvi_stat.v += val;
	dbg(("=%d ", dvi_stat.v));
}

static int text_hpos = 0;	/* number of text characters past */
static int text_cnt = 0;	/* number of text characters past */
static int text_height = 0;	/* last character height */
static int text_depth = 0;	/* last character depth */

static double acc_height = 0.;	/* accumulated width weighted height */
static double acc_depth = 0.;	/* accumulated width weighted depth */
static double acc_width = 0.;	/* accumulated width */

static void text_beg (DviFile *out)
{
	if	(text_cnt)	return;

	text_hpos = dvi_stat.h;

	if	(cbmode[page_stat])
	{
		dout_special(out, cbstart);
		cbstat = 1;
	}

	if	(osmode[page_stat])
	{
		dout_special(out, osstart);
		osstat = 1;
	}

	return;
}

static void text_end (DviFile *out)
{
	if	(!text_cnt)	return;

	text_cnt = 0;

	if	(osmode[page_stat])
	{
		if	(cbframe)
		{
			dout_down(out, text_depth);
			dout_putrule(out, text_depth + text_height, osrule);
			dout_down(out, -text_depth);
		}

		{
			int l, o;
			double x;
			
			l = dvi_stat.h - text_hpos;
			x = (acc_height - acc_depth) / acc_width;
			o = 0.5 * (1. - osrule + x);

			dout_down(out, -o);
			dout_right(out, -l);
			dout_setrule(out, osrule, l);
			dout_down(out, o);
		}
	}

	if	(osstat)
	{
		dout_special(out, osend);
		osstat = 0;
	}

	if	(cbstat)
	{
		dout_special(out, cbend);
		cbstat = 0;
	}

	text_height = 0;
	text_depth = 0;
	acc_height = 0.;
	acc_depth = 0.;
	acc_width = 0.;
}

static void setchar (DviFile *out, int c)
{
	int w, mh, md;
	
	w = dvi_font->width[c];
	mh = text_height;
	md = text_depth;
	text_height = dvi_font->height[c];
	text_depth = dvi_font->depth[c];
	acc_width += w;
	acc_height += w * (double) text_height;
	acc_depth += w * (double) text_depth;

	if	(mh < text_height)	mh = text_height;
	if	(md < text_depth)	md = text_depth;

	mv_right(w);
	text_cnt++;

	if	(!cbmode[page_stat])	return;

	pos_add(&pos_changed, dvi_stat.v - text_height - cbexp,
		dvi_stat.v + text_depth + cbexp);

	if	(cbframe)
	{
		dout_right(out, -w);
		dout_down(out, md);
		dout_putrule(out, md + mh, osrule);
		dout_down(out, text_depth - md);
		dout_putrule(out, osrule, w);
		dout_down(out, -text_height - text_depth);
		dout_setrule(out, osrule, w);
		dout_down(out, text_height);
	}
}

/*	main routine
*/

int process_dvi (const char *id, FILE *ifile, FILE *ofile)
{
	DviFile dvifile[2], *df, *out;
	DviToken *token;
	int level;

	df = df_init(dvifile, id, ifile);
	out = df_init(dvifile + 1, "<tmpfile>", ofile);
	level = 0;

	for (;;)
	{
		dbg_beg(df);
		token = din_token(df);

		if	(token == NULL)
		{
			text_end(out);
			break;
		}

		dbg_type(token);

		if	(token->type <= DVI_SETC_127)
		{
			text_beg(out);
			dout_token(out, token);
			setchar(out, token->type);
			dbg_end();
			continue;
		}

		switch (token->type)
		{
		case DVI_SET1:
		case DVI_SET2:
		case DVI_SET3:
		case DVI_SET4:
			text_beg(out);
			dout_token(out, token);
			setchar(out, token->par[0] & 0xff);
			dbg_end();
			continue;
		case DVI_RIGHT1:
		case DVI_RIGHT2:
		case DVI_RIGHT3:
		case DVI_RIGHT4:
			dbg(("%d ", token->par[0]));
			dout_token(out, token);
			mv_right(token->par[0]);
			continue;
		case DVI_W1:
		case DVI_W2:
		case DVI_W3:
		case DVI_W4:
			dvi_stat.w = token->par[0];
			/*FALLTHROUGH*/
		case DVI_W0:
			dbg(("%d ", dvi_stat.w));
			dout_token(out, token);
			mv_right(dvi_stat.w);
			continue;
		case DVI_X1:
		case DVI_X2:
		case DVI_X3:
		case DVI_X4:
			dvi_stat.x = token->par[0];
			/*FALLTHROUGH*/
		case DVI_X0:
			dbg(("%d ", dvi_stat.x));
			dout_token(out, token);
			mv_right(dvi_stat.x);
			continue;
		default:
			break;
		}

		text_end(out);

		switch (token->type)
		{
		case DVI_SET_RULE:
			dbg(("height %d, width %d \n ",
				token->par[0], token->par[1]));
			mv_right(token->par[1]);
			break;
		case DVI_PUT_RULE:
			dbg(("height %d, width %d \n ",
				token->par[0], token->par[1]));
			break;
		case DVI_DOWN1:
		case DVI_DOWN2:
		case DVI_DOWN3:
		case DVI_DOWN4:
			dbg(("%d ", token->par[0]));
			mv_down(token->par[0]);
			break;
		case DVI_Y1:
		case DVI_Y2:
		case DVI_Y3:
		case DVI_Y4:
			dvi_stat.y = token->par[0];
			/*FALLTHROUGH*/
		case DVI_Y0:
			dbg(("%d ", dvi_stat.y));
			mv_down(dvi_stat.y);
			break;
		case DVI_Z1:
		case DVI_Z2:
		case DVI_Z3:
		case DVI_Z4:
			dvi_stat.z = token->par[0];
			/*FALLTHROUGH*/
		case DVI_Z0:
			dbg(("%d ", dvi_stat.z));
			mv_down(dvi_stat.z);
			break;
		case DVI_BOP:
			dbg(("%d ", token->par[0]));
			dvi_stat.next = NULL;
			dvi_stat.h = 0;
			dvi_stat.v = 0;
			dvi_stat.w = 0;
			dvi_stat.x = 0;
			dvi_stat.y = 0;
			dvi_stat.z = 0;
			dvi_font = NULL;
			page_num = token->par[0];
			page_stat = PAGE_HEAD;
			pos_init(&pos_changed);
			break;
		case DVI_EOP:
			endpage(out);
			break;
		case DVI_PUSH:
			dbg_level(level);
			dvi_push();
			level++;
			break;
		case DVI_POP:
			level--;
			dvi_pop();
			dbg_level(level);
			break;
		case DVI_XXX1:
		case DVI_XXX2:
		case DVI_XXX3:
		case DVI_XXX4:
			dbg(("'%s' ", token->str));

			if	(postcmd(token->str))
				continue;
			break;
		case DVI_FNT_DEF1:
		case DVI_FNT_DEF2:
		case DVI_FNT_DEF3:
		case DVI_FNT_DEF4:
			dbg(("%d: %s ", token->par[0], token->str));
			break;
		case DVI_PRE:
			set_unit(token);
			break;
		case DVI_POST:	/* postamble beginning */
			dout_token(out, token);
			dbg_end();
			return df->ok ? 0 : 1;
		case DVI_POST_POST: /* postamble ending */
			df_fatal(df, "POST_POST without PRE.");
			break;
		default:
			if	(token->type < DVI_FONT_00)
			{
				;
			}
			else if	(token->type <= DVI_FONT_63)
			{
				int font = token->type - DVI_FONT_00;
				dvi_font = DviFont_get(font);
				dbg_font(dvi_font);
			}

			break;
		}

		dout_token(out, token);
		dbg_end();
	}

	return 1;
}
