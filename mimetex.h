#ifndef _MIMETEX
#define _MIMETEX
/****************************************************************************
 *
 * Copyright(c) 2002-2008, John Forkosh Associates, Inc. All rights reserved.
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
 * --------------------------------------------------------------------------
 *
 * Purpose: Structures, macros, symbols,
 *      and static font data for mimetex (and friends)
 *
 * Source:  mimetex.h
 *
 * Notes:     o #define TEXFONTS before #include "mimetex.h"
 *      if you need the fonttable[] (of fontfamily's) set up.
 *      mimetex.c needs this; other modules probably don't
 *      because they'll call access functions from mimetex.c
 *      that hide the underlying font data
 *
 * --------------------------------------------------------------------------
 * Revision History:
 * 09/18/02 J.Forkosh   Installation.
 * 12/11/02 J.Forkosh   Version 1.00 released.
 * 07/04/03 J.Forkosh   Version 1.01 released.
 * ---
 * 09/06/08 J.Forkosh   Version 1.70 released.
 *
 ***************************************************************************/

#include <stdio.h>

/* ------------------------------------------------------------
Program id
------------------------------------------------------------ */
#define REVISIONDATE "17 June 2009" /* date of most recent revision */

/* ---
 * internal buffer sizes
 * --------------------- */
#define MAXEXPRSZ (32768-1)       /*max #bytes in input tex expression*/
#define MAXSUBXSZ (((MAXEXPRSZ+1)/2)-1)/*max #bytes in input subexpression*/
#define MAXTOKNSZ (((MAXSUBXSZ+1)/4)-1) /* max #bytes in input token */
#define MAXFILESZ (65536-1)       /*max #bytes in input (output) file*/
#define MAXLINESZ (4096-1)        /* max #chars in line from file */
#define MAXGIFSZ 131072       /* max #bytes in output GIF image */

/* -------------------------------------------------------------------------
Raster structure (bitmap or bytemap, along with its width and height in bits)
-------------------------------------------------------------------------- */
/* --- 8-bit datatype (always unsigned) --- */
typedef unsigned char intbyte;
/* --- datatype for pixels --- */
typedef unsigned char pixbyte;
/* --- raster structure --- */
typedef struct raster_struct
{
    /* -----------------------------------------------------------------------
    dimensions of raster
    ------------------------------------------------------------------------ */
    int   width;              /* #pixels wide */
    int   height;             /* #pixels high */
    int   format;             /* 1=bitmap, 2=gf/8bits,3=gf/4bits */
    int   pixsz;              /* #bits per pixel, 1 or 8 */
    /* -----------------------------------------------------------------------
    memory for raster
    ------------------------------------------------------------------------ */
    pixbyte *pixmap;      /* memory for width*height bits or bytes */
} raster; /* --- end-of-raster_struct --- */

/* ---
 * associated raster constants and macros
 * -------------------------------------- */
#define maxraster 1048576 /*99999*/ /* max #pixels for raster pixmap */
/* --- #bytes in pixmap raster needed to contain width x height pixels --- */
#define bitmapsz(width,height) (((width)*(height)+7)/8) /*#bytes if a bitmap*/
#define pixmapsz(rp) (((rp)->pixsz)*bitmapsz((rp)->width,(rp)->height))
/* --- #bytes in raster struct, by its format --- */
#define pixbytes(rp) ((rp)->format==1? pixmapsz(rp) : /*#bytes in bitmap*/  \
    ((rp)->format==2? (rp)->pixsz : (1+(rp)->pixsz)/2) ) /*gf-formatted*/
/* --- pixel index calculation used by getpixel() and setpixel() below --- */
#define PIXDEX(rp,irow,icol) (((irow)*((rp)->width))+(icol))/*irow,icol indx*/
/* --- get value of pixel, either one bit or one byte, at (irow,icol) --- */
#define getpixel(rp,irow,icol)      /*get bit or byte based on pixsz*/  \
    ((rp)->pixsz==1? getlongbit((rp)->pixmap,PIXDEX(rp,(irow),(icol))) :\
     ((rp)->pixsz==8? ((rp)->pixmap)[PIXDEX(rp,(irow),(icol))] : (-1)) )
