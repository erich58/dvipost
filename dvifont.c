/*	Dvi font definations
	$Copyright (C) 2002 Erich Frühstück
	A-3423 St.Andrä/Wördern, Südtirolergasse 17-21/5
*/

#include "ltxpost.h"
#include "dvi.h"

#define	BSIZE	8

#define	FONT(x)		(x)->token.par[0]
#define	CHECKSUM(x)	(x)->token.par[1]
#define	SCALE(x)	(x)->token.par[2]
#define	DSIZE(x)	(x)->token.par[3]

DviFont *DviFontTab = NULL;
int DviFontDim = 0;
static int size = 0;

static int cmp_font (const void *p1, const void *p2)
{
	const DviFont *f1 = p1;
	const DviFont *f2 = p2;

	return (FONT(f2) - FONT(f1));
}

DviFont *DviFont_get (int font)
{
	DviFont *fp;
	int n;

	for (fp = DviFontTab, n = DviFontDim; n-- > 0; fp++)
		if (fp->token.par[0] == font) return fp;

	return NULL;
}


void DviFont_add (DviToken *token)
{
	DviFont *fp;

	fp = DviFont_get (token->par[0]);

	if	(fp)
		return;

	if	(DviFontDim >= size)
	{
		fp = DviFontTab;
		size += BSIZE;
		DviFontTab = xalloc(size * sizeof(DviFont));
		memcpy(DviFontTab, fp, DviFontDim * sizeof(DviFont));
		xfree(fp);
	}

	fp = DviFontTab + DviFontDim++;
	fp->token = *token;

	message(STAT, "Font %d: %s", token->par[0], token->str);

	if	(SCALE(fp) > 0 && DSIZE(fp) > 0)
	{
		int m = (1000. * dvi_unit.conv * SCALE(fp)) /
			(dvi_unit.true_conv * DSIZE(fp)) + 0.5;

		if	(m != 1000)
			message(STAT, " scaled %d", m);
	}

	message(STAT, "---loaded at size %d DVI units \n", SCALE(fp));

	tfm_load(fp->width, token->str, SCALE(fp));
	qsort(DviFontTab, DviFontDim, sizeof(DviFont), cmp_font);
}
