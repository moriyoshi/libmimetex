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

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "mimetex_priv.h"

/* ==========================================================================
 * Function:    new_chardef (  )
 * Purpose: Allocates and initializes a chardef struct,
 *      but _not_ the embedded raster struct.
 * --------------------------------------------------------------------------
 * Arguments:   none
 * --------------------------------------------------------------------------
 * Returns: ( chardef * )   ptr to allocated and initialized
 *              chardef struct, or NULL for any error.
 * --------------------------------------------------------------------------
 * Notes:
 * ======================================================================= */
/* --- entry point --- */
chardef *new_chardef(mimetex_ctx *mctx)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* chardef ptr returned to caller */
    chardef *cp = (chardef *)NULL;
    /* ------------------------------------------------------------
    allocate and initialize chardef struct
    ------------------------------------------------------------ */
    /* malloc chardef struct */
    cp = (chardef *)malloc(sizeof(chardef));
    if (cp == (chardef *)NULL)       /* malloc failed */
        /* return error to caller */
        goto end_of_job;
    /* init character description */
    cp->charnum = cp->location = 0;
    /* init upper-left corner */
    cp->toprow = cp->topleftcol = 0;
    /* init lower-left corner */
    cp->botrow = cp->botleftcol = 0;
    /* init raster dimensions */
    cp->image.width = cp->image.height = 0;
    /* init raster format */
    cp->image.format = 0;
    /* and #bits per pixel */
    cp->image.pixsz = 0;
    /* init raster pixmap as null */
    cp->image.pixmap = NULL;
    /* ------------------------------------------------------------
    Back to caller with address of chardef struct, or NULL ptr for any error.
    ------------------------------------------------------------ */
end_of_job:
    return (cp);
} /* --- end-of-function new_chardef() --- */

/* ==========================================================================
 * Function:    delete_chardef ( cp )
 * Purpose: Deallocates a chardef (and bitmap of embedded raster)
 * --------------------------------------------------------------------------
 * Arguments:   cp (I)      ptr to chardef struct to be deleted.
 * --------------------------------------------------------------------------
 * Returns: ( int )     1 if completed successfully,
 *              or 0 otherwise (for any error).
 * --------------------------------------------------------------------------
 * Notes:
 * ======================================================================= */
/* --- entry point --- */
int delete_chardef(mimetex_ctx *mctx, chardef *cp)
{
    /* ------------------------------------------------------------
    free chardef struct
    ------------------------------------------------------------ */
    if (cp != (chardef *)NULL) {     /* can't free null ptr */
        if (cp->image.pixmap != NULL)      /* pixmap allocated within raster */
            /* so free embedded pixmap */
            free((void *)cp->image.pixmap);
        /* and free chardef struct itself */
        free((void *)cp);
    } /* --- end-of-if(cp!=NULL) --- */
    /* ------------------------------------------------------------
    Back to caller with 1=okay, 0=failed.
    ------------------------------------------------------------ */
    return (1);
} /* --- end-of-function delete_chardef() --- */

/* ==========================================================================
 * Function:    get_symdef ( symbol )
 * Purpose: returns mathchardef struct for symbol
 * --------------------------------------------------------------------------
 * Arguments:   symbol (I)  char *  containing symbol
 *              whose corresponding mathchardef is wanted
 * --------------------------------------------------------------------------
 * Returns: ( mathchardef * )  pointer to struct defining symbol,
 *              or NULL for any error
 * --------------------------------------------------------------------------
 * Notes:     o Input symbol need only contain a leading substring to match,
 *      e.g., \gam passed in symbol will match \gamma in the table.
 *      If the table contains two or more possible matches,
 *      the shortest is returned, e.g., input \e will return with
 *      data for \eta rather than \epsilon.  To get \epsilon,
 *      you must pass a leading substring long enough to eliminate
 *      shorter table matches, i.e., in this case \ep
 * ======================================================================= */
/* --- entry point --- */
mathchardef *get_symdef(mimetex_ctx *mctx, char *symbol)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* table of mathchardefs */
    mathchardef *symdef, *bestdef = NULL;
    /* or we may have a ligature */
    int idef = 0;          /* symdefs[] index */
    int symlen = strlen(symbol),    /* length of input symbol */
        deflen, minlen = 9999; /*length of shortest matching symdef*/
    int alphasym = (symlen == 1 && isalpha(*symbol)); /*alphanumeric sym*/
    /* current font family */
    int family = fontinfo[mctx->fontnum].family;
    static  char *displaysyms[][2] = {  /*xlate to Big sym for \displaystyle*/
        /* --- see table on page 536 in TLC2 --- */
        {"\\int",   "\\Bigint"},
        {"\\oint",  "\\Bigoint"},
        {"\\sum",   "\\Bigsum"},
        {"\\prod",  "\\Bigprod"},
        {"\\coprod",    "\\Bigcoprod"},
        /* --- must be 'big' when related to similar binary operators --- */
        {"\\bigcup",    "\\Bigcup"},
        {"\\bigsqcup",  "\\Bigsqcup"},
        {"\\bigcap",    "\\Bigcap"},
        /*{"\\bigsqcap", "\\sqcap"},*/  /* don't have \Bigsqcap */
        {"\\bigodot",   "\\Bigodot"},
        {"\\bigoplus",  "\\Bigoplus"},
        {"\\bigominus", "\\ominus"},
        {"\\bigotimes", "\\Bigotimes"},
        {"\\bigoslash", "\\oslash"},
        {"\\biguplus",  "\\Biguplus"},
        {"\\bigwedge",  "\\Bigwedge"},
        {"\\bigvee",    "\\Bigvee"},
        {NULL, NULL}
    };
    /* ------------------------------------------------------------
    First check for ligature
    ------------------------------------------------------------ */
    /* init signal for no ligature */
    mctx->isligature = 0;
    if (family == CYR10)             /*only check for cyrillic ligatures*/
        if ((symdef = get_ligature(mctx, mctx->subexprptr, family))) {
            /* set bestdef for ligature */
            bestdef = symdef;
            /* signal we found a ligature */
            mctx->isligature = 1;
            goto end_of_job;
        }          /* so just give it to caller */
    /* ------------------------------------------------------------
    If in \displaystyle mode, first xlate int to Bigint, etc.
    ------------------------------------------------------------ */
    if (mctx->isdisplaystyle > 1) {
        /* we're in \displaystyle mode */
        for (idef = 0; ; idef++) {     /* lookup symbol in displaysyms */
            char *fromsym = displaysyms[idef][0]; /* look for this symbol */
            char *tosym = displaysyms[idef][1]; /* and xlate it to this symbol */
            /* end-of-table */
            if (fromsym == NULL)
                break;
            if (!strcmp(symbol, fromsym)) {  /* found a match */
                if (mctx->msglevel >= 99 && mctx->msgfp != NULL) { /* debugging output */
                    fprintf(mctx->msgfp, "get_symdef> mctx->isdisplaystyle=%d, xlated %s to %s\n",
                            mctx->isdisplaystyle, symbol, tosym);
                    fflush(mctx->msgfp);
                }
                /* so look up tosym instead */
                symbol = tosym;
                /* reset symbol length */
                symlen = strlen(symbol);
                break;
            }            /* no need to search further */
        } /* --- end-of-for(idef) --- */
    }
    /* ------------------------------------------------------------
    search symdefs[] in order for first occurrence of symbol
    ------------------------------------------------------------ */
    for (idef = 0; symtables[idef].table; idef++) {
        for (symdef = symtables[idef].table; symdef->symbol; symdef++) {
            /* until trailer record found */
            /* check against caller's symbol */
            if (strncmp(symbol, symdef->symbol, symlen) == 0) {
                /* found match */
                if ((mctx->fontnum == 0 || family == CYR10)    /* mathmode, so check every match */
                        || (0 && fontinfo[mctx->fontnum].istext == 1 &&
                                (!alphasym  /* text mode and not alpha symbol */
                                 || symdef->handler != NULL)) /* or text mode and directive */
                        || (symdef->family == family /* have correct family */
                            && symdef->handler == NULL)) /* and not a handler collision */ {
                    if ((deflen = strlen(symdef->symbol)) < minlen) {
                        /*new best match*/
                        /* save index of new best match */
                        bestdef = symdef;
                        /* and save its len for next test */
                        if ((minlen = deflen) ==  symlen)
                            /*perfect match, so return with it*/
                            break;
                    }
                }
            }
        }
    }
    if (!bestdef) {
        /* failed to look up symbol */
        if (mctx->fontnum != 0) {            /* we're in a restricted font mode */
            /* save current font family */
            int oldfontnum = mctx->fontnum;
            /* lookup result with mctx->fontnum=0 */
            mathchardef *symdef = NULL;
            /*try to look up symbol in any font*/
            mctx->fontnum = 0;
            /* repeat lookup with mctx->fontnum=0 */
            symdef = get_symdef(mctx, symbol);
                /* reset font family */
            mctx->fontnum = oldfontnum;
            return symdef;
        }  /* caller gets mctx->fontnum=0 lookup */
    }