/* --- set value of pixel, either one bit or one byte, at (irow,icol) --- */
#define setpixel(rp,irow,icol,value)    /*set bit or byte based on pixsz*/  \
    if ( (rp)->pixsz == 1 )     /*set pixel to 1 or 0 for bitmap*/  \
     if ( (value) != 0 )        /* turn bit pixel on */             \
      { setlongbit((rp)->pixmap,PIXDEX(rp,(irow),(icol))); }            \
     else               /* or turn bit pixel 0ff */         \
      { unsetlongbit((rp)->pixmap,PIXDEX(rp,(irow),(icol))); }      \
    else                /* set 8-bit bytemap pixel value */ \
      if ( (rp)->pixsz == 8 )   /* check pixsz=8 for bytemap */     \
         ((rp)->pixmap)[PIXDEX(rp,(irow),(icol))]=(pixbyte)(value);     \
      else              /* let user supply final ; */

/* --------------------------------------------------------------------------
some char classes tokenizer needs to recognize, and macros to check for them
-------------------------------------------------------------------------- */
/* --- some character classes --- */
#define istextmode  (fontinfo[mctx->fontnum].istext==1) /* true for text font*/
#define WHITEMATH   "~ \t\n\r\f\v"  /* white chars in display/math mode*/
#define WHITETEXT   "\t\n\r\f\v"    /* white chars in text mode */
#define WHITEDELIM  "~ "        /*always ignored following \sequence*/
#define WHITESPACE  (istextmode?WHITETEXT:WHITEMATH) /*whitespace chars*/
#define LEFTBRACES  "{([<|-="   /* opening delims are left{([< |,|| */
#define RIGHTBRACES "})]>|-="   /* corresponding closing delims */
#define ESCAPE      "\\"        /* introduce escape sequence */
#define SUPERSCRIPT "^"     /* introduce superscript */
#define SUBSCRIPT   "_"     /* introduce subscript */
#define SCRIPTS     SUPERSCRIPT SUBSCRIPT /* either "script" */
/* --- macros to check for them --- */
#define isthischar(thischar,accept) \
    ( (thischar)!='\000' && *(accept)!='\000' \
    && strchr(accept,(thischar))!=(char *)NULL )
#define isthisstr(thisstr,accept) \
    ((*(thisstr))!='\000' && strspn(thisstr,accept)==strlen(thisstr))
#define skipwhite(thisstr)  if ( (thisstr) != NULL ) \
    while ( isthischar(*(thisstr),WHITESPACE) ) (thisstr)++
#define isnextchar(thisstr,accept) \
    ({skipwhite(thisstr);},isthischar(*thisstr,accept))

/* -------------------------------------------------------------------------
character definition struct (font info from .gf file describing a char)
-------------------------------------------------------------------------- */
typedef struct chardef_struct
{
    /* -----------------------------------------------------------------------
    character description
    ------------------------------------------------------------------------ */
    /* --- character identification as given in .gf font file --- */
    int   charnum;            /*different gf files resuse same num*/
    int   location;           /* location in font */
    /* --- upper-left and lower-left corners of char (topcol=botcol?) --- */
    int   toprow, topleftcol;     /* upper-left corner */
    int   botrow, botleftcol;     /* lower-left corner */
    /* -----------------------------------------------------------------------
    character bitmap raster (image.width is character width, ditto height)
    ------------------------------------------------------------------------ */
    raster  image;            /* bitmap image of character */
} chardef; /* --- end-of-chardef_struct --- */

typedef struct mimetex_ctx_struct mimetex_ctx;
typedef struct subraster_struct subraster;

