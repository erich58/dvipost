/*	process dvi file
	$Copyright (C) 2002 Erich Frühstück
	A-3423 St.Andrä/Wördern, Südtirolergasse 17-21/5
*/

#include "ltxpost.h"
#include "dvi.h"

static unsigned getval (FILE *file, int nbyte)
{
	unsigned val;
	int c;

	val = 0;

	while (nbyte-- > 0)
	{
		if	((c = getc(file)) != EOF)
			val = (val << 8) | (c & 0xFF);
	}

	return val;
}

static void putval (FILE *file, int len, unsigned val)
{
	if	(--len > 0)
		putval(file, len, val >> 8);

	putc(val, file);
}

static int parse_pre (const char *id, FILE *in, FILE *out)
{
	unsigned n;

	putc(DVI_PRE, out);
	n = getval(in, 1);

	if	(n != 2)
	{
		message(ERR, "$!: %s: Bad DVI file: id byte not 2.\n", id);
		return 1;
	}

	putval(out, 1, n);
	putval(out, 4, getval(in, 4));
	putval(out, 4, getval(in, 4));
	putval(out, 4, getval(in, 4));
	n = getval(in, 1);
	putval(out, 1, n);

	while (n-- > 0)
	{
		int c = getc(in);
		putc(c, out);
	}

	return 0;
}

static int parse_fntdef (const char *id, FILE *in, FILE *out, int n)
{
	int font;
	int a, l;

	font = getval(in, n);
	putval(out, n, font);
	message(STAT, "Font %d:\n", font);
	putval(out, 4, getval(in, 4));
	putval(out, 4, getval(in, 4));
	putval(out, 4, getval(in, 4));
	a = getval(in, 1);
	l = getval(in, 1);
	n = a + l;

	while (n-- > 0)
	{
		int c = getc(in);
		putc(c, out);
	}

	return 0;
}

