/*	tfm commands
	$Copyright (C) 2002 Erich Fr�hst�ck
	A-3423 St.Andr�/W�rdern, S�dtirolergasse 17-21/5
*/

#ifndef	tfm_h
#define	tfm_h	1

#include <stdio.h>

typedef struct {
	int dsize;	/* design size */
	int width[256]; /* width table */
} TFM;

extern double tfm_conv;
extern int tfm_load (TFM *tfm, const char *name, int size);

#endif	/* tfm.h */