/* -------------------------------------------------------------------------
Font info corresponding to TeX \matchardef, see TeXbook Appendix F (page 431)
-------------------------------------------------------------------------- */
typedef subraster *(*HANDLER)(mimetex_ctx *mctx, char **expression, int size, subraster *basesp, int arg1, int arg2, int arg3);       /* ptr to function returning void* */
typedef struct mathchardef_struct {
    /* -----------------------------------------------------------------------
    symbol name ("a", "\alpha", "1", etc)
    ------------------------------------------------------------------------ */
    char  *symbol;            /* as it appears in a source file */
    /* -----------------------------------------------------------------------
    components of \mathchardef hexadecimal code assigned to this symbol
    ------------------------------------------------------------------------ */
    int   charnum;            /* char# (as given in .gf file) */
    int   family;             /* font family e.g., 2=math symbol */
    int   klass;              /* e.g., 3=relation, TexBook pg.154*/
    /* ------------------------------------------------------------------------
    Extra info: some math "functions" require special processing (e.g., \frac)
    ------------------------------------------------------------------------ */
    /* --- function that performs special processing required by symbol --- */
    /* subraster *((*handler)()); -- handler is ultimately recast like this */
    HANDLER handler;          /* e.g., rastfrac() for \frac's */
} mathchardef ; /* --- end-of-mathchardef_struct --- */

typedef struct mathchardef_table_struct {
    int family;
    mathchardef *table;
} mathchardef_table;

/* ---
 * classes for mathchardef (TeXbook pg.154)
 * ---------------------------------------- */
#define ORDINARY    (0)     /* e.g., /    */
#define OPERATOR    (1)     /* e.g., \sum */
#define BINARYOP    (2)     /* e.g., +    */
#define RELATION    (3)     /* e.g., =    */
#define OPENING     (4)     /* e.g., (    */
#define CLOSING     (5)     /* e.g., }    */
#define PUNCTION    (6)     /* e.g., , (punctuation) */
#define VARIABLE    (7)     /* e.g., x    */
#define DISPOPER    (8)     /* e.g., Bigint (displaymath opers)*/
#define SPACEOPER   (9)     /* e.g., \hspace{} */
#define MAXCLASS    (9)     /* just for index checks */
#define UPPERBIG    DISPOPER    /*how to interpret Bigxxx operators*/
#define LOWERBIG    DISPOPER    /*how to interpret bigxxx operators*/
/* --- class aliases --- */
#define ARROW       RELATION
/* --- families for mathchardef (TeXbook, top of pg.431) --- */
#define CMR10       (1)     /* normal roman */
#define CMMI10      (2)     /* math italic */
#define CMMIB10     (3)     /* math italic bold */
#define CMSY10      (4)     /* math symbol */
#define CMEX10      (5)     /* math extension */
#define RSFS10      (6)     /* rsfs \scrA ... \scrZ */
#define BBOLD10     (7)     /* blackboard bold \mathbb A ... */
#define STMARY10    (8)     /* stmaryrd math symbols */
#define CYR10       (9)     /* cyrillic (wncyr10.mf) */
#define NOTACHAR    (99)        /* e.g., \frac */
/* --- dummy argument value for handlers --- */
#define NOVALUE     (-989898)   /*charnum,family,class used as args*/

/* ---
 * additional font attributes (only size is implemented)
 * ----------------------------------------------------- */
/* --- font sizes 0-7 = tiny,small,normal,large,Large,LARGE,huge,Huge --- */
#define LARGESTSIZE (7)
#ifdef DEFAULTSIZE
#ifndef NORMALSIZE
#define NORMALSIZE (DEFAULTSIZE)
#endif
#endif
#ifndef NORMALSIZE
/*#define NORMALSIZE  (2)*/
#define NORMALSIZE    (3)
#endif
#ifndef DISPLAYSIZE
/* --- automatically sets scripts in \displaystyle when fontsize>= --- */
/*#define DISPLAYSIZE (NORMALSIZE+1)*/
#define DISPLAYSIZE   (3)
#endif

