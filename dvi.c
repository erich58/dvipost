/*	process dvi file
	$Copyright (C) 2002 Erich Frühstück
	A-3423 St.Andrä/Wördern, Südtirolergasse 17-21/5
*/

#include "ltxpost.h"
#include "dvi.h"

#define	DEBUG		1	/* Allow command debugging */

#define	RESOLUTION	300.

DviStat dvi_stat = { NULL, 0, 0, 0, 0, 0, 0 };
DviUnit dvi_unit = { 0 };
DviFont *dvi_font = NULL;

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

static void set_unit (DviToken *token)
{
	dvi_unit.num = token->par[0];
	dvi_unit.den = token->par[1];
	dvi_unit.mag = token->par[2];
	dvi_unit.true_conv = (dvi_unit.num / 254000.0);
	dvi_unit.true_conv *= (RESOLUTION / dvi_unit.den);
	dvi_unit.conv = dvi_unit.true_conv * (dvi_unit.mag / 1000.);
}

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

static void dbg_beg (DviFile *df)
{
	dbg_pos = (verboselevel >= STAT) ? df->pos : 0;
}

static void dbg_end (void)
{
	if	(dbg_pos)
	{
		putc('\n', stderr);
		dbg_pos = 0;
	}
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
	if	(!(token && dbg_pos))
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
dbg_printf("\nlevel %d:(h=%d,v=%d,w=%d,x=%d,y=%d,z=%d) ", \
	level, dvi_stat.h, dvi_stat.v, dvi_stat.w, dvi_stat.x,	\
	dvi_stat.y, dvi_stat.z, pixel(dvi_stat.h), pixel(dvi_stat.v))

#define	dbg_font(font)	\
dbg_printf("current font is %s ", font ? font->token.str : "<undef>");

static int pixel (int val)
{
	if	(val < 0)
		return -(int) (-val * dvi_unit.conv + 0.5);
	
	return (int) (val * dvi_unit.conv + 0.5);
}

#else

#define	dbg_beg(df)
#define	dbg_end()
#define	dbg(args)
#define	dbg_type(token)
#define	dbg_level(level)
#define	dbg_font(font)
#define	pixel(x)	0

#endif

static void mv_right(int val)
{
	dbg(("h:=%d%+d", dvi_stat.h, val));
	dvi_stat.h += val;
	dbg(("=%d ", dvi_stat.h));
}

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
		dbg_type(token);

		if	(token == NULL)	break;

		switch (token->type)
		{
		case DVI_SET1:
		case DVI_SET2:
		case DVI_SET3:
		case DVI_SET4:
			mv_right(dvi_font->width[token->par[0] & 0xff]);
			break;
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
		case DVI_RIGHT1:
		case DVI_RIGHT2:
		case DVI_RIGHT3:
		case DVI_RIGHT4:
			dbg(("%d ", token->par[0]));
			mv_right(token->par[0]);
			break;
		case DVI_W1:
		case DVI_W2:
		case DVI_W3:
		case DVI_W4:
			dvi_stat.w = token->par[0];
			/*FALLTHROUGH*/
		case DVI_W0:
			dbg(("%d ", dvi_stat.w));
			mv_right(dvi_stat.w);
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
			if	(token->type <= DVI_SETC_127)
			{
				mv_right(dvi_font->width[token->type]);
			}
			else if	(token->type < DVI_FONT_00)
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
