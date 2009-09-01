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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <string.h>
#include "mimetex_priv.h"

/* ==========================================================================
 * Function:    new_raster ( width, height, pixsz )
 * Purpose: Allocation and constructor for raster.
 *      mallocs and initializes memory for width*height pixels,
 *      and returns raster struct ptr to caller.
 * --------------------------------------------------------------------------
 * Arguments:   width (I)   int containing width, in bits,
 *              of raster pixmap to be allocated
 *      height (I)  int containing height, in bits/scans,
 *              of raster pixmap to be allocated
 *      pixsz (I)   int containing #bits per pixel, 1 or 8
 * --------------------------------------------------------------------------
 * Returns: ( raster * )    ptr to allocated and initialized
 *              raster struct, or NULL for any error.
 * --------------------------------------------------------------------------
 * Notes:
 * ======================================================================= */
/* --- entry point --- */
raster  *new_raster(mimetex_ctx *mctx, int width, int height, int pixsz)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* raster ptr returned to caller */
    raster  *rp = (raster *)NULL;
    /* raster pixel map to be malloced */
    pixbyte *pixmap = NULL;
    /* #bytes needed for pixmap */
    int nbytes = pixsz * bitmapsz(width, height);
    /* pixmap filler */
    int filler = (mctx->isstring ? ' ' : 0);
    /* in case pixmap malloc() fails */
    int delete_raster();
    /* padding bytes */
    int npadding = 0;
    /* ------------------------------------------------------------
    allocate and initialize raster struct and embedded bitmap
    ------------------------------------------------------------ */
    if (mctx->msgfp != NULL && mctx->msglevel >= 9999) {
        fprintf(mctx->msgfp, "new_raster(%d,%d,%d)> entry point\n",
                width, height, pixsz);
        fflush(mctx->msgfp);
    }
    /* --- allocate and initialize raster struct --- */
    /* malloc raster struct */
    rp = (raster *)malloc(sizeof(raster));
    if (mctx->msgfp != NULL && mctx->msglevel >= 9999) {
        fprintf(mctx->msgfp, "new_raster> rp=malloc(%d) returned (%s)\n",
                sizeof(raster), (rp == NULL ? "null ptr" : "success"));
        fflush(mctx->msgfp);
    }
    if (rp == (raster *)NULL)        /* malloc failed */
        /* return error to caller */
        goto end_of_job;
    /* store width in raster struct */
    rp->width = width;
    /* and store height */
    rp->height = height;
    /* initialize as bitmap format */
    rp->format = 1;
    /* store #bits per pixel */
    rp->pixsz = pixsz;
    /* init bitmap as null ptr */
    rp->pixmap = (pixbyte *)NULL;
    /* --- allocate and initialize bitmap array --- */
    if (mctx->msgfp != NULL && mctx->msglevel >= 9999) {
        fprintf(mctx->msgfp, "new_raster> calling pixmap=malloc(%d)\n",
                nbytes);
        fflush(mctx->msgfp);
    }
    if (nbytes > 0 && nbytes <= pixsz*maxraster)  /* fail if width*height too big*/
        /*bytes for width*height bits*/
        pixmap = (pixbyte *)malloc(nbytes + npadding);
    if (mctx->msgfp != NULL && mctx->msglevel >= 9999) {
        fprintf(mctx->msgfp, "new_raster> pixmap=malloc(%d) returned (%s)\n",
                nbytes, (pixmap == NULL ? "null ptr" : "success"));
        fflush(mctx->msgfp);
    }
    if (pixmap == (pixbyte *)NULL) { /* malloc failed */
        /* so free everything */
        delete_raster(mctx, rp);
        /* reset pointer */
        rp = (raster *)NULL;
        goto end_of_job;
    }          /* and return error to caller */
    /* init bytes to binary 0's or ' 's*/
    memset((void *)pixmap, filler, nbytes);
    /* and first byte alwasy 0 */
    *pixmap = (pixbyte)0;
    /* store ptr to malloced memory */
    rp->pixmap = pixmap;
    /* ------------------------------------------------------------
    Back to caller with address of raster struct, or NULL ptr for any error.
    ------------------------------------------------------------ */
end_of_job:
    if (mctx->msgfp != NULL && mctx->msglevel >= 9999) {
        fprintf(mctx->msgfp, "new_raster(%d,%d,%d)> returning (%s)\n",
                width, height, pixsz, (rp == NULL ? "null ptr" : "success"));
        fflush(mctx->msgfp);
    }
    /* back to caller with raster */
    return (rp);
} /* --- end-of-function new_raster() --- */


/* ==========================================================================
 * Function:    delete_raster ( rp )
 * Purpose: Destructor for raster.
 *      Frees memory for raster bitmap and struct.
 * --------------------------------------------------------------------------
 * Arguments:   rp (I)      ptr to raster struct to be deleted.
 * --------------------------------------------------------------------------
 * Returns: ( int )     1 if completed successfully,
 *              or 0 otherwise (for any error).
 * --------------------------------------------------------------------------
 * Notes:
 * ======================================================================= */
/* --- entry point --- */
int delete_raster(mimetex_ctx *mctx, raster *rp)
{
    /* ------------------------------------------------------------
    free raster bitmap and struct
    ------------------------------------------------------------ */
    if (rp != (raster *)NULL) {      /* can't free null ptr */
        if (rp->pixmap != (pixbyte *)NULL)     /* can't free null ptr */
            /* free pixmap within raster */
            free((void *)rp->pixmap);
        /* lastly, free raster struct */
        free((void *)rp);
    } /* --- end-of-if(rp!=NULL) --- */
    /* back to caller, 1=okay 0=failed */
    return (1);
} /* --- end-of-function delete_raster() --- */

/* ==========================================================================
 * Function:    rastcpy ( rp )
 * Purpose: makes duplicate copy of rp
 * --------------------------------------------------------------------------
 * Arguments:   rp (I)      ptr to raster struct to be copied
 * --------------------------------------------------------------------------
 * Returns: ( raster * )    ptr to new copy rp,
 *              or NULL for any error.
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
raster *rastcpy(mimetex_ctx *mctx, raster *rp)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /*copied raster returned to caller*/
    raster  *new_raster(), *newrp = NULL;
    int height = (rp == NULL ? 0 : rp->height), /* original and copied height */
                 width = (rp == NULL ? 0 : rp->width), /* original and copied width */
                         pixsz = (rp == NULL ? 0 : rp->pixsz), /* #bits per pixel */
                                 /* #bytes in rp's pixmap */
                                 nbytes = (rp == NULL ? 0 : (pixmapsz(rp)));
    /* ------------------------------------------------------------
    allocate rotated raster and fill it
    ------------------------------------------------------------ */
    /* --- allocate copied raster with same width,height, and copy bitmap --- */
    if (rp != NULL)              /* nothing to copy if ptr null */
        if ((newrp = new_raster(mctx, width, height, pixsz)) /*same width,height in copy*/
                !=   NULL)                /* check that allocate succeeded */
            /* fill copied raster pixmap */
            memcpy(newrp->pixmap, rp->pixmap, nbytes);
    /* return copied raster to caller */
    return (newrp);
} /* --- end-of-function rastcpy() --- */