/* ---
aspect ratio is width/height of the displayed image of a pixel
-------------------------------------------------------------- */
#define ASPECTRATIO 1.0 /*(16.0/9.0)*/
#define SURDSERIFWIDTH(sqrtht) max2(1, ( 1 + (((sqrtht)+8)/20) ) )
#define SURDWIDTH(sqrtht,x) ( SURDSERIFWIDTH((sqrtht)) + \
        (((sqrtht)+1)*ASPECTRATIO + 1) / ((((sqrtht))/20)+(x))  )
/* ((int)(.5*((double)((sqrtht)+1))*ASPECTRATIO + 0.5)) ) */
#define SQRTWIDTH(sqrtht,x) min2(32,max2(10,SURDWIDTH((sqrtht),(x))))


/* -------------------------------------------------------------------------
subraster (bitmap image, its attributes, overlaid position in raster, etc)
-------------------------------------------------------------------------- */
struct subraster_struct /* "typedef" for subraster_struct*/
{
    /* --- subraster type --- */
    int   type;               /* charcter or image raster */
    /* --- character info (if subraster represents a character) --- */
    mathchardef *symdef;          /* mathchardef identifying image */
    int   baseline;           /*0 if image is entirely descending*/
    int   size;               /* font size 0-4 */
    /* --- upper-left corner for bitmap (as overlaid on a larger raster) --- */
    int   toprow, leftcol;        /* upper-left corner of subraster */
    /* --- pointer to raster bitmap image of subraster --- */
    raster *image;            /*ptr to bitmap image of subraster*/
}; /* --- end-of-subraster_struct --- */

/* --- subraster types --- */
#define CHARASTER   (1)     /* character */
#define STRINGRASTER    (2)     /* string of characters */
#define IMAGERASTER (3)     /* image */
#define FRACRASTER  (4)     /* image of \frac{}{} */
#define ASCIISTRING (5)     /* ascii string (not a raster) */

#define make_raster(expression,size)    ((rasterize(expression,size))->image)


/* -------------------------------------------------------------------------
font family
-------------------------------------------------------------------------- */
typedef struct fontfamily_struct /* typedef for fontfamily */
{
    /* -----------------------------------------------------------------------
    several sizes, fontdef[0-7]=tiny,small,normal,large,Large,LARGE,huge,HUGE
    ------------------------------------------------------------------------ */
    int   family;             /* font family e.g., 2=math symbol */
    chardef *fontdef[LARGESTSIZE+2];  /*small=(fontdef[1])[charnum].image*/
} fontfamily; /* --- end-of-fontfamily_struct --- */

/* --- sqrt --- */
#define SQRTACCENT  (1)     /* \sqrt */
/* --- accents --- */
#define BARACCENT   (11)        /* \bar \overline*/
#define UNDERBARACCENT  (12)        /* \underline */
#define HATACCENT   (13)        /* \hat */
#define DOTACCENT   (14)        /* \dot */
#define DDOTACCENT  (15)        /* \ddot */
#define VECACCENT   (16)        /* \vec */
#define TILDEACCENT (17)        /* \tilde */
#define OVERBRACE   (18)        /* \overbrace */
#define UNDERBRACE  (19)        /* \underbrace */
/* --- flags/modes --- */
#define ISFONTFAM   (1)     /* set font family */
#define ISDISPLAYSTYLE  (2)     /* set isdisplaystyle */
#define ISDISPLAYSIZE   (21)        /* set displaysize */
#define ISFONTSIZE  (3)     /* set fontsize */
#define ISWEIGHT    (4)     /* set aa params */
#define ISOPAQUE    (5)     /* set background opaque */
#define ISAAALGORITHM   (61)        /* set anti-aliasing algorithm */
#define ISCENTERWT  (62)        /* set anti-aliasing center weight */
#define ISADJACENTWT    (63)        /* set anti-aliasing adjacent weight*/
#define ISCORNERWT  (64)        /* set anti-aliasing adjacent weight*/
#define PNMPARAMS   (65)        /* set fgalias,fgonly,bgalias,bgonly*/
#define ISGAMMA     (66)        /* set gamma correction */
#define UNITLENGTH  (8)     /* set unitlength */
#define ISCOLOR     (9)     /* set color */
#define ISREVERSE   (10)        /* set reverse video colors */
#define ISSTRING    (11)        /* set ascii string mode */
#define ISSMASH     (12)        /* set (minimum) "smash" margin */
#define ISCONTENTTYPE   (13)        /*enable/disable Content-type lines*/

