/*	process dvi file
	$Copyright (C) 2002 Erich Frühstück
	A-3423 St.Andrä/Wördern, Südtirolergasse 17-21/5
*/

#include "ltxpost.h"
#include "dvi.h"

static void putval (FILE *file, int len, unsigned val)
{
	if	(--len > 0)
		putval(file, len, val >> 8);

	putc(val, file);
}

static void parse_pre (DviInput *df, FILE *out)
{
	unsigned n;
	char *desc;

	n = din_byte(df);
	putc(n, out);
	n = din_byte(df);

	if	(n != 2)
	{
		din_fatal(df, "Bad DVI file: id byte not 2.");
		return;
	}

	putval(out, 1, n);
	putval(out, 4, din_unsigned(df, 4));
	putval(out, 4, din_unsigned(df, 4));
	putval(out, 4, din_unsigned(df, 4));
	n = din_byte(df);
	putval(out, 1, n);
	desc = din_string(df, NULL, n);
	fwrite(desc, 1, n, out);
	din_trace(df, "'%s'\n", desc);
}

static void parse_fntdef (DviInput *df, FILE *out, int n)
{
	int font;
	int a, l;
	char *name;

	font = din_unsigned(df, n);
	putval(out, n, font);
	din_trace(df, "Font %d:", font);

	putval(out, 4, din_unsigned(df, 4));
	putval(out, 4, din_unsigned(df, 4));
	putval(out, 4, din_unsigned(df, 4));

	a = din_unsigned(df, 1);
	l = din_unsigned(df, 1);
	putval(out, 1, a);
	putval(out, 1, l);

	name = din_string(df, NULL, a + l);
	fwrite(name, 1, a + l, out);
	din_trace(df, " %s", name);

	din_trace(df, "\n");
}

static int parse_post(DviInput *df, FILE *out)
{
	int c, n;
	unsigned start;

	start = df->pos - 1;
	din_trace(df, "Postamble starts at byte %d.\n", start);
	putval(out, 4, din_unsigned(df, 4));
	putval(out, 4, din_unsigned(df, 4));
	putval(out, 4, din_unsigned(df, 4));
	putval(out, 4, din_unsigned(df, 4));
	putval(out, 4, din_unsigned(df, 4));
	putval(out, 4, din_unsigned(df, 4));
	putval(out, 2, din_unsigned(df, 2));
	putval(out, 2, din_unsigned(df, 2));

	c = din_byte(df);

	while (df->ok)
	{
		putc(c, out);

		switch (c)
		{
		case DVI_NOP:	/* no operation */
			break;
		case DVI_FNT_DEF1: /* define the meaning of a font number */
			parse_fntdef(df, out, 1);
			break;
		case DVI_FNT_DEF2: /* ??? */
			parse_fntdef(df, out, 2);
			break;
		case DVI_FNT_DEF3: /* ??? */
			parse_fntdef(df, out, 3);
			break;
		case DVI_FNT_DEF4: /* ??? */
			parse_fntdef(df, out, 4);
			break;
		case DVI_POST_POST:	/* postamble beginning */
			n = df->pos - 1;

			if	(din_unsigned(df, 4) != start)
				din_fatal(df, "Incorrect start of Postamble.");

			if	(din_byte(df) != 2)
				din_fatal(df, "Bad Postamble: id byte not 2.");

			putval(out, 4, start);
			putval(out, 1, 2);
			n = 8 - (n + 6) % 4;

			if	(n == 8)	n = 4;

			while (n-- > 0)
				putc(223, out);

			return 0;
		default:
			din_fatal(df, "Command %d not allowed in postamble.",
				c);
			break;
		}

		c = din_byte(df);
	}

	return 1;
}


