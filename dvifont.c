/*	Dvi font definations
	$Copyright (C) 2002 Erich Frühstück
	A-3423 St.Andrä/Wördern, Südtirolergasse 17-21/5
*/

#include "ltxpost.h"
#include "dvi.h"

#define	BSIZE	8

#define	E_ALLOC	"Font %d: No space to load font defination."

DviFont *DviFontTab = NULL;
int DviFontDim = 0;
static int size = 0;

static int cmp_font (const void *p1, const void *p2)
{
	const DviFont *f1 = p1;
	const DviFont *f2 = p2;

	return (f2->token.par[0] - f1->token.par[0]);
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

	message(NOTE, "Font %d: %s", token->par[0], token->str);
	fp = DviFont_get (token->par[0]);

	if	(fp)
	{
		message(NOTE, "---this font was already defined!\n");
		return;
	}

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
	
#if	0
	if	(fdef->scale > 0 && fdef->dsize > 0)
	{
		int m = (1000. * df->conv * fdef->scale) /
			(df->true_conv * fdef->dsize) + 0.5;

		if	(m != 1000)
			df_trace(df, " scaled %d", m);
	}

	/* the next output information is a lie */
	message(STAT, "---loaded at size %d DVI units ", fdef->scale);
	df_trace(df, "\n");
#endif
	message(STAT, "\n");
	qsort(DviFontTab, DviFontDim, sizeof(DviFont), cmp_font);
}