#define DBGLEVEL 9          /* debugging if msglevel>=DBGLEVEL */
#define LOGLEVEL 3          /* logging if msglevel>=LOGLEVEL */

/* --- supersampling shrink factors corresponding to displayed sizes --- */
extern int shrinkfactors[];

struct mimetex_ctx_struct {
    FILE *msgfp;            /* output in command-line mode */
    int msglevel       ;    /* message level for verbose/debug */
    /* --- embed warnings in rendered expressions, [\xxx?] if \xxx unknown --- */
    int warninglevel;  /* warning level */
    int isblackonwhite; /*1=black on white,0=reverse*/
    int fgred;
    int fggreen;
    int fgblue;      /* fg r,g,b */
    int bgred;
    int bggreen;
    int bgblue;      /* bg r,g,b */
    /* --- supersampling shrink factors corresponding to displayed sizes --- */
    int shrinkfactor;   /* shrinkfactors[fontsize] */
    int patternnumcount0[99];
    int patternnumcount1[99]; /*aalookup() counts*/
    int istransparent;/* true sets background transparent*/
    int isplusblank;  /*interpret +'s in query as blanks?*/
    int aaalgorithm;  /* for lp, 1=aalowpass, 2 =aapnm */
    int recurlevel;     /* inc/decremented in rasterize() */
    int scriptlevel;    /* inc/decremented in rastlimits() */
    int isstring;       /*pixmap is ascii string, not raster*/
    int isligature;     /* true if ligature found */
    char *subexprptr;  /* ptr within expression to subexpr*/
    int isdisplaystyle;     /* displaystyle mode (forced if 2) */
    int ispreambledollars;  /* displaystyle mode set by $$...$$ */
    int fontnum;        /* cal=1,scr=2,rm=3,it=4,bb=5,bf=6 */
    int fontsize;  /* current size */
    int displaysize;  /* use \displaystyle when fontsize>=*/
    double unitlength;    /* #pixels per unit (may be <1.0) */
    int isnocatspace;   /* >0 to not add space in rastcat()*/
    int smashmargin;  /* minimum "smash" margin */
    int mathsmashmargin; /* needed for \text{if $n-m$ even}*/
    int issmashdelta;   /* true if smashmargin is a delta */
    int isexplicitsmash;    /* true if \smash explicitly given */
    int smashcheck;  /* check if terms safe to smash */
    int isscripted;     /* is (lefthand) term text-scripted*/
    int isdelimscript;      /* is \right delim text-scripted */
    int issmashokay;    /*is leading char okay for smashing*/
    int blanksignal;  /*rastsmash signal right-hand blank*/
    int blanksymspace;      /* extra (or too much) space wanted*/
    double gammacorrection; /* gamma correction */
    int maxfollow;  /* aafollowline() maxturn parameter*/
    int fgalias;
    int fgonly;
    int bgalias;
    int bgonly;       /* aapnm() params */
    int *workingparam;  /* working parameter */
    subraster *workingbox; /*working subraster box*/
    int isreplaceleft;      /* true to replace leftexpression */
    subraster *leftexpression; /*rasterized so far*/
    mathchardef *leftsymdef; /* mathchardef for preceding symbol*/
    int fraccenterline; /* baseline for punct. after \frac */
    int centerwt;
    int minadjacent;
    int maxadjacent;
    int adjacentwt;
    int weightnum;
    int maxaaparams;
    int cornerwt;
    int ispatternnumcount;
    /* --- for low-pass anti-aliasing --- */
    fontfamily *fonttable;
};

/* ---
 * mathchardefs for symbols recognized by mimetex
 * ---------------------------------------------- */
extern mathchardef_table symtables[16];

