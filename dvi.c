/*	process dvi file
	$Copyright (C) 2002 Erich Frühstück
	A-3423 St.Andrä/Wördern, Südtirolergasse 17-21/5
*/

#include "ltxpost.h"
#include "dvi.h"

#define	RESOLUTION	300.

static void parse_pre (DviFile *df, DviFile *out)
{
	unsigned n;
	char *desc;

	n = din_byte(df);
	dout_byte(out, n);
	n = din_byte(df);

	if	(n != 2)
	{
		df_fatal(df, "Bad DVI file: id byte not 2.");
		return;
	}

	dout_byte(out, n);

	df->num = din_signed(df, 4);
	df->den = din_signed(df, 4);
	df->mag = din_signed(df, 4);

	df_trace(df, "  Resolution = %.8f pixels per inch \n", RESOLUTION);
	df_trace(df, "numerator/denominator=%d/%d \n", df->num, df->den);
	df->true_conv = (df->num / 254000.0) * (RESOLUTION / df->den);
	df->conv = df->true_conv * (df->mag / 1000.);
	df_trace(df, "magnification=%d; %16.8f pixels per DVI unit \n",
		df->mag, df->conv);

	dout_signed(out, df->num, 4);
	dout_signed(out, df->den, 4);
	dout_signed(out, df->mag, 4);

	n = din_byte(df);
	desc = din_string(df, NULL, n);

	dout_byte(out, n);
	dout_string(out, desc, n);
	df_trace(df, "'%s'\n", desc);
}

static void trace_fdef (DviFile *df, DviFontDef *fdef)
{
	df_trace(df, "Font %d: %s", fdef->font, fdef->name);

	if	(fdef->scale > 0 && fdef->dsize > 0)
	{
		int m = (1000. * df->conv * fdef->scale) /
			(df->true_conv * fdef->dsize) + 0.5;

		if	(m != 1000)
			df_trace(df, " scaled %d", m);
	}

	/* the next output information is a lie */
	df_trace(df, "---loaded at size %d DVI units ", fdef->scale);
	df_trace(df, "\n");
}

static int parse_post(DviFile *df, DviFile *out)
{
	int c, n;
	unsigned start;
	DviFontDef *fdef;

	start = df->pos - 1;
	df_trace(df, "Postamble starts at byte %d.\n", start);
	dout_unsigned(out, din_unsigned(df, 4), 4);
	dout_unsigned(out, din_unsigned(df, 4), 4);
	dout_unsigned(out, din_unsigned(df, 4), 4);
	dout_unsigned(out, din_unsigned(df, 4), 4);
	dout_unsigned(out, din_unsigned(df, 4), 4);
	dout_unsigned(out, din_unsigned(df, 4), 4);
	dout_unsigned(out, din_unsigned(df, 2), 2);
	dout_unsigned(out, din_unsigned(df, 2), 2);

	c = din_byte(df);

	while (df->ok)
	{
		switch (c)
		{
		case DVI_NOP:	/* no operation */
			dout_byte(out, c);
			break;
		case DVI_FNT_DEF1: /* define the meaning of a font number */
		case DVI_FNT_DEF2: /* ??? */
		case DVI_FNT_DEF3: /* ??? */
		case DVI_FNT_DEF4: /* ??? */
			fdef = din_fontdef(df, c);

			if	(fdef)
			{
				df->trace = 1;
				trace_fdef(df, fdef);
				df->trace = 0;
			}

			break;
		case DVI_POST_POST:	/* postamble beginning */
			dout_fonttab(out);
			n = df->pos - 1;

			if	(din_unsigned(df, 4) != start)
				df_fatal(df, "Incorrect start of Postamble.");

			if	(din_byte(df) != 2)
				df_fatal(df, "Bad Postamble: id byte not 2.");

			dout_byte(out, c);
			dout_unsigned(out, start, 4);
			dout_byte(out, 2);
			n = 8 - (n + 6) % 4;

			if	(n == 8)	n = 4;

			while (n-- > 0)
				dout_byte(out, 223);

			return out->ok ? 0 : 1;
		default:
			df_fatal(df, "Command %d not allowed in postamble.",
				c);
			break;
		}

		c = din_byte(df);
	}

	return 1;
}


