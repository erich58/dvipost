/*	Dvi font definations
	$Copyright (C) 2002 Erich Frühstück
	A-3423 St.Andrä/Wördern, Südtirolergasse 17-21/5
*/

#include "ltxpost.h"
#include "dvi.h"

#define	BSIZE	8

#define	E_DIFF	"Font %d: Allready defined with different values."
#define	E_ALLOC	"Font %d: No space to load font defination."

static DviFontDef *font_tab = NULL;
static size_t font_dim = 0;
static size_t tab_size = 0;

static int check_font (DviFile *df, DviFontDef *fdef)
{
	int n;

	if	(fdef->csum != din_unsigned(df, 4))	return 1;
	if	(fdef->scale != din_signed(df, 4))	return 1;
	if	(fdef->dsize != din_signed(df, 4))	return 1;
	if	(fdef->dlen != din_byte(df))		return 1;
	if	(fdef->nlen != din_byte(df))		return 1;

	for (n = 0; n < fdef->dlen + fdef->nlen; n++)
		if ((unsigned char) fdef->name[n] != din_byte(df))
			return 1;

	return 0;
}

DviFontDef *din_fontdef (DviFile *df, int type)
{
	DviFontDef *fdef;
	int font;
	int n;

	switch (type)
	{
	case DVI_FNT_DEF1:	font = din_unsigned(df, 1); break;
	case DVI_FNT_DEF2:	font = din_unsigned(df, 2); break;
	case DVI_FNT_DEF3:	font = din_unsigned(df, 3); break;
	case DVI_FNT_DEF4:	font = din_unsigned(df, 4); break;
	default:		font = 0; break;
	}

	fdef = NULL;

	for (n = 0; n < font_dim; n++)
	{
		if	(font_tab[n].font == font)
		{
			if	(check_font(df, font_tab + n))
			{
				df_fatal(df, E_DIFF, font);
				return NULL;
			}

			return font_tab + n;
		}
	}

	if	(font_dim >= tab_size)
	{
		tab_size += BSIZE;
		font_tab = realloc(font_tab, tab_size * sizeof(DviFontDef));

		if	(font_tab == NULL)
		{
			df_fatal(df, E_ALLOC, font);
			return NULL;
		}
	}

	fdef = font_tab + font_dim++;
	fdef->type = type;
	fdef->font = font;
	fdef->csum = din_unsigned(df, 4);
	fdef->scale = din_signed(df, 4);
	fdef->dsize = din_signed(df, 4);
	fdef->dlen = din_byte(df);
	fdef->nlen = din_byte(df);
	fdef->name = malloc(fdef->dlen + fdef->nlen + 1);

	if	(fdef->name == NULL)
	{
		df_fatal(df, E_ALLOC, font);
		return NULL;
	}

	din_string(df, fdef->name, fdef->dlen + fdef->nlen);
	return fdef;
}

void dout_fontdef (DviFile *df, DviFontDef *fdef)
{
	if	(!fdef)	return;

	dout_byte(df, fdef->type);

	switch (fdef->type)
	{
	case DVI_FNT_DEF1:	dout_unsigned(df, fdef->font, 1); break;
	case DVI_FNT_DEF2:	dout_unsigned(df, fdef->font, 2); break;
	case DVI_FNT_DEF3:	dout_unsigned(df, fdef->font, 3); break;
	case DVI_FNT_DEF4:	dout_unsigned(df, fdef->font, 4); break;
	default:		df->ok = 0; break;
	}

	dout_unsigned(df, fdef->csum, 4);
	dout_signed(df, fdef->scale, 4);
	dout_signed(df, fdef->dsize, 4);
	dout_byte(df, fdef->dlen);
	dout_byte(df, fdef->nlen);
	dout_string(df, fdef->name, fdef->dlen + fdef->nlen);
}

static int cmp_font (const void *p1, const void *p2)
{
	const DviFontDef *f1 = p1;
	const DviFontDef *f2 = p2;

	return (f2->font - f1->font);
}

extern void dout_fonttab (DviFile *df)
{
	int n;

	if	(font_dim > 1)
		qsort(font_tab, font_dim, sizeof(DviFontDef), cmp_font);

	for (n = 0; n < font_dim; n++)
		dout_fontdef(df, font_tab + n);
}