/* mimetex.c */
int mimetex_ctx_init(mimetex_ctx *mctx);

/* raster.c */
raster *new_raster(mimetex_ctx *mctx, int width, int height, int pixsz);
int delete_raster(mimetex_ctx *mctx, raster *rp);
raster *rastcpy(mimetex_ctx *mctx, raster *rp);
raster  *rastrot(mimetex_ctx *mctx, raster *rp);
raster  *rastref(mimetex_ctx *mctx, raster *rp, int axis);
int rastput(mimetex_ctx *mctx, raster *target, raster *source,
            int top, int left, int isopaque);
subraster *rastcompose(mimetex_ctx *mctx, subraster *sp1, subraster *sp2, int offset2, int isalign, int isfree);
subraster *rastcat(mimetex_ctx *mctx, subraster *sp1, subraster *sp2, int isfree);
subraster *rastack(mimetex_ctx *mctx, subraster *sp1, subraster *sp2, int base, int space, int iscenter, int isfree);
raster  *rastile(mimetex_ctx *mctx, subraster *tiles, int ntiles);
int rastsmash(mimetex_ctx *mctx, subraster *sp1, subraster *sp2);
int rastsmashcheck(mimetex_ctx *mctx, char *term);
subraster *new_subraster(mimetex_ctx *mctx, int width, int height, int pixsz);
int delete_subraster(mimetex_ctx *mctx, subraster *sp);
subraster *subrastcpy(mimetex_ctx *mctx, subraster *sp);

/* tex.c */
char *texchar(mimetex_ctx *mctx, char *expression, char *chartoken);
char *texsubexpr(mimetex_ctx *mctx, char *expression, char *subexpr, int maxsubsz, char *left, char *right, int isescape, int isdelim);
char *texleft(mimetex_ctx *mctx, char *expression, char *subexpr, int maxsubsz, char *ldelim, char *rdelim);
char *texscripts(mimetex_ctx *mctx, char *expression, char *subscript, char *superscript, int which);
int isbrace(mimetex_ctx *mctx, char *expression, char *braces, int isescape);
char *strdetex(char *s, int mode);
char *mimeprep(mimetex_ctx *mctx, char *expression);
char *strtexchr(char *string, char *texchr);
char *preamble(mimetex_ctx *mctx, char *expression, int *size, char *subexpr);

/* chardef.c */
chardef *new_chardef(mimetex_ctx *mctx);
int delete_chardef(mimetex_ctx *mctx, chardef *cp);
mathchardef *get_ligature(mimetex_ctx *mctx, char *expression, int family);
mathchardef *get_symdef(mimetex_ctx *mctx, char *symbol);
subraster *make_delim(mimetex_ctx *mctx, char *symbol, int height);
subraster *get_delim(mimetex_ctx *mctx, char *symbol, int height, int family);
subraster *get_charsubraster(mimetex_ctx *mctx, mathchardef *symdef, int size);

/* render.c */
int type_raster(mimetex_ctx *mctx, raster *rp, FILE *fp);
raster *border_raster(mimetex_ctx *mctx, raster *rp, int ntop, int nbot, int isline, int isfree);
raster *gftobitmap(mimetex_ctx *mctx, raster *gf);
subraster *arrow_subraster(mimetex_ctx *mctx, int width, int height, int pixsz, int drctn, int isBig);
subraster *uparrow_subraster(mimetex_ctx *mctx, int width, int height, int pixsz, int drctn, int isBig);
subraster *rastparen(mimetex_ctx *mctx, char **subexpr, int size, subraster *basesp);
subraster *rastlimits(mimetex_ctx *mctx, char **expression, int size, subraster *basesp);
subraster *rastdispmath(mimetex_ctx *mctx, char **expression, int size, subraster *sp);
subraster *rastscripts(mimetex_ctx *mctx, char **expression, int size, subraster *basesp);
int circle_raster(mimetex_ctx *mctx, raster *rp, int row0, int col0, int row1, int col1, int thickness, char *quads);
int rule_raster(mimetex_ctx *mctx, raster *rp, int top, int left, int width, int height, int type);
int line_raster(mimetex_ctx *mctx, raster *rp, int row0, int col0, int row1, int col1, int thickness);
int circle_recurse(mimetex_ctx *mctx, raster *rp, int row0, int col0, int row1, int col1, int thickness, double theta0, double theta1);
int line_recurse(mimetex_ctx *mctx, raster *rp, double row0, double col0, double row1, double col1, int thickness);
raster  *backspace_raster(mimetex_ctx *mctx, raster *rp, int nback, int *pback, int minspace, int isfree);
subraster *rasterize(mimetex_ctx *mctx, char *expression, int size);