int process_dvi (const char *id, FILE *in, FILE *out)
{
	int c;
	int stat;

	stat = parse_pre(id, in, out);

	while (stat == 0 && (c = getc(in)) != EOF)
	{
		putc(c, out);

		if	(DVI_SETC_000 <= c && c <= DVI_SETC_127)
		{
			continue;
		}
		else if	(DVI_FONT_00 <= c && c <= DVI_FONT_63)
		{
			continue;
		}

		switch (c)
		{
		case DVI_SET1:	/* typeset a character and move right */
			putval(out, 1, getval(in, 1));
			break;
		case DVI_SET2:	/* ??? */
			putval(out, 2, getval(in, 2));
			break;
		case DVI_SET3:	/* ??? */
			putval(out, 3, getval(in, 3));
			break;
		case DVI_SET4:	/* ??? */
			putval(out, 4, getval(in, 4));
			break;
		case DVI_SET_RULE:	/* typeset a rule and move right */
			putval(out, 4, getval(in, 4));
			putval(out, 4, getval(in, 4));
			break;
		case DVI_PUT1:	/* typeset a character */
			putval(out, 1, getval(in, 1));
			break;
		case DVI_PUT2:	/* ??? */
			putval(out, 2, getval(in, 2));
			break;
		case DVI_PUT3:	/* ??? */
			putval(out, 3, getval(in, 3));
			break;
		case DVI_PUT4:	/* ??? */
			putval(out, 4, getval(in, 4));
			break;
		case DVI_PUT_RULE:	/* typeset a rule */
			putval(out, 4, getval(in, 4));
			putval(out, 4, getval(in, 4));
			break;
		case DVI_NOP:	/* no operation */
			break;
		case DVI_BOP:	/* beginning of page */
			putval(out, 4, getval(in, 4));
			putval(out, 4, getval(in, 4));
			putval(out, 4, getval(in, 4));
			putval(out, 4, getval(in, 4));
			putval(out, 4, getval(in, 4));
			putval(out, 4, getval(in, 4));
			putval(out, 4, getval(in, 4));
			putval(out, 4, getval(in, 4));
			putval(out, 4, getval(in, 4));
			putval(out, 4, getval(in, 4));
			putval(out, 4, getval(in, 4));
			break;
		case DVI_EOP:	/* ending of page */
			break;
		case DVI_PUSH:	/* save the current positions */
			break;
		case DVI_POP:	/* restore previous positions */
			break;
		case DVI_RIGHT1:	/* move right */
			putval(out, 1, getval(in, 1));
			break;
		case DVI_RIGHT2:	/* ??? */
			putval(out, 2, getval(in, 2));
			break;
		case DVI_RIGHT3:	/* ??? */
			putval(out, 3, getval(in, 3));
			break;
		case DVI_RIGHT4:	/* ??? */
			putval(out, 4, getval(in, 4));
			break;
		case DVI_W0:	/* move right by |w| */
			break;
		case DVI_W1:	/* move right and set |w| */
			putval(out, 1, getval(in, 1));
			break;
		case DVI_W2:	/* ??? */
			putval(out, 2, getval(in, 2));
			break;
		case DVI_W3:	/* ??? */
			putval(out, 3, getval(in, 3));
			break;
		case DVI_W4:	/* ??? */
			putval(out, 4, getval(in, 4));
			break;
		case DVI_X0:	/* move right by |x| */
			break;
		case DVI_X1:	/* move right and set |x| */
			putval(out, 1, getval(in, 1));
			break;
		case DVI_X2:	/* ??? */
			putval(out, 2, getval(in, 2));
			break;
		case DVI_X3:	/* ??? */
			putval(out, 3, getval(in, 3));
			break;
		case DVI_X4:	/* ??? */
			putval(out, 4, getval(in, 4));
			break;
		case DVI_DOWN1:	/* move down */
			putval(out, 1, getval(in, 1));
			break;
		case DVI_DOWN2:	/* ??? */
			putval(out, 2, getval(in, 2));
			break;
		case DVI_DOWN3:	/* ??? */
			putval(out, 3, getval(in, 3));
			break;
		case DVI_DOWN4:	/* ??? */
			putval(out, 4, getval(in, 4));
			break;
		case DVI_Y0:	/* move down by |y| */
			break;
		case DVI_Y1:	/* move down and set |y| */
			putval(out, 1, getval(in, 1));
			break;
		case DVI_Y2:	/* ??? */
			putval(out, 2, getval(in, 2));
			break;
		case DVI_Y3:	/* ??? */
			putval(out, 3, getval(in, 3));
			break;
		case DVI_Y4:	/* ??? */
			putval(out, 4, getval(in, 4));
			break;
		case DVI_Z0:	/* move down by |z| */
			break;
		case DVI_Z1:	/* move down and set |z| */
			putval(out, 1, getval(in, 1));
			break;
		case DVI_Z2:	/* ??? */
			putval(out, 2, getval(in, 2));
			break;
		case DVI_Z3:	/* ??? */
			putval(out, 3, getval(in, 3));
			break;
		case DVI_Z4:	/* ??? */
			putval(out, 4, getval(in, 4));
			break;
		case DVI_FNT1:	/* set current font */
			putval(out, 1, getval(in, 1));
			break;
		case DVI_FNT2:	/* Same as FNT1, except that arg is 2 bytes */
			putval(out, 2, getval(in, 2));
			break;
		case DVI_FNT3:	/* Same as FNT1, except that arg is 3 bytes */
			putval(out, 3, getval(in, 3));
			break;
		case DVI_FNT4:	/* Same as FNT1, except that arg is 4 bytes */
			putval(out, 4, getval(in, 4));
			break;
		case DVI_XXX1:	/* extension to \.DVI primitives */
			putval(out, 1, getval(in, 1));
			break;
		case DVI_XXX2:	/* Like XXX1, but 0<=k<65536 */
			putval(out, 2, getval(in, 2));
			break;
		case DVI_XXX3:	/* Like XXX1, but 0<=k<@t$2^{24}$@> */
			putval(out, 3, getval(in, 3));
			break;
		case DVI_XXX4:	/* long extension to \.DVI primitives */
			putval(out, 4, getval(in, 4));
			break;
		case DVI_FNT_DEF1: /* define the meaning of a font number */
			stat = parse_fntdef(id, in, out, 1);
			break;
		case DVI_FNT_DEF2: /* ??? */
			stat = parse_fntdef(id, in, out, 2);
			break;
		case DVI_FNT_DEF3: /* ??? */
			stat = parse_fntdef(id, in, out, 3);
			break;
		case DVI_FNT_DEF4: /* ??? */
			stat = parse_fntdef(id, in, out, 4);
			break;
		case DVI_PRE:	/* preamble */
			message(ERR, "$!: PRE occures within file.\n");
			stat = 1;
			break;
		case DVI_POST:	/* postamble beginning */
			break;
		case DVI_POST_POST: /* postamble ending */
			break;
		default:
			message(ERR, "$!: undefined command %d.\n", c);
			break;
		}
	}

	return 0;
}