/* ==========================================================================
 * Function:    rastrot ( rp )
 * Purpose: rotates rp image 90 degrees right/clockwise
 * --------------------------------------------------------------------------
 * Arguments:   rp (I)      ptr to raster struct to be rotated
 * --------------------------------------------------------------------------
 * Returns: ( raster * )    ptr to new raster rotated relative to rp,
 *              or NULL for any error.
 * --------------------------------------------------------------------------
 * Notes:     o An underbrace is } rotated 90 degrees clockwise,
 *      a hat is <, etc.
 * ======================================================================= */
/* --- entry point --- */
raster  *rastrot(mimetex_ctx *mctx, raster *rp)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /*rotated raster returned to caller*/
    raster  *new_raster(), *rotated = NULL;
    int height = rp->height, irow,  /* original height, row index */
                 width = rp->width, icol,    /* original width, column index */
                         /* #bits per pixel */
                         pixsz = rp->pixsz;
    /* ------------------------------------------------------------
    allocate rotated raster and fill it
    ------------------------------------------------------------ */
    /* --- allocate rotated raster with flipped width<-->height --- */
    if ((rotated = new_raster(mctx, height, width, pixsz)) /* flip width,height */
            !=   NULL)              /* check that allocation succeeded */
        /* --- fill rotated raster --- */
        for (irow = 0; irow < height; irow++)  /* for each row of rp */
            for (icol = 0; icol < width; icol++) { /* and each column of rp */
                int value = getpixel(rp, irow, icol);
                /* setpixel(rotated,icol,irow,value); } */
                setpixel(rotated, icol, (height - 1 - irow), value);
            }
    /* return rotated raster to caller */
    return (rotated);
} /* --- end-of-function rastrot() --- */


/* ==========================================================================
 * Function:    rastref ( rp, axis )
 * Purpose: reflects rp, horizontally about y-axis |_ becomes _| if axis=1
 *      or vertically about x-axis M becomes W if axis=2.
 * --------------------------------------------------------------------------
 * Arguments:   rp (I)      ptr to raster struct to be reflected
 *      axis (I)    int containing 1 for horizontal reflection,
 *              or 2 for vertical
 * --------------------------------------------------------------------------
 * Returns: ( raster * )    ptr to new raster reflected relative to rp,
 *              or NULL for any error.
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
raster  *rastref(mimetex_ctx *mctx, raster *rp, int axis)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* reflected raster back to caller */
    raster  *new_raster(), *reflected = NULL;
    int height = rp->height, irow,  /* height, row index */
                 width = rp->width, icol,    /* width, column index */
                         /* #bits per pixel */
                         pixsz = rp->pixsz;
    /* ------------------------------------------------------------
    allocate reflected raster and fill it
    ------------------------------------------------------------ */
    /* --- allocate reflected raster with same width, height --- */
    if (axis == 1 || axis == 2)      /* first validate axis arg */
        if ((reflected = new_raster(mctx, width, height, pixsz)) /* same width, height */
                !=   NULL)                 /* check that allocation succeeded */
            /* --- fill reflected raster --- */
            for (irow = 0; irow < height; irow++)  /* for each row of rp */
                for (icol = 0; icol < width; icol++) { /* and each column of rp */
                    int value = getpixel(rp, irow, icol);
                    if (axis == 1) {
                        setpixel(reflected, irow, width - 1 - icol, value);
                    }
                    if (axis == 2) {
                        setpixel(reflected, height - 1 - irow, icol, value);
                    }
                }
    /*return reflected raster to caller*/
    return (reflected);
} /* --- end-of-function rastref() --- */


/* ==========================================================================
 * Function:    rastput ( target, source, top, left, isopaque )
 * Purpose: Overlays source onto target,
 *      with the 0,0-bit of source onto the top,left-bit of target.
 * --------------------------------------------------------------------------
 * Arguments:   target (I)  ptr to target raster struct
 *      source (I)  ptr to source raster struct
 *      top (I)     int containing 0 ... target->height - 1
 *      left (I)    int containing 0 ... target->width - 1
 *      isopaque (I)    int containing false (zero) to allow
 *              original 1-bits of target to "show through"
 *              0-bits of source.
 * --------------------------------------------------------------------------
 * Returns: ( int )     1 if completed successfully,
 *              or 0 otherwise (for any error).
 * --------------------------------------------------------------------------
 * Notes:
 * ======================================================================= */
/* --- entry point --- */
int rastput(mimetex_ctx *mctx, raster *target, raster *source,
            int top, int left, int isopaque)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    int irow, icol,     /* indexes over source raster */
    twidth = target->width, theight = target->height, /*target width,height*/
                                      /* #pixels in target */
                                      tpix, ntpix = twidth * theight;
    int isfatal = 0,        /* true to abend on out-of-bounds error */
                  isstrict = 0/*1*/,  /* true for strict bounds check - no "wrap"*/
                             /* true if no pixels out-of-bounds */
                             isokay = 1;
    /* ------------------------------------------------------------
    superimpose source onto target, one bit at a time
    ------------------------------------------------------------ */
    if (isstrict && (top < 0 || left < 0))   /* args fail strict test */
        /* so just return error */
        isokay = 0;
    else
        for (irow = 0; irow < source->height; irow++) { /* for each scan line */
            /*first target pixel (-1)*/
            tpix = (top + irow) * target->width + left - 1;
            for (icol = 0; icol < source->width; icol++) { /* each pixel in scan line */
                /* source pixel value */
                int svalue = getpixel(source, irow, icol);
                /* bump target pixel */
                ++tpix;
                if (mctx->msgfp != NULL && mctx->msglevel >= 9999) { /* debugging output */
                    fprintf(mctx->msgfp, "rastput> tpix,ntpix=%d,%d top,irow,theight=%d,%d,%d "
                            "left,icol,twidth=%d,%d,%d\n", tpix, ntpix, top, irow, theight,
                            left, icol, twidth);
                    fflush(mctx->msgfp);
                }
                if (tpix >= ntpix                /* bounds check failed */
                        || (isstrict && (irow + top >= theight || icol + left >= twidth))) {
                    /* reset okay flag */
                    isokay = 0;
                    /* abort if error is fatal */
                    if (isfatal) goto end_of_job;
                    else break;
                }               /*or just go on to next row*/
                if (tpix >= 0)               /* bounds check okay */
                    if (svalue != 0 || isopaque) {      /*got dark or opaque source*/
                        setpixel(target, irow + top, icol + left, svalue);
                    }/*overlay source on targ*/
            } /* --- end-of-for(icol) --- */
        } /* --- end-of-for(irow) --- */
    /* ------------------------------------------------------------
    Back to caller with 1=okay, 0=failed.
    ------------------------------------------------------------ */
end_of_job:
    return (isokay /*isfatal? (tpix<ntpix? 1:0) : 1*/);
} /* --- end-of-function rastput() --- */

/* ==========================================================================
 * Function:    rastcompose ( sp1, sp2, offset2, isalign, isfree )
 * Purpose: Overlays sp2 on top of sp1, leaving both unchanged
 *      and returning a newly-allocated composite subraster.
 *      Frees/deletes input sp1 and/or sp2 depending on value
 *      of isfree (0=none, 1=sp1, 2=sp2, 3=both).
 * --------------------------------------------------------------------------
 * Arguments:   sp1 (I)     subraster *  to "underneath" subraster,
 *              whose baseline is preserved
 *      sp2 (I)     subraster *  to "overlaid" subraster
 *      offset2 (I) int containing 0 or number of pixels
 *              to horizontally shift sp2 relative to sp1,
 *              either positive (right) or negative
 *      isalign (I) int containing 1 to align baselines,
 *              or 0 to vertically center sp2 over sp1
 *      isfree (I)  int containing 1=free sp1 before return,
 *              2=free sp2, 3=free both, 0=free none.
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) pointer to constructed subraster
 *              or  NULL for any error
 * --------------------------------------------------------------------------
 * Notes:
 * ======================================================================= */