end_of_job:
    if (mctx->msgfp != NULL && mctx->msglevel >= 999) { /* debugging output */
        fprintf(mctx->msgfp,
            "get_symdef> symbol=%s is %smatched (mctx->isligature=%d)\n",
            symbol, bestdef ? "": "not ", mctx->isligature);
        fflush(mctx->msgfp);
    }
    /*NULL or best symdef[]*/
    return bestdef;
} /* --- end-of-function get_symdef() --- */

/* ==========================================================================
 * Function:    get_ligature ( expression, family )
 * Purpose: returns symtable[] index for ligature
 * --------------------------------------------------------------------------
 * Arguments:   expression (I)  char *  containing ligature
 *              whose corresponding mathchardef is wanted
 *      family (I)  int containing NOVALUE for any family,
 *              or, e.g., CYR10 for cyrillic, etc.
 * --------------------------------------------------------------------------
 * Returns: ( int )     symtable[] index defining ligature,
 *              or -9999 if no ligature found or for any error
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
mathchardef *get_ligature(mimetex_ctx *mctx, char *expression, int family)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* table of mathchardefs */
    mathchardef *symdef, *bestdef = NULL;
    char    *ligature = expression /*- 1*/, /* expression ptr */
            *symbol = NULL; /* symdefs[idef].symbol */
    /* #chars remaining in expression */
    int liglen = strlen(ligature);
    /* true for cyrillic families */
    int iscyrfam = (family == CYR10);
    int idef = 0, /* symdefs[] index */
        maxlen = (-9999); /*length of longest matching symdef*/
    /* ------------------------------------------------------------
    search symdefs[] in order for first occurrence of symbol
    ------------------------------------------------------------ */
    if (!mctx->isstring) {
        for (idef = 0; symtables[idef].table; idef++) {
            /* skip handler tables */
            if (symtables[idef].family == NOVALUE)
                continue;
            /* no ligatures in "string" mode */
            for (symdef = symtables[idef].table; symdef->symbol; symdef++) {
                /* #chars in symbol */
                int symlen = strlen(symbol);
                if ((symlen > 1 || iscyrfam)  /*ligature >1 char long or cyrillic*/
                        &&   symlen <= liglen       /* and enough remaining chars */
                        && (*symbol != '\\' || iscyrfam)) /* not escaped or cyrillic */ {
                    if (strncmp(ligature, symbol, symlen) == 0) {
                        /* found match */
                        if (family < 0             /* no family specifies */
                                ||   symdef->family == family) {  /* or have correct family */
                            if (symlen > maxlen) {        /* new longest ligature */
                                /* save index of new best match */
                                bestdef = symdef;
                                /* and save its len for next test */
                                maxlen = symlen;
                            }
                        }
                    }
                } /* --- end-of-if/else(symbol==NULL) --- */
            }
        }
        if (mctx->msgfp != NULL && mctx->msglevel >= 999) { /* debugging output */
            if (bestdef)
                fprintf(mctx->msgfp, "get_ligature> ligature=%.4s is matched to symbol %s\n", ligature, bestdef->symbol);
            else
                fprintf(mctx->msgfp, "get_ligature> ligature=%.4s is not matched to any symbol\n", ligature);
            fflush(mctx->msgfp);
        }
    } /* --- end-of-if(!mctx->isstring) --- */
    /* -9999 or index of best symdef[] */
    return bestdef;
} /* --- end-of-function get_ligature --- */


/* ==========================================================================
 * Function:    get_chardef ( symdef, size )
 * Purpose: returns chardef ptr containing data for symdef at given size
 * --------------------------------------------------------------------------
 * Arguments:   symdef (I)  mathchardef *  corresponding to symbol
 *              whose corresponding chardef is wanted
 *      size (I)    int containing 0-5 for desired size
 * --------------------------------------------------------------------------
 * Returns: ( chardef * )   pointer to struct defining symbol at size,
 *              or NULL for any error
 * --------------------------------------------------------------------------
 * Notes:     o if size unavailable, the next-closer-to-normalsize
 *      is returned instead.
 * ======================================================================= */
