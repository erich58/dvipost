/*	reading tools for dvi-files
*/

#include "ltxpost.h"
#include "dvi.h"
#include <stdarg.h>


int din_byte (DviFile *df)
{
	int c = getc(df->file);

	if	(c == EOF)
	{
		df_fatal(df, "Unexpected end of file.");
		return 0;
	}

	df->pos++;
	return c;
}


unsigned din_unsigned (DviFile *df, unsigned len)
{
	unsigned val = din_byte(df);

	while (df->ok && --len > 0)
		val = val * 256 + din_byte(df);

	return val;
}


int din_signed (DviFile *df, unsigned len)
{
	int val = din_byte(df);

	if	(val >= 128)
		val -= 256;

	while (df->ok && --len > 0)
		val = val * 256 + din_byte(df);

	return val;
}


char *din_string (DviFile *df, unsigned len)
{
	char *tg;

	tg = xalloc(len + 1);

	if	(fread(tg, 1, len, df->file) != len)
	{
		df_fatal(df, "Input error.");
		xfree(tg);
		return NULL;
	}

	df->pos += len;
	tg[len] = 0;
	return tg;
}

DviToken *din_token (DviFile *df)
{
	static DviToken token;
	int pos;
	int i;

	if	(!df->ok)	return NULL;

	pos = df->pos;
	token.type = din_byte(df);
			
	if	(DVI_SETC_000 <= token.type && token.type <= DVI_SETC_127)
	{
		token.par[0] = token.type;
		return &token;
	}

	if	(DVI_FONT_00 <= token.type && token.type <= DVI_FONT_63)
	{
		token.par[0] = token.type;
		return &token;
	}
	
	switch (token.type)
	{
	case DVI_NOP:
	case DVI_EOP:
	case DVI_PUSH:
	case DVI_POP:
	case DVI_W0:
	case DVI_X0:
	case DVI_Y0:
	case DVI_Z0:
		break;
	case DVI_SET1:
	case DVI_PUT1:
	case DVI_FNT1:
		token.par[0] = din_unsigned(df, 1);
		break;
	case DVI_SET2:
	case DVI_PUT2:
	case DVI_FNT2:
		token.par[0] = din_unsigned(df, 2);
		break;
	case DVI_SET3:
	case DVI_PUT3:
	case DVI_FNT3:
		token.par[0] = din_unsigned(df, 3);
		break;
	case DVI_SET4:
	case DVI_PUT4:
	case DVI_FNT4:
		token.par[0] = din_signed(df, 4);
		break;
	case DVI_SET_RULE:
	case DVI_PUT_RULE:
		token.par[0] = din_signed(df, 4);
		token.par[1] = din_signed(df, 4);
		break;
	case DVI_BOP: /* beginning of page */
		for (i = 0; i < 10; i++)
			token.par[i] = din_signed(df, 4);

		if	(din_signed(df, 4) != df->last_page)
			df_fatal(df, "page offset error.");

		df->npages++;
		df->last_page = pos;
		break;
	case DVI_RIGHT1:
	case DVI_DOWN1:
	case DVI_W1:
	case DVI_X1:
	case DVI_Y1:
	case DVI_Z1:
		token.par[0] = din_signed(df, 1);
		break;
	case DVI_RIGHT2:
	case DVI_DOWN2:
	case DVI_W2:
	case DVI_X2:
	case DVI_Y2:
	case DVI_Z2:
		token.par[0] = din_signed(df, 2);
		break;
	case DVI_RIGHT3:
	case DVI_DOWN3:
	case DVI_W3:
	case DVI_X3:
	case DVI_Y3:
	case DVI_Z3:
		token.par[0] = din_signed(df, 3);
		break;
	case DVI_RIGHT4:
	case DVI_DOWN4:
	case DVI_W4:
	case DVI_X4:
	case DVI_Y4:
	case DVI_Z4:
		token.par[0] = din_signed(df, 4);
		break;
	case DVI_XXX1:
	case DVI_XXX2:
	case DVI_XXX3:
	case DVI_XXX4:
		token.par[0] = din_unsigned(df, 1 + token.type - DVI_XXX1);
		token.str = din_string(df, token.par[0]);
		break;
	case DVI_FNT_DEF1:
	case DVI_FNT_DEF2:
	case DVI_FNT_DEF3:
	case DVI_FNT_DEF4:
		token.par[0] = din_unsigned(df, 1 + token.type - DVI_FNT_DEF1);
		token.par[1] = din_unsigned(df, 4);
		token.par[2] = din_signed(df, 4);
		token.par[3] = din_signed(df, 4);
		token.par[4] = din_byte(df);
		token.par[5] = din_byte(df);
		token.str = din_string(df, token.par[4] + token.par[5]);
		DviFont_add(&token);
		break;
	case DVI_PRE:

		if	(din_byte(df) != 2)
			df_fatal(df, "Bad DVI file: id byte not 2.");

		df->num = token.par[0] = din_signed(df, 4);
		df->den = token.par[1] = din_signed(df, 4);
		df->mag = token.par[2] = din_signed(df, 4);
		token.par[3] = din_byte(df);
		token.str = din_string(df, token.par[3]);
		break;
	case DVI_POST:
		if	(din_signed(df, 4) != df->last_page)
			df_fatal(df, "page offset error.");

		din_signed(df, 4);
		din_signed(df, 4);
		din_signed(df, 4);
		token.par[0] = din_signed(df, 4);
		token.par[1] = din_signed(df, 4);
		din_unsigned(df, 2);
		din_unsigned(df, 2);
		break;
	case DVI_POST_POST:
		token.par[0] = din_unsigned(df, 4);

		if	(din_byte(df) != 2)
			df_fatal(df, "Bad DVI file: id byte not 2.");

		break;
	default:
		df_fatal(df, "undefined command %d.", token.type);
		return NULL;
	}

	return df->ok ? &token : NULL;
}