/* --- entry point --- */
subraster *rastcompose(mimetex_ctx *mctx, subraster *sp1, subraster *sp2, int offset2,
                       int isalign, int isfree)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* returned subraster */
    subraster *new_subraster(), *sp = (subraster *)NULL;
    /* new composite raster in sp */
    raster  *rp = (raster *)NULL;
    /* in case isfree non-zero */
    int delete_subraster();
    /*place sp1,sp2 in composite raster*/
    int rastput();
    int base1   = sp1->baseline,    /*baseline for underlying subraster*/
        height1 = (sp1->image)->height, /* height for underlying subraster */
        width1  = (sp1->image)->width,  /* width for underlying subraster */
        pixsz1  = (sp1->image)->pixsz,  /* pixsz for underlying subraster */
        base2   = sp2->baseline,    /*baseline for overlaid subraster */
        height2 = (sp2->image)->height, /* height for overlaid subraster */
        width2  = (sp2->image)->width,  /* width for overlaid subraster */
        pixsz2  = (sp2->image)->pixsz; /* pixsz for overlaid subraster */
    /* overlaid composite */
    int height = 0, width = 0, pixsz = 0, base = 0;
    /* ------------------------------------------------------------
    Initialization
    ------------------------------------------------------------ */
    /* --- determine height, width and baseline of composite raster --- */
    if (isalign) {               /* baselines of sp1,sp2 aligned */
        height = max2(base1 + 1, base2 + 1)  /* max height above baseline */
                 /*+ max descending below*/
                 + max2(height1 - base1 - 1, height2 - base2 - 1);
        base   = max2(base1, base2);
    }   /* max space above baseline */
    else {                  /* baselines not aligned */
        /* max height */
        height = max2(height1, height2);
        base   = base1 + (height - height1) / 2;
    } /* baseline for sp1 */
    /* max width */
    width      = max2(width1, width2 + abs(offset2));
    /* bitmap,bytemap becomes bytemap */
    pixsz      = max2(pixsz1, pixsz2);
    /* ------------------------------------------------------------
    allocate concatted composite subraster
    ------------------------------------------------------------ */
    /* --- allocate returned subraster (and then initialize it) --- */
    if ((sp = new_subraster(mctx, width, height, pixsz)) /* allocate new subraster */
            /* failed, so quit */
            == (subraster *)NULL) goto end_of_job;
    /* --- initialize subraster parameters --- */
    /* image */
    sp->type = IMAGERASTER;
    /* composite baseline */
    sp->baseline = base;
    /* underlying char is sp1 */
    sp->size = sp1->size;
    /* --- extract raster from subraster --- */
    /* raster allocated in subraster */
    rp = sp->image;
    /* ------------------------------------------------------------
    overlay sp1 and sp2 in new composite raster
    ------------------------------------------------------------ */
    if (isalign) {
        /*underlying*/
        rastput(mctx, rp, sp1->image, base - base1, (width - width1) / 2, 1);
        rastput(mctx, rp, sp2->image, base - base2,           /*overlaid*/
                (width - width2) / 2 + offset2, 0);
    } else {
        /*underlying*/
        rastput(mctx, rp, sp1->image, base - base1, (width - width1) / 2, 1);
        rastput(mctx, rp, sp2->image, (height - height2) / 2,     /*overlaid*/
                (width - width2) / 2 + offset2, 0);
    }
    /* ------------------------------------------------------------
    free input if requested
    ------------------------------------------------------------ */
    if (isfree > 0) {            /* caller wants input freed */
        /* free sp1 */
        if (isfree == 1 || isfree > 2) delete_subraster(mctx, sp1);
        if (isfree >= 2) delete_subraster(mctx, sp2);
    }      /* and/or sp2 */
    /* ------------------------------------------------------------
    Back to caller with pointer to concatted subraster or with null for error
    ------------------------------------------------------------ */
end_of_job:
    /* back with subraster or null ptr */
    return (sp);
} /* --- end-of-function rastcompose() --- */


/* ==========================================================================
 * Function:    rastcat ( sp1, sp2, isfree )
 * Purpose: "Concatanates" subrasters sp1||sp2, leaving both unchanged
 *      and returning a newly-allocated subraster.
 *      Frees/deletes input sp1 and/or sp2 depending on value
 *      of isfree (0=none, 1=sp1, 2=sp2, 3=both).
 * --------------------------------------------------------------------------
 * Arguments:   sp1 (I)     subraster *  to left-hand subraster
 *      sp2 (I)     subraster *  to right-hand subraster
 *      isfree (I)  int containing 1=free sp1 before return,
 *              2=free sp2, 3=free both, 0=free none.
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) pointer to constructed subraster sp1||sp2
 *              or  NULL for any error
 * --------------------------------------------------------------------------
 * Notes:
 * ======================================================================= */