/* --- entry point --- */
chardef *get_chardef(mimetex_ctx *mctx, mathchardef *symdef, int size)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* table of font families */
    fontfamily  *fonts = mctx->fonttable;
    chardef **fontdef,          /*tables for desired font, by size*/
    /* chardef for symdef,size */
    *gfdata = (chardef *)NULL;
    /* fonts[] index */
    int ifont;
    /* indexes retrieved from symdef */
    int family, charnum;
    int sizeinc = 0,    /*+1 or -1 to get closer to normal*/
        normalsize = 2; /* this size always present */
    /*true if symbol's 1st char is upper*/
    int isBig = 0;
    /* look for 1st alpha of symbol */
    char    *symptr = NULL;
    /* ------------------------------------------------------------
    initialization
    ------------------------------------------------------------ */
    /* --- check symdef --- */
    /* get_symdef() probably failed */
    if (symdef == NULL) goto end_of_job;
    /* --- get local copy of indexes from symdef --- */
    /* font family containing symbol */
    family = symdef->family;
    /* char# of symbol within font */
    charnum = symdef->charnum;
    /* --- check requested size, and set size increment --- */
    /* size was definitely too small */
    if (size < 0) size = 0;
    /* or definitely too large */
    if (size > LARGESTSIZE) size = LARGESTSIZE;
    /*use next larger if size too small*/
    if (size < normalsize) sizeinc = (+1);
    /*or next smaller if size too large*/
    if (size > normalsize) sizeinc = (-1);
    /* --- check for really big symbol (1st char of symbol name uppercase) --- */
    for (symptr = symdef->symbol; *symptr != '\000'; symptr++) {
        /*skip leading \'s*/
        if (isalpha(*symptr)) {        /* found leading alpha char */
            /* is 1st char of name uppercase? */
            isBig = isupper(*symptr);
            if (!isBig             /* 1st char lowercase */
                    &&   strlen(symptr) >= 4)     /* but followed by at least 3 chars */
                isBig = !memcmp(symptr, "big\\", 4) /* isBig if name starts with big\ */
                        /* or with bigg */
                        || !memcmp(symptr, "bigg", 4);
                /* don't check beyond 1st char */
                break;
        }
    }
    /* ------------------------------------------------------------
    find font family in table of fonts[]
    ------------------------------------------------------------ */
    /* --- look up font family --- */
    for (ifont = 0; ; ifont++)       /* until trailer record found */
        if (fonts[ifont].family < 0) {     /* error, no such family */
            if (mctx->msgfp != NULL && mctx->msglevel >= 99) { /* emit error */
                fprintf(mctx->msgfp, "get_chardef> failed to find font family %d\n",
                        family);
                fflush(mctx->msgfp);
            }
            goto end_of_job;
        }          /* quit if can't find font family*/
        /* found font family */
        else if (fonts[ifont].family == family) break;
    /* --- get local copy of table for this family by size --- */
    /* font by size */
    fontdef = fonts[ifont].fontdef;
    /* ------------------------------------------------------------
    get font in desired size, or closest available size, and return symbol
    ------------------------------------------------------------ */
    /* --- get font in desired size --- */
    while (1) {
        /* find size or closest available */
        /* found available size */
        if (fontdef[size] != NULL)
            break;
        /* adjust size closer to normal */
        if (size == NORMALSIZE       /* already normal so no more sizes,*/
                || sizeinc == 0) {          /* or must be supersampling */
            if (mctx->msgfp != NULL && mctx->msglevel >= 99) { /* emit error */
                fprintf(mctx->msgfp, "get_chardef> failed to find font size %d\n",
                        size);
                fflush(mctx->msgfp);
            }
            /* quit if can't find desired size */
            goto end_of_job;
        } else {
            /*bump size 1 closer to NORMALSIZE*/
            /* see if adjusted size available */
            size += sizeinc;
        }
    }
    /* --- ptr to chardef struct --- */
    /*ptr to chardef for symbol in size*/
    gfdata = &((fontdef[size])[charnum]);
    /* ------------------------------------------------------------
    kludge to tweak CMEX10 (which appears to have incorrect descenders)
    ------------------------------------------------------------ */
    if (family == CMEX10) {          /* cmex10 needs tweak */
        /*total height of char*/
        int height = gfdata->toprow - gfdata->botrow + 1;
        gfdata->botrow = (isBig ? (-height / 3) : (-height / 4));
        gfdata->toprow = gfdata->botrow + gfdata->image.height;
    }
    /* ------------------------------------------------------------
    return subraster containing chardef data for symbol in requested size
    ------------------------------------------------------------ */
end_of_job:
    if (mctx->msgfp != NULL && mctx->msglevel >= 999) {
        if (symdef == NULL) fprintf(mctx->msgfp, "get_chardef> input symdef==NULL\n");
        else
            fprintf(mctx->msgfp, "get_chardef> requested symbol=\"%s\" size=%d  %s\n",
                    symdef->symbol, size, (gfdata == NULL ? "FAILED" : "Succeeded"));
        fflush(mctx->msgfp);
    }
    /*ptr to chardef for symbol in size*/
    return (gfdata);
} /* --- end-of-function get_chardef() --- */

/* ==========================================================================
 * Function:    get_baseline ( gfdata )
 * Purpose: returns baseline for a chardef struct
 * --------------------------------------------------------------------------
 * Arguments:   gfdata (I)  chardef *  containing chardef for symbol
 *              whose baseline is wanted
 * --------------------------------------------------------------------------
 * Returns: ( int )     baseline for symdef,
 *              or -1 for any error
 * --------------------------------------------------------------------------
 * Notes:     o Unlike TeX, the top-left corners of our rasters are (0,0),
 *      with (row,col) increasing as you move down and right.
 *      Baselines are calculated with respect to this scheme,
 *      so 0 would mean the very top row is on the baseline
 *      and everything else descends below the baseline.
 * ======================================================================= */
/* --- entry point --- */
int get_baseline(mimetex_ctx *mctx, chardef *gfdata)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    int
    /*toprow = gfdata->toprow,*/
/*TeX top row from .gf file info*/
    botrow = gfdata->botrow,    /*TeX bottom row from .gf file info*/
             /* #rows comprising symbol */
             height = gfdata->image.height;
    /* ------------------------------------------------------------
    give caller baseline
    ------------------------------------------------------------ */
    /* note: descenders have botrow<0 */
    return ((height - 1) + botrow);
} /* --- end-of-function get_baseline() --- */


/* ==========================================================================
 * Function:    get_charsubraster ( symdef, size )
 * Purpose: returns new subraster ptr containing
 *      data for symdef at given size
 * --------------------------------------------------------------------------
 * Arguments:   symdef (I)  mathchardef *  corresponding to symbol whose
 *              corresponding chardef subraster is wanted
 *      size (I)    int containing 0-5 for desired size
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) pointer to struct defining symbol at size,
 *              or NULL for any error
 * --------------------------------------------------------------------------
 * Notes:     o just wraps a subraster envelope around get_chardef()
 * ======================================================================= */
