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
    int class;
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

/* ------------------------------------------------------------
control flags and values
------------------------------------------------------------ */
extern int recurlevel;     /* inc/decremented in rasterize() */
extern int scriptlevel;    /* inc/decremented in rastlimits() */
extern int isstring;       /*pixmap is ascii string, not raster*/
extern int isligature;     /* true if ligature found */
extern char *subexprptr;  /* ptr within expression to subexpr*/
extern int isdisplaystyle;     /* displaystyle mode (forced if 2) */
extern int ispreambledollars;  /* displaystyle mode set by $$...$$ */
extern int fontnum;        /* cal=1,scr=2,rm=3,it=4,bb=5,bf=6 */
extern int fontsize;  /* current size */
extern int displaysize;  /* use \displaystyle when fontsize>=*/
extern double unitlength;    /* #pixels per unit (may be <1.0) */
extern int isnocatspace;   /* >0 to not add space in rastcat()*/
extern int smashmargin;  /* minimum "smash" margin */
extern int mathsmashmargin; /* needed for \text{if $n-m$ even}*/
extern int issmashdelta;   /* true if smashmargin is a delta */
extern int isexplicitsmash;    /* true if \smash explicitly given */
extern int smashcheck;  /* check if terms safe to smash */
extern int isscripted;     /* is (lefthand) term text-scripted*/
extern int isdelimscript;      /* is \right delim text-scripted */
extern int issmashokay;    /*is leading char okay for smashing*/
extern int blanksignal;  /*rastsmash signal right-hand blank*/
extern int blanksymspace;      /* extra (or too much) space wanted*/
extern double gammacorrection; /* gamma correction */
extern int maxfollow;  /* aafollowline() maxturn parameter*/
extern int fgalias;
extern int fgonly;
extern int bgalias;
extern int bgonly;       /* aapnm() params */
extern int *workingparam;  /* working parameter */
extern subraster *workingbox; /*working subraster box*/
extern int isreplaceleft;      /* true to replace leftexpression */
extern subraster *leftexpression; /*rasterized so far*/
extern mathchardef *leftsymdef; /* mathchardef for preceding symbol*/
extern int fraccenterline; /* baseline for punct. after \frac */
extern int iscaching;  /* true if caching images */
extern char cachepath[256];  /* relative path to cached files */
extern char pathprefix[256]; /*prefix for \input,\counter paths*/

extern int centerwt;
extern int minadjacent;
extern int maxadjacent;
extern int adjacentwt;
extern int weightnum;
extern int maxaaparams;
extern aaparameters aaparams[]; /* set params by weight */
extern int cornerwt;
extern int ispatternnumcount;

extern int nfontinfo;
extern struct fontinfo_struct fontinfo[];
extern int symspace[11][11];
/* --- for low-pass anti-aliasing --- */
extern fontfamily aafonttable[];
extern fontfamily ssfonttable[];
extern fontfamily *fonttable;


/* --- dummy font table (for contexts requiring const) --- */
#define dummyfonttable \
  { \
   {   -999, {  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  } } \
  }

/* ---
 * handler functions for math operations
 * ------------------------------------- */
subraster *rastflags();         /* set flags, e.g., for \rm */
subraster *rastfrac();          /* handle \frac \atop expressions */
subraster *rastackrel();        /* handle \stackrel expressions */
subraster *rastmathfunc();      /* handle \lim,\log,etc expressions*/
subraster *rastoverlay();       /* handle \not */
subraster *rastspace();         /* handle math space, \hspace,\hfill*/
subraster *rastnewline();       /* handle \\ newline */
subraster *rastarrow();         /* handle \longrightarrow, etc */
subraster *rastuparrow();       /* handle \longuparrow, etc */
subraster *rastsqrt();          /* handle \sqrt */
subraster *rastaccent();        /* handle \hat \vec \braces, etc */
subraster *rastfont();          /* handle \cal{} \scr{}, etc */
subraster *rastbegin();         /* handle \begin{}...\end{} */
subraster *rastleft();          /* handle \left...\right */
subraster *rastmiddle();        /* handle \left...\middle...\right */
subraster *rastarray();         /* handle \array{...} */
subraster *rastpicture();       /* handle \picture(,){...} */
subraster *rastline();          /* handle \line(xinc,yinc){xlen} */
subraster *rastrule();          /* handle \rule[lift]{width}{height}*/
subraster *rastcircle();        /* handle \circle(xdiam[,ydiam]) */
subraster *rastbezier();        /*handle\bezier(c0,r0)(c1,r1)(ct,rt)*/
subraster *rastraise();         /* handle \raisebox{lift}{expr} */
subraster *rastrotate();        /* handle \rotatebox{degs}{expr} */
subraster *rastreflect();       /* handle \reflectbox[axis]{expr} */
subraster *rastfbox();          /* handle \fbox{expr} */
subraster *rastinput();         /* handle \input{filename} */
subraster *rastcounter();       /* handle \counter{filename} */
subraster *rasttoday();         /* handle \today[+/-tzdelta,ifmt] */
subraster *rastcalendar();      /* handle \calendar[yaer,month] */
subraster *rastnoop();          /* handle \escape's to be flushed */
subraster *rastparen();

int delete_raster();
int delete_subraster();
int line_raster();
int rastput();
int rastsmashcheck();
int rule_raster();
raster *backspace_raster();
raster *gftobitmap();
raster *new_raster();
raster *rastcpy();
raster *rastrot();
subraster *arrow_subraster();
subraster *get_charsubraster();
subraster *get_delim();
subraster *new_subraster();
subraster *rastack();
subraster *rastcat();
subraster *rastcompose();
subraster *rasterize();
subraster *rasterize();
subraster *rastflags();
subraster *rastlimits();
subraster *subrastcpy();
subraster *uparrow_subraster();


/* tex.c */
char *texchar();
char *texsubexpr();
char *texsubexpr();
char *texscripts();
int isbrace();

/* chardef.c */
int get_ligature();
mathchardef *get_symdef();

/* render.c */
int type_raster();

#endif /* MIMETEX_PRIV_H */