/* --- entry point --- */
subraster *rastcat(mimetex_ctx *mctx, subraster *sp1, subraster *sp2, int isfree)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* returned subraster */
    subraster *sp = (subraster *)NULL;
    /* new concatted raster */
    raster  *rp = (raster *)NULL;
    /* in case isfree non-zero */
    int base1   = sp1->baseline,    /*baseline for left-hand subraster*/
        height1 = (sp1->image)->height, /* height for left-hand subraster */
        width1  = (sp1->image)->width,  /* width for left-hand subraster */
        pixsz1  = (sp1->image)->pixsz,  /* pixsz for left-hand subraster */
        type1   = sp1->type,        /* image type for left-hand */
        base2   = sp2->baseline,    /*baseline for right-hand subraster*/
        height2 = (sp2->image)->height, /* height for right-hand subraster */
        width2  = (sp2->image)->width,  /* width for right-hand subraster */
        pixsz2  = (sp2->image)->pixsz,  /* pixsz for right-hand subraster */
        type2   = sp2->type; /* image type for right-hand */
    /*concatted sp1||sp2 composite*/
    int height = 0, width = 0, pixsz = 0, base = 0;
    int issmash = (mctx->smashmargin != 0 ? 1 : 0), /* true to "squash" sp1||sp2 */
        isopaque = (issmash ? 0 : 1),   /* not oppaque if smashing */
        rastsmash(), isblank = 0, nsmash = 0, /* #cols to smash */
        oldsmashmargin = mctx->smashmargin,   /* save original mctx->smashmargin */
        oldblanksymspace = mctx->blanksymspace, /* save original mctx->blanksymspace */
        oldnocatspace = mctx->isnocatspace; /* save original mctx->isnocatspace */
    mathchardef *symdef1 = sp1->symdef, /*mathchardef of last left-hand char*/
                *symdef2 = sp2->symdef; /* mathchardef of right-hand char */

    int class1 = (symdef1 == NULL ? ORDINARY : symdef1->klass), /* symdef->class */
        class2 = (symdef2 == NULL ? ORDINARY : symdef2->klass), /* or default */
        smash1 = (symdef1 != NULL) &&
            (class1 == ORDINARY || class1 == VARIABLE || class1 == OPENING
                || class1 == CLOSING || class1 == PUNCTION),
        smash2 = (symdef2 != NULL) &&
            (class2 == ORDINARY || class2 == VARIABLE || class2 == OPENING
                || class2 == CLOSING || class2 == PUNCTION),
        space = mctx->fontsize / 2 + 1; /* #cols between sp1 and sp2 */
    int isfrac = (type1 == FRACRASTER   /* sp1 is a \frac */
              && class2 == PUNCTION); /* and sp2 is punctuation */
    /* ------------------------------------------------------------
    Initialization
    ------------------------------------------------------------ */
    /* --- determine inter-character space from character class --- */
    if (!mctx->isstring)
        /* space */
        space = max2(2, (symspace[class1][class2] + mctx->fontsize - 3));
    else
        /* space for ascii string */
        space = 1;
    if (mctx->isnocatspace > 0) {
        /* spacing explicitly turned off */
        /* reset space */
        space = 0;
         /* and decrement mctx->isnocatspace flag */
        mctx->isnocatspace--;
    }
    /*implicitly turn off spacing*/
    if (0 && sp1->type == BLANKSIGNAL) space = 0;
    if (sp1->type == BLANKSIGNAL && sp2->type == BLANKSIGNAL) /* both blank */
        /* no extra space between spaces */
        space = 0;
    if (sp2->type != BLANKSIGNAL)        /* not a blank space signal */
        if (mctx->blanksymspace != 0) {          /* and we have a space adjustment */
            /* adjust as much as possible */
            space = max2(0, space + mctx->blanksymspace);
            mctx->blanksymspace = 0;
        }        /* and reset adjustment */
    if (mctx->msgfp != NULL && mctx->msglevel >= 999) { /* display space results */
        fprintf(mctx->msgfp, "rastcat> space=%d, mctx->blanksymspace=%d, mctx->isnocatspace=%d\n",
                space, oldblanksymspace, oldnocatspace);
        fflush(mctx->msgfp);
    }
    /* --- determine smash --- */
    if (!mctx->isstring && !isfrac) {
        /* don't smash strings or \frac's */
        if (issmash) {              /* raster smash wanted */
            int  maxsmash = rastsmash(mctx, sp1, sp2), /* calculate max smash space */
                            /* init margin without delta */
                            margin = mctx->smashmargin;
            if ((1 && smash1 && smash2)       /* concatanating two chars */
                    || (1 && type1 != IMAGERASTER && type2 != IMAGERASTER
                        && type1 != FRACRASTER  && type2 != FRACRASTER))
                /*maxsmash = 0;*/          /* turn off smash */
                /* force small mctx->smashmargin */
                margin = max2(space - 1, 0);
            else
            /* adjust for delta if images */
                if (mctx->issmashdelta)       /* mctx->smashmargin is a delta value */
                    /* add displaystyle base to margin */
                    margin += mctx->fontsize;
            if (maxsmash == mctx->blanksignal)      /* sp2 is intentional blank */
                /* set blank flag signal */
                isblank = 1;
            else
            /* see how much extra space we have*/
                if (maxsmash > margin)          /* enough space for adjustment */
                    /* make adjustment */
                    nsmash = maxsmash - margin;
            if (mctx->msgfp != NULL && mctx->msglevel >= 99) { /* display smash results */
                fprintf(mctx->msgfp, "rastcat> maxsmash=%d, margin=%d, nsmash=%d\n",
                        maxsmash, margin, nsmash);
                fprintf(mctx->msgfp, "rastcat> type1=%d,2=%d, class1=%d,2=%d\n", type1, type2,
                        (symdef1 == NULL ? -999 : class1), (symdef2 == NULL ? -999 : class2));
                fflush(mctx->msgfp);
            }
        } /* --- end-of-if(issmash) --- */
    }
    /* --- determine height, width and baseline of composite raster --- */
    if (!mctx->isstring) {
        height = max2(base1 + 1, base2 + 1)   /* max height above baseline */
                 /*+ max descending below*/
                 + max2(height1 - base1 - 1, height2 - base2 - 1);
        /*add widths and space-smash*/
        width  = width1 + width2 + space - nsmash;
        /* don't "over-smash" composite */
        width  = max3(width, width1, width2);
    } else {
        /* ascii string */
        /* default */
        height = 1;
        /* no need for two nulls */
        width  = width1 + width2 + space - 1;
    }
    /* bitmap||bytemap becomes bytemap */
    pixsz  = max2(pixsz1, pixsz2);
    /* max space above baseline */
    base   = max2(base1, base2);
    if (mctx->msgfp != NULL && mctx->msglevel >= 9999) { /* display components */
        fprintf(mctx->msgfp, "rastcat> Left-hand ht,width,pixsz,base = %d,%d,%d,%d\n",
                height1, width1, pixsz1, base1);
        /* display left-hand raster */
        type_raster(mctx, sp1->image, mctx->msgfp);
        fprintf(mctx->msgfp, "rastcat> Right-hand ht,width,pixsz,base = %d,%d,%d,%d\n",
                height2, width2, pixsz2, base2);
        /* display right-hand raster */
        type_raster(mctx, sp2->image, mctx->msgfp);
        fprintf(mctx->msgfp,
                "rastcat> Composite ht,width,smash,pixsz,base = %d,%d,%d,%d,%d\n",
                height, width, nsmash, pixsz, base);
        fflush(mctx->msgfp);
    }            /* flush mctx->msgfp buffer */
    /* ------------------------------------------------------------
    allocate concatted composite subraster
    ------------------------------------------------------------ */
    /* --- allocate returned subraster (and then initialize it) --- */
    if (mctx->msgfp != NULL && mctx->msglevel >= 9999) {
        fprintf(mctx->msgfp, "rastcat> calling new_subraster(%d,%d,%d)\n",
                width, height, pixsz);
        fflush(mctx->msgfp);
    }
    if ((sp = new_subraster(mctx, width, height, pixsz)) /* allocate new subraster */
            == (subraster *)NULL) {         /* failed */
        if (mctx->msgfp != NULL && mctx->msglevel >= 1) { /* report failure */
            fprintf(mctx->msgfp, "rastcat> new_subraster(%d,%d,%d) failed\n",
                    width, height, pixsz);
            fflush(mctx->msgfp);
        }
        goto end_of_job;
    }          /* failed, so quit */
    /* --- initialize subraster parameters --- */
    /* sp->type = (!mctx->isstring?STRINGRASTER:ASCIISTRING); */  /*concatted string*/
    if (!mctx->isstring)
        sp->type = /*type2;*//*(type1==type2?type2:IMAGERASTER);*/
            (type2 != CHARASTER ? type2 :
             (type1 != CHARASTER && type1 != BLANKSIGNAL
              && type1 != FRACRASTER ? type1 : IMAGERASTER));
    else
        /* concatted ascii string */
        sp->type = ASCIISTRING;
    /* rightmost char is sp2 */
    sp->symdef = symdef2;
    /* composite baseline */
    sp->baseline = base;
    /* rightmost char is sp2 */
    sp->size = sp2->size;
    if (isblank)                 /* need to propagate mctx->blanksignal */
        /* may not be completely safe??? */
        sp->type = mctx->blanksignal;
    /* --- extract raster from subraster --- */
    /* raster allocated in subraster */
    rp = sp->image;
    /* ------------------------------------------------------------
    overlay sp1 and sp2 in new composite raster
    ------------------------------------------------------------ */
    if (mctx->msgfp != NULL && mctx->msglevel >= 9999) {
        fprintf(mctx->msgfp, "rastcat> calling rastput() to concatanate left||right\n");
        fflush(mctx->msgfp);
    }            /* flush mctx->msgfp buffer */
    if (!mctx->isstring) {
        rastput(mctx, rp, sp1->image, base - base1,/* overlay left-hand */
                max2(0, nsmash - width1), 1);/* plus any residual smash space */
    } else {
        /*init left string*/
        memcpy(rp->pixmap, (sp1->image)->pixmap, width1 - 1);
    }
    if (mctx->msgfp != NULL && mctx->msglevel >= 9999) {
        /* display composite raster */
        type_raster(mctx, sp->image, mctx->msgfp);
        fflush(mctx->msgfp);
    }            /* flush mctx->msgfp buffer */
    if (!mctx->isstring) {
        int  fracbase = (isfrac ?     /* baseline for punc after \frac */
                         /*adjust baseline or use original*/
                         max2(mctx->fraccenterline, base2) : base);
        rastput(mctx, rp, sp2->image, fracbase - base2, /* overlay right-hand */
                /* minus any smashed space */
                max2(0, width1 + space - nsmash), isopaque);
        if (1 && type1 == FRACRASTER  /* we're done with \frac image */
                &&   type2 != FRACRASTER)        /* unless we have \frac\frac */
            /* so reset centerline signal */
            mctx->fraccenterline = NOVALUE;
        if (mctx->fraccenterline != NOVALUE)    /* sp2 is a fraction */
            mctx->fraccenterline += (base - base2);
    }  /* so adjust its centerline */
    else {
        strcpy((char *)(rp->pixmap) + width1 - 1 + space, (char *)((sp2->image)->pixmap));
        ((char *)(rp->pixmap))[width1+width2+space-2] = '\000';
    } /*null-term*/
    if (mctx->msgfp != NULL && mctx->msglevel >= 9999) {
        /* display composite raster */
        type_raster(mctx, sp->image, mctx->msgfp);
        fflush(mctx->msgfp);
    }            /* flush mctx->msgfp buffer */
    /* ------------------------------------------------------------
    free input if requested
    ------------------------------------------------------------ */
    if (isfree > 0) {            /* caller wants input freed */
        /* free sp1 */
        if (isfree == 1 || isfree > 2)
            delete_subraster(mctx, sp1);
        if (isfree >= 2)
            delete_subraster(mctx, sp2);
    }      /* and/or sp2 */
    /* ------------------------------------------------------------
    Back to caller with pointer to concatted subraster or with null for error
    ------------------------------------------------------------ */