/* --- entry point --- */
subraster *get_charsubraster(mimetex_ctx *mctx, mathchardef *symdef, int size)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* chardef struct for symdef,size */
    chardef *gfdata = NULL;
    /* subraster containing gfdata */
    subraster *sp = NULL;
    /* convert .gf-format to bitmap */
    raster  *bitmaprp = NULL;
    /* ------------------------------------------------------------
    look up chardef for symdef at size, and embed data (gfdata) in subraster
    ------------------------------------------------------------ */
    if ((gfdata = get_chardef(mctx, symdef, size)) /* look up chardef for symdef,size */
            !=   NULL)              /* and check that we found it */
        if ((sp = new_subraster(mctx, 0, 0, 0))   /* allocate subraster "envelope" */
                !=   NULL) {               /* and check that we succeeded */
            /* ptr to chardef's bitmap or .gf */
            raster *image = &(gfdata->image);
            /* 1=bitmap, else .gf */
            int format = image->format;
            /* replace NULL with caller's arg */
            sp->symdef = symdef;
            /*replace default with caller's size*/
            sp->size = size;
            /* get baseline of character */
            sp->baseline = get_baseline(mctx, gfdata);
            if (format == 1) {         /* already a bitmap */
                /* static char raster */
                sp->type = CHARASTER;
                /* store ptr to its bitmap */
                sp->image = image;
            } else {
                /* need to convert .gf-to-bitmap */
                if ((bitmaprp = gftobitmap(mctx, image))    /* convert */
                        != (raster *)NULL) {         /* successful */
                    /* allocated raster will be freed */
                    sp->type = IMAGERASTER;
                    sp->image = bitmaprp;
                }       /* store ptr to converted bitmap */
                else {               /* conversion failed */
                    /* free unneeded subraster */
                    delete_subraster(mctx, sp);
                    /* signal error to caller */
                    sp = (subraster *)NULL;
                    goto end_of_job;
                }        /* quit */
            }
        } /* --- end-of-if(sp!=NULL) --- */
end_of_job:
    if (mctx->msgfp != NULL && mctx->msglevel >= 999) {
        fprintf(mctx->msgfp, "get_charsubraster> requested symbol=\"%s\" baseline=%d"
                " %s %s\n", symdef->symbol, (sp == NULL ? 0 : sp->baseline),
                (sp == NULL ? "FAILED" : "Succeeded"), (gfdata == NULL ? "(gfdata=NULL)" : " "));
        fflush(mctx->msgfp);
    }
    /* back to caller */
    return (sp);
} /* --- end-of-function get_charsubraster() --- */


/* ==========================================================================
 * Function:    get_symsubraster ( symbol, size )
 * Purpose: returns new subraster ptr containing
 *      data for symbol at given size
 * --------------------------------------------------------------------------
 * Arguments:   symbol (I)  char *  corresponding to symbol
 *              whose corresponding subraster is wanted
 *      size (I)    int containing 0-5 for desired size
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) pointer to struct defining symbol at size,
 *              or NULL for any error
 * --------------------------------------------------------------------------
 * Notes:     o just combines get_symdef() and get_charsubraster()
 * ======================================================================= */
/* --- entry point --- */
subraster *get_symsubraster(mimetex_ctx *mctx, char *symbol, int size)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* subraster containing gfdata */
    subraster *sp = NULL;
    /* mathchardef lookup for symbol */
    mathchardef *symdef = NULL;
    /* ------------------------------------------------------------
    look up mathchardef for symbol
    ------------------------------------------------------------ */
    if (symbol != NULL)              /* user supplied input symbol */
        /*look up corresponding mathchardef*/
        symdef = get_symdef(mctx, symbol);
    /* ------------------------------------------------------------
    look up chardef for mathchardef and wrap a subraster structure around data
    ------------------------------------------------------------ */
    if (symdef != NULL)              /* lookup succeeded */
        /* so get symbol data in subraster */
        sp = get_charsubraster(mctx, symdef, size);
    /* back to caller with sp or NULL */
    return (sp);
} /* --- end-of-function get_symsubraster() --- */

/* ==========================================================================
 * Function:    get_delim ( char *symbol, int height, int family )
 * Purpose: returns subraster corresponding to the samllest
 *      character containing symbol, but at least as large as height,
 *      and in caller's family (if specified).
 *      If no symbol character as large as height is available,
 *      then the largest availabale character is returned instead.
 * --------------------------------------------------------------------------
 * Arguments:   symbol (I)  char *  containing (substring of) desired
 *              symbol, e.g., if symbol="(", then any
 *              mathchardef like "(" or "\\(", etc, match.
 *      height (I)  int containing minimum acceptable height
 *              for returned character
 *      family (I)  int containing -1 to consider all families,
 *              or, e.g., CMEX10 for only that family
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) best matching character available,
 *              or NULL for any error
 * --------------------------------------------------------------------------
 * Notes:     o If height is passed as negative, its absolute value is used
 *      but the best-fit width is searched for (rather than height)
 * ======================================================================= */