/* utils.c */
char *dbltoa(double dblval, int npts);
int emit_string(FILE *fp, int col1, char *string, char *comment);
char *calendar(int year, int month, int day);
char *timestamp(int tzdelta, int ifmt);
int tzadjust(int tzdelta, int *year, int *month, int *day, int *hour);
int daynumber(int year, int month, int day);
char *strwrap(mimetex_ctx *mctx, char *s, int linelen, int tablen);
char *strnlower(char *s, int n);
int strreplace(char *string, char *from, char *to, int nreplace);
char *strchange(int nfirst, char *from, char *to);
char *strwstr(mimetex_ctx *mctx, char *string, char *substr, char *white, int *sublen);
int isstrstr(char *string, char *snippets, int iscase);
char x2c(char *what);
int hex_bitmap(raster *rp, FILE *fp, int col1, int isstr);

/* aa.c */
int aalowpass(mimetex_ctx *mctx, raster *rp, intbyte *bytemap, int grayscale);
int aapnm(mimetex_ctx *mctx, raster *rp, intbyte *bytemap, int grayscale);
int aapnmlookup(mimetex_ctx *mctx, raster *rp, intbyte *bytemap, int grayscale);
int aalowpasslookup(mimetex_ctx *mctx, raster *rp, intbyte *bytemap, int grayscale);
int aacolormap(mimetex_ctx *mctx, intbyte *bytemap, int nbytes, intbyte *colors, intbyte *colormap);

/* ------------------------------------------------------------
miscellaneous macros
------------------------------------------------------------ */
#define compress(s,c) if((s)!=NULL) /* remove embedded c's from s */ \
    { char *p; while((p=strchr((s),(c)))!=NULL) strcpy(p,p+1); } else
#define slower(s)  if ((s)!=NULL)   /* lowercase all chars in s */ \
    { char *p=(s); while(*p!='\000'){*p=tolower(*p); p++;} } else
/* --- check if a string is empty --- */
#define isempty(s)  ((s)==NULL?1:(*(s)=='\000'?1:0))
/* --- last char of a string --- */
#define lastchar(s) (isempty(s)?'\000':*((s)+(strlen(s)-1)))

/* --- lowercase a string --- */
#define strlower(s) strnlower((s),0)    /* lowercase an entire string */
/* --- strip leading and trailing whitespace (including ~) --- */
#define trimwhite(thisstr) if ( (thisstr) != NULL ) { \
    int thislen = strlen(thisstr); \
    while ( --thislen >= 0 ) \
      if ( isthischar((thisstr)[thislen]," \t\n\r\f\v") ) \
        (thisstr)[thislen] = '\000'; \
      else break; \
    if ( (thislen = strspn((thisstr)," \t\n\r\f\v")) > 0 ) \
      strcpy((thisstr),(thisstr)+thislen); } else
/* --- strncpy() n bytes and make sure it's null-terminated --- */
#define strninit(target,source,n) if( (target)!=NULL && (n)>=0 ) { \
      char *thissource = (source); \
      (target)[0] = '\000'; \
      if ( (n)>0 && thissource!=NULL ) { \
        strncpy((target),thissource,(n)); \
        (target)[(n)] = '\000'; } }