int process_dvi (const char *id, FILE *ifile, FILE *ofile)
{
	DviFile dvifile[2], *df, *out;
	DviExtension *ext;
	DviFontDef *fdef;
	int par;
	int pos;
	int c;

	df = df_init(dvifile, id, ifile);
	out = df_init(dvifile + 1, "<tmpfile>", ofile);
	parse_pre(df, out);
	pos = df->pos;
	c = din_byte(df);

	while (df->ok)
	{
		switch (c)
		{
		case DVI_SET1:	/* typeset a character and move right */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 1), 1);
			break;
		case DVI_SET2:	/* ??? */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 2), 2);
			break;
		case DVI_SET3:	/* ??? */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 3), 3);
			break;
		case DVI_SET4:	/* ??? */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 4), 4);
			break;
		case DVI_SET_RULE:	/* typeset a rule and move right */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 4), 4);
			dout_unsigned(out, din_unsigned(df, 4), 4);
			break;
		case DVI_PUT1:	/* typeset a character */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 1), 1);
			break;
		case DVI_PUT2:	/* ??? */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 2), 2);
			break;
		case DVI_PUT3:	/* ??? */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 3), 3);
			break;
		case DVI_PUT4:	/* ??? */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 4), 4);
			break;
		case DVI_PUT_RULE:	/* typeset a rule */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 4), 4);
			dout_unsigned(out, din_unsigned(df, 4), 4);
			break;
		case DVI_NOP:	/* no operation */
			dout_byte(out, c);
			break;
		case DVI_BOP:	/* beginning of page */
			dout_byte(out, c);
			par = din_unsigned(df, 4);
			dout_unsigned(out, par, 4);
			df_trace(df, "%d: beginning of page %d\n", pos, par);
			dout_unsigned(out, din_unsigned(df, 4), 4);
			dout_unsigned(out, din_unsigned(df, 4), 4);
			dout_unsigned(out, din_unsigned(df, 4), 4);
			dout_unsigned(out, din_unsigned(df, 4), 4);
			dout_unsigned(out, din_unsigned(df, 4), 4);
			dout_unsigned(out, din_unsigned(df, 4), 4);
			dout_unsigned(out, din_unsigned(df, 4), 4);
			dout_unsigned(out, din_unsigned(df, 4), 4);
			dout_unsigned(out, din_unsigned(df, 4), 4);

			par = din_unsigned(df, 4);
			df_trace(df, "%d: %d\n", df->pos - 4, par);
			dout_unsigned(out, par, 4);
			/*
			dout_unsigned(out, din_unsigned(df, 4), 4);
			*/
			break;
		case DVI_EOP:	/* ending of page */
			dout_byte(out, c);
			df_trace(df, "%d: eop\n", pos);
			break;
		case DVI_PUSH:	/* save the current positions */
			dout_byte(out, c);
			df_trace(df, "%d: push\n", pos);
			break;
		case DVI_POP:	/* restore previous positions */
			dout_byte(out, c);
			df_trace(df, "%d: pop\n", pos);
			break;
		case DVI_RIGHT1:	/* move right */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 1), 1);
			break;
		case DVI_RIGHT2:	/* ??? */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 2), 2);
			break;
		case DVI_RIGHT3:	/* ??? */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 3), 3);
			break;
		case DVI_RIGHT4:	/* ??? */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 4), 4);
			break;
		case DVI_W0:	/* move right by |w| */
			dout_byte(out, c);
			break;
		case DVI_W1:	/* move right and set |w| */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 1), 1);
			break;
		case DVI_W2:	/* ??? */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 2), 2);
			break;
		case DVI_W3:	/* ??? */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 3), 3);
			break;
		case DVI_W4:	/* ??? */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 4), 4);
			break;
		case DVI_X0:	/* move right by |x| */
			dout_byte(out, c);
			break;
		case DVI_X1:	/* move right and set |x| */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 1), 1);
			break;
		case DVI_X2:	/* ??? */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 2), 2);
			break;
		case DVI_X3:	/* ??? */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 3), 3);
			break;
		case DVI_X4:	/* ??? */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 4), 4);
			break;
		case DVI_DOWN1:	/* move down */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 1), 1);
			break;
		case DVI_DOWN2:	/* ??? */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 2), 2);
			break;
		case DVI_DOWN3:	/* ??? */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 3), 3);
			break;
		case DVI_DOWN4:	/* ??? */
			dout_byte(out, c);
			df_trace(df, "%d: down4\n", pos);
			dout_unsigned(out, din_unsigned(df, 4), 4);
			break;
		case DVI_Y0:	/* move down by |y| */
			dout_byte(out, c);
			break;
		case DVI_Y1:	/* move down and set |y| */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 1), 1);
			break;
		case DVI_Y2:	/* ??? */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 2), 2);
			break;
		case DVI_Y3:	/* ??? */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 3), 3);
			break;
		case DVI_Y4:	/* ??? */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 4), 4);
			break;
		case DVI_Z0:	/* move down by |z| */
			dout_byte(out, c);
			break;
		case DVI_Z1:	/* move down and set |z| */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 1), 1);
			break;
		case DVI_Z2:	/* ??? */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 2), 2);
			break;
		case DVI_Z3:	/* ??? */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 3), 3);
			break;
		case DVI_Z4:	/* ??? */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 4), 4);
			break;
		case DVI_FNT1:	/* set current font */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 1), 1);
			break;
		case DVI_FNT2:	/* Same as FNT1, except that arg is 2 bytes */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 2), 2);
			break;
		case DVI_FNT3:	/* Same as FNT1, except that arg is 3 bytes */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 3), 3);
			break;
		case DVI_FNT4:	/* Same as FNT1, except that arg is 4 bytes */
			dout_byte(out, c);
			dout_unsigned(out, din_unsigned(df, 4), 4);
			break;
		case DVI_XXX1:	/* extension to .DVI primitives */
		case DVI_XXX2:	/* Like XXX1, but 0<=k<65536 */
		case DVI_XXX3:	/* Like XXX1, but 0<=k<@t$2^{24}$@> */
		case DVI_XXX4:	/* long extension to .DVI primitives */
			ext = din_extension(df, NULL, c);
			dout_extension(out, ext);
			df_trace(df, "%d: xxx '%s'\n", pos, ext->buf);
			break;
		case DVI_FNT_DEF1: /* define the meaning of a font number */
		case DVI_FNT_DEF2: /* ??? */
		case DVI_FNT_DEF3: /* ??? */
		case DVI_FNT_DEF4: /* ??? */
			fdef = din_fontdef(df, c);

			if	(fdef)
			{
				trace_fdef(df, fdef);
				dout_fontdef(out, fdef);
			}

			break;
		case DVI_PRE:	/* preamble */
			df_fatal(df, "PRE occures within file.");
			break;
		case DVI_POST:	/* postamble beginning */
			dout_byte(out, c);
			return parse_post(df, out);
		case DVI_POST_POST: /* postamble ending */
			df_fatal(df, "POST_POST without PRE.");
			break;
		default:

			if	(DVI_SETC_000 <= c && c <= DVI_SETC_127)
			{
				dout_byte(out, c);
			}
			else if	(DVI_FONT_00 <= c && c <= DVI_FONT_63)
			{
				dout_byte(out, c);
			}
			else	df_fatal(df, "undefined command %d.", c);

			break;
		}

		pos = df->pos;
		c = din_byte(df);
	}

	return 1;
}