/* --- entry point --- */
subraster *get_delim(mimetex_ctx *mctx, char *symbol, int height, int family)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* table of mathchardefs */
    mathchardef *bestdef = NULL, *bigdef = NULL;
    /* best match char */
    subraster *sp = (subraster *)NULL;
    /* get chardef struct for a symdef */
    chardef *gfdata = NULL;
    char    lcsymbol[256], *symptr,     /* lowercase symbol for comparison */
    /* unescaped symbol */
    *unescsymbol = symbol;
    int symlen = (symbol == NULL ? 0 : strlen(symbol)), /* #chars in caller's sym*/
        deflen = 0; /* length of symdef (aka lcsymbol) */
    int idef = 0;           /* symdefs[] index */
    int size = 0,           /* size index 0...LARGESTSIZE */
        bestsize = (-9999),     /* index of best fit size */
        bigsize = (-9999); /*index of biggest (in case no best)*/
    int defheight, bestheight = 9999, /* height of best fit symdef */
        bigheight = (-9999); /*height of biggest(in case no best)*/
    /* true if best-fit width desired */
    int iswidth = 0;
    int isunesc = 0,            /* true if leading escape removed */
        issq = 0, isoint = 0; /* true for \sqcup,etc, \oint,etc */
    /* true for StMary's curly symbols */
    int iscurly = 0;
    /* substitutes for int, oint */
    char *bigint = "bigint", *bigoint = "bigoint";
    /* ------------------------------------------------------------
    determine if searching height or width, and search symdefs[] for best-fit
    ------------------------------------------------------------ */
    /* --- arg checks --- */
    /* no input symbol suplied */
    if (symlen < 1) return (sp);
    /* e causes segfault??? */
    if (strcmp(symbol, "e") == 0) return(sp);
    /* user wants curly delim */
    if (strstr(symbol, "curly") != NULL) iscurly = 1;
    /* --- ignore leading escapes for CMEX10 --- */
    if (1) {              /* ignore leading escape */
        if ((family == CMEX10 || family == CMSYEX)) { /* for CMEX10 or CMSYEX */
            if (strstr(symbol, "sq") != NULL)  /* \sq symbol requested */
                /* seq \sq signal */
                issq = 1;
            if (strstr(symbol, "oint") != NULL)    /* \oint symbol requested */
                /* seq \oint signal */
                isoint = 1;
            if (*symbol == '\\') {         /* have leading \ */
                /* push past leading \ */
                unescsymbol = symbol + 1;
                /* one less char */
                if (--symlen < 1) return(sp);
                if (strcmp(unescsymbol, "int") == 0)  /* \int requested by caller */
                    /* but big version looks better */
                    unescsymbol = bigint;
                if (strcmp(unescsymbol, "oint") == 0)  /* \oint requested by caller */
                    /* but big version looks better */
                    unescsymbol = bigoint;
                /* explicitly recalculate length */
                symlen = strlen(unescsymbol);
                isunesc = 1;
            }         /* signal leading escape removed */
        } /* --- end-of-if(family) --- */
    }
    /* --- determine whether searching for best-fit height or width --- */
    if (height < 0) {            /* negative signals width search */
        /* flip "height" positive */
        height = (-height);
        iswidth = 1;
    }          /* set flag for width search */
    /* --- search symdefs[] for best-fit height (or width) --- */
    for (idef = 0; symtables[idef].table; idef++) {
        mathchardef *symdef;
        for (symdef = symtables[idef].table; symdef->symbol; symdef++) {
            /* local copies */
            char *defsym = symdef->symbol;
            int  deffam  = symdef->family;
            /* reached end-of-table */
            /* check against caller's symbol */
            if (family < 0 || deffam == family /* if explicitly in caller's family*/
                    || (family == CMSYEX && (deffam == CMSY10 || deffam == CMEX10 || deffam == STMARY10))) {
                /* local copy of symdefs[] symbol */
                strcpy(lcsymbol, defsym);
                if (isunesc && *lcsymbol == '\\')    /* ignored leading \ in symbol */
                    /* so squeeze it out of lcsymbol too*/
                    strcpy(lcsymbol, lcsymbol + 1);
                if (0)               /* don't ignore case */
                    for (symptr = lcsymbol; *symptr != '\000'; symptr++) /*for each symbol ch*/
                        /*lowercase the char*/
                        if (isalpha(*symptr)) *symptr = tolower(*symptr);
                /* #chars in symbol we're checking */
                deflen = strlen(lcsymbol);
                if ((symptr = strstr(lcsymbol, unescsymbol)) != NULL) /*found caller's sym*/ {
                    if ((isoint || strstr(lcsymbol, "oint") == NULL) /* skip unwanted "oint"*/
                            && (issq || strstr(lcsymbol, "sq") == NULL)) /* skip unwanted "sq" */ {
                        if ((deffam == CMSY10 ?         /* CMSY10 or not CMSY10 */
                                symptr == lcsymbol        /* caller's sym is a prefix */
                                && deflen == symlen :     /* and same length */
                                (iscurly || strstr(lcsymbol, "curly") == NULL) &&/*not unwanted curly*/
                                (symptr == lcsymbol       /* caller's sym is a prefix */
                                 || symptr == lcsymbol + deflen - symlen))) /* or a suffix */ {
                            for (size = 0; size <= LARGESTSIZE; size++) /* check all font sizes */ {
                                if ((gfdata = get_chardef(mctx, symdef, size)) != NULL) { /*got one*/
                                    /* height of this character */
                                    defheight = gfdata->image.height;
                                    if (iswidth)         /* width search wanted instead... */
                                        /* ...so substitute width */
                                        defheight = gfdata->image.width;
                                    /* set symbol class, etc */
                                    mctx->leftsymdef = symdef;
                                    if (defheight >= height && defheight < bestheight) { /*new best fit*/
                                        bestdef = symdef;
                                        /* save indexes of best fit */
                                        bestsize = size;
                                        bestheight = defheight;
                                    }   /* and save new best height */
                                    if (defheight >= bigheight) {    /* new biggest character */
                                        bigdef = symdef;
                                        /* save indexes of biggest */
                                        bigsize = size;
                                        bigheight = defheight;
                                    }    /* and save new big height */
                                } /* --- end-of-if(gfdata!=NULL) --- */
                            }
                        }
                    }
                }
            } /* --- end-of-if(family) --- */
        } /* --- end-of-for(idef) --- */
    }
    /* ------------------------------------------------------------
    construct subraster for best fit character, and return it to caller
    ------------------------------------------------------------ */
    if (bestdef)            /* found a best fit for caller */
        /* best subraster */
        sp = get_charsubraster(mctx, bestdef, bestsize);
    if ((sp == NULL && height - bigheight > 5)    /* try to construct delim */
            ||   !bigdef)            /* delim not in font tables */
        /* try to build delim */
        sp = make_delim(mctx, symbol, (iswidth ? -height : height));
    if (sp == NULL && bigdef)   /* just give biggest to caller */
        /* biggest subraster */
        sp = get_charsubraster(mctx, bigdef, bigsize);
    if (mctx->msgfp != NULL && mctx->msglevel >= 99)
        fprintf(mctx->msgfp, "get_delim> symbol=%.50s, height=%d family=%d isokay=%s\n",
                (symbol == NULL ? "null" : symbol), height, family, (sp == NULL ? "fail" : "success"));
    return (sp);
} /* --- end-of-function get_delim() --- */

/* ==========================================================================
 * Function:    make_delim ( char *symbol, int height )
 * Purpose: constructs subraster corresponding to symbol
 *      exactly as large as height,
 * --------------------------------------------------------------------------
 * Arguments:   symbol (I)  char *  containing, e.g., if symbol="("
 *              for desired delimiter
 *      height (I)  int containing height
 *              for returned character
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) constructed delimiter
 *              or NULL for any error
 * --------------------------------------------------------------------------
 * Notes:     o If height is passed as negative, its absolute value is used
 *      and interpreted as width (rather than height)
 * ======================================================================= */