end_of_job:
    /* reset original mctx->smashmargin */
    mctx->smashmargin = oldsmashmargin;
    /* back with subraster or null ptr */
    return (sp);
} /* --- end-of-function rastcat() --- */


/* ==========================================================================
 * Function:    rastack ( sp1, sp2, base, space, iscenter, isfree )
 * Purpose: Stack subrasters sp2 atop sp1, leaving both unchanged
 *      and returning a newly-allocated subraster,
 *      whose baseline is sp1's if base=1, or sp2's if base=2.
 *      Frees/deletes input sp1 and/or sp2 depending on value
 *      of isfree (0=none, 1=sp1, 2=sp2, 3=both).
 * --------------------------------------------------------------------------
 * Arguments:   sp1 (I)     subraster *  to lower subraster
 *      sp2 (I)     subraster *  to upper subraster
 *      base (I)    int containing 1 if sp1 is baseline,
 *              or 2 if sp2 is baseline.
 *      space (I)   int containing #rows blank space inserted
 *              between sp1's image and sp2's image.
 *      iscenter (I)    int containing 1 to center both sp1 and sp2
 *              in stacked array, 0 to left-justify both
 *      isfree (I)  int containing 1=free sp1 before return,
 *              2=free sp2, 3=free both, 0=free none.
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) pointer to constructed subraster sp2 atop sp1
 *              or  NULL for any error
 * --------------------------------------------------------------------------
 * Notes:
 * ======================================================================= */
/* --- entry point --- */
subraster *rastack(mimetex_ctx *mctx, subraster *sp1, subraster *sp2,
                   int base, int space, int iscenter, int isfree)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* returned subraster */
    subraster *sp = (subraster *)NULL;
    /* new stacked raster in sp */
    raster  *rp = (raster *)NULL;
    /* in case isfree non-zero */
    int delete_subraster();
    /* place sp1,sp2 in stacked raster */
    int rastput();
    int base1   = sp1->baseline,    /* baseline for lower subraster */
        height1 = (sp1->image)->height, /* height for lower subraster */
        width1  = (sp1->image)->width,  /* width for lower subraster */
        pixsz1  = (sp1->image)->pixsz,  /* pixsz for lower subraster */
        base2   = sp2->baseline,    /* baseline for upper subraster */
        height2 = (sp2->image)->height, /* height for upper subraster */
        width2  = (sp2->image)->width,  /* width for upper subraster */
        pixsz2  = (sp2->image)->pixsz; /* pixsz for upper subraster */
    /*for stacked sp2 atop sp1*/
    int height = 0, width = 0, pixsz = 0, baseline = 0;
    mathchardef *symdef1 = sp1->symdef, /* mathchardef of right lower char */
                *symdef2 = sp2->symdef; /* mathchardef of right upper char */
    /* ------------------------------------------------------------
    Initialization
    ------------------------------------------------------------ */
    /* --- determine height, width and baseline of composite raster --- */
    /* sum of heights plus space */
    height   = height1 + space + height2;
    /* max width is overall width */
    width    = max2(width1, width2);
    /* bitmap||bytemap becomes bytemap */
    pixsz    = max2(pixsz1, pixsz2);
    baseline = (base == 1 ? height2 + space + base1 : (base == 2 ? base2 : 0));
    /* ------------------------------------------------------------
    allocate stacked composite subraster (with embedded raster)
    ------------------------------------------------------------ */
    /* --- allocate returned subraster (and then initialize it) --- */
    if ((sp = new_subraster(mctx, width, height, pixsz)) /* allocate new subraster */
            /* failed, so quit */
            == (subraster *)NULL) goto end_of_job;
    /* --- initialize subraster parameters --- */
    /* stacked rasters */
    sp->type = IMAGERASTER;
    /* symdef */
    sp->symdef = (base == 1 ? symdef1 : (base == 2 ? symdef2 : NULL));
    /* composite baseline */
    sp->baseline = baseline;
    /*size*/
    sp->size = (base == 1 ? sp1->size : (base == 2 ? sp2->size : NORMALSIZE));
    /* --- extract raster from subraster --- */
    /* raster embedded in subraster */
    rp = sp->image;
    /* ------------------------------------------------------------
    overlay sp1 and sp2 in new composite raster
    ------------------------------------------------------------ */
    if (iscenter == 1) {         /* center both sp1 and sp2 */
        /* overlay upper */
        rastput(mctx, rp, sp2->image, 0, (width - width2) / 2, 1);
        rastput(mctx, rp, sp1->image, height2 + space, (width - width1) / 2, 1);
    } /*lower*/
    else {                  /* left-justify both sp1 and sp2 */
        /* overlay upper */
        rastput(mctx, rp, sp2->image, 0, 0, 1);
        rastput(mctx, rp, sp1->image, height2 + space, 0, 1);
    } /*lower*/
    /* ------------------------------------------------------------
    free input if requested
    ------------------------------------------------------------ */
    if (isfree > 0) {            /* caller wants input freed */
        /* free sp1 */
        if (isfree == 1 || isfree > 2)
            delete_subraster(mctx, sp1);
        if (isfree >= 2)
            delete_subraster(mctx, sp2);
    } /* and/or sp2 */
    /* ------------------------------------------------------------
    Back to caller with pointer to stacked subraster or with null for error
    ------------------------------------------------------------ */