#define max2(x,y)  ((x)>(y)? (x):(y))   /* larger of 2 arguments */
#define min2(x,y)  ((x)<(y)? (x):(y))   /* smaller of 2 arguments */
#define max3(x,y,z) max2(max2(x,y),(z)) /* largest of 3 arguments */
#define min3(x,y,z) min2(min2(x,y),(z)) /* smallest of 3 arguments */
#define absval(x)  ((x)>=0?(x):(-(x)))  /* absolute value */
#define iround(x)  ((int)((x)>=0?(x)+0.5:(x)-0.5)) /* round double to int */
#define dmod(x,y)  ((x)-((y)*((double)((int)((x)/(y)))))) /*x%y for doubles*/

/* --------------------------------------------------------------------------
macros to get/set/unset a single bit (in rasters), and some bitfield macros
-------------------------------------------------------------------------- */
/* --- single-bit operations on a byte-addressable argument (x) --- */
#define getlongbit(x,bit) get1bit(*((x)+(bit)/8),(bit)%8)   /* get bit */
#define setlongbit(x,bit) set1bit(*((x)+(bit)/8),(bit)%8)   /* set bit */
#define unsetlongbit(x,bit) unset1bit(*((x)+(bit)/8),(bit)%8)   /*unset bit*/
/* --- single-bit operations on a scalar argument (x) --- */
#define get1bit(x,bit)   ( ((x)>>(bit)) & 1 )   /* get the bit-th bit of x */
#define set1bit(x,bit)   ( (x) |=  (1<<(bit)) ) /* set the bit-th bit of x */
#define unset1bit(x,bit) ( (x) &= ~(1<<(bit)) ) /*unset the bit-th bit of x*/
/* --- a few bitfield macros --- */
#define bitmask(nbits)  ((1<<(nbits))-1)    /* a mask of nbits 1's */
#define getbitfld(x,bit1,nbits) (((x)>>(bit1)) & (bitmask(nbits)))

/* --------------------------------------------------------------------------
macros to get/clear/set a single 4-bit nibble (in rasters)
-------------------------------------------------------------------------- */
#define getnibble(x,i)              /* get i-th 4-bit nibble */ \
    ( (i)%2==0? ((x)[(i)/2] & 0xF0) >> 4:   /* left/high nibble */      \
    (x)[(i)/2] & 0x0F )         /* right/low-order nibble */
#define clearnibble(x,i) ((x)[(i)/2] &= ((i)%2==0?0x0F:0xF0)) /*clear ith*/
#define setnibble(x,i,n)            /*set ith nibble of x to n*/\
    if ( (i)%2 == 0 )           /* setting left nibble */   \
      { clearnibble(x,i);           /* first clear the nibble*/ \
        (x)[(i)/2] |= ((n)&0x0F)<<4; }  /* set high-order 4 bits */ \
    else                    /* setting right nibble */  \
     if ( 1 )               /* dummy -- always true */  \
      { clearnibble(x,i);           /* first clear the nibble*/ \
        (x)[(i)/2] |= (n)&0x0F; }       /* set low-order 4 bits */  \
     else                   /* let user supply final ;*/
/* --- macros to get/set/clear byte (format=2) or nibble (format=3) --- */
#define getbyfmt(fmt,x,i)           /*byte(fmt=2) or nibble(3)*/\
    ( ((fmt)==2? ((int)((x)[(i)])) :    /* get full 8-bit byte */   \
       ((fmt)==3? getnibble(x,i) : 0)) )    /* or 4-bit nibble (err=0)*/
#define clearbyfmt(fmt,x,i)         /*byte(fmt=2) or nibble(3)*/\
    if((fmt)==2) (x)[(i)] = ((unsigned char)0); /* clear 8-bit byte */  \
    else if((fmt)==3) clearnibble(x,i)  /* or clear 4-bit nibble */
#define setbyfmt(fmt,x,i,n)         /*byte(fmt=2) or nibble(3)*/\
    if((fmt)==2) (x)[(i)] = ((unsigned char)n); /*set full 8-bit byte*/ \
    else if((fmt)==3) setnibble(x,i,n); else /* or set 4-bit nibble */

#endif
