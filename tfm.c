/*	tfm data

$Copyright (C) 2002 Erich Fruehstueck

Dvipost is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

Dvipost is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public
License along with dvipost; see the file COPYING.
If not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
*/

#include "dvipost.h"
#include "dvi.h"
#include <ctype.h>

static char tfm_buf[1024];
static int tfm_err = 0;
static int tfm_pos = 0;

#ifdef	KPSEWHICH_NEED_TYPE
#define	GET_TFM	"kpsewhich tfm"
#else
#define	GET_TFM	"kpsewhich"
#endif

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

static int tfm_scale (int a, int b)
{
	register int al, bl;

	if (a < 0)	return -tfm_scale(-a, b);
	if (b < 0)	return -tfm_scale(a, -b);

	al = a & 32767;
	bl = b & 32767;
	a >>= 15;
	b >>= 15;
	return ((al * bl / 32768) + a * bl + al * b) / 32 + a * b * 1024;
}

int DviFont_tfm (DviFont *font)
{
	FILE *file;
	int *tab;
	int scale;
	int lh, bc, ec, nw, nh, nd;
	int i, n;

	scale = DviFont_scale(font);
	sprintf(tfm_buf, "%s %s.tfm\n", GET_TFM, font->token.str);
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
	nh = tfm_dim(file);	/* words in height table */
	nd = tfm_dim(file);	/* words in depth table */
	(void) tfm_dim(file);	/* words in italic correction table */
	(void) tfm_dim(file);	/* words in lig/kern table */
	(void) tfm_dim(file);	/* words in kern table */
	(void) tfm_dim(file);	/* words in extensible char table */
	(void) tfm_dim(file);	/* words of font parameter data */

	tfm_skip(file, 4 * lh);

	for (i = 0; i < 256; i++)
		font->width[i] = font->height[i] = font->depth[i] = 0;

	for (i = bc; i <= ec; i++)
	{
		int w = tfm_byte(file);
		int h = tfm_byte(file);

		if	(i < 256)
		{
			font->width[i] = w;
			font->height[i] = h / 16;
			font->depth[i] = h % 16;
		}

		tfm_skip(file, 2);
	}

	if	(ec >= 255)	ec = 255;

	n = nw;

	if	(nh > n)	n = nh;
	if	(nd > n)	n = nd;

	tab = xalloc(n * sizeof(int));

	for (i = 0; i < nw; i++)
		tab[i] = tfm_scale(tfm_word(file), scale);
	
	for (i = bc; i <= ec; i++)
		font->width[i] = tab[font->width[i]];

	for (i = 0; i < nh; i++)
		tab[i] = tfm_scale(tfm_word(file), scale);
	
	for (i = bc; i <= ec; i++)
		font->height[i] = tab[font->height[i]];

	for (i = 0; i < nd; i++)
		tab[i] = tfm_scale(tfm_word(file), scale);
	
	for (i = bc; i <= ec; i++)
		font->depth[i] = tab[font->depth[i]];

	fclose(file);
	return tfm_err;
}
