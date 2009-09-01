/****************************************************************************
 *
 * Copyright(c) 2002-2009, John Forkosh Associates, Inc. All rights reserved.
 *           http://www.forkosh.com   mailto: john@forkosh.com
 * --------------------------------------------------------------------------
 * This file is part of mimeTeX, which is free software. You may redistribute
 * and/or modify it under the terms of the GNU General Public License,
 * version 3 or later, as published by the Free Software Foundation.
 *      MimeTeX is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY, not even the implied warranty of MERCHANTABILITY.
 * See the GNU General Public License for specific details.
 *      By using mimeTeX, you warrant that you have read, understood and
 * agreed to these terms and conditions, and that you possess the legal
 * right and ability to enter into this agreement and to use mimeTeX
 * in accordance with it.
 *      Your mimetex.zip distribution file should contain the file COPYING,
 * an ascii text copy of the GNU General Public License, version 3.
 * If not, point your browser to  http://www.gnu.org/licenses/
 * or write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330,  Boston, MA 02111-1307 USA.
 *
 ****************************************************************************/

#ifndef MIMETEX_PRIV_H
#define MIMETEX_PRIV_H

#include <stdio.h>
#include "mimetex.h"

/* --- font "combinations" --- */
#define CMSYEX (109)            /*select CMSY10, CMEX10 or STMARY10*/

#define BLANKSIGNAL (-991234)       /*rastsmash signal right-hand blank*/

struct fontinfo_struct {
    char *name;
    int family;
    int istext;
    int klass;
};

typedef struct aaparameters_struct
{
    int centerwt;          /* lowpass matrix center   pixel wt*/
    /* lowpass matrix adjacent pixel wt*/
    int adjacentwt;
    /* lowpass matrix corner   pixel wt*/
    int cornerwt;
    /* darken if >= adjacent pts black */
    int minadjacent;
    /* darken if <= adjacent pts black */
    int maxadjacent;
    int fgalias, fgonly, bgalias, bgonly;
    /* aapnm() params */
} aaparameters;

extern aaparameters aaparams[]; /* set params by weight */

extern int nfontinfo;
extern struct fontinfo_struct fontinfo[];
extern int symspace[11][11];
/* --- for low-pass anti-aliasing --- */
extern fontfamily aafonttable[];

/* --- dummy font table (for contexts requiring const) --- */
#define dummyfonttable \
  { \
   {   -999, {  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  } } \
  }

mathchardef *get_ligature(mimetex_ctx *mctx, char *expression, int family);

subraster *rastleft(mimetex_ctx *mctx, char **expression, int size, subraster *basesp, int ildelim, int arg2, int arg3);
subraster *rastright(mimetex_ctx *mctx, char **expression, int size, subraster *basesp, int ildelim, int arg2, int arg3);
subraster *rastmiddle(mimetex_ctx *mctx, char **expression, int size, subraster *basesp, int arg1, int arg2, int arg3);
subraster *rastflags(mimetex_ctx *mctx, char **expression, int size, subraster *basesp, int flag, int value, int arg3);
subraster *rastspace(mimetex_ctx *mctx, char **expression, int size, subraster *basesp, int width, int isfill, int isheight);
subraster *rastnewline(mimetex_ctx *mctx, char **expression, int size, subraster *basesp, int arg1, int arg2, int arg3);
subraster *rastarrow(mimetex_ctx *mctx, char **expression, int size, subraster *basesp, int drctn, int isBig, int arg3);
subraster *rastuparrow(mimetex_ctx *mctx, char **expression, int size, subraster *basesp, int drctn, int isBig, int arg3);
subraster *rastoverlay(mimetex_ctx *mctx, char **expression, int size, subraster *basesp, int overlay, int offset2, int arg3);
subraster *rastfrac(mimetex_ctx *mctx, char **expression, int size, subraster *basesp, int isfrac, int arg2, int arg3);
subraster *rastackrel(mimetex_ctx *mctx, char **expression, int size, subraster *basesp, int base, int arg2, int arg3);
subraster *rastmathfunc(mimetex_ctx *mctx, char **expression, int size, subraster *basesp, int mathfunc, int islimits, int arg3);
subraster *rastsqrt(mimetex_ctx *mctx, char **expression, int size, subraster *basesp, int arg1, int arg2, int arg3);
subraster *rastaccent(mimetex_ctx *mctx, char **expression, int size, subraster *basesp, int accent, int isabove, int isscript);
subraster *rastfont(mimetex_ctx *mctx, char **expression, int size, subraster *basesp, int ifontnum, int arg2, int arg3);
subraster *rastbegin(mimetex_ctx *mctx, char **expression, int size, subraster *basesp, int arg1, int arg2, int arg3);
subraster *rastarray(mimetex_ctx *mctx, char **expression, int size, subraster *basesp, int arg1, int arg2, int arg3);
subraster *rastpicture(mimetex_ctx *mctx, char **expression, int size, subraster *basesp, int arg1, int arg2, int arg3);
subraster *rastline(mimetex_ctx *mctx, char **expression, int size, subraster *basesp, int arg1, int arg2, int arg3);
subraster *rastrule(mimetex_ctx *mctx, char **expression, int size, subraster *basesp, int arg1, int arg2, int arg3);
subraster *rastcircle(mimetex_ctx *mctx, char **expression, int size, subraster *basesp, int arg1, int arg2, int arg3);
subraster *rastbezier(mimetex_ctx *mctx, char **expression, int size, subraster *basesp, int arg1, int arg2, int arg3);
subraster *rastraise(mimetex_ctx *mctx, char **expression, int size, subraster *basesp, int arg1, int arg2, int arg3);
subraster *rastrotate(mimetex_ctx *mctx, char **expression, int size, subraster *basesp, int arg1, int arg2, int arg3);
subraster *rastreflect(mimetex_ctx *mctx, char **expression, int size, subraster *basesp, int arg1, int arg2, int arg3);
subraster *rastfbox(mimetex_ctx *mctx, char **expression, int size, subraster *basesp, int arg1, int arg2, int arg3);
subraster *rasttoday(mimetex_ctx *mctx, char **expression, int size, subraster *basesp, int arg1, int arg2, int arg3);
subraster *rastcalendar(mimetex_ctx *mctx, char **expression, int size, subraster *basesp, int arg1, int arg2, int arg3);
subraster *rastnoop(mimetex_ctx *mctx, char **expression, int size, subraster *basesp, int nargs, int arg2, int arg3);

#endif /* MIMETEX_PRIV_H */