end_of_job:
    /* back with subraster or null ptr */
    return (sp);
} /* --- end-of-function rastack() --- */


/* ==========================================================================
 * Function:    rastile ( tiles, ntiles )
 * Purpose: Allocate and build up a composite raster
 *      from the ntiles components/characters supplied in tiles.
 * --------------------------------------------------------------------------
 * Arguments:   tiles (I)   subraster *  to array of subraster structs
 *              describing the components and their locations
 *      ntiles (I)  int containing number of subrasters in tiles[]
 * --------------------------------------------------------------------------
 * Returns: ( raster * )    ptr to composite raster,
 *              or NULL for any error.
 * --------------------------------------------------------------------------
 * Notes:     o The top,left corner of a raster is row=0,col=0
 *      with row# increasing as you move down,
 *      and col# increasing as you move right.
 *      Metafont numbers rows with the baseline=0,
 *      so the top row is a positive number that
 *      decreases as you move down.
 *        o rastile() is no longer used.
 *      It was used by an earlier rasterize() algorithm,
 *      and I've left it in place should it be needed again.
 *      But recent changes haven't been tested/exercised.
 * ======================================================================= */
/* --- entry point --- */
raster  *rastile(mimetex_ctx *mctx, subraster *tiles, int ntiles)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /*raster back to caller*/
    raster  *new_raster(), *composite = (raster *)NULL;
    int width = 0, height = 0, pixsz = 0, /*width,height,pixsz of composite raster*/
        toprow = 9999, rightcol = -999, /* extreme upper-right corner of tiles */
        /* extreme lower-left corner of tiles */
        botrow = -999, leftcol = 9999;
    /* tiles[] index */
    int itile;
    /* overlay each tile in composite raster */
    int rastput();
    /* ------------------------------------------------------------
    run through tiles[] to determine dimensions for composite raster
    ------------------------------------------------------------ */
    /* --- determine row and column bounds of composite raster --- */
    for (itile = 0; itile < ntiles; itile++) {
        /* ptr to current tile */
        subraster *tile = &(tiles[itile]);
        /* --- upper-left corner of composite --- */
        toprow = min2(toprow, tile->toprow);
        leftcol = min2(leftcol, tile->leftcol);
        /* --- lower-right corner of composite --- */
        botrow = max2(botrow, tile->toprow + (tile->image)->height - 1);
        rightcol = max2(rightcol, tile->leftcol + (tile->image)->width  - 1);
        /* --- pixsz of composite --- */
        pixsz = max2(pixsz, (tile->image)->pixsz);
    } /* --- end-of-for(itile) --- */
    /* --- calculate width and height from bounds --- */
    width  = rightcol - leftcol + 1;
    height = botrow - toprow + 1;
    /* --- sanity check (quit if bad dimensions) --- */
    if (width < 1 || height < 1) goto end_of_job;
    /* ------------------------------------------------------------
    allocate composite raster, and embed tiles[] within it
    ------------------------------------------------------------ */
    /* --- allocate composite raster --- */
    if ((composite = new_raster(mctx, width, height, pixsz))  /*allocate composite raster*/
            /* and quit if failed */
            == (raster *)NULL) goto end_of_job;
    /* --- embed tiles[] in composite --- */
    for (itile = 0; itile < ntiles; itile++) {
        /* ptr to current tile */
        subraster *tile = &(tiles[itile]);
        rastput(mctx, composite, tile->image,         /* overlay tile image at...*/
                tile->toprow - toprow, tile->leftcol - leftcol, 1);
    } /*upper-left corner*/
    /* ------------------------------------------------------------
    Back to caller with composite raster (or null for any error)
    ------------------------------------------------------------ */
end_of_job:
    /* back with composite or null ptr */
    return (composite);
} /* --- end-of-function rastile() --- */


/* ==========================================================================
 * Function:    rastsmash ( sp1, sp2 )
 * Purpose: When concatanating sp1||sp2, calculate #pixels
 *      we can "smash sp2 left"
 * --------------------------------------------------------------------------
 * Arguments:   sp1 (I)     subraster *  to left-hand raster
 *      sp2 (I)     subraster *  to right-hand raster
 * --------------------------------------------------------------------------
 * Returns: ( int )     max #pixels we can smash sp1||sp2,
 *              or "mctx->blanksignal" if sp2 intentionally blank,
 *              or 0 for any error.
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int rastsmash(mimetex_ctx *mctx, subraster *sp1, subraster *sp2)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* #pixels to smash sp1||sp2 */
    int nsmash = 0;
    int base1   = sp1->baseline,    /*baseline for left-hand subraster*/
        height1 = (sp1->image)->height, /* height for left-hand subraster */
        width1  = (sp1->image)->width,  /* width for left-hand subraster */
        base2   = sp2->baseline,    /*baseline for right-hand subraster*/
        height2 = (sp2->image)->height, /* height for right-hand subraster */
        width2  = (sp2->image)->width; /* width for right-hand subraster */
    int base = max2(base1, base2),  /* max ascenders - 1 above baseline*/
        top1 = base - base1, top2 = base - base2, /* top irow indexes for sp1, sp2 */
        bot1 = top1 + height1 - 1, bot2 = top2 + height2 - 1, /* bot irow indexes */
        height = max2(bot1, bot2) + 1; /* total height */
    /* row,col indexes */
    int irow1 = 0, irow2 = 0, icol = 0;
    int firstcol1[1025], nfirst1 = 0, /* 1st sp1 col containing set pixel*/
        firstcol2[1025], nfirst2 = 0; /* 1st sp2 col containing set pixel*/
    /* min separation (s=x+y) */
    int smin = 9999, xmin = 9999, ymin = 9999;
    /* ------------------------------------------------------------
    find right edge of sp1 and left edge of sp2 (these will be abutting edges)
    ------------------------------------------------------------ */
    /* --- check args --- */
    /* ignore string rasters */
    if (mctx->isstring) goto end_of_job;
    /* don't smash in text mode */
    if (0 && istextmode) goto end_of_job;
    /* don't try to smash huge image */
    if (height > 1023) goto end_of_job;
    if (sp2->type == mctx->blanksignal)        /*mctx->blanksignal was propagated to us*/
        /* don't smash intentional blank */
        goto end_of_job;
    /* --- init firstcol1[], firstcol2[] --- */
    for (irow1 = 0; irow1 < height; irow1++) /* for each row */
        /* signal empty rows */
        firstcol1[irow1] = firstcol2[irow1] = mctx->blanksignal;
    /* --- set firstcol2[] indicating left edge of sp2 --- */
    for (irow2 = top2; irow2 <= bot2; irow2++) {
        /* for each row inside sp2 */
        for (icol = 0; icol < width2; icol++) {
             /* find first non-empty col in row */
            if (getpixel(sp2->image, irow2 - top2, icol) != 0) {
                /* found a set pixel */
                /* icol is #cols from left edge */
                firstcol2[irow2] = icol;
                /* bump #rows containing set pixels*/
                nfirst2++;
                break;
            } /* and go on to next row */
        }
    }
    if (nfirst2 < 1) {           /*right-hand sp2 is completely blank*/
        /* signal intentional blanks */
        nsmash = mctx->blanksignal;
        goto end_of_job;
    }          /* don't smash intentional blanks */
    /* --- now check if preceding image in sp1 was an intentional blank --- */
    if (sp1->type == mctx->blanksignal)        /*mctx->blanksignal was propagated to us*/
        /* don't smash intentional blank */
        goto end_of_job;
    /* --- set firstcol1[] indicating right edge of sp1 --- */
    for (irow1 = top1; irow1 <= bot1; irow1++) {
        /* for each row inside sp1 */
        for (icol = width1 - 1; icol >= 0; icol--) {
            /* find last non-empty col in row */
            if (getpixel(sp1->image, irow1 - top1, icol) != 0) {
                /* found a set pixel */
                /* save #cols from right edge */
                firstcol1[irow1] = (width1 - 1) - icol;
                /* bump #rows containing set pixels*/
                nfirst1++;
                break;
            } /* and go on to next row */
        }
    }
    if (nfirst1 < 1)             /*left-hand sp1 is completely blank*/
        /* don't smash intentional blanks */
        goto end_of_job;
    /* ------------------------------------------------------------
    find minimum separation
    ------------------------------------------------------------ */
    for (irow2 = top2; irow2 <= bot2; irow2++) { /* check each row inside sp2 */
        /* #cols to first set pixel */
        int margin1, margin2 = firstcol2[irow2];
        if (margin2 != mctx->blanksignal) {   /* irow2 not an empty/blank row */
            for (irow1 = max2(irow2 - smin, top1); ; irow1++)
                /* upper bound check */
                if (irow1 > min2(irow2 + smin, bot1)) break;
                else if ((margin1 = firstcol1[irow1]) != mctx->blanksignal) { /*have non-blank row*/
                    /* deltas */
                    int dx = (margin1 + margin2), dy = absval(irow2 - irow1), ds = dx + dy;
                    /* min unchanged */
                    if (ds >= smin) continue;
                    /* dy alone */
                    if (dy > mctx->smashmargin && dx < xmin && smin < 9999) continue;
                    smin = ds;
                    xmin = dx;
                    /* set new min */
                    ymin = dy;
                } /* --- end-of-if(margin1!=mctx->blanksignal) --- */
        } /* --- end-of-if(margin2!=mctx->blanksignal) --- */
        /* can't smash */
        if (smin < 2) goto end_of_job;
    } /* --- end-of-for(irow2) --- */
    /*nsmash = min2(xmin,width2);*/     /* permissible smash */
    /* permissible smash */
    nsmash = xmin;
    /* ------------------------------------------------------------
    Back to caller with #pixels to smash sp1||sp2
    ------------------------------------------------------------ */
