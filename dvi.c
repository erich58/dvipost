/*	process dvi file
	$Copyright (C) 2002 Erich Frühstück
	A-3423 St.Andrä/Wördern, Südtirolergasse 17-21/5
*/

#include "ltxpost.h"
#include "dvi.h"

#define	RESOLUTION	300.

#define	IS_SETC(x) (DVI_SETC_000 <= (x)->type && (x)->type <= DVI_SETC_127)
#define	IS_FONT(x) (DVI_FONT_00 <= (x)->type && (x)->type <= DVI_FONT_63)

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
	dvi_unit.tfm_conv = (25400000.0 / dvi_unit.num);
	dvi_unit.tfm_conv *= (dvi_unit.den / 473628672.) / 16.0;
	dvi_unit.true_conv = (dvi_unit.num / 254000.0);
	dvi_unit.true_conv *= (RESOLUTION / dvi_unit.den);
	dvi_unit.conv = dvi_unit.true_conv * (dvi_unit.mag / 1000.);

	if	(verboselevel < STAT)	return;

	fprintf(stderr, "  Resolution = %.8f pixels per inch \n", RESOLUTION);
	fprintf(stderr, "numerator/denominator=%d/%d \n",
		dvi_unit.num, dvi_unit.den);
	fprintf(stderr, "magnification=%d; %16.8f pixels per DVI unit \n",
		dvi_unit.mag, dvi_unit.conv);
	fprintf(stderr, "'%s'\n", token->str);
}

static void setchar (DviFile *df, int pos, int c)
{
	df_trace(df, "%d: setchar%d\n", pos, c);
}

int process_dvi (const char *id, FILE *ifile, FILE *ofile)
{
	DviFile dvifile[2], *df, *out;
	DviToken *token;
	int pos, level;

	df = df_init(dvifile, id, ifile);
	out = df_init(dvifile + 1, "<tmpfile>", ofile);

	pos = df->pos;
	token = din_token(df);
	level = 0;

	while (token != NULL)
	{
		switch (token->type)
		{
		case DVI_BOP:
			df_trace(df, "%d: beginning of page %d\n",
				pos, token->par[0]);
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
			df_trace(df, "%d: eop\n", pos);
			break;
		case DVI_PUSH:
			df_trace(df, "%d: push %d: ", pos, level);
			df_trace(df, "h=%d, v=%d, w=%d, x=%d, y=%d, z=%d\n",
				dvi_stat.h, dvi_stat.v,
				dvi_stat.w, dvi_stat.x,
				dvi_stat.y, dvi_stat.z);
			level++;
			dvi_push();
			break;
		case DVI_POP:
			df_trace(df, "%d: pop\n", pos);
			level--;
			dvi_pop();
			break;
		case DVI_XXX1:
		case DVI_XXX2:
		case DVI_XXX3:
		case DVI_XXX4:
			df_trace(df, "%d: xxx '%s'\n", pos, token->str);
			break;
		case DVI_PRE:
			set_unit(token);
			break;
		case DVI_POST:	/* postamble beginning */
			dout_token(out, token);
			return df->ok ? 0 : 1;
		case DVI_POST_POST: /* postamble ending */
			df_fatal(df, "POST_POST without PRE.");
			break;
		case DVI_SET1:
		case DVI_SET2:
		case DVI_SET3:
		case DVI_SET4:
			setchar(df, pos, token->par[0]);
			break;
		default:
			if	(IS_SETC(token))
			{
				setchar(df, pos, token->type);
			}
			else if	(IS_FONT(token))
			{
				;
			}

			break;
		}

		dout_token(out, token);
		pos = df->pos;
		token = din_token(df);
	}

	return 1;
}