/* --- entry point --- */
subraster *make_delim(mimetex_ctx *mctx, char *symbol, int height)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    subraster *sp = NULL;  /* subraster returned to caller */
    subraster *symtop = NULL, *symbot = NULL, *symmid = NULL, *symbar = NULL, /* pieces */
              *topsym = NULL, *botsym = NULL, *midsym = NULL, *barsym = NULL; /* +filler */
    /*1=draw paren, 0=build from pieces*/
    int isdrawparen = 0;
    /* sp->image */
    raster  *rasp = (raster *)NULL;
    /* set true if delimiter drawn ok */
    int isokay = 0;
    int pixsz = 1,          /* pixels are one bit each */
                /* size arg for get_symsubraster() */
                symsize = 0;
    /* drawn lines are one pixel thick */
    int thickness = 1;
    int aspectratio = 8;        /* default height/width for parens */
    int iswidth = 0,            /*true if width specified by height*/
        width = height; /* #pixels width (e.g., of ellipse)*/
    char *lp = NULL,  *rp = NULL,  /* check symbol for left or right */
         *lp2 = NULL, *rp2 = NULL, /* synonym for lp,rp */
         *lp3 = NULL, *rp3 = NULL, /* synonym for lp,rp */
         *lp4 = NULL, *rp4 = NULL; /* synonym for lp,rp */
    /*pre-alloc subraster, except arrow*/
    int isprealloc = 1;
    int oldsmashmargin = mctx->smashmargin,   /* save original mctx->smashmargin */
        wasnocatspace = mctx->isnocatspace; /* save original mctx->isnocatspace */
    /* ------------------------------------------------------------
    initialization
    ------------------------------------------------------------ */
    /* --- determine whether constructing height or width --- */
    if (height < 0) {            /* negative "height" signals width */
        /* flip height positive */
        width = height = (-height);
        iswidth = 1;
    }          /* set flag for width */
    /* too small, must be error */
    if (height < 3) goto end_of_job;
    /* --- set default width (or height) accordingly --- */
    if (iswidth) height = (width + (aspectratio + 1) / 2) / aspectratio;
    else            width = (height + (aspectratio + 1) / 2) / aspectratio;
    if (strchr(symbol, '=') != NULL      /* left or right || bracket wanted */
            ||   strstr(symbol, "\\|") != NULL  /* same || in standard tex notation*/
            ||   strstr(symbol, "dbl") != NULL) /* semantic bracket with ||'s */
        /* need space between two |'s */
        width = max2(width, 6);
    /* set min width */
    if (width < 2) width = 2;
    if (strchr(symbol, '(') != NULL      /* if left ( */
            ||   strchr(symbol, ')') != NULL) { /* or right ) paren wanted */
        /* adjust width */
        width = (3 * width) / 2;
        if (!isdrawparen) isprealloc = 0;
    }  /* don't prealloc if building */
    if (strchr(symbol, '/') != NULL      /* left / */
            ||   strstr(symbol, "\\\\") != NULL /* or \\ for right \ */
            ||   strstr(symbol, "backsl") != NULL)  /* or \backslash for \ */
        width = max2(height / 3, 5);
    if (strstr(symbol, "arrow") != NULL) {   /* arrow wanted */
        /* adjust width */
        width = min2(height / 3, 20);
        isprealloc = 0;
    }           /* don't preallocate subraster */
    if (strchr(symbol, '{') != NULL      /* if left { */
            ||   strchr(symbol, '}') != NULL) { /* or right } brace wanted */
        /* don't preallocate */
        isprealloc = 0;
    }
    /* --- allocate and initialize subraster for constructed delimiter --- */
    if (isprealloc) {            /* pre-allocation wanted */
        if ((sp = new_subraster(mctx, width, height, pixsz)) /* allocate new subraster */
                /* quit if failed */
                ==   NULL)  goto end_of_job;
        /* --- initialize delimiter subraster parameters --- */
        /* image */
        sp->type = IMAGERASTER;
        /* not applicable for image */
        sp->symdef = NULL;
        /* is a little above center good? */
        sp->baseline = height / 2 + 2;
        /* size (probably unneeded) */
        sp->size = NORMALSIZE;
        rasp = sp->image;
    }          /* pointer to image in subraster */
    /* ------------------------------------------------------------
    ( ) parens
    ------------------------------------------------------------ */
    if ((lp = strchr(symbol, '(')) != NULL /* left ( paren wanted */
            || (rp = strchr(symbol, ')')) != NULL) { /* right ) paren wanted */
        if (isdrawparen) {             /* draw the paren */
            /* max width for ()'s */
            int  mywidth = min2(width, 20);
            circle_raster(mctx, rasp,
            /* embedded raster image */
                          0, 0,               /* row0,col0 are upper-left corner */
                          height - 1, mywidth - 1,    /* row1,col1 are lower-right */
                          thickness,
                          /* line thickness is 1 pixel */
                          /* "1234" quadrants to be drawn */
                          (rp == NULL ? "23" : "41"));
            isokay = 1;
        }            /* set flag */
        else {
            /* true for left, false for right */
            int  isleft = (lp != NULL ? 1 : 0);
            char *parentop = (isleft ? "\\leftparentop" : "\\rightparentop"),
                 *parenbot = (isleft ? "\\leftparenbot" : "\\rightparenbot"),
                 *parenbar = (isleft ? "\\leftparenbar" : "\\rightparenbar");
            int  baseht = 0, barht = 0,  /* height of base=top+bot, bar */
                 ibar = 0, nbars = 0; /* bar index, #bars between top&bot*/
            int  largestsize = min2(2, LARGESTSIZE), /* largest size for parens */
                 topfill = (isleft ? 0 : 0), botfill = (isleft ? 0 : 0),
                 barfill = (isleft ? 0 : 7); /* alignment fillers */
            /* --- get pieces at largest size smaller than total height --- */
            for (symsize = largestsize; symsize >= 0; symsize--) { /*largest to smallest*/
                /* --- get pieces at current test size --- */
                /* check for all pieces */
                isokay = 1;
                if ((symtop = get_symsubraster(mctx, parentop, symsize)) == NULL) isokay = 0;
                if ((symbot = get_symsubraster(mctx, parenbot, symsize)) == NULL) isokay = 0;
                if ((symbar = get_symsubraster(mctx, parenbar, symsize)) == NULL) isokay = 0;
                /* --- check sum of pieces against total desired height --- */
                if (isokay) {            /* all pieces retrieved */
                    /*top+bot*/
                    baseht = (symtop->image)->height + (symbot->image)->height;
                    /* bar height */
                    barht  = (symbar->image)->height;
                    /* largest base that's not too big */
                    if (baseht < height + 5) break;
                    /* or smallest available base */
                    if (symsize < 1) break;
                } /* --- end-of-if(isokay) --- */
                /* --- free test pieces that were too big --- */
                /* free top */
                if (symtop != NULL) delete_subraster(mctx, symtop);
                /* free bot */
                if (symbot != NULL) delete_subraster(mctx, symbot);
                /* free bar */
                if (symbar != NULL) delete_subraster(mctx, symbar);
                /* nothing available */
                isokay = 0;
                /* leave isokay=0 after smallest */
                if (symsize < 1) break;
            } /* --- end-of-for(symsize) --- */
            /* --- construct brace from pieces --- */
            if (isokay) {             /* we have the pieces */
                /* --- add alignment fillers --- */
                mctx->smashmargin = 0;
                /*turn off rastcat smashing,space*/
                mctx->isnocatspace = 99;
                topsym = (topfill > 0 ? rastcat(mctx, new_subraster(mctx, topfill, 1, 1), symtop, 3) : symtop);
                botsym = (botfill > 0 ? rastcat(mctx, new_subraster(mctx, botfill, 1, 1), symbot, 3) : symbot);
                barsym = (barfill > 0 ? rastcat(mctx, new_subraster(mctx, barfill, 1, 1), symbar, 3) : symbar);
                /* reset mctx->smashmargin */
                mctx->smashmargin = oldsmashmargin;
                /* reset mctx->isnocatspace */
                mctx->isnocatspace = wasnocatspace;
                /* --- #bars needed between top and bot --- */
                /* #bars needed */
                nbars = (barht < 1 ? 0 : max2(0, 1 + (height - baseht) / barht));
                /* --- stack pieces --- */
                /* start with top piece */
                sp = topsym;
                if (nbars > 0)           /* need nbars between top and bot */
                    for (ibar = 1; ibar <= nbars; ibar++)
                        sp = rastack(mctx, barsym, sp, 1, 0, 0, 2);
                /* bottom below bars or middle */
                sp = rastack(mctx, botsym, sp, 1, 0, 0, 3);
                /* barsym no longer needed */
                delete_subraster(mctx, barsym);
            } /* --- end-of-if(isokay) --- */
        } /* --- end-of-if/else(isdrawparen) --- */
    } /* --- end-of-if(left- or right-() paren wanted) --- */
    /* ------------------------------------------------------------
    { } braces
    ------------------------------------------------------------ */
    else if ((lp = strchr(symbol, '{')) != NULL /* left { brace wanted */
             || (rp = strchr(symbol, '}')) != NULL) { /* right } brace wanted */
        /* true for left, false for right */
        int   isleft = (lp != NULL ? 1 : 0);
        char  *bracetop = (isleft ? "\\leftbracetop" : "\\rightbracetop"),
              *bracebot = (isleft ? "\\leftbracebot" : "\\rightbracebot"),
              *bracemid = (isleft ? "\\leftbracemid" : "\\rightbracemid"),
              *bracebar = (isleft ? "\\leftbracebar" : "\\rightbracebar");
        int   baseht = 0, barht = 0,  /* height of base=top+bot+mid, bar */
              ibar = 0, nbars = 0; /* bar index, #bars above,below mid*/
        int   largestsize = min2(2, LARGESTSIZE), /* largest size for braces */
              topfill = (isleft ? 4 : 0), botfill = (isleft ? 4 : 0),
              midfill = (isleft ? 0 : 4), barfill = (isleft ? 4 : 4); /* alignment fillers */
        /* --- get pieces at largest size smaller than total height --- */
        for (symsize = largestsize; symsize >= 0; symsize--) { /*largest to smallest*/
            /* --- get pieces at current test size --- */
            /* check for all pieces */
            isokay = 1;
            if ((symtop = get_symsubraster(mctx, bracetop, symsize)) == NULL) isokay = 0;
            if ((symbot = get_symsubraster(mctx, bracebot, symsize)) == NULL) isokay = 0;
            if ((symmid = get_symsubraster(mctx, bracemid, symsize)) == NULL) isokay = 0;
            if ((symbar = get_symsubraster(mctx, bracebar, symsize)) == NULL) isokay = 0;
            /* --- check sum of pieces against total desired height --- */
            if (isokay) {            /* all pieces retrieved */
                baseht = (symtop->image)->height + (symbot->image)->height
                         /* top+bot+mid height */
                         + (symmid->image)->height;
                /* bar height */
                barht = (symbar->image)->height;
                /* largest base that's not too big */
                if (baseht < height + 5) break;
                /* or smallest available base */
                if (symsize < 1) break;
            } /* --- end-of-if(isokay) --- */
            /* --- free test pieces that were too big --- */
            /* free top */
            if (symtop != NULL) delete_subraster(mctx, symtop);
            /* free bot */
            if (symbot != NULL) delete_subraster(mctx, symbot);
            /* free mid */
            if (symmid != NULL) delete_subraster(mctx, symmid);
            /* free bar */
            if (symbar != NULL) delete_subraster(mctx, symbar);
            /* nothing available */
            isokay = 0;
            /* leave isokay=0 after smallest */
            if (symsize < 1) break;
        } /* --- end-of-for(symsize) --- */
        /* --- construct brace from pieces --- */
        if (isokay) {              /* we have the pieces */
            /* --- add alignment fillers --- */
            mctx->smashmargin = 0;
            /*turn off rastcat smashing,space*/
            mctx->isnocatspace = 99;
            topsym = (topfill > 0 ? rastcat(mctx, new_subraster(mctx, topfill, 1, 1), symtop, 3) : symtop);
            botsym = (botfill > 0 ? rastcat(mctx, new_subraster(mctx, botfill, 1, 1), symbot, 3) : symbot);
            midsym = (midfill > 0 ? rastcat(mctx, new_subraster(mctx, midfill, 1, 1), symmid, 3) : symmid);
            barsym = (barfill > 0 ? rastcat(mctx, new_subraster(mctx, barfill, 1, 1), symbar, 3) : symbar);
            /* reset mctx->smashmargin */
            mctx->smashmargin = oldsmashmargin;
            /* reset mctx->isnocatspace */
            mctx->isnocatspace = wasnocatspace;
            /* --- #bars needed on each side of mid piece --- */
            /*#bars per side*/
            nbars = (barht < 1 ? 0 : max2(0, 1 + (height - baseht) / barht / 2));
            /* --- stack pieces --- */
            /* start with top piece */
            sp = topsym;
            if (nbars > 0)           /* need nbars above middle */
                for (ibar = 1; ibar <= nbars; ibar++) sp = rastack(mctx, barsym, sp, 1, 0, 0, 2);
            /*mid after top or bars*/
            sp = rastack(mctx, midsym, sp, 1, 0, 0, 3);
            if (nbars > 0)           /* need nbars below middle */
                for (ibar = 1; ibar <= nbars; ibar++) sp = rastack(mctx, barsym, sp, 1, 0, 0, 2);
            /* bottom below bars or middle */
            sp = rastack(mctx, botsym, sp, 1, 0, 0, 3);
            /* barsym no longer needed */
            delete_subraster(mctx, barsym);
        } /* --- end-of-if(isokay) --- */
    } /* --- end-of-if(left- or right-{} brace wanted) --- */
    /* ------------------------------------------------------------
    [ ] brackets
    ------------------------------------------------------------ */
    else if ((lp = strchr(symbol, '[')) != NULL /* left [ bracket wanted */
             || (rp = strchr(symbol, ']')) != NULL  /* right ] bracket wanted */
             || (lp2 = strstr(symbol, "lceil")) != NULL /* left ceiling wanted */
             || (rp2 = strstr(symbol, "rceil")) != NULL /* right ceiling wanted */
             || (lp3 = strstr(symbol, "lfloor")) != NULL /* left floor wanted */
             || (rp3 = strstr(symbol, "rfloor")) != NULL /* right floor wanted */
             || (lp4 = strstr(symbol, "llbrack")) != NULL /* left semantic bracket */
             || (rp4 = strstr(symbol, "rrbrack")) != NULL) { /* right semantic bracket */
        /* --- use rule_raster ( rasp, top, left, width, height, type=0 ) --- */
        int   mywidth = min2(width, 12),  /* max width for horizontal bars */
                        /* thickness of top.bottom bars */
                        wthick = 1;
        /* set lines 1 or 2 pixels thick */
        thickness = (height < 25 ? 1 : 2);
        if (lp2 != NULL || rp2 != NULL || lp3 != NULL || rp3 != NULL) /*ceil or floor*/
            wthick = thickness;         /* same thickness for top/bot bar */
        if (lp3 == NULL && rp3 == NULL)    /* set top bar if floor not wanted */
            /* top horizontal bar */
            rule_raster(mctx, rasp, 0, 0, mywidth, wthick, 0);
        if (lp2 == NULL && rp2 == NULL)    /* set bot bar if ceil not wanted */
            /* bottom */
            rule_raster(mctx, rasp, height - wthick, 0, mywidth, thickness, 0);
        if (lp != NULL || lp2 != NULL || lp3 != NULL || lp4 != NULL) /* left bracket */
            /* left vertical bar */
            rule_raster(mctx, rasp, 0, 0, thickness, height, 0);
        if (lp4 != NULL)           /* 2nd left vertical bar needed */
            /* 2nd left vertical bar */
            rule_raster(mctx, rasp, 0, thickness + 1, 1, height, 0);
        if (rp != NULL || rp2 != NULL || rp3 != NULL || rp4 != NULL) /* right bracket */
            /* right */
            rule_raster(mctx, rasp, 0, mywidth - thickness, thickness, height, 0);
        if (rp4 != NULL)           /* 2nd right vertical bar needed */
            /*2nd right vert*/
            rule_raster(mctx, rasp, 0, mywidth - thickness - 2, 1, height, 0);
        /* set flag */
        isokay = 1;
    } /* --- end-of-if(left- or right-[] bracket wanted) --- */
    /* ------------------------------------------------------------
    < > brackets
    ------------------------------------------------------------ */
    else if ((lp = strchr(symbol, '<')) != NULL /* left < bracket wanted */
             || (rp = strchr(symbol, '>')) != NULL) { /* right > bracket wanted */
        /* --- use line_raster( rasp,  row0, col0,  row1, col1,  thickness ) --- */
        int   mywidth = min2(width, 12),  /* max width for brackets */
                        /* all lines one pixel thick */
                        mythick = 1;
        /* set line pixel thickness */
        thickness = (height < 25 ? 1 : 2);
        if (lp != NULL) {          /* left < bracket wanted */
            line_raster(mctx, rasp, height / 2, 0, 0, mywidth - 1, mythick);
            if (thickness > 1)
                line_raster(mctx, rasp, height / 2, 1, 0, mywidth - 1, mythick);
            line_raster(mctx, rasp, height / 2, 0, height - 1, mywidth - 1, mythick);
            if (thickness > 1)
                line_raster(mctx, rasp, height / 2, 1, height - 1, mywidth - 1, mythick);
        }
        if (rp != NULL) {          /* right > bracket wanted */
            line_raster(mctx, rasp, height / 2, mywidth - 1, 0, 0, mythick);
            if (thickness > 1)
                line_raster(mctx, rasp, height / 2, mywidth - 2, 0, 0, mythick);
            line_raster(mctx, rasp, height / 2, mywidth - 1, height - 1, 0, mythick);
            if (thickness > 1)
                line_raster(mctx, rasp, height / 2, mywidth - 2, height - 1, 0, mythick);
        }
        /* set flag */
        isokay = 1;
    } /* --- end-of-if(left- or right-<> bracket wanted) --- */
    /* ------------------------------------------------------------
    / \ delimiters
    ------------------------------------------------------------ */
    else if ((lp = strchr(symbol, '/')) != NULL /* left /  wanted */
             || (rp = strstr(symbol, "\\\\")) != NULL /* right \ wanted */
             || (rp2 = strstr(symbol, "backsl")) != NULL) { /* right \ wanted */
        /* --- use line_raster( rasp,  row0, col0,  row1, col1,  thickness ) --- */
        int   mywidth = width;        /* max width for / \ */
        /* set line pixel thickness */
        thickness = 1;
        if (lp != NULL)            /* left / wanted */
            line_raster(mctx, rasp, 0, mywidth - 1, height - 1, 0, thickness);
        if (rp != NULL || rp2 != NULL)     /* right \ wanted */
            line_raster(mctx, rasp, 0, 0, height - 1, mywidth - 1, thickness);
        /* set flag */
        isokay = 1;
    } /* --- end-of-if(left- or right-/\ delimiter wanted) --- */
    /* ------------------------------------------------------------
    arrow delimiters
    ------------------------------------------------------------ */
    else if (strstr(symbol, "arrow") != NULL) { /* arrow delimiter wanted */
        /* --- use uparrow_subraster(width,height,pixsz,drctn,isBig) --- */
        int   mywidth = width;        /* max width for / \ */
        int   isBig = (strstr(symbol, "Up") != NULL /* isBig if we have an Up */
                       /* or a Down */
                       || strstr(symbol, "Down") != NULL);
        /* init for uparrow */
        int   drctn = +1;
        if (strstr(symbol, "down") != NULL /* down if we have down */
                ||   strstr(symbol, "Down") != NULL) { /* or Down */
            /* reset direction to down */
            drctn = (-1);
            if (strstr(symbol, "up") != NULL  /* updown if we have up or Up */
                    ||   strstr(symbol, "Up") != NULL) /* and down or Down */
                drctn = 0;
        }          /* reset direction to updown */
        sp = uparrow_subraster(mctx, mywidth, height, pixsz, drctn, isBig);
        if (sp != NULL) {
            /* image */
            sp->type = IMAGERASTER;
            /* not applicable for image */
            sp->symdef = NULL;
            /* is a little above center good? */
            sp->baseline = height / 2 + 2;
            /* size (probably unneeded) */
            sp->size = NORMALSIZE;
            isokay = 1;
        }          /* set flag */
    } /* --- end-of-if(arrow delimiter wanted) --- */
    /* ------------------------------------------------------------
    \- for | | brackets or \= for || || brackets
    ------------------------------------------------------------ */
    else if ((lp = strchr(symbol, '-')) != NULL /* left or right | bracket wanted */
             || (lp2 = strchr(symbol, '|')) != NULL /* synonym for | bracket */
             || (rp = strchr(symbol, '=')) != NULL  /* left or right || bracket wanted */
             || (rp2 = strstr(symbol, "\\|")) != NULL) { /* || in standard tex notation */
        /* --- rule_raster ( rasp, top, left, width, height, type=0 ) --- */
        /* middle col, left of mid if even */
        int   midcol = width / 2;
        if (rp  != NULL            /* left or right || bracket wanted */
                ||   rp2 != NULL) {           /* or || in standard tex notation */
            /* each | of || 1 or 2 pixels thick*/
            thickness = (height < 75 ? 1 : 2);
            /* left */
            rule_raster(mctx, rasp, 0, max2(0, midcol - 2), thickness, height, 0);
            rule_raster(mctx, rasp, 0, min2(width, midcol + 2), thickness, height, 0);
        } else                 /*nb, lp2 spuriously set if rp2 set*/
            if (lp  != NULL           /* left or right | bracket wanted */
                    ||   lp2 != NULL) {          /* ditto for synomym */
                /* set | 1 or 2 pixels thick */
                thickness = (height < 75 ? 1 : 2);
                rule_raster(mctx, rasp, 0, midcol, thickness, height, 0);
            } /*mid vertical bar*/
        /* set flag */
        isokay = 1;
    } /* --- end-of-if(left- or right-[] bracket wanted) --- */
    /* ------------------------------------------------------------
    back to caller
    ------------------------------------------------------------ */
end_of_job:
    if (mctx->msgfp != NULL && mctx->msglevel >= 99)
        fprintf(mctx->msgfp, "make_delim> symbol=%.50s, isokay=%d\n",
                (symbol == NULL ? "null" : symbol), isokay);
    if (!isokay) {         /* don't have requested delimiter */
        /* so free unneeded structure */
        if (sp != NULL) delete_subraster(mctx, sp);
        sp = NULL;
    }          /* and signal error to caller */
    /*back to caller with delim or NULL*/
    return (sp);
} /* --- end-of-function make_delim() --- */


