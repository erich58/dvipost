/*	Dvi extensions
	$Copyright (C) 2002 Erich Frühstück
	A-3423 St.Andrä/Wördern, Südtirolergasse 17-21/5
*/

#include "ltxpost.h"
#include "dvi.h"

#define	BSIZE	256

static DviExtension ext_buf = { 0, 0, 0, NULL };

DviExtension *din_extension (DviFile *df, DviExtension *buf, int type)
{
	if	(!buf)	buf = &ext_buf;

	buf->type = type;

	switch (type)
	{
	case DVI_XXX1:	buf->len = din_unsigned(df, 1); break;
	case DVI_XXX2:	buf->len = din_unsigned(df, 2); break;
	case DVI_XXX3:	buf->len = din_unsigned(df, 3); break;
	case DVI_XXX4:	buf->len = din_unsigned(df, 4); break;
	default:	buf->len = 0; break;
	}

	if	(buf->size <= buf->len)
	{
		if	(buf->size)
			free(buf->buf);

		buf->size = BSIZE * (1 + buf->len / BSIZE);
		buf->buf = malloc(buf->size);

		if	(buf->buf == NULL)
		{
			df_fatal(df, "can't allocate %d byte.", buf->size);
			buf->size = 0;
			buf->len = 0;
		}
	}

	din_string(df, buf->buf, buf->len);

	if	(!df->ok)	buf->len = 0;

	return buf;
}

void dout_extension (DviFile *df, DviExtension *buf)
{
	if	(!buf)	return;

	dout_byte(df, buf->type);

	switch (buf->type)
	{
	case DVI_XXX1:	dout_unsigned(df, buf->len, 1); break;
	case DVI_XXX2:	dout_unsigned(df, buf->len, 2); break;
	case DVI_XXX3:	dout_unsigned(df, buf->len, 3); break;
	case DVI_XXX4:	dout_unsigned(df, buf->len, 4); break;
	default:	df->ok = 0; break;
	}

	dout_string(df, buf->buf, buf->len);
}