int process_dvi (const char *id, FILE *in, FILE *out)
{
	DviInput *df;
	int par;
	int post;
	int c;

	df = din_init(NULL, id, in);
	parse_pre(df, out);
	c = din_byte(df);
	post = 0;

	while (df->ok)
	{
		putc(c, out);

		switch (c)
		{
		case DVI_SET1:	/* typeset a character and move right */
			putval(out, 1, din_unsigned(df, 1));
			break;
		case DVI_SET2:	/* ??? */
			putval(out, 2, din_unsigned(df, 2));
			break;
		case DVI_SET3:	/* ??? */
			putval(out, 3, din_unsigned(df, 3));
			break;
		case DVI_SET4:	/* ??? */
			putval(out, 4, din_unsigned(df, 4));
			break;
		case DVI_SET_RULE:	/* typeset a rule and move right */
			putval(out, 4, din_unsigned(df, 4));
			putval(out, 4, din_unsigned(df, 4));
			break;
		case DVI_PUT1:	/* typeset a character */
			putval(out, 1, din_unsigned(df, 1));
			break;
		case DVI_PUT2:	/* ??? */
			putval(out, 2, din_unsigned(df, 2));
			break;
		case DVI_PUT3:	/* ??? */
			putval(out, 3, din_unsigned(df, 3));
			break;
		case DVI_PUT4:	/* ??? */
			putval(out, 4, din_unsigned(df, 4));
			break;
		case DVI_PUT_RULE:	/* typeset a rule */
			putval(out, 4, din_unsigned(df, 4));
			putval(out, 4, din_unsigned(df, 4));
			break;
		case DVI_NOP:	/* no operation */
			break;
		case DVI_BOP:	/* beginning of page */
			par = din_unsigned(df, 4);
			putval(out, 4, par);
			din_trace(df, "%d: beginning of page %d\n",
				df->pos - 1, par);
			putval(out, 4, din_unsigned(df, 4));
			putval(out, 4, din_unsigned(df, 4));
			putval(out, 4, din_unsigned(df, 4));
			putval(out, 4, din_unsigned(df, 4));
			putval(out, 4, din_unsigned(df, 4));
			putval(out, 4, din_unsigned(df, 4));
			putval(out, 4, din_unsigned(df, 4));
			putval(out, 4, din_unsigned(df, 4));
			putval(out, 4, din_unsigned(df, 4));

			putval(out, 4, din_unsigned(df, 4));
			break;
		case DVI_EOP:	/* ending of page */
			din_trace(df, "%d: eop\n", df->pos - 1);
			break;
		case DVI_PUSH:	/* save the current positions */
			din_trace(df, "%d: push\n", df->pos - 1);
			break;
		case DVI_POP:	/* restore previous positions */
			din_trace(df, "%d: pop\n", df->pos - 1);
			break;
		case DVI_RIGHT1:	/* move right */
			putval(out, 1, din_unsigned(df, 1));
			break;
		case DVI_RIGHT2:	/* ??? */
			putval(out, 2, din_unsigned(df, 2));
			break;
		case DVI_RIGHT3:	/* ??? */
			putval(out, 3, din_unsigned(df, 3));
			break;
		case DVI_RIGHT4:	/* ??? */
			putval(out, 4, din_unsigned(df, 4));
			break;
		case DVI_W0:	/* move right by |w| */
			break;
		case DVI_W1:	/* move right and set |w| */
			putval(out, 1, din_unsigned(df, 1));
			break;
		case DVI_W2:	/* ??? */
			putval(out, 2, din_unsigned(df, 2));
			break;
		case DVI_W3:	/* ??? */
			putval(out, 3, din_unsigned(df, 3));
			break;
		case DVI_W4:	/* ??? */
			putval(out, 4, din_unsigned(df, 4));
			break;
		case DVI_X0:	/* move right by |x| */
			break;
		case DVI_X1:	/* move right and set |x| */
			putval(out, 1, din_unsigned(df, 1));
			break;
		case DVI_X2:	/* ??? */
			putval(out, 2, din_unsigned(df, 2));
			break;
		case DVI_X3:	/* ??? */
			putval(out, 3, din_unsigned(df, 3));
			break;
		case DVI_X4:	/* ??? */
			putval(out, 4, din_unsigned(df, 4));
			break;
		case DVI_DOWN1:	/* move down */
			putval(out, 1, din_unsigned(df, 1));
			break;
		case DVI_DOWN2:	/* ??? */
			putval(out, 2, din_unsigned(df, 2));
			break;
		case DVI_DOWN3:	/* ??? */
			putval(out, 3, din_unsigned(df, 3));
			break;
		case DVI_DOWN4:	/* ??? */
			putval(out, 4, din_unsigned(df, 4));
			break;
		case DVI_Y0:	/* move down by |y| */
			break;
		case DVI_Y1:	/* move down and set |y| */
			putval(out, 1, din_unsigned(df, 1));
			break;
		case DVI_Y2:	/* ??? */
			putval(out, 2, din_unsigned(df, 2));
			break;
		case DVI_Y3:	/* ??? */
			putval(out, 3, din_unsigned(df, 3));
			break;
		case DVI_Y4:	/* ??? */
			putval(out, 4, din_unsigned(df, 4));
			break;
		case DVI_Z0:	/* move down by |z| */
			break;
		case DVI_Z1:	/* move down and set |z| */
			putval(out, 1, din_unsigned(df, 1));
			break;
		case DVI_Z2:	/* ??? */
			putval(out, 2, din_unsigned(df, 2));
			break;
		case DVI_Z3:	/* ??? */
			putval(out, 3, din_unsigned(df, 3));
			break;
		case DVI_Z4:	/* ??? */
			putval(out, 4, din_unsigned(df, 4));
			break;
		case DVI_FNT1:	/* set current font */
			putval(out, 1, din_unsigned(df, 1));
			break;
		case DVI_FNT2:	/* Same as FNT1, except that arg is 2 bytes */
			putval(out, 2, din_unsigned(df, 2));
			break;
		case DVI_FNT3:	/* Same as FNT1, except that arg is 3 bytes */
			putval(out, 3, din_unsigned(df, 3));
			break;
		case DVI_FNT4:	/* Same as FNT1, except that arg is 4 bytes */
			putval(out, 4, din_unsigned(df, 4));
			break;
		case DVI_XXX1:	/* extension to \.DVI primitives */
			putval(out, 1, din_unsigned(df, 1));
			break;
		case DVI_XXX2:	/* Like XXX1, but 0<=k<65536 */
			putval(out, 2, din_unsigned(df, 2));
			break;
		case DVI_XXX3:	/* Like XXX1, but 0<=k<@t$2^{24}$@> */
			putval(out, 3, din_unsigned(df, 3));
			break;
		case DVI_XXX4:	/* long extension to \.DVI primitives */
			putval(out, 4, din_unsigned(df, 4));
			break;
		case DVI_FNT_DEF1: /* define the meaning of a font number */
			parse_fntdef(df, out, 1);
			break;
		case DVI_FNT_DEF2: /* ??? */
			parse_fntdef(df, out, 2);
			break;
		case DVI_FNT_DEF3: /* ??? */
			parse_fntdef(df, out, 3);
			break;
		case DVI_FNT_DEF4: /* ??? */
			parse_fntdef(df, out, 4);
			break;
		case DVI_PRE:	/* preamble */
			din_fatal(df, "PRE occures within file.");
			break;
		case DVI_POST:	/* postamble beginning */
			return parse_post(df, out);
		case DVI_POST_POST: /* postamble ending */
			din_fatal(df, "POST_POST without PRE.");
			break;
		default:

			if	(DVI_SETC_000 <= c && c <= DVI_SETC_127)
			{
				;
			}
			else if	(DVI_FONT_00 <= c && c <= DVI_FONT_63)
			{
				;
			}
			else	din_fatal(df, "undefined command %d.", c);

			break;
		}

		c = din_byte(df);
	}

	return 1;
}
