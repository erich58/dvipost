/*	process dvi file
	$Copyright (C) 2002 Erich Frühstück
	A-3423 St.Andrä/Wördern, Südtirolergasse 17-21/5
*/

#include "ltxpost.h"
#include "dvi.h"
#include <ctype.h>

#define	DEBUG		1	/* Allow command debugging */

#define	RESOLUTION	300.

static char *magic = "dvipost";

DviStat dvi_stat = { NULL, 0, 0, 0, 0, 0, 0 };
DviUnit dvi_unit = { 0 };
DviFont *dvi_font = NULL;

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

static void set_unit (DviToken *token)
{
	dvi_unit.num = token->par[0];
	dvi_unit.den = token->par[1];
	dvi_unit.mag = token->par[2];
	dvi_unit.true_conv = (dvi_unit.num / 254000.0);
	dvi_unit.true_conv *= (RESOLUTION / dvi_unit.den);
	dvi_unit.conv = dvi_unit.true_conv * (dvi_unit.mag / 1000.);

	pt_to_dvi = (254000. / dvi_unit.num) * (dvi_unit.den / 72.27);
	message(NOTE, "1pt = %.0f dvi units.\n", pt_to_dvi);
	/*
	in_to_dvi = (254000. / dvi_unit.num) * (double) dvi_unit.den;
	cm_to_dvi = (100000. / dvi_unit.num) * (double) dvi_unit.den;
	mm_to_dvi = (10000. / dvi_unit.num) * (double) dvi_unit.den;
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

static int thickness = 0;
static int cbrule = 0;
static int cbmode = 0;
static int textwidth = 0;
static int textheight = 0;
static int evensidemargin = 0;
static int oddsidemargin = 0;
static int topmargin = 0;
static int headheight = 0;
static int headsep = 0;
static int footskip = 0;
static int showlayout = 0;
static int cbframe = 0;
static char *pre = NULL;
static char *post = NULL;

static void set_length (void *par, const char *arg)
{
	int *length = par;
	*length = pt_to_dvi * strtod(arg, NULL);
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

	if	(atoi(arg))	(*depth)++;
	else if	(*depth)	(*depth)--;
}

static PostCmd cmd_tab[] = {
	{ "textwidth", &textwidth, set_length },
	{ "textheight", &textheight, set_length },
	{ "oddsidemargin", &oddsidemargin, set_length },
	{ "evensidemargin", &evensidemargin, set_length },
	{ "topmargin", &topmargin, set_length },
	{ "headheight", &headheight, set_length },
	{ "headsep", &headsep, set_length },
	{ "footskip", &footskip, set_length },
	{ "showlayout", &showlayout, set_flag },
	{ "cbframe", &cbframe, set_flag },
	{ "thickness", &thickness, set_length },
	{ "cbrule", &cbrule, set_length },
	{ "cbmode", &cbmode, set_depth },
	{ "pre", &pre, set_string },
	{ "post", &post, set_string },
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
	if	(showlayout)
	{
		int footheight;
		int margin;
		int n;

		margin = oddsidemargin;
		dout_special(out, pre);
		cmd_goto(out, 0, 0);
		dout_putrule(out, thickness, margin + textwidth);
		n = topmargin + headheight + headsep + textheight + footskip;
		cmd_move(out, n, 0);
		dout_putrule(out, n, thickness);

		cmd_goto(out, topmargin, margin);
		dout_putrule(out, thickness, textwidth);
		cmd_move(out, headheight, 0);
		dout_putrule(out, headheight, thickness);
		dout_setrule(out, thickness, textwidth);
		dvi_stat.h += textwidth;
		dout_putrule(out, headheight, thickness);
		cmd_move(out, headsep, -textwidth);
		dout_putrule(out, thickness, textwidth);
		cmd_move(out, textheight, 0);
		dout_putrule(out, textheight, thickness);
		dout_setrule(out, thickness, textwidth);
		dvi_stat.h += textwidth;
		dout_putrule(out, textheight, thickness);

		footheight = headheight;

		if	(footheight > footskip)
			footheight = footskip;

		cmd_move(out, footskip - footheight, -textwidth);
		dout_putrule(out, thickness, textwidth);
		cmd_move(out, footheight, 0);
		dout_putrule(out, footheight, thickness);
		dout_setrule(out, thickness, textwidth);
		dvi_stat.h += textwidth;
		dout_putrule(out, footheight, thickness);

		dout_special(out, post);
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
	return;
}

static void text_end (DviFile *out)
{
	if	(!text_cnt)	return;

	text_cnt = 0;

	if	(cbmode)
	{
		dout_special(out, pre);

		if	(cbframe)
		{
			dout_down(out, text_depth);
			dout_putrule(out, text_depth + text_height, thickness);
			dout_down(out, -text_depth);
		}

		{
			int l, o;
			double x;
			
			l = dvi_stat.h - text_hpos;
			dbg(("[l=%d] ", l));
			x = (acc_height - acc_depth) / acc_width;
			o = 0.5 * (1. - thickness + x);

			dout_down(out, -o);
			dout_right(out, -l);
			dout_setrule(out, thickness, l);
			dout_down(out, o);
		}

		dout_special(out, post);
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

	if	(!cbmode)	return;

	if	(cbframe)
	{
		dout_special(out, pre);
		dout_right(out, -w);
		dout_down(out, md);
		dout_putrule(out, md + mh, thickness);
		dout_down(out, text_depth - md);
		dout_putrule(out, thickness, w);
		dout_down(out, -text_height - text_depth);
		dout_setrule(out, thickness, w);
		dout_down(out, text_height);
		dout_special(out, post);
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
		case DVI_DOWN1:
		case DVI_DOWN2:
		case DVI_DOWN3:
		case DVI_DOWN4:
			dbg(("%d v:=%d%+d", token->par[0],
				dvi_stat.v, token->par[0]));
			dvi_stat.v += token->par[0];
			dbg(("=%d ", dvi_stat.v));
			break;
		case DVI_X1:
		case DVI_X2:
		case DVI_X3:
		case DVI_X4:
			dvi_stat.x = token->par[0];
			/*FALLTHROUGH*/
		case DVI_X0:
			dbg(("%d ", dvi_stat.x));
			mv_right(dvi_stat.x);
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
