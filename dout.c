/*	writing tools for dvi-files
*/

#include "ltxpost.h"
#include "dvi.h"
#include <stdarg.h>


void dout_byte (DviFile *df, int c)
{
	if	(!df->ok)
	{
		;
	}
	else if	(putc(c, df->file) == EOF)
	{
		df_fatal(df, "Output error.");
	}
	else	df->pos++;
}

void dout_unsigned (DviFile *df, unsigned val, unsigned len)
{
	if	(--len > 0)
		dout_unsigned(df, val >> 8, len);

	dout_byte(df, val & 0xFF);
}


void dout_signed (DviFile *df, int val, unsigned len)
{
	if	(val < 0)
	{
		unsigned x = 1 + (unsigned) - (val + 1);
		dout_unsigned(df, 1 + ~x, len);
	}
	else	dout_unsigned(df, val, len);
}


void dout_string (DviFile *df, char *buf, unsigned len)
{
	while (len-- > 0)
		dout_byte(df, *(buf++));
}


void dout_token (DviFile *df, DviToken *tok)
{
	int pos;
	int i;

	if	(!df->ok)	return;

	pos = df->pos;
	dout_byte(df, tok->type);
			
	switch (tok->type)
	{
	case DVI_NOP:
	case DVI_EOP:
	case DVI_W0:
	case DVI_X0:
	case DVI_Y0:
	case DVI_Z0:
		break;
	case DVI_PUSH:
		df->depth++;

		if	(df->mdepth < df->depth)
			df->mdepth = df->depth;

		break;
	case DVI_POP:
		df->depth--;
		break;
	case DVI_SET1:
	case DVI_PUT1:
	case DVI_FNT1:
		dout_unsigned(df, tok->par[0], 1);
		break;
	case DVI_SET2:
	case DVI_PUT2:
	case DVI_FNT2:
		dout_unsigned(df, tok->par[0], 2);
		break;
	case DVI_SET3:
	case DVI_PUT3:
	case DVI_FNT3:
		dout_unsigned(df, tok->par[0], 3);
		break;
	case DVI_SET4:
	case DVI_PUT4:
	case DVI_FNT4:
		dout_signed(df, tok->par[0], 4);
		break;
	case DVI_SET_RULE:
	case DVI_PUT_RULE:
		dout_signed(df, tok->par[0], 4);
		dout_signed(df, tok->par[1], 4);
		break;
	case DVI_BOP: /* beginning of page */
		for (i = 0; i < 10; i++)
			dout_signed(df, tok->par[i], 4);

		dout_signed(df, df->last_page, 4);
		df->last_page = pos;
		df->npages++;
		break;
	case DVI_RIGHT1:
	case DVI_DOWN1:
	case DVI_W1:
	case DVI_X1:
	case DVI_Y1:
	case DVI_Z1:
		dout_signed(df, tok->par[0], 1);
		break;
	case DVI_RIGHT2:
	case DVI_DOWN2:
	case DVI_W2:
	case DVI_X2:
	case DVI_Y2:
	case DVI_Z2:
		dout_signed(df, tok->par[0], 2);
		break;
	case DVI_RIGHT3:
	case DVI_DOWN3:
	case DVI_W3:
	case DVI_X3:
	case DVI_Y3:
	case DVI_Z3:
		dout_signed(df, tok->par[0], 3);
		break;
	case DVI_RIGHT4:
	case DVI_DOWN4:
	case DVI_W4:
	case DVI_X4:
	case DVI_Y4:
	case DVI_Z4:
		dout_signed(df, tok->par[0], 4);
		break;
	case DVI_XXX1:
	case DVI_XXX2:
	case DVI_XXX3:
	case DVI_XXX4:
		dout_unsigned(df, tok->par[0], 1 + tok->type - DVI_XXX1);
		dout_string(df, tok->str, tok->par[0]);
		break;
	case DVI_FNT_DEF1:
	case DVI_FNT_DEF2:
	case DVI_FNT_DEF3:
	case DVI_FNT_DEF4:
		dout_unsigned(df, tok->par[0], 1 + tok->type - DVI_FNT_DEF1);
		dout_signed(df, tok->par[1], 4);
		dout_signed(df, tok->par[2], 4);
		dout_signed(df, tok->par[3], 4);
		dout_byte(df, tok->par[4]);
		dout_byte(df, tok->par[5]);
		dout_string(df, tok->str, tok->par[4] + tok->par[5]);
		break;
	case DVI_PRE:
		dout_byte(df, 2);
		dout_signed(df, tok->par[0], 4);
		dout_signed(df, tok->par[1], 4);
		dout_signed(df, tok->par[2], 4);
		dout_byte(df, tok->par[3]);
		dout_string(df, tok->str, tok->par[3]);
		break;
	case DVI_POST:
		dout_signed(df, df->last_page, 4);
		dout_signed(df, tok->par[0], 4);
		dout_signed(df, tok->par[1], 4);
		dout_signed(df, tok->par[2], 4);
		dout_signed(df, tok->par[3], 4);
		dout_signed(df, tok->par[4], 4);
		dout_unsigned(df, df->mdepth, 2);
		dout_unsigned(df, df->npages, 2);

		for (i = 0; i < DviFontDim; i++)
			dout_token(df, &DviFontTab[i].token);

		dout_byte(df, DVI_POST_POST);
		dout_unsigned(df, pos, 4);
		dout_byte(df, 2);

		for (i = 0; i < 4; i++)
			dout_byte(df, 223);

		while (df->ok && df->pos % 4 != 0)
			dout_byte(df, 223);

		break;
	default:
		break;
	}
}
