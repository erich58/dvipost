/*	process dvi file
	$Copyright (C) 2002 Erich Frühstück
	A-3423 St.Andrä/Wördern, Südtirolergasse 17-21/5
*/

#include "ltxpost.h"
#include "dvi.h"

#define	RESOLUTION	300.

static void trace_pre (DviFile *df, DviToken *token)
{
	df_trace(df, "  Resolution = %.8f pixels per inch \n", RESOLUTION);
	df_trace(df, "numerator/denominator=%d/%d \n", df->num, df->den);
	df->true_conv = (df->num / 254000.0) * (RESOLUTION / df->den);
	df->conv = df->true_conv * (df->mag / 1000.);
	df_trace(df, "magnification=%d; %16.8f pixels per DVI unit \n",
		df->mag, df->conv);
	df_trace(df, "'%s'\n", token->str);
}

int process_dvi (const char *id, FILE *ifile, FILE *ofile)
{
	DviFile dvifile[2], *df, *out;
	DviToken *token;
	int pos;

	df = df_init(dvifile, id, ifile);
	out = df_init(dvifile + 1, "<tmpfile>", ofile);

	pos = df->pos;
	token = din_token(df);

	while (token != NULL)
	{
		switch (token->type)
		{
		case DVI_BOP:
			df_trace(df, "%d: beginning of page %d\n",
				pos, token->par[0]);
			break;
		case DVI_EOP:
			df_trace(df, "%d: eop\n", pos);
			break;
		case DVI_PUSH:
			df_trace(df, "%d: push\n", pos);
			break;
		case DVI_POP:
			df_trace(df, "%d: pop\n", pos);
			break;
		case DVI_XXX1:
		case DVI_XXX2:
		case DVI_XXX3:
		case DVI_XXX4:
			df_trace(df, "%d: xxx '%s'\n", pos, token->str);
			break;
		case DVI_PRE:
			trace_pre(df, token);
			break;
		case DVI_POST:	/* postamble beginning */
			dout_token(out, token);
			return df->ok ? 0 : 1;
		case DVI_POST_POST: /* postamble ending */
			df_fatal(df, "POST_POST without PRE.");
			break;
		default:
			break;
		}

		dout_token(out, token);
		pos = df->pos;
		token = din_token(df);
	}

	return 1;
}
