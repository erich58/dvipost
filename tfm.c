/*	tfm data
	$Copyright (C) 2002 Erich Frühstück
	A-3423 St.Andrä/Wördern, Südtirolergasse 17-21/5
*/

#include "ltxpost.h"
#include "tfm.h"
#include <ctype.h>

double tfm_conv = 0.;
static char tfm_buf[1024];
static int tfm_err = 0;
static int tfm_pos = 0;

static unsigned tfm_byte (FILE *file)
{
	int c;
	
	if	(tfm_err)	return 0;

	if	((c = getc(file)) != EOF)
	{
		tfm_pos++;
		return c;
	}

	fprintf(stderr, "%s:%d: Unexpected end of file.\n", tfm_buf, tfm_pos);
	tfm_err = 1;
	return 0;
}

static int tfm_dim (FILE *file)
{
	unsigned n = tfm_byte(file) << 8;
	return n + tfm_byte(file);
}

static int tfm_word (FILE *file)
{
	int val = tfm_byte(file);

	if	(val >= 128)	val -= 256;

	val = val * 256 + tfm_byte(file);
	val = val * 256 + tfm_byte(file);
	val = val * 256 + tfm_byte(file);
	return val;
}

static void tfm_skip (FILE *file, int n)
{
	while (n-- > 0)
		tfm_byte(file);
}

static int tfm2dvi (int val)
{
	if	(val < 0)
	{
		return -(tfm_conv * -val + 0.5);
	}
	else	return tfm_conv * val + 0.5;
}

#if	0
static double tfm2pt (int val)
{
	return val / (double) 0x100000;
}
#endif

int tfm_load (TFM *tfm, const char *name)
{
	FILE *file;
	unsigned width[256];
	int lh, bc, ec, nw;
	int i, n;

	sprintf(tfm_buf, "kpsewhich %s.tfm\n", name);
	file = popen(tfm_buf, "r");

	if	(!file)
	{
		perror(tfm_buf);
		return -1;
	}

	fgets(tfm_buf, sizeof(tfm_buf), file);
	pclose(file);

	for (n = 0; n < sizeof(tfm_buf); n++)
	{
		if	(tfm_buf[n] == '\n')
		{
			tfm_buf[n] = 0;
			break;
		}
	}

	file = fopen(tfm_buf, "rb");

	if	(!file)
	{
		perror(tfm_buf);
		return -1;
	}

	tfm_pos = 0;
	tfm_err = 0;
	(void) tfm_dim(file);	/* word length of file */
	lh = tfm_dim(file);	/* words of header data */
	bc = tfm_dim(file);	/* smallest character code */
	ec = tfm_dim(file);	/* largest character code */
	nw = tfm_dim(file);	/* words in width table */
	(void) tfm_dim(file);	/* words in height table */
	(void) tfm_dim(file);	/* words in depth table */
	(void) tfm_dim(file);	/* words in italic correction table */
	(void) tfm_dim(file);	/* words in lig/kern table */
	(void) tfm_dim(file);	/* words in kern table */
	(void) tfm_dim(file);	/* words in extensible char table */
	(void) tfm_dim(file);	/* words of font parameter data */

	if	(lh > 2)
	{
		(void) tfm_word(file);
		tfm->dsize = tfm2dvi(tfm_word(file));
		tfm_skip(file, 4 * lh - 8);
	}
	else	tfm_skip(file, 4 * lh);

	for (i = 0; i < 256; i++)
		tfm->width[i] = 0;

	for (i = bc; i <= ec; i++)
	{
		tfm->width[i] = tfm_byte(file);
		tfm_skip(file, 3);
	}

	for (i = 0; i < nw; i++)
		width[i] = tfm2dvi(tfm_word(file));
	
	for (i = bc; i <= ec; i++)
		tfm->width[i] = width[tfm->width[i]];

	fclose(file);
	return tfm_err;
}
