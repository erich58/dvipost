/*	dvi command names

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

#ifndef	dvicmd_h
#define	dvicmd_h	1

#define	DVI_SETC_000         0	/* typeset character 0 and move right */
#define	DVI_SETC_127       127	/* typeset character 127 and move right */
#define	DVI_SET1           128	/* typeset a character and move right */
#define	DVI_SET2           129	/* ??? */
#define	DVI_SET3           130	/* ??? */
#define	DVI_SET4           131	/* ??? */
#define	DVI_SET_RULE       132	/* typeset a rule and move right */
#define	DVI_PUT1           133	/* typeset a character */
#define	DVI_PUT2           134	/* ??? */
#define	DVI_PUT3           135	/* ??? */
#define	DVI_PUT4           136	/* ??? */
#define	DVI_PUT_RULE       137	/* typeset a rule */
#define	DVI_NOP            138	/* no operation */
#define	DVI_BOP            139	/* beginning of page */
#define	DVI_EOP            140	/* ending of page */
#define	DVI_PUSH           141	/* save the current positions */
#define	DVI_POP            142	/* restore previous positions */
#define	DVI_RIGHT1         143	/* move right */
#define	DVI_RIGHT2         144	/* ??? */
#define	DVI_RIGHT3         145	/* ??? */
#define	DVI_RIGHT4         146	/* ??? */
#define	DVI_W0             147	/* move right by |w| */
#define	DVI_W1             148	/* move right and set |w| */
#define	DVI_W2             149	/* ??? */
#define	DVI_W3             150	/* ??? */
#define	DVI_W4             151	/* ??? */
#define	DVI_X0             152	/* move right by |x| */
#define	DVI_X1             153	/* move right and set |x| */
#define	DVI_X2             154	/* ??? */
#define	DVI_X3             155	/* ??? */
#define	DVI_X4             156	/* ??? */
#define	DVI_DOWN1          157	/* move down */
#define	DVI_DOWN2          158	/* ??? */
#define	DVI_DOWN3          159	/* ??? */
#define	DVI_DOWN4          160	/* ??? */
#define	DVI_Y0             161	/* move down by |y| */
#define	DVI_Y1             162	/* move down and set |y| */
#define	DVI_Y2             163	/* ??? */
#define	DVI_Y3             164	/* ??? */
#define	DVI_Y4             165	/* ??? */
#define	DVI_Z0             166	/* move down by |z| */
#define	DVI_Z1             167	/* move down and set |z| */
#define	DVI_Z2             168	/* ??? */
#define	DVI_Z3             169	/* ??? */
#define	DVI_Z4             170	/* ??? */
#define	DVI_FONT_00        171	/* set current font to 0 */
#define	DVI_FONT_63        234	/* set current font to 63 */
#define	DVI_FNT1           235	/* set current font */
#define	DVI_FNT2           236	/* Same as FNT1, except that arg is 2 bytes */
#define	DVI_FNT3           237	/* Same as FNT1, except that arg is 3 bytes */
#define	DVI_FNT4           238	/* Same as FNT1, except that arg is 4 bytes */
#define	DVI_XXX1           239	/* extension to \.DVI primitives */
#define	DVI_XXX2           240	/* Like XXX1, but 0<=k<65536 */
#define	DVI_XXX3           241	/* Like XXX1, but 0<=k<@t$2^{24}$@> */
#define	DVI_XXX4           242	/* potentially long extension to \.DVI
                                   primitives */
#define	DVI_FNT_DEF1       243	/* define the meaning of a font number */
#define	DVI_FNT_DEF2       244	/* ??? */
#define	DVI_FNT_DEF3       245	/* ??? */
#define	DVI_FNT_DEF4       246	/* ??? */
#define	DVI_PRE            247	/* preamble */
#define	DVI_POST           248	/* postamble beginning */
#define	DVI_POST_POST      249	/* postamble ending */

#endif	/* dvicmd.h */