end_of_job:
    /* --- debugging output --- */
    if (mctx->msgfp != NULL && mctx->msglevel >= 99) { /* display for debugging */
        fprintf(mctx->msgfp, "rastsmash> nsmash=%d, mctx->smashmargin=%d\n",
                nsmash, mctx->smashmargin);
        if (mctx->msglevel >= 999) {     /* also display rasters */
            fprintf(mctx->msgfp, "rastsmash>left-hand image...\n");
            /* left image */
            if (sp1 != NULL) type_raster(mctx, sp1->image, mctx->msgfp);
            fprintf(mctx->msgfp, "rastsmash>right-hand image...\n");
            if (sp2 != NULL) type_raster(mctx, sp2->image, mctx->msgfp);
        } /* right image */
        fflush(mctx->msgfp);
    }
    /* back with #smash pixels */
    return (nsmash);
} /* --- end-of-function rastsmash() --- */


/* ==========================================================================
 * Function:    rastsmashcheck ( term )
 * Purpose: Check an exponent term to see if its leading symbol
 *      would make smashing dangerous
 * --------------------------------------------------------------------------
 * Arguments:   term (I)    char *  to null-terminated string
 *              containing right-hand exponent term about to
 *              be smashed against existing left-hand.
 * --------------------------------------------------------------------------
 * Returns: ( int )     1 if it's okay to smash term, or
 *              0 if smash is dangerous.
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int rastsmashcheck(mimetex_ctx *mctx, char *term)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* 1 to signal okay to caller */
    int isokay = 0;
    /* don't smash these leading chars */
    static  char nosmashchars[64] = "-.,=";
    /* or leading strings */
    static  char *nosmashstrs[64] = { "\\frac", NULL };
    static  char *grayspace[64] = {
        "\\tiny", "\\small", "\\normalsize",
        "\\large", "\\Large", "\\LARGE", "\\huge", "\\Huge", NULL
    };
    /* local ptr to beginning of expression */
    char    *expression = term;
    char    *token = NULL;
    /* token = nosmashstrs[i] or grayspace[i] */
    int i;
    /* ------------------------------------------------------------
    see if smash check enabled
    ------------------------------------------------------------ */
    if (mctx->smashcheck < 1) {        /* no smash checking wanted */
        if (mctx->smashcheck >= 0)   /* -1 means check should always fail */
            /* otherwise (if 0), signal okay to smash */
            isokay = 1;
        goto end_of_job;
    }        /* return to caller */
    /* ------------------------------------------------------------
    skip leading white and gray space
    ------------------------------------------------------------ */
    /* --- first check input --- */
    /* no input so return 0 to caller */
    if (isempty(term)) goto end_of_job;
    /* --- skip leading white space --- */
    /* skip leading white space */
    skipwhite(term);
    /* nothing but white space */
    if (*term == '\000') goto end_of_job;
    /* --- skip leading gray space --- */
skipgray:
    for (i = 0; (token = grayspace[i]) != NULL; i++) /* check each grayspace */
        if (strncmp(term, token, strlen(token)) == 0) { /* found grayspace */
            /* skip past this grayspace token */
            term += strlen(token);
            /* and skip any subsequent white space */
            skipwhite(term);
            if (*term == '\000') {    /* nothing left so quit */
                if (mctx->msgfp != NULL && mctx->msglevel >= 99) /* display for debugging */
                    fprintf(mctx->msgfp, "rastsmashcheck> only grayspace in %.32s\n", expression);
                goto end_of_job;
            }
            goto skipgray;
        }     /* restart grayspace check from beginning */
    /* ------------------------------------------------------------
    check for leading no-smash single char
    ------------------------------------------------------------ */
    /* --- don't smash if term begins with a "nosmash" char --- */
    if ((token = strchr(nosmashchars, *term)) != NULL) {
        if (mctx->msgfp != NULL && mctx->msglevel >= 99)   /* display for debugging */
            fprintf(mctx->msgfp, "rastsmashcheck> char %.1s found in %.32s\n", token, term);
        goto end_of_job;
    }
    /* ------------------------------------------------------------
    check for leading no-smash token
    ------------------------------------------------------------ */
    for (i = 0; (token = nosmashstrs[i]) != NULL; i++) /* check each nosmashstr */
        if (strncmp(term, token, strlen(token)) == 0) { /* found a nosmashstr */
            if (mctx->msgfp != NULL && mctx->msglevel >= 99)   /* display for debugging */
                fprintf(mctx->msgfp, "rastsmashcheck> token %s found in %.32s\n", token, term);
            goto end_of_job;
        }        /* so don't smash term */
    /* ------------------------------------------------------------
    back to caller
    ------------------------------------------------------------ */
    /* no problem, so signal okay to smash */
    isokay = 1;
