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

	out->num = df->num;
	out->den = df->den;
	out->mag = df->mag;
	dout_signed(out, out->num, 4);
	dout_signed(out, out->den, 4);
	dout_signed(out, out->mag, 4);

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

static int parse_post (DviFile *df, DviFile *out)
{
	int c, n;
	unsigned start;
	DviFontDef *fdef;
	int l, w;

	start = df->pos - 1;

	if	(din_signed(df, 4) != df->last_page)
		df_fatal(df, "page offset error.");

	din_signed(df, 4);
	din_signed(df, 4);
	din_signed(df, 4);
	l = din_signed(df, 4);
	w = din_signed(df, 4);
	din_unsigned(df, 2);
	din_unsigned(df, 2);

	start = out->pos;
	dout_byte(out, DVI_POST);
	dout_signed(out, out->last_page, 4);
	dout_signed(out, out->num, 4);
	dout_signed(out, out->den, 4);
	dout_signed(out, out->mag, 4);
	dout_signed(out, l, 4);
	dout_signed(out, w, 4);
	dout_unsigned(out, out->mdepth, 2);
	dout_unsigned(out, out->npages, 2);

	dout_fonttab(out);

	dout_byte(out, DVI_POST_POST);
	dout_unsigned(out, start, 4);
	dout_byte(out, 2);

	for (n = 0; n < 4; n++)
		dout_byte(out, 223);

	while (out->pos % 4 != 0)
		dout_byte(out, 223);

	return out->ok ? 0 : 1;
}


int process_dvi (const char *id, FILE *ifile, FILE *ofile)
{
	DviFile dvifile[2], *df, *out;
	DviExtension *ext;
	DviFontDef *fdef;
	int page_count[10];
	int pos;
	int c;
	int i;

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
			for (i = 0; i < 10; i++)
				page_count[i] = din_signed(df, 4);

			if	(din_signed(df, 4) != df->last_page)
				df_fatal(df, "page offset error.");

			df->npages++;
			df->last_page = pos;

			df_trace(df, "%d: beginning of page %d\n",
				pos, page_count[0]);

			pos = out->pos;
			dout_byte(out, c);

			for (i = 0; i < 10; i++)
				dout_signed(out, page_count[i], 4);

			dout_signed(out, out->last_page, 4);
			out->last_page = pos;
			out->npages++;
			break;
		case DVI_EOP:	/* ending of page */
			dout_byte(out, c);
			df_trace(df, "%d: eop\n", pos);
			break;
		case DVI_PUSH:	/* save the current positions */
			df_trace(df, "%d: push\n", pos);

			out->depth++;

			if	(out->mdepth < out->depth)
				out->mdepth = out->depth;

			dout_byte(out, c);
			break;
		case DVI_POP:	/* restore previous positions */
			out->depth--;
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
			df_trace(df, "Postamble starts at byte %d.\n", pos);
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