end_of_job:
    if (mctx->msgfp != NULL && mctx->msglevel >= 999)  /* display for debugging */
        fprintf(mctx->msgfp, "rastsmashcheck> returning isokay=%d for \"%.32s\"\n",
                isokay, (expression == NULL ? "<no input>" : expression));
    /* back to caller with 1 if okay to smash */
    return (isokay);
} /* --- end-of-function rastsmashcheck() --- */



/* ==========================================================================
 * Function:    new_subraster ( width, height, pixsz )
 * Purpose: Allocate a new subraster along with
 *      an embedded raster of width x height.
 * --------------------------------------------------------------------------
 * Arguments:   width (I)   int containing width of embedded raster
 *      height (I)  int containing height of embedded raster
 *      pixsz (I)   int containing #bits per pixel, 1 or 8
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to newly-allocated subraster,
 *              or NULL for any error.
 * --------------------------------------------------------------------------
 * Notes:     o if width or height <=0, embedded raster not allocated
 * ======================================================================= */
/* --- entry point --- */
subraster *new_subraster(mimetex_ctx *mctx, int width, int height, int pixsz)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* subraster returned to caller */
    subraster *sp = NULL;
    /* image raster embedded in sp */
    raster  *new_raster(), *rp = NULL;
    /* in case new_raster() fails */
    int delete_subraster();
    int size = NORMALSIZE,      /* default size */
               /* and baseline */
               baseline = height - 1;
    /* ------------------------------------------------------------
    allocate and initialize subraster struct
    ------------------------------------------------------------ */
    if (mctx->msgfp != NULL && mctx->msglevel >= 9999) {
        fprintf(mctx->msgfp, "new_subraster(%d,%d,%d)> entry point\n",
                width, height, pixsz);
        fflush(mctx->msgfp);
    }
    /* --- allocate subraster struct --- */
    /* malloc subraster struct */
    sp = (subraster *)malloc(sizeof(subraster));
    if (sp == (subraster *)NULL)         /* malloc failed */
        /* return error to caller */
        goto end_of_job;
    /* --- initialize subraster struct --- */
    /* character or image raster */
    sp->type = NOVALUE;
    /* mathchardef identifying image */
    sp->symdef = (mathchardef *)NULL;
    /*0 if image is entirely descending*/
    sp->baseline = baseline;
    /* font size 0-4 */
    sp->size = size;
    /* upper-left corner of subraster */
    sp->toprow = sp->leftcol = (-1);
    /*ptr to bitmap image of subraster*/
    sp->image = (raster *)NULL;
    /* ------------------------------------------------------------
    allocate raster and embed it in subraster, and return to caller
    ------------------------------------------------------------ */
    /* --- allocate raster struct if desired --- */
    if (width > 0 && height > 0 && pixsz > 0) {  /* caller wants raster */
        if ((rp = new_raster(mctx, width, height, pixsz)) /* allocate embedded raster */
                !=   NULL)              /* if allocate succeeded */
            /* embed raster in subraster */
            sp->image = rp;
        else {              /* or if allocate failed */
            /* free non-unneeded subraster */
            delete_subraster(mctx, sp);
            sp = NULL;
        }
    }          /* signal error */
    /* --- back to caller with new subraster or NULL --- */
end_of_job:
    if (mctx->msgfp != NULL && mctx->msglevel >= 9999) {
        fprintf(mctx->msgfp, "new_subraster(%d,%d,%d)> returning (%s)\n",
                width, height, pixsz, (sp == NULL ? "null ptr" : "success"));
        fflush(mctx->msgfp);
    }
    return (sp);
} /* --- end-of-function new_subraster() --- */



/* ==========================================================================
 * Function:    delete_subraster ( sp )
 * Purpose: Deallocates a subraster (and embedded raster)
 * --------------------------------------------------------------------------
 * Arguments:   sp (I)      ptr to subraster struct to be deleted.
 * --------------------------------------------------------------------------
 * Returns: ( int )     1 if completed successfully,
 *              or 0 otherwise (for any error).
 * --------------------------------------------------------------------------
 * Notes:
 * ======================================================================= */
/* --- entry point --- */
int delete_subraster(mimetex_ctx *mctx, subraster *sp)
{
    /* ------------------------------------------------------------
    free subraster struct
    ------------------------------------------------------------ */
    /* to delete embedded raster */
    if (sp != (subraster *)NULL) {       /* can't free null ptr */
        if (sp->type != CHARASTER)         /* not static character data */
            if (sp->image != NULL)       /*raster allocated within subraster*/
                /* so free embedded raster */
                delete_raster(mctx, sp->image);
        /* and free subraster struct itself*/
        free((void *)sp);
    } /* --- end-of-if(sp!=NULL) --- */
    /* back to caller, 1=okay 0=failed */
    return (1);
} /* --- end-of-function delete_subraster() --- */


/* ==========================================================================
 * Function:    subrastcpy ( sp )
 * Purpose: makes duplicate copy of sp
 * --------------------------------------------------------------------------
 * Arguments:   sp (I)      ptr to subraster struct to be copied
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to new copy sp,
 *              or NULL for any error.
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
subraster *subrastcpy(mimetex_ctx *mctx, subraster *sp)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* allocate new subraster */
    subraster *new_subraster(), *newsp = NULL;
    /* and new raster image within it */
    raster  *rastcpy(), *newrp = NULL;
    /* dealloc newsp if rastcpy() fails*/
    int delete_subraster();
    /* ------------------------------------------------------------
    make copy, and return it to caller
    ------------------------------------------------------------ */
    /* nothing to copy */
    if (sp == NULL) goto end_of_job;
    /* --- allocate new subraster "envelope" for copy --- */
    if ((newsp = new_subraster(mctx, 0, 0, 0)) /* allocate subraster "envelope" */
            /* and quit if we fail to allocate */
            ==   NULL) goto end_of_job;
    /* --- transparently copy original envelope to new one --- */
    /* copy envelope */
    memcpy((void *)newsp, (void *)sp, sizeof(subraster));
    /* --- make a copy of the rasterized image itself, if there is one --- */
    if (sp->image != NULL)       /* there's an image embedded in sp */
        if ((newrp = rastcpy(mctx, sp->image))   /* so copy rasterized image in sp */
                ==   NULL) {              /* failed to copy successfully */
            /* won't need newsp any more */
            delete_subraster(mctx, newsp);
            /* because we're returning error */
            newsp = NULL;
            goto end_of_job;
        }        /* back to caller with error signal*/
    /* --- set new params in new envelope --- */
    /* new raster image we just copied */
    newsp->image = newrp;
    switch (sp->type) {          /* set new raster image type */
    case STRINGRASTER:
    case CHARASTER:
        newsp->type = STRINGRASTER;
        break;
    case ASCIISTRING:
        newsp->type = ASCIISTRING;
        break;
    case FRACRASTER:
        newsp->type = FRACRASTER;
        break;
    case BLANKSIGNAL:
        newsp->type = mctx->blanksignal;
        break;
    case IMAGERASTER:
    default:
        newsp->type = IMAGERASTER;
        break;
    }
    /* --- return copy of sp to caller --- */
end_of_job:
    /* copy back to caller */
    return (newsp);
} /* --- end-of-function subrastcpy() --- */




