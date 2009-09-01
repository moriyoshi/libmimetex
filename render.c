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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mimetex_priv.h"
  
#define REVERSEGAMMA 0.5		/* for \reverse white-on-black */

/* ==========================================================================
 * Function:    accent_subraster ( accent, width, height, pixsz )
 * Purpose: Allocate a new subraster of width x height
 *      (or maybe different dimensions, depending on accent),
 *      and draw an accent (\hat or \vec or \etc) that fills it
 * --------------------------------------------------------------------------
 * Arguments:   accent (I)  int containing either HATACCENT or VECACCENT,
 *              etc, indicating the type of accent desired
 *      width (I)   int containing desired width of accent (#cols)
 *      height (I)  int containing desired height of accent(#rows)
 *      pixsz (I)   int containing 1 for bitmap, 8 for bytemap
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to newly-allocated subraster with accent,
 *              or NULL for any error.
 * --------------------------------------------------------------------------
 * Notes:     o Some accents have internally-determined dimensions,
 *      and caller should check dimensions in returned subraster
 * ======================================================================= */
/* --- entry point --- */
subraster *accent_subraster(mimetex_ctx *mctx, int accent, int width, int height, int pixsz)
{
    /* --- general info --- */
    raster *rp = NULL;
    /* subraster returning accent */
    subraster *sp = NULL;
    /*free allocated raster on err*/
    /* draw solid boxes */
    /* line thickness */
    int thickness = 1;
    /* --- other working info --- */
    int col0, col1,         /* cols for line */
    /* rows for line */
    row0, row1;
    subraster *accsp = NULL;  /*find suitable cmex10 symbol/accent*/
    /* --- info for under/overbraces, tildes, etc --- */
    /*"{" for over, "}" for under, etc*/
    char    brace[16];
    /* set true if width<0 arg passed */
    int iswidthneg = 0;
    /* serif for surd */
    int serifwidth = 0;
    /* ------------------------------------------------------------
    initialization
    ------------------------------------------------------------ */
    if (width < 0) {
        /* set neg width flag */
        width = (-width);
        iswidthneg = 1;
    }
    /* ------------------------------------------------------------
    outer switch() traps accents that may change caller's height,width
    ------------------------------------------------------------ */
    switch (accent) {
    default:
        /* ------------------------------------------------------------
        inner switch() first allocates fixed-size raster for accents that don't
        ------------------------------------------------------------ */
        if ((rp = new_raster(mctx, width, height, pixsz)) /* allocate fixed-size raster */
                !=   NULL)                /* and if we succeeded... */
            switch (accent) {         /* ...draw requested accent in it */
                /* --- unrecognized request --- */
            default:
                /* unrecognized accent requested */
                delete_raster(mctx, rp);
                rp = NULL;
                /* so free raster and signal error */
                break;
                /* --- bar request --- */
            case UNDERBARACCENT:
            case BARACCENT:
                /*height-1;*/
                thickness = 1;/* adjust thickness */
                if (accent == BARACCENT) {   /* bar is above expression */
                    /* row numbers for overbar */
                    row0 = row1 = max2(height - 3, 0);
                    line_raster(mctx, rp, row0, 0, row1, width - 1, thickness);
                } /*blanks at bot*/
                else {              /* underbar is below expression */
                    /* row numbers for underbar */
                    row0 = row1 = min2(2, height - 1);
                    line_raster(mctx, rp, row0, 0, row1, width - 1, thickness);
                } /*blanks at top*/
                break;
                /* --- dot request --- */
            case DOTACCENT:
                /* adjust thickness */
                thickness = height - 1;
                /*line_raster(mctx, rp,0,width/2,1,(width/2)+1,thickness);*//*centered dot*/
                /*box*/
                rule_raster(mctx, rp, 0, (width + 1 - thickness) / 2, thickness, thickness, 3);
                break;
                /* --- ddot request --- */
            case DDOTACCENT:
                /* adjust thickness */
                thickness = height - 1;
                /* one-third of width */
                col0 = max2((width + 1) / 3 - (thickness / 2) - 1, 0);
                col1 = min2((2 * width + 1) / 3 - (thickness / 2) + 1, width - thickness); /*2/3rds*/
                if (col0 + thickness >= col1) {  /* dots overlap */
                    /* try moving left dot more left */
                    col0 = max2(col0 - 1, 0);
                    col1 = min2(col1 + 1, width - thickness);
                } /* and right dot right */
                if (col0 + thickness >= col1)    /* dots _still_ overlap */
                    /* so try reducing thickness */
                    thickness = max2(thickness - 1, 1);
                /*line_raster(mctx, rp,0,col0,1,col0+1,thickness);*//*set dot at 1st third*/
                /*line_raster(mctx, rp,0,col1,1,col1+1,thickness);*//*and another at 2nd*/
                /*box at 1st third*/
                rule_raster(mctx, rp, 0, col0, thickness, thickness, 3);
                /*box at 2nd third*/
                rule_raster(mctx, rp, 0, col1, thickness, thickness, 3);
                break;
                /* --- hat request --- */
            case HATACCENT:
                /*(width<=12? 2 : 3);*/
                thickness = 1;/* adjust thickness */
                line_raster(mctx, rp, height - 1, 0, 0, width / 2, thickness);    /* / part of hat*/
                /* \ part*/
                line_raster(mctx, rp, 0, (width - 1) / 2, height - 1, width - 1, thickness);
                break;
                /* --- sqrt request --- */
            case SQRTACCENT:
                /* leading serif on surd */
                serifwidth = SURDSERIFWIDTH(height);
                /*right col of sqrt*/
                col1 = SQRTWIDTH(height, (iswidthneg ? 1 : 2)) - 1;
                /*col0 = (col1-serifwidth+2)/3;*/ /* midpoint col of sqrt */
                /* midpoint col of sqrt */
                col0 = (col1 - serifwidth + 1) / 2;
                /* midpoint row of sqrt */
                row0 = max2(1, ((height + 1) / 2) - 2);
                /* bottom row of sqrt */
                row1 = height - 1;
                /*line_raster(mctx, rp,row0,0,row1,col0,thickness);*/ /*descending portion*/
                line_raster(mctx, rp, row0 + serifwidth, 0, row0, serifwidth, thickness);
                /* descending */
                line_raster(mctx, rp, row0, serifwidth, row1, col0, thickness);
                /* ascending portion */
                line_raster(mctx, rp, row1, col0, 0, col1, thickness);
                /*overbar of thickness 1*/
                line_raster(mctx, rp, 0, col1, 0, width - 1, thickness);
                break;
            } /* --- end-of-inner-switch(accent) --- */
        /* break from outer accent switch */
        break;
        /* --- underbrace, overbrace request --- */
    case UNDERBRACE:
    case OVERBRACE:
        /* start with } brace */
        if (accent == UNDERBRACE) strcpy(brace, "}");
        /* start with { brace */
        if (accent ==  OVERBRACE) strcpy(brace, "{");
        if ((accsp = get_delim(mctx, brace, width, CMEX10)) /* use width for height */
                !=  NULL) {             /* found desired brace */
            /* rotate 90 degrees clockwise */
            rp = rastrot(mctx, accsp->image);
            delete_subraster(mctx, accsp);
        }  /* and free subraster "envelope" */
        break;
        /* --- hat request --- */
    case HATACCENT:
        /* start with < */
        if (accent == HATACCENT) strcpy(brace, "<");
        if ((accsp = get_delim(mctx, brace, width, CMEX10)) /* use width for height */
                !=  NULL) {             /* found desired brace */
            /* rotate 90 degrees clockwise */
            rp = rastrot(mctx, accsp->image);
            delete_subraster(mctx, accsp);
        }  /* and free subraster "envelope" */
        break;
        /* --- vec request --- */
    case VECACCENT:
        /* force height odd */
        height = 2 * (height / 2) + 1;
        if ((accsp = arrow_subraster(mctx, width, height, pixsz, 1, 0)) /*build rightarrow*/
                !=  NULL) {             /* succeeded */
            /* "extract" raster with bitmap */
            rp = accsp->image;
            free((void *)accsp);
        }    /* and free subraster "envelope" */
        break;
        /* --- tilde request --- */
    case TILDEACCENT:
        accsp = (width < 25 ? get_delim(mctx, "\\sim", -width, CMSY10) :
                 /*width search for tilde*/
                 get_delim(mctx, "~", -width, CMEX10));
        if (accsp !=  NULL)          /* found desired tilde */
            if ((sp = rastack(mctx, new_subraster(mctx, 1, 1, pixsz), accsp, 1, 0, 1, 3))/*space below*/
                    !=  NULL) {           /* have tilde with space below it */
                /* "extract" raster with bitmap */
                rp = sp->image;
                /* and free subraster "envelope" */
                free((void *)sp);
                mctx->leftsymdef = NULL;
            }      /* so \tilde{x}^2 works properly */
        break;
    } /* --- end-of-outer-switch(accent) --- */
    /* ------------------------------------------------------------
    if we constructed accent raster okay, embed it in a subraster and return it
    ------------------------------------------------------------ */
    /* --- if all okay, allocate subraster to contain constructed raster --- */
    if (rp != NULL) {            /* accent raster constructed okay */
        if ((sp = new_subraster(mctx, 0, 0, 0)) /* allocate subraster "envelope" */
                ==   NULL)                /* and if we fail to allocate */
            /* free now-unneeded raster */
            delete_raster(mctx, rp);
        else
        /* subraster allocated okay */
        {
        /* --- init subraster parameters, embedding raster in it --- */
            /* constructed image */
            sp->type = IMAGERASTER;
            /* raster we just constructed */
            sp->image = rp;
            /* can't set font size here */
            sp->size = (-1);
            sp->baseline = 0;
        }       /* can't set baseline here */
    } /* --- end-of-if(rp!=NULL) --- */
    /* --- return subraster containing desired accent to caller --- */
    /* return accent or NULL to caller */
    return (sp);
} /* --- end-of-function accent_subraster() --- */


/* ==========================================================================
 * Function:    arrow_subraster ( width, height, pixsz, drctn, isBig )
 * Purpose: Allocate a raster/subraster and draw left/right arrow in it
 * --------------------------------------------------------------------------
 * Arguments:   width (I)   int containing number of cols for arrow
 *      height (I)  int containing number of rows for arrow
 *      pixsz (I)   int containing 1 for bitmap, 8 for bytemap
 *      drctn (I)   int containing +1 for right arrow,
 *              or -1 for left, 0 for leftright
 *      isBig (I)   int containing 1/true for \Long arrows,
 *              or false for \long arrows, i.e.,
 *              true for ===> or false for --->.
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to constructed left/right arrow
 *              or NULL for any error.
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
subraster *arrow_subraster(mimetex_ctx *mctx, int width, int height, int pixsz,
                           int drctn, int isBig)
{
    subraster *arrowsp = NULL;
    /* index, midrow is arrowhead apex */
    int irow, midrow = height / 2;
    /* arrowhead thickness and index */
    int icol, thickness = (height > 15 ? 2 : 2);
    /* black pixel value */
    int pixval = (pixsz == 1 ? 1 : (pixsz == 8 ? 255 : (-1)));
    int ipix,               /* raster pixmap[] index */
        npix = width * height; /* #pixels malloced in pixmap[] */
    /* ------------------------------------------------------------
    allocate raster/subraster and draw arrow line
    ------------------------------------------------------------ */
    if (height < 3) {
        /* set minimum height */
        height = 3;
        midrow = 1;
    }
    if ((arrowsp = new_subraster(mctx, width, height, pixsz)) /* allocate empty raster */
            /* and quit if failed */
            ==   NULL) goto end_of_job;
    if (!isBig)                      /* single line */
        /*draw line across midrow*/
        rule_raster(mctx, arrowsp->image, midrow, 0, width, 1, 0);
    else {
        int delta = (width > 6 ? (height > 15 ? 3 : (height > 7 ? 2 : 1)) : 1);
        rule_raster(mctx, arrowsp->image, midrow - delta, delta, width - 2*delta, 1, 0);
        rule_raster(mctx, arrowsp->image, midrow + delta, delta, width - 2*delta, 1, 0);
    }
    /* ------------------------------------------------------------
    construct arrowhead(s)
    ------------------------------------------------------------ */
    for (irow = 0; irow < height; irow++) {  /* for each row of arrow */
        /*arrowhead offset for irow*/
        int   delta = abs(irow - midrow);
        /* --- right arrowhead --- */
        if (drctn >= 0) {
            /* right arrowhead wanted */
            for (icol = 0; icol < thickness; icol++) { /* for arrowhead thickness */
                /* rightmost-delta-icol */
                ipix = ((irow + 1) * width - 1) - delta - icol;
                if (ipix >= 0) {                  /* bounds check */
                    if (pixsz == 1)              /* have a bitmap */
                        /*turn on arrowhead bit*/
                        setlongbit((arrowsp->image)->pixmap, ipix);
                    else
                    /* should have a bytemap */
                        if (pixsz == 8)             /* check pixsz for bytemap */
                            ((arrowsp->image)->pixmap)[ipix] = pixval;
                }
            }/*set arrowhead byte*/
        }
        /* --- left arrowhead (same as right except for ipix calculation) --- */
        if (drctn <= 0) {
            /* left arrowhead wanted */
            for (icol = 0; icol < thickness; icol++) { /* for arrowhead thickness */
                /* leftmost bit+delta+icol */
                ipix = irow * width + delta + icol;
                if (ipix < npix) {            /* bounds check */
                    if (pixsz == 1)              /* have a bitmap */
                        /*turn on arrowhead bit*/
                        setlongbit((arrowsp->image)->pixmap, ipix);
                    else
                    /* should have a bytemap */
                        if (pixsz == 8)             /* check pixsz for bytemap */
                            ((arrowsp->image)->pixmap)[ipix] = pixval;
                }
            }/*set arrowhead byte*/
        }
    } /* --- end-of-for(irow) --- */
end_of_job:
    /*back to caller with arrow or NULL*/
    return (arrowsp);
} /* --- end-of-function arrow_subraster() --- */


/* ==========================================================================
 * Function:    uparrow_subraster ( width, height, pixsz, drctn, isBig )
 * Purpose: Allocate a raster/subraster and draw up/down arrow in it
 * --------------------------------------------------------------------------
 * Arguments:   width (I)   int containing number of cols for arrow
 *      height (I)  int containing number of rows for arrow
 *      pixsz (I)   int containing 1 for bitmap, 8 for bytemap
 *      drctn (I)   int containing +1 for up arrow,
 *              or -1 for down, or 0 for updown
 *      isBig (I)   int containing 1/true for \Long arrows,
 *              or false for \long arrows, i.e.,
 *              true for ===> or false for --->.
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to constructed up/down arrow
 *              or NULL for any error.
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
subraster *uparrow_subraster(mimetex_ctx *mctx, int width, int height, int pixsz,
                             int drctn, int isBig)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* allocate arrow subraster */
    subraster *arrowsp = NULL;
    /* index, midcol is arrowhead apex */
    int icol, midcol = width / 2;
    /* arrowhead thickness and index */
    int irow, thickness = (width > 15 ? 2 : 2);
    /* black pixel value */
    int pixval = (pixsz == 1 ? 1 : (pixsz == 8 ? 255 : (-1)));
    int ipix,               /* raster pixmap[] index */
        npix = width * height; /* #pixels malloced in pixmap[] */
    /* ------------------------------------------------------------
    allocate raster/subraster and draw arrow line
    ------------------------------------------------------------ */
    if (width < 3) {
        /* set minimum width */
        width = 3;
        midcol = 1;
    }
    if ((arrowsp = new_subraster(mctx, width, height, pixsz)) /* allocate empty raster */
            /* and quit if failed */
            ==   NULL) goto end_of_job;
    if (!isBig)                      /* single line */
        /*draw line down midcol*/
        rule_raster(mctx, arrowsp->image, 0, midcol, 1, height, 0);
    else {
        int delta = (height > 6 ? (width > 15 ? 3 : (width > 7 ? 2 : 1)) : 1);
        rule_raster(mctx, arrowsp->image, delta, midcol - delta, 1, height - 2*delta, 0);
        rule_raster(mctx, arrowsp->image, delta, midcol + delta, 1, height - 2*delta, 0);
    }
    /* ------------------------------------------------------------
    construct arrowhead(s)
    ------------------------------------------------------------ */
    for (icol = 0; icol < width; icol++) {   /* for each col of arrow */
        /*arrowhead offset for icol*/
        int   delta = abs(icol - midcol);
        /* --- up arrowhead --- */
        if (drctn >= 0)                /* up arrowhead wanted */
            for (irow = 0; irow < thickness; irow++) { /* for arrowhead thickness */
                /* leftmost+icol */
                ipix = (irow + delta) * width + icol;
                if (ipix < npix) {            /* bounds check */
                    if (pixsz == 1)              /* have a bitmap */
                        /*turn on arrowhead bit*/
                        setlongbit((arrowsp->image)->pixmap, ipix);
                    else
                    /* should have a bytemap */
                        if (pixsz == 8)             /* check pixsz for bytemap */
                            ((arrowsp->image)->pixmap)[ipix] = pixval;
                }
            }/*set arrowhead byte*/
        /* --- down arrowhead (same as up except for ipix calculation) --- */
        if (drctn <= 0)                /* down arrowhead wanted */
            for (irow = 0; irow < thickness; irow++) { /* for arrowhead thickness */
                /* leftmost + icol */
                ipix = (height - 1 - delta - irow) * width + icol;
                if (ipix > 0) {           /* bounds check */
                    if (pixsz == 1)              /* have a bitmap */
                        /*turn on arrowhead bit*/
                        setlongbit((arrowsp->image)->pixmap, ipix);
                    else
                    /* should have a bytemap */
                        if (pixsz == 8)             /* check pixsz for bytemap */
                            ((arrowsp->image)->pixmap)[ipix] = pixval;
                }
            }/*set arrowhead byte*/
    } /* --- end-of-for(icol) --- */
end_of_job:
    /*back to caller with arrow or NULL*/
    return (arrowsp);
} /* --- end-of-function uparrow_subraster() --- */


/* ==========================================================================
 * Function:    rule_raster ( rp, top, left, width, height, type )
 * Purpose: Draw a solid or dashed line (or box) in existing raster rp,
 *      starting at top,left with dimensions width,height.
 * --------------------------------------------------------------------------
 * Arguments:   rp (I)      raster *  to raster in which rule
 *              will be drawn
 *      top (I)     int containing row at which top-left corner
 *              of rule starts (0 is topmost)
 *      left (I)    int containing col at which top-left corner
 *              of rule starts (0 is leftmost)
 *      width (I)   int containing number of cols for rule
 *      height (I)  int containing number of rows for rule
 *      type (I)    int containing 0 for solid rule,
 *              1 for horizontal dashes, 2 for vertical
 *              3 for solid rule with corners removed (bevel)
 *              4 for strut (nothing drawn)
 * --------------------------------------------------------------------------
 * Returns: ( int )     1 if rule drawn okay,
 *              or 0 for any error.
 * --------------------------------------------------------------------------
 * Notes:     o Rule line is implicitly "horizontal" or "vertical" depending
 *      on relative width,height dimensions.  It's a box if they're
 *      more or less comparable.
 * ======================================================================= */
/* --- entry point --- */
int rule_raster(mimetex_ctx *mctx, raster *rp, int top, int left,
                int width, int height, int type)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* indexes over rp raster */
    int irow = 0, icol = 0;
    int ipix = 0,       /* raster pixmap[] index */
               /* #pixels malloced in rp->pixmap[] */
               npix = rp->width * rp->height;
    /* true to abend on out-of-bounds error */
    int isfatal = 0;
    int hdash = 1, vdash = 2,   /* type for horizontal, vertical dashes */
        bevel = 99/*3*/, strut = 4; /* type for bevel (turned off), strut */
    int dashlen = 3, spacelen = 2,  /* #pixels for dash followed by space */
        isdraw = 1; /* true when drawing dash (init for solid) */
    /* ------------------------------------------------------------
    Check args
    ------------------------------------------------------------ */
    if (rp == (raster *)NULL) {      /* no raster arg supplied */
        if (mctx->workingbox != (subraster *)NULL)    /* see if we have a mctx->workingbox */
            /* use mctx->workingbox if possible */
            rp = mctx->workingbox->image;
        else return (0);
    }       /* otherwise signal error to caller */
    if (type == bevel)       /* remove corners of solid box */
        /* too small to remove corners */
        if (width < 3 || height < 3) type = 0;
    /* ------------------------------------------------------------
    Fill line/box
    ------------------------------------------------------------ */
    if (width > 0) { /* zero width implies strut*/
        for (irow = top; irow < top + height; irow++) { /* for each scan line */
            /* draw nothing for strut */
            if (type == strut) isdraw = 0;
            if (type == vdash)                 /*set isdraw for vert dash*/
                isdraw = (((irow - top) % (dashlen + spacelen)) < dashlen);
            /*first pixel preceding icol*/
            ipix = irow * rp->width + left - 1;
            for (icol = left; icol < left + width; icol++) { /* each pixel in scan line */
                if (type == bevel) {             /* remove corners of box */
                    if ((irow == top && icol == left)  /* top-left corner */
                            || (irow == top && icol >= left + width - 1)    /* top-right corner */
                            || (irow >= top + height - 1 && icol == left)   /* bottom-left corner */
                            || (irow >= top + height - 1 && icol >= left + width - 1)) /* bottom-right */
                        isdraw = 0;
                    else isdraw = 1;
                }     /*set isdraw to skip corner*/
                if (type == hdash)           /*set isdraw for horiz dash*/
                    isdraw = (((icol - left) % (dashlen + spacelen)) < dashlen);
                if (++ipix >= npix) {
                    /* bounds check failed */
                    if (isfatal)
                        /* abort if error is fatal */
                        goto end_of_job;
                    else
                        /*or just go on to next row*/
                        break;
                } else {
                /*ibit is within rp bounds*/
                    if (isdraw) {                  /*and we're drawing this bit*/
                        if (rp->pixsz == 1)              /* have a bitmap */
                            /* so turn on bit in line */
                            setlongbit(rp->pixmap, ipix);
                        else
                        /* should have a bytemap */
                            if (rp->pixsz == 8)             /* check pixsz for bytemap */
                                ((unsigned char *)(rp->pixmap))[ipix] = 255;
                    } /* set black byte */
                }
            } /* --- end-of-for(icol) --- */
        } /* --- end-of-for(irow) --- */
    }
end_of_job:
    return (isfatal ? (ipix < npix ? 1 : 0) : 1);
} /* --- end-of-function rule_raster() --- */


/* ==========================================================================
 * Function:    line_raster ( rp,  row0, col0,  row1, col1,  thickness )
 * Purpose: Draw a line from row0,col0 to row1,col1 of thickness
 *      in existing raster rp.
 * --------------------------------------------------------------------------
 * Arguments:   rp (I)      raster *  to raster in which a line
 *              will be drawn
 *      row0 (I)    int containing row at which
 *              line will start (0 is topmost)
 *      col0 (I)    int containing col at which
 *              line will start (0 is leftmost)
 *      row1 (I)    int containing row at which
 *              line will end (rp->height-1 is bottom-most)
 *      col1 (I)    int containing col at which
 *              line will end (rp->width-1 is rightmost)
 *      thickness (I)   int containing number of pixels/bits
 *              thick the line will be
 * --------------------------------------------------------------------------
 * Returns: ( int )     1 if line drawn okay,
 *              or 0 for any error.
 * --------------------------------------------------------------------------
 * Notes:     o if row0==row1, a horizontal line is drawn
 *      between col0 and col1, with row0(==row1) the top row
 *      and row0+(thickness-1) the bottom row
 *        o if col0==col1, a vertical bar is drawn
 *      between row0 and row1, with col0(==col1) the left col
 *      and col0+(thickness-1) the right col
 *        o if both the above, you get a square thickness x thickness
 *      whose top-left corner is row0,col0.
 * ======================================================================= */
/* --- entry point --- */
int line_raster(mimetex_ctx *mctx, raster *rp, int row0, int col0,
                int row1, int col1, int thickness)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    int irow = 0, icol = 0, /* indexes over rp raster */
        locol = col0, hicol = col1, /* col limits at irow */
        lorow = row0, hirow = row1; /* row limits at icol */
    /* dimensions of input raster */
    int width = rp->width, height = rp->height;
    int ipix = 0,       /* raster pixmap[] index */
        npix = width * height; /* #pixels malloced in rp->pixmap[] */
    /* true to abend on out-of-bounds error */
    int isfatal = 0;
    /*true if slope a=0,\infty*/
    int isline = (row1 == row0), isbar = (col1 == col0);
    double  dy = row1 - row0 /* + (row1>=row0? +1.0 : -1.0) */, /* delta-x */
            dx = col1 - col0 /* + (col1>=col0? +1.0 : -1.0) */, /* delta-y */
            a = (isbar || isline ? 0.0 : dy / dx), /* slope = tan(theta) = dy/dx */
            xcol = 0, xrow = 0; /* calculated col at irow, or row at icol */
    double  ar = ASPECTRATIO,   /* aspect ratio width/height of one pixel */
            xwidth = (isline ? 0.0 :  /*#pixels per row to get sloped line thcknss*/
                           ((double)thickness) * sqrt((dx * dx) + (dy * dy * ar * ar)) / fabs(dy * ar)),
                          xheight = 1.0;
    /* true to draw line recursively */
    int isrecurse = 1;
    /* ------------------------------------------------------------
    Check args
    ------------------------------------------------------------ */
    if (rp == (raster *)NULL) {      /* no raster arg supplied */
        if (mctx->workingbox != (subraster *)NULL)    /* see if we have a mctx->workingbox */
            /* use mctx->workingbox if possible */
            rp = mctx->workingbox->image;
        else return (0);
    }       /* otherwise signal error to caller */
    /* ------------------------------------------------------------
    Initialization
    ------------------------------------------------------------ */
    if (mctx->msgfp != NULL && mctx->msglevel >= 29) {   /* debugging */
        fprintf(mctx->msgfp, "line_raster> row,col0=%d,%d row,col1=%d,%d, thickness=%d\n"
                "\t dy,dx=%3.1f,%3.1f, a=%4.3f, xwidth=%4.3f\n",
                row0, col0, row1, col1, thickness,  dy, dx, a, xwidth);
        fflush(mctx->msgfp);
    }
    /* --- check for recursive line drawing --- */
    if (isrecurse) {         /* drawing lines recursively */
        for (irow = 0; irow < thickness; irow++) {  /* each line 1 pixel thick */
            double xrow0 = (double)row0, xcol0 = (double)col0,
                                                 xrow1 = (double)row1, xcol1 = (double)col1;
            if (isline) xrow0 = xrow1 = (double)(row0 + irow);
            else if (isbar) xcol0 = xcol1 = (double)(col0 + irow);
            if (xrow0 > (-0.001) && xcol0 > (-0.001) /*check line inside raster*/
                    &&  xrow1 < ((double)(height - 1) + 0.001) && xcol1 < ((double)(width - 1) + 0.001))
                line_recurse(mctx, rp, xrow0, xcol0, xrow1, xcol1, thickness);
        }
        return (1);
    }
    /* --- set params for horizontal line or vertical bar --- */
    if (isline)                      /*interpret row as top row*/
        /* set bottom row for line */
        row1 = row0 + (thickness - 1);
    if (0 && isbar)                  /*interpret col as left col*/
        /* set right col for bar */
        hicol = col0 + (thickness - 1);
    /* ------------------------------------------------------------
    draw line one row at a time
    ------------------------------------------------------------ */
    for (irow = min2(row0, row1); irow <= max2(row0, row1); irow++) { /*each scan line*/
        if (!isbar && !isline) {           /* neither vert nor horiz */
            /* "middle" col in irow */
            xcol  = col0 + ((double)(irow - row0)) / a;
            /* leftmost col */
            locol = max2((int)(xcol - 0.5 * (xwidth - 1.0)), 0);
            hicol = min2((int)(xcol + 0.5 * (xwidth - 0.0)), max2(col0, col1));
        } /*right*/
        if (mctx->msgfp != NULL && mctx->msglevel >= 29)   /* debugging */
            fprintf(mctx->msgfp, "\t irow=%d, xcol=%4.2f, lo,hicol=%d,%d\n",
                    irow, xcol, locol, hicol);
        /*first pix preceding icol*/
        ipix = irow * rp->width + min2(locol, hicol) - 1;
        for (icol = min2(locol, hicol); icol <= max2(locol, hicol); icol++) /*each pix*/
            if (++ipix >= npix)              /* bounds check failed */
                /* abort if error is fatal */
                if (isfatal) goto end_of_job;
                /*or just go on to next row*/
                else break;
            else
            /* turn on pixel in line */
                if (rp->pixsz == 1)              /* have a pixel bitmap */
                    /* so turn on bit in line */
                    setlongbit(rp->pixmap, ipix);
                else
                /* should have a bytemap */
                    if (rp->pixsz == 8)             /* check pixsz for bytemap */
                        /* set black byte */
                        ((unsigned char *)(rp->pixmap))[ipix] = 255;
    } /* --- end-of-for(irow) --- */
    /* ------------------------------------------------------------
    now _redraw_ line one col at a time to avoid "gaps"
    ------------------------------------------------------------ */
    if (1)
        for (icol = min2(col0, col1); icol <= max2(col0, col1); icol++) { /*each scan line*/
            if (!isbar && !isline) {           /* neither vert nor horiz */
                /* "middle" row in icol */
                xrow  = row0 + ((double)(icol - col0)) * a;
                /* topmost row */
                lorow = max2((int)(xrow - 0.5 * (xheight - 1.0)), 0);
                hirow = min2((int)(xrow + 0.5 * (xheight - 0.0)), max2(row0, row1));
            } /*bot*/
            if (mctx->msgfp != NULL && mctx->msglevel >= 29)   /* debugging */
                fprintf(mctx->msgfp, "\t icol=%d, xrow=%4.2f, lo,hirow=%d,%d\n",
                        icol, xrow, lorow, hirow);
            /*first pix preceding icol*/
            ipix = irow * rp->width + min2(locol, hicol) - 1;
            for (irow = min2(lorow, hirow); irow <= max2(lorow, hirow); irow++) /*each pix*/
                if (irow < 0 || irow >= rp->height
                        ||   icol < 0 || icol >= rp->width)     /* bounds check */
                    /* abort if error is fatal */
                    if (isfatal) goto end_of_job;
                    /*or just go on to next row*/
                    else continue;
                else
                    /* set pixel at irow,icol */
                    setpixel(rp, irow, icol, 255);
        } /* --- end-of-for(irow) --- */
    /* ------------------------------------------------------------
    Back to caller with 1=okay, 0=failed.
    ------------------------------------------------------------ */
end_of_job:
    return (isfatal ? (ipix < npix ? 1 : 0) : 1);
} /* --- end-of-function line_raster(mctx, ) --- */


/* ==========================================================================
 * Function:    line_recurse ( rp,  row0, col0,  row1, col1,  thickness )
 * Purpose: Draw a line from row0,col0 to row1,col1 of thickness
 *      in existing raster rp.
 * --------------------------------------------------------------------------
 * Arguments:   rp (I)      raster *  to raster in which a line
 *              will be drawn
 *      row0 (I)    double containing row at which
 *              line will start (0 is topmost)
 *      col0 (I)    double containing col at which
 *              line will start (0 is leftmost)
 *      row1 (I)    double containing row at which
 *              line will end (rp->height-1 is bottom-most)
 *      col1 (I)    double containing col at which
 *              line will end (rp->width-1 is rightmost)
 *      thickness (I)   int containing number of pixels/bits
 *              thick the line will be
 * --------------------------------------------------------------------------
 * Returns: ( int )     1 if line drawn okay,
 *              or 0 for any error.
 * --------------------------------------------------------------------------
 * Notes:     o Recurses, drawing left- and right-halves of line
 *      until a horizontal or vertical segment is found
 * ======================================================================= */
/* --- entry point --- */
int line_recurse(mimetex_ctx *mctx, raster *rp, double row0, double col0,
                 double row1, double col1, int thickness)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    double  delrow = fabs(row1 - row0), /* 0 if line horizontal */
                     delcol = fabs(col1 - col0), /* 0 if line vertical */
                              /* draw line when it goes to point */
                              tolerance = 0.5;
    double  midrow = 0.5 * (row0 + row1),   /* midpoint row */
                     /* midpoint col */
                     midcol = 0.5 * (col0 + col1);
    /* ------------------------------------------------------------
    recurse if either delta > tolerance
    ------------------------------------------------------------ */
    if (delrow > tolerance           /* row hasn't converged */
            ||   delcol > tolerance) {      /* col hasn't converged */
        /* left half */
        line_recurse(mctx, rp, row0, col0, midrow, midcol, thickness);
        /* right half */
        line_recurse(mctx, rp, midrow, midcol, row1, col1, thickness);
        return (1);
    }
    /* ------------------------------------------------------------
    draw converged point
    ------------------------------------------------------------ */
    /*set pixel at midrow,midcol*/
    setpixel(rp, iround(midrow), iround(midcol), 255);
    return (1);
} /* --- end-of-function line_recurse() --- */


/* ==========================================================================
 * Function:    circle_raster ( rp,  row0, col0,  row1, col1,
 *      thickness, quads )
 * Purpose: Draw quad(rant)s of an ellipse in box determined by
 *      diagonally opposite corner points (row0,col0) and
 *      (row1,col1), of thickness pixels in existing raster rp.
 * --------------------------------------------------------------------------
 * Arguments:   rp (I)      raster *  to raster in which an ellipse
 *              will be drawn
 *      row0 (I)    int containing 1st corner row bounding ellipse
 *              (0 is topmost)
 *      col0 (I)    int containing 1st corner col bounding ellipse
 *              (0 is leftmost)
 *      row1 (I)    int containing 2nd corner row bounding ellipse
 *              (rp->height-1 is bottom-most)
 *      col1 (I)    int containing 2nd corner col bounding ellipse
 *              (rp->width-1 is rightmost)
 *      thickness (I)   int containing number of pixels/bits
 *              thick the ellipse arc line will be
 *      quads (I)   char * to null-terminated string containing
 *              any subset/combination of "1234" specifying
 *              which quadrant(s) of ellipse to draw.
 *              NULL ptr draws all four quadrants;
 *              otherwise 1=upper-right quadrant,
 *              2=uper-left, 3=lower-left, 4=lower-right,
 *              i.e., counterclockwise from 1=positive quad.
 * --------------------------------------------------------------------------
 * Returns: ( int )     1 if ellipse drawn okay,
 *              or 0 for any error.
 * --------------------------------------------------------------------------
 * Notes:     o row0==row1 or col0==col1 are errors
 *        o using ellipse equation x^2/a^2 + y^2/b^2 = 1
 * ======================================================================= */
/* --- entry point --- */
int circle_raster(mimetex_ctx *mctx, raster *rp, int row0, int col0,
                  int row1, int col1, int thickness, char *quads)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* --- lower-left and upper-right bounding points (in our coords) --- */
    int lorow = min2(row0, row1),   /* lower bounding row (top of box) */
        locol = min2(col0, col1),   /* lower bounding col (left of box)*/
        hirow = max2(row0, row1),   /* upper bounding row (bot of box) */
        hicol = max2(col0, col1); /* upper bounding col (right of box)*/
    /* --- a and b ellipse params --- */
    int width = hicol - locol + 1,  /* width of bounding box */
        height = hirow - lorow + 1, /* height of bounding box */
        islandscape = (width >= height ? 1 : 0); /*true if ellipse lying on side*/
    double a = ((double)width) / 2.0,  /* x=a when y=0 */
           b = ((double)height) / 2.0, /* y=b when x=0 */
           abmajor = (islandscape ? a : b), /* max2(a,b) */
           abminor = (islandscape ? b : a), /* min2(a,b) */
           abmajor2 = abmajor * abmajor, /* abmajor^2 */
           abminor2 = abminor * abminor; /* abminor^2 */
    /* --- other stuff --- */
    int imajor = 0, nmajor = max2(width, height), /*index, #pixels on major axis*/
        iminor = 0, nminor = min2(width, height); /* solved index on minor axis */
    int irow, icol,         /* raster indexes at circumference */
    /* row,col signs, both +1 in quad 1*/
    rsign = 1, csign = 1;
    double  midrow = ((double)(row0 + row1)) / 2.0, /* center row */
            midcol = ((double)(col0 + col1)) / 2.0; /* center col */
    double  xy, xy2,            /* major axis ellipse coord */
    /* solved minor ellipse coord */
    yx2, yx;
    /* true if no pixels out-of-bounds */
    int isokay = 1;
    /* quadrants if input quads==NULL */
    char    *qptr = NULL, *allquads = "1234";
    /* true to draw ellipse recursively*/
    int isrecurse = 1;
    /* ------------------------------------------------------------
    pixel-by-pixel along positive major axis, quit when it goes negative
    ------------------------------------------------------------ */
    /* draw all quads, or only user's */
    if (quads == NULL) quads = allquads;
    if (mctx->msgfp != NULL && mctx->msglevel >= 39) /* debugging */
        fprintf(mctx->msgfp, "circle_raster> width,height;quads=%d,%d,%s\n",
                width, height, quads);
    /* problem with input args */
    if (nmajor < 1) isokay = 0;
    else {
        if (isrecurse) {            /* use recursive algorithm */
            for (qptr = quads; *qptr != '\000'; qptr++) { /* for each character in quads */
                /* set thetas based on quadrant */
                double theta0 = 0.0, theta1 = 0.0;
                switch (*qptr) {          /* check for quadrant 1,2,3,4 */
                default:
                /* unrecognized, assume quadrant 1 */
                case '1':
                    theta0 =  0.0;
                    theta1 = 90.0;
                    /* first quadrant */
                    break;
                case '2':
                    theta0 = 90.0;
                    theta1 = 180.0;
                    /* second quadrant */
                    break;
                case '3':
                    theta0 = 180.0;
                    theta1 = 270.0;
                    /* third quadrant */
                    break;
                case '4':
                    theta0 = 270.0;
                    theta1 = 360.0;
                    break;
                } /* fourth quadrant */
                circle_recurse(mctx, rp, row0, col0, row1, col1, thickness, theta0, theta1);
            } /* --- end-of-for(qptr) --- */
            return (1);
        } /* --- end-of-if(isrecurse) --- */
        for (imajor = (nmajor + 1) / 2; ; imajor--) {
            /* --- xy is coord along major axis, yx is "solved" along minor axis --- */
            /* xy = abmajor ... 0 */
            xy  = ((double)imajor);
            /* negative side symmetrical */
            if (xy < 0.0) break;
            /* "solve" ellipse equation */
            yx2 = abminor2 * (1.0 - xy * xy / abmajor2);
            /* take sqrt if possible */
            yx  = (yx2 > 0.0 ? sqrt(yx2) : 0.0);
            /* nearest integer */
            iminor = iround(yx);
            /* --- set pixels for each requested quadrant --- */
            for (qptr = quads; *qptr != '\000'; qptr++) { /* for each character in quads */
                rsign = (-1);
                /* init row,col in user quadrant 1 */
                csign = 1;
                switch (*qptr) {          /* check for quadrant 1,2,3,4 */
                default:
                    /* unrecognized, assume quadrant 1 */
                    break;
                case '4':
                    rsign = 1;
                    /* row,col both pos in quadrant 4 */
                    break;
                case '3':
                    /* row pos, col neg in quadrant 3 */
                    rsign = 1;
                case '2':
                    csign = (-1);
                    break;
                }  /* row,col both neg in quadrant 2 */
                irow = iround(midrow + (double)rsign * (islandscape ? yx : xy));
                /* keep irow in bounds */
                irow = min2(hirow, max2(lorow, irow));
                icol = iround(midcol + (double)csign * (islandscape ? xy : yx));
                /* keep icol in bounds */
                icol = min2(hicol, max2(locol, icol));
                if (mctx->msgfp != NULL && mctx->msglevel >= 49)  /* debugging */
                    fprintf(mctx->msgfp, "\t...imajor=%d; iminor,quad,irow,icol=%d,%c,%d,%d\n",
                            imajor, iminor, *qptr, irow, icol);
                if (irow < 0 || irow >= rp->height   /* row outside raster */
                        ||   icol < 0 || icol >= rp->width) { /* col outside raster */
                    /* signal out-of-bounds pixel */
                    isokay = 0;
                    continue;
                }         /* but still try remaining points */
                /* set pixel at irow,icol */
                setpixel(rp, irow, icol, 255);
            } /* --- end-of-for(qptr) --- */
        } /* --- end-of-for(imajor) --- */
        /* ------------------------------------------------------------
        now do it _again_ along minor axis to avoid "gaps"
        ------------------------------------------------------------ */
        if (1 && iminor > 0)
            for (iminor = (nminor + 1) / 2; ; iminor--) {
                /* --- yx is coord along minor axis, xy is "solved" along major axis --- */
                /* yx = abminor ... 0 */
                yx  = ((double)iminor);
                /* negative side symmetrical */
                if (yx < 0.0) break;
                /* "solve" ellipse equation */
                xy2 = abmajor2 * (1.0 - yx * yx / abminor2);
                /* take sqrt if possible */
                xy  = (xy2 > 0.0 ? sqrt(xy2) : 0.0);
                /* nearest integer */
                imajor = iround(xy);
                /* --- set pixels for each requested quadrant --- */
                for (qptr = quads; *qptr != '\000'; qptr++) { /* for each character in quads */
                    rsign = (-1);
                    /* init row,col in user quadrant 1 */
                    csign = 1;
                    switch (*qptr) {         /* check for quadrant 1,2,3,4 */
                    default:
                        /* unrecognized, assume quadrant 1 */
                        break;
                    case '4':
                        rsign = 1;
                        /* row,col both pos in quadrant 4 */
                        break;
                    case '3':
                        /* row pos, col neg in quadrant 3 */
                        rsign = 1;
                    case '2':
                        csign = (-1);
                        break;
                    } /* row,col both neg in quadrant 2 */
                    irow = iround(midrow + (double)rsign * (islandscape ? yx : xy));
                    /* keep irow in bounds */
                    irow = min2(hirow, max2(lorow, irow));
                    icol = iround(midcol + (double)csign * (islandscape ? xy : yx));
                    /* keep icol in bounds */
                    icol = min2(hicol, max2(locol, icol));
                    if (mctx->msgfp != NULL && mctx->msglevel >= 49) /* debugging */
                        fprintf(mctx->msgfp, "\t...iminor=%d; imajor,quad,irow,icol=%d,%c,%d,%d\n",
                                iminor, imajor, *qptr, irow, icol);
                    if (irow < 0 || irow >= rp->height  /* row outside raster */
                            ||   icol < 0 || icol >= rp->width) { /* col outside raster */
                        /* signal out-of-bounds pixel */
                        isokay = 0;
                        continue;
                    }         /* but still try remaining points */
                    /* set pixel at irow,icol */
                    setpixel(rp, irow, icol, 255);
                } /* --- end-of-for(qptr) --- */
            } /* --- end-of-for(iminor) --- */
    } /* --- end-of-if/else(nmajor<1) --- */
    return (isokay);
} /* --- end-of-function circle_raster() --- */


/* ==========================================================================
 * Function:    circle_recurse ( rp,  row0, col0,  row1, col1,
 *      thickness, theta0, theta1 )
 * Purpose: Recursively draws arc theta0<=theta<=theta1 of the ellipse
 *      in box determined by diagonally opposite corner points
 *      (row0,col0) and (row1,col1), of thickness pixels in raster rp.
 * --------------------------------------------------------------------------
 * Arguments:   rp (I)      raster *  to raster in which an ellipse
 *              will be drawn
 *      row0 (I)    int containing 1st corner row bounding ellipse
 *              (0 is topmost)
 *      col0 (I)    int containing 1st corner col bounding ellipse
 *              (0 is leftmost)
 *      row1 (I)    int containing 2nd corner row bounding ellipse
 *              (rp->height-1 is bottom-most)
 *      col1 (I)    int containing 2nd corner col bounding ellipse
 *              (rp->width-1 is rightmost)
 *      thickness (I)   int containing number of pixels/bits
 *              thick the ellipse arc line will be
 *      theta0 (I)  double containing first angle -360 -> +360
 *      theta1 (I)  double containing second angle -360 -> +360
 *              0=x-axis, positive moving counterclockwise
 * --------------------------------------------------------------------------
 * Returns: ( int )     1 if ellipse drawn okay,
 *              or 0 for any error.
 * --------------------------------------------------------------------------
 * Notes:     o row0==row1 or col0==col1 are errors
 *        o using ellipse equation x^2/a^2 + y^2/b^2 = 1
 *      Then, with x=r*cos(theta), y=r*sin(theta), ellipse
 *      equation is r = ab/sqrt(a^2*sin^2(theta)+b^2*cos^2(theta))
 * ======================================================================= */
/* --- entry point --- */
int circle_recurse(mimetex_ctx *mctx, raster *rp, int row0, int col0,
                   int row1, int col1, int thickness, double theta0, double theta1)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* --- lower-left and upper-right bounding points (in our coords) --- */
    int lorow = min2(row0, row1),   /* lower bounding row (top of box) */
        locol = min2(col0, col1),   /* lower bounding col (left of box)*/
        hirow = max2(row0, row1),   /* upper bounding row (bot of box) */
        hicol = max2(col0, col1); /* upper bounding col (right of box)*/
    /* --- a and b ellipse params --- */
    int width = hicol - locol + 1,  /* width of bounding box */
        height = hirow - lorow + 1; /* height of bounding box */
    double  a = ((double)width) / 2.0,  /* col x=a when row y=0 */
            b = ((double)height) / 2.0, /* row y=b when col x=0 */
            ab = a * b, a2 = a * a, b2 = b * b; /* product and squares */
    /* --- arc parameters --- */
    double  rads = 0.017453292,     /* radians per degree = 1/57.29578 */
            lotheta = rads * dmod(min2(theta0, theta1), 360), /* smaller angle */
            hitheta = rads * dmod(max2(theta0, theta1), 360), /* larger angle */
            locos = cos(lotheta), losin = sin(lotheta), /* trigs for lotheta */
            hicos = cos(hitheta), hisin = sin(hitheta), /* trigs for hitheta */
            rlo = ab / sqrt(b2 * locos * locos + a2 * losin * losin), /* r for lotheta */
            rhi = ab / sqrt(b2 * hicos * hicos + a2 * hisin * hisin), /* r for hitheta */
            xlo = rlo * locos, ylo = rlo * losin,   /*col,row pixel coords for lotheta*/
            xhi = rhi * hicos, yhi = rhi * hisin,   /*col,row pixel coords for hitheta*/
            xdelta = fabs(xhi - xlo), ydelta = fabs(yhi - ylo), /* col,row deltas */
            tolerance = 0.5; /* convergence tolerance */
    /* ------------------------------------------------------------
    recurse if either delta > tolerance
    ------------------------------------------------------------ */
    if (ydelta > tolerance           /* row hasn't converged */
            ||   xdelta > tolerance) {      /* col hasn't converged */
        /* mid angle for arc */
        double midtheta = 0.5 * (theta0 + theta1);
        /*lo*/
        circle_recurse(mctx, rp, row0, col0, row1, col1, thickness, theta0, midtheta);
        circle_recurse(mctx, rp, row0, col0, row1, col1, thickness, midtheta, theta1);
    }/*hi*/
    /* ------------------------------------------------------------
    draw converged point
    ------------------------------------------------------------ */
    else {
        double xcol = 0.5 * (xlo + xhi), yrow = 0.5 * (ylo + yhi),    /* relative to center*/
               centerrow = 0.5 * ((double)(lorow + hirow)),  /* ellipse y-center */
               centercol = 0.5 * ((double)(locol + hicol)),  /* ellipse x-center */
               midrow = centerrow - yrow, midcol = centercol + xcol; /* pixel coords */
        setpixel(rp, iround(midrow), iround(midcol), 255);
    } /* set midrow,midcol */
    return (1);
} /* --- end-of-function  --- */


/* ==========================================================================
 * Function:    bezier_raster ( rp, r0,c0, r1,c1, rt,ct )
 * Purpose: Recursively draw bezier from r0,c0 to r1,c1
 *      (with tangent point rt,ct) in existing raster rp.
 * --------------------------------------------------------------------------
 * Arguments:   rp (I)      raster *  to raster in which a line
 *              will be drawn
 *      r0 (I)      double containing row at which
 *              bezier will start (0 is topmost)
 *      c0 (I)      double containing col at which
 *              bezier will start (0 is leftmost)
 *      r1 (I)      double containing row at which
 *              bezier will end (rp->height-1 is bottom-most)
 *      c1 (I)      double containing col at which
 *              bezier will end (rp->width-1 is rightmost)
 *      rt (I)      double containing row for tangent point
 *      ct (I)      double containing col for tangent point
 * --------------------------------------------------------------------------
 * Returns: ( int )     1 if line drawn okay,
 *              or 0 for any error.
 * --------------------------------------------------------------------------
 * Notes:     o Recurses, drawing left- and right-halves of bezier curve
 *      until a point is found
 * ======================================================================= */
/* --- entry point --- */
int bezier_raster(mimetex_ctx *mctx, raster *rp, double r0, double c0,
                  double r1, double c1, double rt, double ct)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    double  delrow = fabs(r1 - r0),     /* 0 if same row */
            delcol = fabs(c1 - c0),     /* 0 if same col */
            tolerance = 0.5; /* draw curve when it goes to point*/
    double  midrow = 0.5 * (r0 + r1),   /* midpoint row */
            midcol = 0.5 * (c0 + c1); /* midpoint col */
    /* point to be drawn */
    int irow = 0, icol = 0;
    /* return status */
    int status = 1;
    /* ------------------------------------------------------------
    recurse if either delta > tolerance
    ------------------------------------------------------------ */
    if (delrow > tolerance           /* row hasn't converged */
            ||   delcol > tolerance) {      /* col hasn't converged */
        bezier_raster(mctx, rp, r0, c0,       /* left half */
                      0.5*(rt + midrow), 0.5*(ct + midcol),
                      0.5*(r0 + rt), 0.5*(c0 + ct));
        bezier_raster(mctx, rp, 0.5*(rt + midrow), 0.5*(ct + midcol), /* right half */
                      r1, c1,
                      0.5*(r1 + rt), 0.5*(c1 + ct));
        return (1);
    }
    /* ------------------------------------------------------------
    draw converged point
    ------------------------------------------------------------ */
    /* --- get integer point --- */
    /* row pixel coord */
    irow = iround(midrow);
    /* col pixel coord */
    icol = iround(midcol);
    /* --- bounds check --- */
    if (irow >= 0 && irow < rp->height   /* row in bounds */
            &&   icol >= 0 && icol < rp->width) /* col in bounds */
        /* so set pixel at irow,icol*/
        setpixel(rp, irow, icol, 255);
    /* bad status if out-of-bounds */
    else    status = 0;
    return (status);
} /* --- end-of-function  --- */


/* ==========================================================================
 * Function:    border_raster ( rp, ntop, nbot, isline, isfree )
 * Purpose: Allocate a new raster containing a copy of input rp,
 *      along with ntop extra rows at top and nbot at bottom,
 *      and whose width is either adjusted correspondingly,
 *      or is automatically enlarged to a multiple of 8
 *      with original bitmap centered
 * --------------------------------------------------------------------------
 * Arguments:   rp (I)      raster *  to raster on which a border
 *              is to be placed
 *      ntop (I)    int containing number extra rows at top.
 *              if negative, abs(ntop) used, and same
 *              number of extra cols added at left.
 *      nbot (I)    int containing number extra rows at bottom.
 *              if negative, abs(nbot) used, and same
 *              number of extra cols added at right.
 *      isline (I)  int containing 0 to leave border pixels clear
 *              or >0 to draw a line around border of width
 *              isline.
 *      isfree (I)  int containing true to free rp before return
 * --------------------------------------------------------------------------
 * Returns: ( raster * )    ptr to bordered raster,
 *              or NULL for any error.
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
raster  *border_raster(mimetex_ctx *mctx, raster *rp, int ntop, int nbot,
                       int isline, int isfree)
{
    /*raster back to caller*/
    raster  *bp = (raster *)NULL;
    /* overlay rp in new bordered raster */
    int width  = (rp == NULL ? 0 : rp->width),  /* width of raster */
        height = (rp == NULL ? 0 : rp->height), /* height of raster */
        istopneg = 0, isbotneg = 0, /* true if ntop or nbot negative */
        leftmargin = 0; /* adjust width to whole number of bytes */
    /* ------------------------------------------------------------
    Initialization
    ------------------------------------------------------------ */
    /* no input raster provided */
    if (rp == NULL) goto end_of_job;
    if (mctx->isstring || (1 && rp->height == 1)) { /* explicit string signal or infer */
        /* return ascii string unchanged */
        bp = rp;
        goto end_of_job;
    }
    /* --- check for negative args --- */
    if (ntop < 0) {
        /*flip positive and set flag*/
        ntop = -ntop;
        istopneg = 1;
    }
    if (nbot < 0) {
        /*flip positive and set flag*/
        nbot = -nbot;
        isbotneg = 1;
    }
    /* --- adjust height for ntop and nbot margins --- */
    /* adjust height for margins */
    height += (ntop + nbot);
    /* --- adjust width for left and right margins --- */
    if (istopneg || isbotneg)    /*caller wants nleft=ntop and/or nright=nbot*/
    {
    /* --- adjust width (and leftmargin) as requested by caller -- */
        if (istopneg) {
            width += ntop;
            leftmargin = ntop;
        }
        if (isbotneg)   width += nbot;
    } else { /* --- or adjust width (and leftmargin) to whole number of bytes --- */
        /*makes width multiple of 8*/
        leftmargin = (width % 8 == 0 ? 0 : 8 - (width % 8));
        /* width now multiple of 8 */
        width += leftmargin;
        leftmargin /= 2;
    }          /* center original raster */
    /* ------------------------------------------------------------
    allocate bordered raster, and embed rp within it
    ------------------------------------------------------------ */
    /* --- allocate bordered raster --- */
    if ((bp = new_raster(mctx, width, height, rp->pixsz))  /*allocate bordered raster*/
            /* and quit if failed */
            == (raster *)NULL) goto end_of_job;
    /* --- embed rp in it --- */
    /* rp embedded in bp */
    rastput(mctx, bp, rp, ntop, leftmargin, 1);
    /* ------------------------------------------------------------
    draw border if requested
    ------------------------------------------------------------ */
    if (isline) {
        /*height,width index, line thickness*/
        int  irow, icol, nthick = isline;
        /* --- draw left- and right-borders --- */
        for (irow = 0; irow < height; irow++)  /* for each row of bp */
            for (icol = 0; icol < nthick; icol++) { /* and each pixel of thickness */
                /* left border */
                setpixel(bp, irow, icol, 255);
                setpixel(bp, irow, width - 1 - icol, 255);
            } /* right border */
        /* --- draw top- and bottom-borders --- */
        for (icol = 0; icol < width; icol++) /* for each col of bp */
            for (irow = 0; irow < nthick; irow++) { /* and each pixel of thickness */
                /* top border */
                setpixel(bp, irow, icol, 255);
                setpixel(bp, height - 1 - irow, icol, 255);
            } /* bottom border */
    } /* --- end-of-if(isline) --- */
    /* ------------------------------------------------------------
    free rp if no longer needed
    ------------------------------------------------------------ */
    if (isfree)                      /*caller no longer needs rp*/
        /* so free it for him */
        delete_raster(mctx, rp);
    /* ------------------------------------------------------------
    Back to caller with bordered raster (or null for any error)
    ------------------------------------------------------------ */
end_of_job:
    /* back with bordered or null ptr */
    return (bp);
} /* --- end-of-function border_raster() --- */


/* ==========================================================================
 * Function:    backspace_raster ( rp, nback, pback, minspace, isfree )
 * Purpose: Allocate a new raster containing a copy of input rp,
 *      but with trailing nback columns removed.
 *      If minspace>=0 then (at least) that many columns
 *      of whitespace will be left in place, regardless of nback.
 *      If minspace<0 then existing black pixels will be deleted
 *      as required.
 * --------------------------------------------------------------------------
 * Arguments:   rp (I)      raster *  to raster on which a border
 *              is to be placed
 *      nback (I)   int containing number of columns to
 *              backspace (>=0)
 *      pback (O)   ptr to int returning #pixels actually
 *              backspaced (or NULL to not use)
 *      minspace (I)    int containing number of columns
 *              of whitespace to be left in place
 *      isfree (I)  int containing true to free rp before return
 * --------------------------------------------------------------------------
 * Returns: ( raster * )    ptr to backspaced raster,
 *              or NULL for any error.
 * --------------------------------------------------------------------------
 * Notes:     o For \! negative space, for \hspace{-10}, etc.
 * ======================================================================= */
/* --- entry point --- */
raster  *backspace_raster(mimetex_ctx *mctx, raster *rp, int nback, int *pback, int minspace,
                          int isfree)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* raster returned to caller */
    raster  *bp = (raster *)NULL;
    /* free input rp if isfree is true */
    int width  = (rp == NULL ? 0 : rp->width),  /* original width of raster */
        height = (rp == NULL ? 0 : rp->height), /* height of raster */
        mback = nback,          /* nback adjusted for minspace */
        newwidth = width,       /* adjusted width after backspace */
        icol = 0, irow = 0; /* col,row index */
    /* no input given */
    if (rp == NULL) goto end_of_job;
    /* ------------------------------------------------------------
    locate rightmost column of rp containing ink, and determine backspaced width
    ------------------------------------------------------------ */
    /* --- locate rightmost column of rp containing ink --- */
    if (minspace >= 0) {
        /* only needed if given minspace */
        for (icol = width - 1; icol >= 0; icol--) {
            /* find first non-empty col in row */
            for (irow = 0; irow < height; irow++) {
                /* for each row inside rp */
                if (getpixel(rp, irow, icol) != 0) {  /* found a set pixel */
                    /* #cols containing white space */
                    int whitecols = (width - 1) - icol;
                    /*leave minspace cols*/
                    mback = min2(nback, max2(0, whitecols - minspace));
                    goto gotright;
                }           /* no need to look further */
            }
        }
    }
    /* --- determine width of new backspaced raster --- */
gotright:
/* found col with ink (or rp empty)*/
    /* can't backspace before beginning*/
    if (mback > width) mback = width;
    /* #cols in backspaced raster */
    newwidth = max2(1, width - mback);
    /* caller wants #pixels */
    if (pback != NULL) *pback = width - newwidth;
    /* ------------------------------------------------------------
    allocate new raster and fill it with leftmost cols of rp
    ------------------------------------------------------------ */
    /* --- allocate backspaced raster --- */
    if ((bp = new_raster(mctx, newwidth, height, rp->pixsz)) /*allocate backspaced raster*/
            /* and quit if failed */
            == (raster *)NULL) goto end_of_job;
    /* --- fill new raster --- */
    if (width - nback > 0) {
        /* don't fill 1-pixel wide empty bp*/
        for (icol = 0; icol < newwidth; icol++) { /* find first non-empty col in row */
            for (irow = 0; irow < height; irow++) { /* for each row inside rp */
                /* original pixel at irow,icol */
                int value = getpixel(rp, irow, icol);
                setpixel(bp, irow, icol, value);
            }   /* saved in backspaced raster */
        }
    }
    /* ------------------------------------------------------------
    Back to caller with backspaced raster (or null for any error)
    ------------------------------------------------------------ */
end_of_job:
    if (mctx->msgfp != NULL && mctx->msglevel >= 999) {
        fprintf(mctx->msgfp,
        /* diagnostics */
                "backspace_raster> nback=%d,minspace=%d,mback=%d, width:old=%d,new=%d\n",
                nback, minspace, mback, width, newwidth);
        fflush(mctx->msgfp);
    }
    /* free original raster */
    if (isfree && bp != NULL) delete_raster(mctx, rp);
    /* back with backspaced or null ptr*/
    return (bp);
} /* --- end-of-function backspace_raster() --- */


/* ==========================================================================
 * Function:    type_raster ( rp, fp )
 * Purpose: Emit an ascii dump representing rp, on fp.
 * --------------------------------------------------------------------------
 * Arguments:   rp (I)      ptr to raster struct for which an
 *              ascii dump is to be constructed.
 *      fp (I)      File ptr to output device (defaults to
 *              stdout if passed as NULL).
 * --------------------------------------------------------------------------
 * Returns: ( int )     1 if completed successfully,
 *              or 0 otherwise (for any error).
 * --------------------------------------------------------------------------
 * Notes:
 * ======================================================================= */
/* --- entry point --- */
int type_raster(mimetex_ctx *mctx, raster *rp, FILE *fp)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* max columns for display */
    static  int display_width = 72;
    static  char display_chars[16] = {
        /* display chars for bytemap */
        ' ', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'A', 'B', 'C', 'D', 'E', '*'
    };
    /* ascii image for one scan line */
    char scanline[133];
    /* #chars in scan (<=display_width)*/
    int scan_width;
    /* height index, width indexes */
    int irow, locol, hicol = (-1);
    /* convert .gf to bitmap if needed */
    raster *bitmaprp = rp;
    /* ------------------------------------------------------------
    initialization
    ------------------------------------------------------------ */
    /* --- redirect null fp --- */
    /* default fp to stdout if null */
    if (fp == (FILE *)NULL) fp = stdout;
    if (mctx->msglevel >= 999) {
        fprintf(fp,
        /* debugging diagnostics */
                "type_raster> width=%d height=%d ...\n",
                rp->width, rp->height);
        fflush(fp);
    }
    /* --- check for ascii string --- */
    if (mctx->isstring                 /* pixmap has string, not raster */
            || (0 && rp->height == 1)) {    /* infer input rp is a string */
        /*interpret pixmap as ascii string*/
        char *string = (char *)(rp->pixmap);
        /* #chars in ascii string */
        int width = strlen(string);
        while (width > display_width - 2) { /* too big for one line */
            /*display leading chars*/
            fprintf(fp, "\"%.*s\"\n", display_width - 2, string);
            /* bump string past displayed chars*/
            string += (display_width - 2);
            width -= (display_width - 2);
        }   /* decrement remaining width */
        /* display trailing chars */
        fprintf(fp, "\"%.*s\"\n", width, string);
        return (1);
    } /* --- end-of-if(mctx->isstring) --- */
    /* ------------------------------------------------------------
    display ascii dump of bitmap image (in segments if display_width < rp->width)
    ------------------------------------------------------------ */
    if (rp->format == 2          /* input is .gf-formatted */
            ||   rp->format == 3)
        /* so convert it for display */
        bitmaprp = gftobitmap(mctx, rp);
    if (bitmaprp != NULL)            /* if we have image for display */
        while ((locol = hicol + 1) < rp->width) { /*start where prev segment left off*/
            /* --- set hicol for this pass (locol set above) --- */
            /* show as much as display allows */
            hicol += display_width;
            /*but not more than raster*/
            if (hicol >= rp->width) hicol = rp->width - 1;
            /* #chars in this scan */
            scan_width = hicol - locol + 1;
            /*separator between segments*/
            if (locol > 0) fprintf(fp, "----------\n");
            /* ------------------------------------------------------------
            display all scan lines for this local...hicol segment range
            ------------------------------------------------------------ */
            for (irow = 0; irow < rp->height; irow++) { /* all scan lines for col range */
                /* --- allocations and declarations --- */
                int ipix,               /* pixmap[] index for this scan */
                /*first pixmap[] pixel in this scan*/
                lopix = irow * rp->width + locol;
                /* --- set chars in scanline[] based on pixels in rp->pixmap[] --- */
                for (ipix = 0; ipix < scan_width; ipix++) /* set each char */
                    if (bitmaprp->pixsz == 1)      /*' '=0 or '*'=1 to display bitmap*/
                        scanline[ipix] = (getlongbit(bitmaprp->pixmap, lopix + ipix) == 1 ? '*' : '.');
                    else
                    /* should have a bytemap */
                        if (bitmaprp->pixsz == 8) {   /* double-check pixsz for bytemap */
                            int pixval = (int)((bitmaprp->pixmap)[lopix+ipix]), /*byte value*/
                                         /* index for ' ', '1'...'e', '*' */
                                         ichar = min2(15, pixval / 16);
                            scanline[ipix] = display_chars[ichar];
                        } /*set ' ' for 0-15, etc*/
                /* --- display completed scan line --- */
                fprintf(fp, "%.*s\n", scan_width, scanline);
            } /* --- end-of-for(irow) --- */
        } /* --- end-of-while(hicol<rp->width) --- */
    /* ------------------------------------------------------------
    Back to caller with 1=okay, 0=failed.
    ------------------------------------------------------------ */
    if (rp->format == 2          /* input was .gf-format */
            ||   rp->format == 3)
        if (bitmaprp != NULL)          /* and we converted it for display */
            /* no longer needed, so free it */
            delete_raster(mctx, bitmaprp);
    return (1);
} /* --- end-of-function type_raster() --- */


/* ==========================================================================
 * Function:    gftobitmap ( gf )
 * Purpose: convert .gf-like pixmap to bitmap image
 * --------------------------------------------------------------------------
 * Arguments:   gf (I)      raster * to struct in .gf-format
 * --------------------------------------------------------------------------
 * Returns: ( raster * )    image-format raster * if successful,
 *              or NULL for any error.
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
raster  *gftobitmap(mimetex_ctx *mctx, raster *gf)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* image raster retuned to caller */
    raster *rp = NULL;
    /* gf->width, gf->height, #bits */
    int width = 0, height = 0, totbits = 0;
    int format = 0, icount = 0, ncounts = 0,  /*.gf format, count index, #counts*/
        ibit = 0, bitval = 0; /* bitmap index, bit value */
    int isrepeat = 1, /* true to process repeat counts */
        repeatcmds[2] = {255, 15},  /*opcode for repeat/duplicate count*/
        nrepeats = 0, irepeat = 0,  /* scan line repeat count,index */
        wbits = 0; /* count bits to width of scan line*/
    /* ------------------------------------------------------------
    initialization
    ------------------------------------------------------------ */
    /* --- check args --- */
    /* input raster not provided */
    if (gf == NULL) goto end_of_job;
    /* 2 or 3 */
    format = gf->format;
    /* invalid raster format */
    if (format != 2 && format != 3) goto end_of_job;
    /*pixsz is really #counts in pixmap*/
    ncounts = gf->pixsz;
    /* --- allocate output raster with proper dimensions for bitmap --- */
    width = gf->width;
    /* dimensions of raster */
    height = gf->height;
    if ((rp = new_raster(mctx, width, height, 1))  /* allocate new raster and bitmap */
            /* quit if failed to allocate */
            ==   NULL) goto end_of_job;
    /* total #bits in image */
    totbits = width * height;
    /* ------------------------------------------------------------
    fill bitmap
    ------------------------------------------------------------ */
    for (icount = 0, bitval = 0; icount < ncounts; icount++) {
        /*#bits to set*/
        int   nbits = (int)(getbyfmt(format, gf->pixmap, icount));
        if (isrepeat               /* we're proxessing repeat counts */
                &&   nbits == repeatcmds[format-2]) {  /* and repeat opcode found */
            if (nrepeats == 0) {          /* recursive repeat is error */
                /*repeat count*/
                nrepeats = (int)(getbyfmt(format, gf->pixmap, icount + 1));
                /*#bits to set*/
                nbits = (int)(getbyfmt(format, gf->pixmap, icount + 2));
                icount += 2;
            }            /* bump byte/nibble count */
            else
            /* some internal error occurred */
                if (mctx->msgfp != NULL && mctx->msglevel >= 1)  /* report error */
                    fprintf(mctx->msgfp, "gftobitmap> found embedded repeat command\n");
        }
        if (0)
            fprintf(stdout,
                    "gftobitmap> icount=%d bitval=%d nbits=%d ibit=%d totbits=%d\n",
                    icount, bitval, nbits, ibit, totbits);
        for (; nbits > 0; nbits--) {   /* count down */
            /* overflow check */
            if (ibit >= totbits) goto end_of_job;
            for (irepeat = 0; irepeat <= nrepeats; irepeat++)
                if (bitval == 1) {        /* set pixel */
                    setlongbit(rp->pixmap, (ibit + irepeat*width));
                } else {          /* clear pixel */
                    unsetlongbit(rp->pixmap, (ibit + irepeat*width));
                }
            /* count another repeated bit */
            if (nrepeats > 0) wbits++;
            ibit++;
        }             /* bump bit index */
        /* flip bit value */
        bitval = 1 - bitval;
        if (wbits >= width) {          /* completed repeats */
            /*bump bit count past repeated scans*/
            ibit += nrepeats * width;
            if (wbits > width)            /* out-of alignment error */
                if (mctx->msgfp != NULL && mctx->msglevel >= 1)  /* report error */
                    fprintf(mctx->msgfp, "gftobitmap> width=%d wbits=%d\n", width, wbits);
            wbits = nrepeats = 0;
        }      /* reset repeat counts */
    } /* --- end-of-for(icount) --- */
end_of_job:
    /* back to caller with image */
    return (rp);
} /* --- end-of-function  --- */

/* ==========================================================================
 * Function:    rasterize ( expression, size )
 * Purpose: returns subraster corresponding to (a valid LaTeX) expression
 *      at font size
 * --------------------------------------------------------------------------
 * Arguments:   expression (I)  char * to first char of null-terminated
 *              string containing valid LaTeX expression
 *              to be rasterized
 *      size (I)    int containing 0-4 default font size
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to subraster corresponding to expression,
 *              or NULL for any parsing error.
 * --------------------------------------------------------------------------
 * Notes:     o This is mimeTeX's "main" reusable entry point.  Easy to use:
 *      just call it with a LaTeX expression, and get back a bitmap
 *      of that expression.  Then do what you want with the bitmap.
 * ======================================================================= */
/* --- entry point --- */
subraster *rasterize(mimetex_ctx *mctx, char *expression, int size)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* process preamble, if present */
    char    pretext[512];
    char    chartoken[MAXSUBXSZ+1], *texsubexpr(), /*get subexpression from expr*/
    /* token may be parenthesized expr */
    *subexpr = chartoken;
    /*get mathchardef struct for symbol*/
    mathchardef *symdef;
    int natoms = 0;         /* #atoms/tokens processed so far */
    /* display debugging output */
    subraster *sp = NULL, *prevsp = NULL, /* raster for current, prev char */
              *expraster = (subraster *)NULL; /* raster returned to caller */
    /* current font family */
    int family = fontinfo[mctx->fontnum].family;
    int isleftscript = 0,       /* true if left-hand term scripted */
                       wasscripted = 0,        /* true if preceding token scripted*/
                                     /* true if preceding delim scripted*/
                                     wasdelimscript = 0;
    /*int   pixsz = 1;*/            /*default #bits per pixel, 1=bitmap*/
    /* detex token for error message */
    char    *strdetex();
    /* --- global values saved/restored at each recursive iteration --- */
    int wasstring = mctx->isstring,       /* initial mctx->isstring mode flag */
        wasdisplaystyle = mctx->isdisplaystyle, /*initial displaystyle mode flag*/
        oldfontnum = mctx->fontnum,       /* initial font family */
        oldfontsize = mctx->fontsize,     /* initial mctx->fontsize */
        olddisplaysize = mctx->displaysize,   /* initial \displaystyle size */
        oldshrinkfactor = mctx->shrinkfactor, /* initial mctx->shrinkfactor */
        oldsmashmargin = mctx->smashmargin,   /* initial mctx->smashmargin */
        oldissmashdelta = mctx->issmashdelta, /* initial mctx->issmashdelta */
        oldisexplicitsmash = mctx->isexplicitsmash, /* initial mctx->isexplicitsmash */
        oldisscripted = mctx->isscripted, /* initial mctx->isscripted */
        *oldworkingparam = mctx->workingparam; /* initial working parameter */
    subraster *oldworkingbox = mctx->workingbox,  /* initial working box */
              *oldleftexpression = mctx->leftexpression; /*left half rasterized so far*/
    double  oldunitlength = mctx->unitlength; /* initial mctx->unitlength */
    mathchardef *oldleftsymdef = mctx->leftsymdef; /* init oldleftsymdef */
    /* ------------------------------------------------------------
    initialization
    ------------------------------------------------------------ */
    /* wind up one more recursion level*/
    mctx->recurlevel++;
    /* no leading left half yet */
    mctx->leftexpression = NULL;
    /* reset replaceleft flag */
    mctx->isreplaceleft = 0;
    /* reset \frac baseline signal */
    if (1)mctx->fraccenterline = NOVALUE;
    /* mctx->shrinkfactor = mctx->shrinkfactors[max2(0,min2(size,LARGESTSIZE))];*/ /*set sf*/
    /* have 17 sf's */
    mctx->shrinkfactor = shrinkfactors[max2(0, min2(size, 16))];
    if (mctx->msgfp != NULL && mctx->msglevel >= 9) {    /*display expression for debugging*/
        fprintf(mctx->msgfp,
                "rasterize> recursion#%d, size=%d,\n\texpression=\"%s\"\n",
                mctx->recurlevel, size, (expression == NULL ? "<null>" : expression));
        fflush(mctx->msgfp);
    }
    /* nothing given to do */
    if (expression == NULL) goto end_of_job;
    /* ------------------------------------------------------------
    preocess optional $-terminated preamble preceding expression
    ------------------------------------------------------------ */
    /* size may be modified */
    expression = preamble(mctx, expression, &size, pretext);
    /* nothing left to do */
    if (*expression == '\000') goto end_of_job;
    /* start at requested size */
    mctx->fontsize = size;
    if (mctx->isdisplaystyle == 1)         /* displaystyle enabled but not set*/
        if (!mctx->ispreambledollars)         /* style fixed by $$...$$'s */
            /*force at large mctx->fontsize*/
            mctx->isdisplaystyle = (mctx->fontsize >= mctx->displaysize ? 2 : 1);
    /* ------------------------------------------------------------
    build up raster one character (or subexpression) at a time
    ------------------------------------------------------------ */
    while (1) {
        /* --- kludge for \= cyrillic ligature --- */
        /* no ligature found yet */
        mctx->isligature = 0;
        /* current font family */
        family = fontinfo[mctx->fontnum].family;
        if (family == CYR10)       /* may have cyrillic \= ligature */
            if ((symdef = get_ligature(mctx, expression, family)) /*check for any ligature*/
                    >=    0)                 /* got some ligature */
                if (memcmp(symdef->symbol, "\\=", 2) == 0) /* starts with \= */
                    /* signal \= ligature */
                    mctx->isligature = 1;
        /* --- get next character/token or subexpression --- */
        /* ptr within expression to subexpr*/
        mctx->subexprptr = expression;
        expression = texsubexpr(mctx, expression, chartoken, 0, LEFTBRACES, RIGHTBRACES, 1, 1);
        /* "local" copy of chartoken ptr */
        subexpr = chartoken;
        /* no character identified yet */
        mctx->leftsymdef = NULL;
        /* no subraster yet */
        sp = NULL;
        /* in case reset by \tiny, etc */
        size = mctx->fontsize;
        /*isleftscript = mctx->isdelimscript;*/ /*was preceding term scripted delim*/
        /* true if preceding token scripted*/
        wasscripted = mctx->isscripted;
        /* preceding \right delim scripted */
        wasdelimscript = mctx->isdelimscript;
        /* no subscripted expression yet */
        if (1)mctx->isscripted = 0;
        /* reset \right delim scripted flag*/
        mctx->isdelimscript = 0;
        /* --- debugging output --- */
        if (mctx->msgfp != NULL && mctx->msglevel >= 9) {  /* display chartoken for debugging */
            fprintf(mctx->msgfp,
                    "rasterize> recursion#%d,atom#%d=\"%s\" (mctx->isligature=%d,isleftscript=%d)\n",
                    mctx->recurlevel, natoms + 1, chartoken, mctx->isligature, isleftscript);
            fflush(mctx->msgfp);
        }
        if (expression == NULL         /* no more tokens */
                /* and this token empty */
                &&   *subexpr == '\000') break;
        /* enough if just this token empty */
        if (*subexpr == '\000') break;
        /* --- check for parenthesized subexpression --- */
        if (isbrace(mctx, subexpr, LEFTBRACES, 1)) { /* got parenthesized subexpression */
            /* rasterize subexpression */
            if ((sp = rastparen(mctx, &subexpr, size, prevsp)) == NULL)
                /* flush it if failed to rasterize */
                continue;
        } else {
        /* --- single-character atomic token --- */
            if (!isthischar(*subexpr, SCRIPTS)) { /* scripts handled below */
                /* --- first check for opening $ in \text{ if $n-m$ even} --- */
                if (istextmode           /* we're in \text mode */
                        &&   *subexpr == '$' && subexpr[1] == '\000') { /* and have an opening $ */
                    /* $expression$ in \text{ }*/
                    char *endptr = NULL, mathexpr[MAXSUBXSZ+1];
                    /* length of $expression$ */
                    int  exprlen = 0;
                    /* current text font number */
                    int  textfontnum = mctx->fontnum;
                    /*if ( (endptr=strrchr(expression,'$')) != NULL )*/ /*ptr to closing $*/
                    if ((endptr = strchr(expression, '$')) != NULL) /* ptr to closing $ */
                        /* #chars preceding closing $ */
                        exprlen = (int)(endptr - expression);
                    else {             /* no closing $ found */
                        /* just assume entire expression */
                        exprlen = strlen(expression);
                        endptr = expression + (exprlen - 1);
                    } /*and push expression to '\000'*/
                    /* don't overflow mathexpr[] */
                    exprlen = min2(exprlen, MAXSUBXSZ);
                    if (exprlen > 0) {          /* have something between $$ */
                        /*local copy of math expression*/
                        memcpy(mathexpr, expression, exprlen);
                        /* null-terminate it */
                        mathexpr[exprlen] = '\000';
                        /* set math mode */
                        mctx->fontnum = 0;
                        /* and rasterize $expression$ */
                        sp = rasterize(mctx, mathexpr, size);
                        mctx->fontnum = textfontnum;
                    }     /* set back to text mode */
                    /* push expression past closing $ */
                    expression = endptr + 1;
                } else {
                    /* --- otherwise, look up mathchardef for atomic token in table --- */
                    if ((mctx->leftsymdef = symdef = get_symdef(mctx, chartoken)) /*mathchardef for token*/
                            ==  NULL) {            /* lookup failed */
                        /*display for unrecognized literal*/
                        char literal[512] = "[?]";
                        /* error display in default mode */
                        int  oldfontnum = mctx->fontnum;
                        if (mctx->msgfp != NULL && mctx->msglevel >= 29) { /* display unrecognized symbol*/
                            fprintf(mctx->msgfp, "rasterize> get_symdef() failed for \"%s\"\n",
                                    chartoken);
                            fflush(mctx->msgfp);
                        }
                        /* init to signal failure */
                        sp = (subraster *)NULL;
                        /* warnings not wanted */
                        if (mctx->warninglevel < 1) continue;
                        /* reset from \mathbb, etc */
                        mctx->fontnum = 0;
                        if (isthischar(*chartoken, ESCAPE))  /* we got unrecognized \escape*/
                        {
                        /* --- so display literal {\rm~[\backslash~chartoken?]} ---  */
                            /* init error message token */
                            strcpy(literal, "{\\rm~[");
                            /* detex the token */
                            strcat(literal, strdetex(chartoken, 0));
                            strcat(literal, "?]}");
                        } /* add closing ? and brace */
                        /* rasterize literal token */
                        sp = rasterize(mctx, literal, size - 1);
                        /* reset font family */
                        mctx->fontnum = oldfontnum;
                        if (sp == (subraster *)NULL)
                        /*flush if rasterize fails*/
                            continue;
                    } else {
                    /* --- check if we have special handler to process this token --- */
                        if (symdef->handler != NULL) { /* have a handler for this token */
                            int arg1 = symdef->charnum, arg2 = symdef->family, arg3 = symdef->klass;
                            if ((sp = (*symdef->handler)(mctx, &expression, size, prevsp, arg1, arg2, arg3)) == NULL)
                                /* flush token if handler failed */
                                continue;
                        } else {
                        /* --- no handler, so just get subraster for this character --- */
                            if (!mctx->isstring) {          /* rasterizing */
                                if (mctx->isligature)        /* found a ligature */
                                    /*push past it*/
                                    expression = mctx->subexprptr + strlen(symdef->symbol);
                                /* get subraster */
                                if ((sp = get_charsubraster(mctx, symdef, size)) ==  NULL)
                                    /* flush token if failed */
                                    continue;
                            } else {
                                /* constructing ascii string */
                                /* symbol for ascii string */
                                char *symbol = symdef->symbol;
                                /*#chars in symbol*/
                                int symlen = (symbol != NULL ? strlen(symbol) : 0);
                                /* no symbol for ascii string */
                                if (symlen < 1) continue;
                                if ((sp = new_subraster(mctx, symlen + 1, 1, 8)) /* subraster for symbol */
                                        /* flush token if malloc failed */
                                        ==  NULL)  continue;
                                /* set subraster type */
                                sp->type = ASCIISTRING;
                                /* and set symbol definition */
                                sp->symdef = symdef;
                                /* default (should be unused) */
                                sp->baseline = 1;
                                /* copy symbol */
                                strcpy((char *)((sp->image)->pixmap), symbol);
                                /*((char *)((sp->image)->pixmap))[symlen] = '\000';*/
                            } /*null*/
                        }
                    }
                }
            } /* --- end-of-if(!isthischar(*subexpr,SCRIPTS)) --- */
        }
        /* --- handle any super/subscripts following symbol or subexpression --- */
        sp = rastlimits(mctx, &expression, size, sp);
        /*preceding term scripted*/
        isleftscript = (wasscripted || wasdelimscript ? 1 : 0);
        /* --- debugging output --- */
        if (mctx->msgfp != NULL && mctx->msglevel >= 9) {  /* display raster for debugging */
            fprintf(mctx->msgfp, "rasterize> recursion#%d,atom#%d%s\n",
                    mctx->recurlevel, natoms + 1, (sp == NULL ? " = <null>" : "..."));
            if (mctx->msglevel >= 9) fprintf(mctx->msgfp,
                                           "  isleftscript=%d is/wasscripted=%d,%d is/wasdelimscript=%d,%d\n",
                                           isleftscript, mctx->isscripted, wasscripted, mctx->isdelimscript, wasdelimscript);
            if (mctx->msglevel >= 99)
                /* display raster */
                if (sp != NULL) type_raster(mctx, sp->image, mctx->msgfp);
            fflush(mctx->msgfp);
        }         /* flush mctx->msgfp buffer */
        /* --- accumulate atom or parenthesized subexpression --- */
        if (natoms < 1             /* nothing previous to concat */
                ||   expraster == NULL        /* or previous was complete error */
                ||   mctx->isreplaceleft) {         /* or we're replacing previous */
            if (1 && expraster != NULL)    /* probably replacing left */
                /* so first free original left */
                delete_subraster(mctx, expraster);
            /* copy static CHARASTER or left */
            expraster = subrastcpy(mctx, sp);
            mctx->isreplaceleft = 0;
        }      /* reset replacement flag */
        else
        /*we've already built up atoms so...*/
            if (sp != NULL) {             /* ...if we have a new term */
                /* save current smash margin */
                int prevsmashmargin = mctx->smashmargin;
                if (isleftscript) {          /* don't smash against scripts */
                    /* reset \right delim scripted flag*/
                    mctx->isdelimscript = 0;
                    if (!mctx->isexplicitsmash) mctx->smashmargin = 0;
                } /* signal no smash wanted */
                /* concat new term, free previous */
                expraster = rastcat(mctx, expraster, sp, 1);
                mctx->smashmargin = prevsmashmargin;
            }    /* restore current smash margin */
        /* free prev (if not a CHARASTER) */
        delete_subraster(mctx, prevsp);
        /* current becomes previous */
        prevsp = sp;
        /* left half rasterized so far */
        mctx->leftexpression = expraster;
        /* --- bump count --- */
        /* bump #atoms count */
        natoms++;
    } /* --- end-of-while(expression!=NULL) --- */
    /* ------------------------------------------------------------
    back to caller with rasterized expression
    ------------------------------------------------------------ */
end_of_job:
    /* free last (if not a CHARASTER) */
    delete_subraster(mctx, prevsp);
    /* --- debugging output --- */
    if (mctx->msgfp != NULL && mctx->msglevel >= 999) { /* display raster for debugging */
        fprintf(mctx->msgfp, "rasterize> Final recursion level=%d, atom#%d...\n",
                mctx->recurlevel, natoms);
        if (expraster != (subraster *)NULL)   /* i.e., if natoms>0 */
            /* display completed raster */
            type_raster(mctx, expraster->image, mctx->msgfp);
        fflush(mctx->msgfp);
    }          /* flush mctx->msgfp buffer */
    /* --- set final raster buffer --- */
    if (1 && expraster != (subraster *)NULL) { /* have an expression */
        /* type of constructed image */
        int type = expraster->type;
        if (type != FRACRASTER)        /* leave \frac alone */
            /* set type to constructed image */
            expraster->type = IMAGERASTER;
        if (istextmode)            /* but in text mode */
            /* set type to avoid smash */
            expraster->type = mctx->blanksignal;
        expraster->size = mctx->fontsize;
    } /* set original input font size */
    /* --- restore flags/values to original saved values --- */
    /* string mode reset */
    mctx->isstring = wasstring;
    /* displaystyle mode reset */
    mctx->isdisplaystyle = wasdisplaystyle;
    /* font family reset */
    mctx->fontnum = oldfontnum;
    /* mctx->fontsize reset */
    mctx->fontsize = oldfontsize;
    /* \displaystyle size reset */
    mctx->displaysize = olddisplaysize;
    /* mctx->shrinkfactor reset */
    mctx->shrinkfactor = oldshrinkfactor;
    /* mctx->smashmargin reset */
    mctx->smashmargin = oldsmashmargin;
    /* mctx->issmashdelta reset */
    mctx->issmashdelta = oldissmashdelta;
    /* mctx->isexplicitsmash reset */
    mctx->isexplicitsmash = oldisexplicitsmash;
    /* mctx->isscripted reset */
    mctx->isscripted = oldisscripted;
    /* working parameter reset */
    mctx->workingparam = oldworkingparam;
    /* working box reset */
    mctx->workingbox = oldworkingbox;
    /* mctx->leftexpression reset */
    mctx->leftexpression = oldleftexpression;
    /* mctx->leftsymdef reset */
    mctx->leftsymdef = oldleftsymdef;
    /* mctx->unitlength reset */
    mctx->unitlength = oldunitlength;
    /* unwind one recursion level */
    mctx->recurlevel--;
    /* --- return final subraster to caller --- */
    return (expraster);
} /* --- end-of-function rasterize() --- */


/* ==========================================================================
 * Function:    rastparen ( subexpr, size, basesp )
 * Purpose: parentheses handler, returns a subraster corresponding to
 *      parenthesized subexpression at font size
 * --------------------------------------------------------------------------
 * Arguments:   subexpr (I) char **  to first char of null-terminated
 *              string beginning with a LEFTBRACES
 *              to be rasterized
 *      size (I)    int containing 0-7 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding leading left{
 *              (unused, but passed for consistency)
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to subraster corresponding to subexpr,
 *              or NULL for any parsing error
 * --------------------------------------------------------------------------
 * Notes:     o This "handler" isn't in the mathchardef symbol table,
 *      but is called directly from rasterize(), as necessary.
 *        o Though subexpr is returned unchanged, it's passed as char **
 *      for consistency with other handlers.  Ditto, basesp is unused
 *      but passed for consistency
 * ======================================================================= */
/* --- entry point --- */
subraster *rastparen(mimetex_ctx *mctx, char **subexpr, int size, subraster *basesp)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* dereference subexpr to get char* */
    char    *expression = *subexpr;
    /* total #chars, including parens */
    int explen = strlen(expression);
    int isescape = 0,           /* true if parens \escaped */
        isrightdot = 0,         /* true if right paren is \right. */
        isleftdot = 0; /* true if left paren is \left. */
    /* parens enclosing expresion */
    char    left[32], right[32];
    /* get subexpr without parens */
    char    noparens[MAXSUBXSZ+1];
    /* rasterize what's between ()'s */
    subraster *sp = NULL;
    /*true=full height, false=baseline*/
    int isheight = 1;
    int height,             /* height of rasterized noparens[] */
    /* and its baseline */
    baseline;
    int family = /*CMSYEX*/ CMEX10; /* family for paren chars */
    /* left and right paren chars */
    subraster *lp = NULL, *rp = NULL;
    /* ------------------------------------------------------------
    rasterize "interior" of expression, i.e., without enclosing parens
    ------------------------------------------------------------ */
    /* --- first see if enclosing parens are \escaped --- */
    if (isthischar(*expression, ESCAPE))     /* expression begins with \escape */
        /* so set flag accordingly */
        isescape = 1;
    /* --- get expression *without* enclosing parens --- */
    /* get local copy of expression */
    strcpy(noparens, expression);
    /* null-terminate before right} */
    noparens[explen-(1+isescape)] = '\000';
    /* and then squeeze out left{ */
    strcpy(noparens, noparens + (1 + isescape));
    /* --- rasterize it --- */
    if ((sp = rasterize(mctx, noparens, size)) /*rasterize "interior" of expression*/
            /* quit if failed */
            ==   NULL) goto end_of_job;
    /* --- no need to add parentheses for unescaped { --- */
    if (!isescape && isthischar(*expression, "{"))  /* don't add parentheses */
        /* just return sp to caller */
        goto end_of_job;
    /* ------------------------------------------------------------
    obtain paren characters to enclose noparens[] raster with
    ------------------------------------------------------------ */
    /* --- first get left and right parens from expression --- */
    memset(left, 0, 16);
    /* init parens with nulls */
    memset(right, 0, 16);
    /* left{ is 1st or 2nd char */
    left[0] = *(expression + isescape);
    /* right} is always last char */
    right[0] = *(expression + explen - 1);
    /* true if \left. */
    isleftdot  = (isescape && isthischar(*left, "."));
    /* true if \right. */
    isrightdot = (isescape && isthischar(*right, "."));
    /* --- need height of noparens[] raster as minimum parens height --- */
    /* height of noparens[] raster */
    height = (sp->image)->height;
    /* baseline of noparens[] raster */
    baseline = sp->baseline;
    /* parens only enclose baseline up */
    if (!isheight) height = baseline + 1;
    /* --- get best-fit parentheses characters --- */
    if (!isleftdot)              /* if not \left. */
        /* get left paren char */
        lp = get_delim(mctx, left, height + 1, family);
    if (!isrightdot)             /* and if not \right. */
        /* get right paren char */
        rp = get_delim(mctx, right, height + 1, family);
    if ((lp == NULL && !isleftdot)       /* check that we got left( */
            || (rp == NULL && !isrightdot)) {   /* and right) if needed */
        /* if failed, free subraster */
        delete_subraster(mctx, sp);
        /*free left-paren subraster envelope*/
        if (lp != NULL) free((void *)lp);
        /*and right-paren subraster envelope*/
        if (rp != NULL) free((void *)rp);
        /* signal error to caller */
        sp = (subraster *)NULL;
        goto end_of_job;
    }          /* and quit */
    /* ------------------------------------------------------------
    set paren baselines to center on noparens[] raster, and concat components
    ------------------------------------------------------------ */
    /* --- set baselines to center paren chars on raster --- */
    if (lp != NULL)              /* ignore for \left. */
        lp->baseline = baseline + ((lp->image)->height - height) / 2;
    if (rp != NULL)              /* ignore for \right. */
        rp->baseline = baseline + ((rp->image)->height - height) / 2;
    /* --- concat lp||sp||rp to obtain final result --- */
    if (lp != NULL)              /* ignore \left. */
        /* concat lp||sp and free sp,lp */
        sp = rastcat(mctx, lp, sp, 3);
    if (sp != NULL)              /* succeeded or ignored \left. */
        if (rp != NULL)            /* ignore \right. */
            /* concat sp||rp and free sp,rp */
            sp = rastcat(mctx, sp, rp, 3);
    /* --- back to caller --- */
end_of_job:
    return (sp);
} /* --- end-of-function rastparen() --- */


/* ==========================================================================
 * Function:    rastlimits ( expression, size, basesp )
 * Purpose: \limits, \nolimts, _ and ^ handler,
 *      dispatches call to rastscripts() or to rastdispmath()
 *      as necessary, to handle sub/superscripts following symbol
 * --------------------------------------------------------------------------
 * Arguments:   expression (I)  char **  to first char of null-terminated
 *              LaTeX expression (unused/unchanged)
 *      size (I)    int containing base font size (not used,
 *              just stored in subraster)
 *      basesp (I)  subraster *  to current character (or
 *              subexpression) immediately preceding script
 *              indicator
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to subraster returned by rastscripts()
 *              or rastdispmath(), or NULL for any error
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
subraster *rastlimits(mimetex_ctx *mctx, char **expression, int size, subraster *basesp)
{
    subraster *scriptsp = basesp,     /* and this will become the result */
              *dummybase = basesp; /* for {}_i construct a dummy base */
    /* set 1 for displaystyle, else 0 */
    int isdisplay = (-1);
    /* save original mctx->smashmargin */
    int oldsmashmargin = mctx->smashmargin;
    /* --- to check for \limits or \nolimits preceding scripts --- */
    /*check for \limits*/
    char    *exprptr = *expression, limtoken[255];
    /* strlen(limtoken) */
    int toklen = 0;
    /* mathchardef struct for limtoken */
    mathchardef *tokdef;
    /*base sym class*/
    int klass = (mctx->leftsymdef == NULL ? NOVALUE : mctx->leftsymdef->klass);
    /* ------------------------------------------------------------
    determine whether or not to use displaymath
    ------------------------------------------------------------ */
    /* first, increment subscript level*/
    mctx->scriptlevel++;
    /* no token yet */
    *limtoken = '\000';
    /* signal term not (text) scripted */
    mctx->isscripted = 0;
    if (mctx->msgfp != NULL && mctx->msglevel >= 999) {
        fprintf(mctx->msgfp, "rastlimits> mctx->scriptlevel#%d exprptr=%.48s\n",
                mctx->scriptlevel, (exprptr == NULL ? "null" : exprptr));
        fflush(mctx->msgfp);
    }
    /* no scripts for ascii string */
    if (mctx->isstring) goto end_of_job;
    /* --- check for \limits or \nolimits --- */
    /* skip white space before \limits */
    skipwhite(exprptr);
    if (!isempty(exprptr))       /* non-empty expression supplied */
        /* retrieve next token */
        exprptr = texchar(mctx, exprptr, limtoken);
    if (*limtoken != '\000')         /* have token */
        if ((toklen = strlen(limtoken)) >= 3)   /* which may be \[no]limits */
            if (memcmp("\\limits", limtoken, toklen) == 0    /* may be \limits */
                    ||   memcmp("\\nolimits", limtoken, toklen) == 0) /* or may be \nolimits */
                if ((tokdef = get_symdef(mctx, limtoken))   /* look up token to be sure */
                        !=   NULL) {             /* found token in table */
                    if (strcmp("\\limits", tokdef->symbol) == 0)   /* found \limits */
                        /* so explicitly set displaymath */
                        isdisplay = 1;
                    else
                    /* wasn't \limits */
                        if (strcmp("\\nolimits", tokdef->symbol) == 0)  /* found \nolimits */
                            isdisplay = 0;
                }        /* so explicitly reset displaymath */
    /* --- see if we found \[no]limits --- */
    if (isdisplay != (-1))       /* explicit directive found */
        /* so bump expression past it */
        *expression = exprptr;
    else {                  /* noexplicit directive */
        /* init displaymath flag off */
        isdisplay = 0;
        if (mctx->isdisplaystyle) {        /* we're in displaystyle math mode */
            if (mctx->isdisplaystyle >= 5) { /* and mode irrevocably forced true */
                if (klass != OPENING && klass != CLOSING) /*don't force ('s and )'s*/
                    isdisplay = 1;
            }        /* set flag if mode forced true */
            else if (mctx->isdisplaystyle >= 2) {   /*or mode forced conditionally true*/
                if (klass != VARIABLE && klass != ORDINARY /*don't force characters*/
                        &&   klass != OPENING  && klass != CLOSING  /*don't force ('s and )'s*/
                        &&   klass != BINARYOP    /* don't force binary operators */
                        &&   klass != NOVALUE)    /* finally, don't force "images" */
                    isdisplay = 1;
            }        /* set flag if mode forced true */
            else
            /* determine mode from base symbol */
                if (klass == DISPOPER)   /* it's a displaystyle operator */
                    isdisplay = 1;
        }
    }        /* so set flag */
    /* ------------------------------------------------------------
    dispatch call to create sub/superscripts
    ------------------------------------------------------------ */
    if (isdisplay)           /* scripts above/below base symbol */
        /* everything all done */
        scriptsp = rastdispmath(mctx, expression, size, basesp);
    else {                  /* scripts alongside base symbol */
        if (dummybase == NULL)         /* no base symbol preceding scripts*/
            /*guess a typical base symbol*/
            dummybase = rasterize(mctx, "\\rule0{10}", size);
        /*haven't found a no-smash char yet*/
        mctx->issmashokay = 1;
        if ((scriptsp = rastscripts(mctx, expression, size, dummybase)) == NULL) /*no scripts*/
            /* so just return unscripted symbol*/
            scriptsp = basesp;
        else {                /* symbols followed by scripts */
            /*signal current term text-scripted*/
            mctx->isscripted = 1;
            if (basesp != NULL)          /* have base symbol */
            {
            /*if(0)mctx->smashmargin = 0;*/
/*don't smash script (doesn't work)*/
                /*scriptsp = rastcat(mctx, basesp,scriptsp,2);*//*concat scripts to base sym*/
                /* --- smash (or just concat) script raster against base symbol --- */
                if (!mctx->issmashokay)         /* don't smash leading - */
                    /*don't smash*/
                    if (!mctx->isexplicitsmash) scriptsp->type = mctx->blanksignal;
                /*concat scripts to base sym*/
                scriptsp = rastcat(mctx, basesp, scriptsp, 3);
                /* flip type of composite object */
                if (1) scriptsp->type = IMAGERASTER;
                /* --- smash (or just concat) scripted term to stuff to its left --- */
                /* okay to smash base expression */
                mctx->issmashokay = 1;
                if (0 && mctx->smashcheck > 1)      /* mctx->smashcheck=2 to check base */
                    /* note -- we _don't_ have base expression available to check */
                    /*check if okay to smash*/
                    mctx->issmashokay = rastsmashcheck(mctx, *expression);
                if (!mctx->issmashokay)         /* don't smash leading - */
                    /*don't smash*/
                    if (!mctx->isexplicitsmash) scriptsp->type = mctx->blanksignal;
                scriptsp->size = size;
            }
        }
    } /* and set font size */
end_of_job:
    /* reset original mctx->smashmargin */
    mctx->smashmargin = oldsmashmargin;
    /*free work area*/
    if (dummybase != basesp) delete_subraster(mctx, dummybase);
    if (mctx->msgfp != NULL && mctx->msglevel >= 99) {
        fprintf(mctx->msgfp, "rastlimits> mctx->scriptlevel#%d returning %s\n",
                mctx->scriptlevel, (scriptsp == NULL ? "null" : "..."));
        if (scriptsp != NULL)          /* have a constructed raster */
            /*display constructed raster*/
            type_raster(mctx, scriptsp->image, mctx->msgfp);
        fflush(mctx->msgfp);
    }
    /*lastly, decrement subscript level*/
    mctx->scriptlevel--;
    return (scriptsp);
} /* --- end-of-function rastlimits() --- */


/* ==========================================================================
 * Function:    rastscripts ( expression, size, basesp )
 * Purpose: super/subscript handler, returns subraster for the leading
 *      scripts in expression, whose base symbol is at font size
 * --------------------------------------------------------------------------
 * Arguments:   expression (I/O) char **  to first char of null-terminated
 *              string beginning with a super/subscript,
 *              and returning ptr immediately following
 *              last script character processed.
 *      size (I)    int containing 0-7 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding leading script
 *              (scripts will be placed relative to base)
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to subraster corresponding to scripts,
 *              or NULL for any parsing error
 * --------------------------------------------------------------------------
 * Notes:     o This "handler" isn't in the mathchardef symbol table,
 *      but is called directly from , as necessary.
 * ======================================================================= */
/* --- entry point --- */
subraster *rastscripts(mimetex_ctx *mctx, char **expression, int size, subraster *basesp)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* scripts parsed from expression */
    char subscript[512], supscript[512];
    /* rasterize scripts */
    subraster *subsp = NULL, *supsp = NULL;
    subraster *sp = NULL; /* super- over subscript subraster */
    raster *rp = NULL; /* image raster embedded in sp */
    int height = 0, width = 0,  baseline = 0, /* height,width,baseline of sp */
        subht = 0,  subwidth = 0,  subln = 0, /* height,width,baseline of sub */
        supht = 0,  supwidth = 0,  supln = 0, /* height,width,baseline of sup */
        baseht = 0, baseln = 0; /* height,baseline of base */
    /* descender of base, subscript */
    int bdescend = 0, sdescend = 0;
    int issub = 0, issup = 0, isboth = 0, /* true if we have sub,sup,both */
        isbase = 0; /* true if we have base symbol */
    int szval = min2(max2(size, 0), LARGESTSIZE), /* 0...LARGESTSIZE */
        vbetween = 2,           /* vertical space between scripts */
        vabove   = szval + 1,   /*sup's top/bot above base's top/bot*/
        vbelow   = szval + 1,   /*sub's top/bot below base's top/bot*/
        vbottom  = szval + 1; /*sup's bot above (sub's below) bsln*/
    /*int   istweak = 1;*/          /* true to tweak script positioning */
    /*default #bits per pixel, 1=bitmap*/
    int pixsz = 1;
    /* ------------------------------------------------------------
    Obtain subscript and/or superscript expressions, and rasterize them/it
    ------------------------------------------------------------ */
    /* --- parse for sub,superscript(s), and bump expression past it(them) --- */
    /* no *ptr given */
    if (expression == NULL) goto end_of_job;
    /* no expression given */
    if (*expression == NULL) goto end_of_job;
    /* nothing in expression */
    if (*(*expression) == '\000') goto end_of_job;
    *expression = texscripts(mctx, *expression, subscript, supscript, 3);
    /* --- rasterize scripts --- */
    if (*subscript != '\000')        /* have a subscript */
        /* so rasterize it at size-1 */
        subsp = rasterize(mctx, subscript, size - 1);
    if (*supscript != '\000')        /* have a superscript */
        /* so rasterize it at size-1 */
        supsp = rasterize(mctx, supscript, size - 1);
    /* --- set flags for convenience --- */
    /* true if we have subscript */
    issub  = (subsp != (subraster *)NULL);
    /* true if we have superscript */
    issup  = (supsp != (subraster *)NULL);
    /* true if we have both */
    isboth = (issub && issup);
    /* quit if we have neither */
    if (!issub && !issup) goto end_of_job;
    /* --- check for leading no-smash chars (if enabled) --- */
    /* default, don't smash scripts */
    mctx->issmashokay = 0;
    if (mctx->smashcheck > 0) {            /* smash checking wanted */
        /*haven't found a no-smash char yet*/
        mctx->issmashokay = 1;
        if (issub)                  /* got a subscript */
            /* check if okay to smash */
            mctx->issmashokay = rastsmashcheck(mctx, subscript);
        if (mctx->issmashokay)            /* clean sub, so check sup */
            if (issup)                 /* got a superscript */
                /* check if okay to smash */
                mctx->issmashokay = rastsmashcheck(mctx, supscript);
    } /* --- end-of-if(mctx->smashcheck>0) --- */
    /* ------------------------------------------------------------
    get height, width, baseline of scripts,  and height, baseline of base symbol
    ------------------------------------------------------------ */
    /* --- get height and width of components --- */
    if (issub) {             /* we have a subscript */
        /* so get its height */
        subht    = (subsp->image)->height;
        /* and width */
        subwidth = (subsp->image)->width;
        subln    =  subsp->baseline;
    }  /* and baseline */
    if (issup) {             /* we have a superscript */
        /* so get its height */
        supht    = (supsp->image)->height;
        /* and width */
        supwidth = (supsp->image)->width;
        supln    =  supsp->baseline;
    }  /* and baseline */
    /* --- get height and baseline of base, and descender of base and sub --- */
    if (basesp == (subraster *)NULL)     /* no base symbol for scripts */
        /* try using left side thus far */
        basesp = mctx->leftexpression;
    if (basesp != (subraster *)NULL) {   /* we have base symbol for scripts */
        /* height of base symbol */
        baseht   = (basesp->image)->height;
        /* and its baseline */
        baseln   =  basesp->baseline;
        /* and base symbol descender */
        bdescend =  baseht - (baseln + 1);
        /*sub must descend by at least this*/
        sdescend =  bdescend + vbelow;
        if (baseht > 0)
            isbase = 1;
    }  /* set flag */
    /* ------------------------------------------------------------
    determine width of constructed raster
    ------------------------------------------------------------ */
    /*widest component is overall width*/
    width = max2(subwidth, supwidth);
    /* ------------------------------------------------------------
    determine height and baseline of constructed raster
    ------------------------------------------------------------ */
    /* --- both super/subscript --- */
    if (isboth) {                /*we have subscript and superscript*/
        height = max2(subht + vbetween + supht, /* script heights + space bewteen */
                      vbelow + baseht + vabove);
        baseline = baseln + (height - baseht) / 2; /*sub below base bot, sup above top*/
    } /*center scripts on base symbol*/
    /* --- superscript only --- */
    if (!issub) {                /* we only have a superscript */
        height = max3(baseln + 1 + vabove,  /* sup's top above base symbol top */
                      supht + vbottom,    /* sup's bot above baseln */
                      supht + vabove - bdescend); /* sup's bot above base symbol bot */
        baseline = height - 1;
    }      /*sup's baseline at bottom of raster*/
    /* --- subscript only --- */
    if (!issup) {                /* we only have a subscript */
        if (subht > sdescend) {        /*sub can descend below base bot...*/
            /* ...without extra space on top */
            height = subht;
            /* sub's bot below base symbol bot */
            baseline = height - (sdescend + 1);
            baseline = min2(baseline, max2(baseln - vbelow, 0));
        }/*top below base top*/
        else {                /* sub's top will be below baseln */
            /* sub's bot below base symbol bot */
            height = sdescend + 1;
            baseline = 0;
        }
    }         /* sub's baseline at top of raster */
    /* ------------------------------------------------------------
    construct raster with superscript over subscript
    ------------------------------------------------------------ */
    /* --- allocate subraster containing constructed raster --- */
    if ((sp = new_subraster(mctx, width, height, pixsz)) /*allocate subraster and raster*/
            ==   NULL)              /* and if we fail to allocate */
        /* quit */
        goto end_of_job;
    /* --- initialize subraster parameters --- */
    /* set type as constructed image */
    sp->type  = IMAGERASTER;
    /* set given size */
    sp->size  = size;
    /* composite scripts baseline */
    sp->baseline = baseline;
    /* raster embedded in subraster */
    rp = sp->image;
    /* --- place super/subscripts in new raster --- */
    if (issup)               /* we have a superscript */
        /* it goes in upper-left corner */
        rastput(mctx, rp, supsp->image, 0, 0, 1);
    if (issub)               /* we have a subscript */
        /*in lower-left corner*/
        rastput(mctx, rp, subsp->image, height - subht, 0, 1);
    /* ------------------------------------------------------------
    free unneeded component subrasters and return final result to caller
    ------------------------------------------------------------ */
end_of_job:
    /* free unneeded subscript */
    if (issub) delete_subraster(mctx, subsp);
    /* and superscript */
    if (issup) delete_subraster(mctx, supsp);
    return (sp);
} /* --- end-of-function rastscripts() --- */


/* ==========================================================================
 * Function:    rastdispmath ( expression, size, sp )
 * Purpose: displaymath handler, returns sp along with
 *      its immediately following super/subscripts
 * --------------------------------------------------------------------------
 * Arguments:   expression (I/O) char **  to first char of null-terminated
 *              string immediately following sp to be
 *              rasterized along with its super/subscripts,
 *              and returning ptr immediately following last
 *              character processed.
 *      size (I)    int containing 0-7 default font size
 *      sp (I)      subraster *  to display math operator
 *              to which super/subscripts will be added
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to subraster corresponding to sp
 *              plus its scripts, or NULL for any error
 * --------------------------------------------------------------------------
 * Notes:     o sp returned unchanged if no super/subscript(s) follow it.
 * ======================================================================= */
/* --- entry point --- */
subraster *rastdispmath(mimetex_ctx *mctx, char **expression, int size, subraster *sp)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* scripts parsed from expression */
    char    subscript[512], supscript[512];
    /* true if we have sub,sup */
    int issub = 0, issup = 0;
    subraster *subsp = NULL, *supsp = NULL;
    /* vertical space between scripts */
    int vspace = 1;
    /* ------------------------------------------------------------
    Obtain subscript and/or superscript expressions, and rasterize them/it
    ------------------------------------------------------------ */
    /* --- parse for sub,superscript(s), and bump expression past it(them) --- */
    /* no *ptr given */
    if (expression == NULL) goto end_of_job;
    /* no expression given */
    if (*expression == NULL) goto end_of_job;
    /* nothing in expression */
    if (*(*expression) == '\000') goto end_of_job;
    *expression = texscripts(mctx, *expression, subscript, supscript, 3);
    /* --- rasterize scripts --- */
    if (*subscript != '\000')        /* have a subscript */
        /* so rasterize it at size-1 */
        subsp = rasterize(mctx, subscript, size - 1);
    if (*supscript != '\000')        /* have a superscript */
        /* so rasterize it at size-1 */
        supsp = rasterize(mctx, supscript, size - 1);
    /* --- set flags for convenience --- */
    /* true if we have subscript */
    issub  = (subsp != (subraster *)NULL);
    /* true if we have superscript */
    issup  = (supsp != (subraster *)NULL);
    /*return operator alone if neither*/
    if (!issub && !issup) goto end_of_job;
    /* ------------------------------------------------------------
    stack operator and its script(s)
    ------------------------------------------------------------ */
    /* --- stack superscript atop operator --- */
    if (issup) {                 /* we have a superscript */
        if (sp == NULL)             /* but no base expression */
            /* so just use superscript */
            sp = supsp;
        else
        /* have base and superscript */
            if ((sp = rastack(mctx, sp, supsp, 1, vspace, 1, 3)) /* stack supsp atop base sp */
                    ==   NULL) goto end_of_job;
    }    /* and quit if failed */
    /* --- stack operator+superscript atop subscript --- */
    if (issub) {                 /* we have a subscript */
        if (sp == NULL)             /* but no base expression */
            /* so just use subscript */
            sp = subsp;
        else
        /* have base and subscript */
            if ((sp = rastack(mctx, subsp, sp, 2, vspace, 1, 3)) /* stack sp atop base subsp */
                    ==   NULL) goto end_of_job;
    }    /* and quit if failed */
    /* flip type of composite object */
    sp->type = IMAGERASTER;
    /* and set font size */
    sp->size = size;
    /* ------------------------------------------------------------
    free unneeded component subrasters and return final result to caller
    ------------------------------------------------------------ */
end_of_job:
    return (sp);
} /* --- end-of-function rastdispmath() --- */


/* ==========================================================================
 * Function:    rastleft ( expression, size, basesp, ildelim, arg2, arg3 )
 * Purpose: \left...\right handler, returns a subraster corresponding to
 *      delimited subexpression at font size
 * --------------------------------------------------------------------------
 * Arguments:   expression (I)  char **  to first char of null-terminated
 *              string beginning with a \left
 *              to be rasterized
 *      size (I)    int containing 0-7 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding leading left{
 *              (unused, but passed for consistency)
 *      ildelim (I) int containing ldelims[] index of
 *              left delimiter
 *      arg2 (I)    int unused
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to subraster corresponding to subexpr,
 *              or NULL for any parsing error
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
subraster *rastleft(mimetex_ctx *mctx, char **expression, int size, subraster *basesp,
                    int ildelim, int arg2, int arg3)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    subraster *sp = NULL, *lp = NULL, *rp = NULL;
    /* concat lp||sp||rp subrasters */
    int family = CMSYEX,        /*  family */
        height = 0, rheight = 0,    /* subexpr, right delim height */
        margin = (size + 1),    /* delim height margin over subexpr*/
        opmargin = (5); /* extra margin for \int,\sum,\etc */
    char subexpr[MAXSUBXSZ+1];/*chars between \left...\right*/
    char ldelim[256] = ".",
         rdelim[256] = "."; /* delims following \left,\right */
    /*locate \right matching our \left*/
    char *pleft, *pright;
    /* true if \left. or \right. */
    int isleftdot = 0, isrightdot = 0;
    /* true if delims are scripted */
    int isleftscript = 0, isrightscript = 0;
    /* strlen(subexpr) */
    int sublen = 0;
    /* 1=left,2=right */
    int idelim = 0;
    /* int  gotldelim = 0; */       /* true if ildelim given by caller */
    /* save current displaystyle */
    int wasdisplaystyle = mctx->isdisplaystyle;
    /* true for non-displaystyle delims*/
    int istextleft = 0, istextright = 0;
    /* --- recognized delimiters --- */
    /* tex delimiters */
    static  char left[16] = "\\left", right[16] = "\\right";
    static  char *ldelims[] = {
        "unused", ".",           /* 1   for \left., \right. */
        "(", ")",           /* 2,3 for \left(, \right) */
        "\\{", "\\}",           /* 4,5 for \left\{, \right\} */
        "[", "]",           /* 6,7 for \left[, \right] */
        "<", ">",           /* 8,9 for \left<, \right> */
        "|", "\\|",         /* 10,11 for \left,\right |,\|*/
        NULL
    };
    /* --- recognized operator delimiters --- */
    static  char *opdelims[] = {        /* operator delims from cmex10 */
        "int",   "sum",    "prod",
        "cup",   "cap",    "dot",
        "plus",  "times",  "wedge",
        "vee",
        NULL
    }; /* --- end-of-opdelims[] --- */
    /* --- delimiter xlation --- */
    static  char *xfrom[] = {
        /* xlate any delim suffix... */
        "\\|", /* \| */
        "\\{", /* \{ */
        "\\}", /* \} */
        "\\lbrace", /* \lbrace */
        "\\rbrace", /* \rbrace */
        "\\langle", /* \langle */
        "\\rangle", /* \rangle */
        NULL
    }; /* --- end-of-xfrom[] --- */
    /* ...to this instead */
    static  char *xto[] = {
        "=", /* \| to = */
        "{", /* \{ to { */
        "}", /* \} to } */
        "{", /* \lbrace to { */
        "}", /* \rbrace to } */
        "<", /* \langle to < */
        ">", /* \rangle to > */
        NULL
    }; /* --- end-of-xto[] --- */
    /* --- non-displaystyle delimiters --- */
    /* these delims _aren't_ display */
    static  char *textdelims[] = {
        "|", "=",
        "(", ")",
        "[", "]",
        "<", ">",
        "{", "}",
        "dbl", /* \lbrackdbl and \rbrackdbl */
        NULL
    } ; /* --- end-of-textdelims[] --- */
    /* ------------------------------------------------------------
    initialization
    ------------------------------------------------------------ */
    /* --- check args --- */
    /* nothing after \left */
    if (*(*expression) == '\000') goto end_of_job;
    /* --- determine left delimiter, and set default \right. delimiter --- */
    if (ildelim != NOVALUE && ildelim >= 1) { /* called with explicit left delim */
        /* so just get a local copy */
        strcpy(ldelim, ldelims[ildelim]);
        /* gotldelim = 1; */
    }       /* and set flag that we got it */
    else {                  /* trapped \left without delim */
        /* interpret \left ( as \left( */
        skipwhite(*expression);
        if (*(*expression) == '\000')     /* end-of-string after \left */
            /* so return NULL */
            goto end_of_job;
        /*pull delim from expression*/
        *expression = texchar(mctx, *expression, ldelim);
        if (*expression == NULL       /* probably invalid end-of-string */
                ||   *ldelim == '\000') goto end_of_job;
    } /* no delimiter */
    /* init default \right. delim */
    strcpy(rdelim, ".");
    /* ------------------------------------------------------------
    locate \right balancing our opening \left
    ------------------------------------------------------------ */
    /* --- first \right following \left --- */
    if ((pright = strtexchr(*expression, right)) /* look for \right after \left */
            !=   NULL) {                /* found it */
        /* --- find matching \right by pushing past any nested \left's --- */
        /* start after first \left( */
        pleft = *expression;
        while (1) {                 /*break when matching \right found*/
            /* -- locate next nested \left if there is one --- */
            if ((pleft = strtexchr(pleft, left)) /* find next \left */
                    /*no more, so matching \right found*/
                    ==   NULL) break;
            /* push ptr past \left token */
            pleft += strlen(left);
            /* not nested if \left after \right*/
            if (pleft >= pright) break;
            /* --- have nested \left, so push forward to next \right --- */
            if ((pright = strtexchr(pright + strlen(right), right)) /* find next \right */
                    /* ran out of \right's */
                    ==   NULL) break;
        } /* --- end-of-while(1) --- */
    } /* --- end-of-if(pright!=NULL) --- */
    /* ------------------------------------------------------------
    push past \left(_a^b sub/superscripts, if present
    ------------------------------------------------------------ */
    /*reset pleft after opening \left( */
    pleft = *expression;
    if ((lp = rastlimits(mctx, expression, size, lp)) /*dummy call push expression past b*/
            !=   NULL) {            /* found actual _a^b scripts, too */
        /* but we don't need them */
        delete_subraster(mctx, lp);
        lp = NULL;
    }            /* reset pointer, too */
    /* ------------------------------------------------------------
    get \right delimiter and subexpression between \left...\right, xlate delims
    ------------------------------------------------------------ */
    /* --- get delimiter following \right --- */
    if (pright == (char *)NULL) {        /* assume \right. at end of exprssn*/
        /* set default \right. */
        strcpy(rdelim, ".");
        /* use entire remaining expression */
        sublen = strlen(*expression);
        /* copy all remaining chars */
        memcpy(subexpr, *expression, sublen);
        *expression += sublen;
    }      /* and push expression to its null */
    else {                  /* have explicit matching \right */
        /* #chars between \left...\right */
        sublen = (int)(pright - (*expression));
        /* copy chars preceding \right */
        memcpy(subexpr, *expression, sublen);
        /* push expression past \right */
        *expression = pright + strlen(right);
        /* interpret \right ) as \right) */
        skipwhite(*expression);
        /*pull delim from expression*/
        *expression = texchar(mctx, *expression, rdelim);
        if (*rdelim == '\000') strcpy(rdelim, ".");
    } /* \right. if no rdelim */
    /* --- get subexpression between \left...\right --- */
    /* nothing between delimiters */
    if (sublen < 1) goto end_of_job;
    /* and null-terminate it */
    subexpr[sublen] = '\000';
    /* --- adjust margin for expressions containing \middle's --- */
    if (strtexchr(subexpr, "\\middle") != NULL)  /* have enclosed \middle's */
        /* so don't "overwhelm" them */
        margin = 1;
    /* --- check for operator delimiter --- */
    for (idelim = 0; opdelims[idelim] != NULL; idelim++)
        if (strstr(ldelim, opdelims[idelim]) != NULL) { /* found operator */
            /* extra height for operator */
            margin += opmargin;
            if (*ldelim == '\\')       /* have leading escape */
                /* squeeze it out */
                strcpy(ldelim, ldelim + 1);
            break;
        }              /* no need to check rest of table */
    /* --- xlate delimiters and check for textstyle --- */
    for (idelim = 1; idelim <= 2; idelim++) {  /* 1=left, 2=right */
        /* ldelim or rdelim */
        char  *lrdelim  = (idelim == 1 ? ldelim : rdelim);
        int   ix;
        /* xfrom[] and xto[] index, delim */
        char *xdelim;
        for (ix = 0; (xdelim = xfrom[ix]) != NULL; ix++)
            if (strcmp(lrdelim, xdelim) == 0) {  /* found delim to xlate */
                /* replace with corresponding xto[]*/
                strcpy(lrdelim, xto[ix]);
                break;
            }            /* no need to check further */
        for (ix = 0; (xdelim = textdelims[ix]) != NULL; ix++)
            if (strstr(lrdelim, xdelim) != 0) {  /* found textstyle delim */
                if (idelim == 1)         /* if it's the \left one */
                    /* set left textstyle flag */
                    istextleft = 1;
                /* else set right textstyle flag */
                else istextright = 1;
                break;
            }            /* no need to check further */
    } /* --- end-of-for(idelim) --- */
    /* --- debugging --- */
    if (mctx->msgfp != NULL && mctx->msglevel >= 99)
        fprintf(mctx->msgfp, "rastleft> left=\"%s\" right=\"%s\" subexpr=\"%s\"\n",
                ldelim, rdelim, subexpr);
    /* ------------------------------------------------------------
    rasterize subexpression
    ------------------------------------------------------------ */
    /* --- rasterize subexpression --- */
    if ((sp = rasterize(mctx, subexpr, size))  /* rasterize chars between delims */
            /* quit if failed */
            ==   NULL) goto end_of_job;
    /* height of subexpr raster */
    height = (sp->image)->height;
    /*default rheight as subexpr height*/
    rheight = height + margin;
    /* ------------------------------------------------------------
    rasterize delimiters, reset baselines, and add  sub/superscripts if present
    ------------------------------------------------------------ */
    /* --- check for dot delimiter --- */
    /* true if \left. */
    isleftdot  = (strchr(ldelim, '.') != NULL);
    /* true if \right. */
    isrightdot = (strchr(rdelim, '.') != NULL);
    /* --- get rasters for best-fit delim characters, add sub/superscripts --- */
    /* force \displaystyle */
    mctx->isdisplaystyle = (istextleft ? 0 : 9);
    if (!isleftdot)              /* if not \left. */
    {
    /* --- first get requested \left delimiter --- */
        /* get \left delim char */
        lp = get_delim(mctx, ldelim, rheight, family);
        /* --- reset lp delim baseline to center delim on subexpr raster --- */
        if (lp != NULL) {         /* if  succeeded */
            /* actual height of left delim */
            int lheight = (lp->image)->height;
            lp->baseline = sp->baseline + (lheight - height) / 2;
            if (lheight > rheight)         /* got bigger delim than requested */
                rheight = lheight - 1;
        }      /* make sure right delim matches */
        /* --- then add on any sub/superscripts attached to \left( --- */
        /*\left(_a^b and push pleft past b*/
        lp = rastlimits(mctx, &pleft, size, lp);
        isleftscript = mctx->isscripted;
    }     /* check if left delim scripted */
    /* force \displaystyle */
    mctx->isdisplaystyle = (istextright ? 0 : 9);
    if (!isrightdot)             /* and if not \right. */
    {
    /* --- first get requested \right delimiter --- */
        /* get \right delim char */
        rp = get_delim(mctx, rdelim, rheight, family);
        /* --- reset rp delim baseline to center delim on subexpr raster --- */
        if (rp != NULL)           /* if  succeeded */
            rp->baseline = sp->baseline + ((rp->image)->height - height) / 2;
        /* --- then add on any sub/superscripts attached to \right) --- */
        /*\right)_c^d, expression past d*/
        rp = rastlimits(mctx, expression, size, rp);
        isrightscript = mctx->isscripted;
    }    /* check if right delim scripted */
    /* original \displystyle default */
    mctx->isdisplaystyle = wasdisplaystyle;
    /* --- check that we got delimiters --- */
    if (0)
        if ((lp == NULL && !isleftdot)      /* check that we got left( */
                || (rp == NULL && !isrightdot)) {  /* and right) if needed */
            /* free \left-delim subraster */
            if (lp != NULL) free((void *)lp);
            /* and \right-delim subraster */
            if (rp != NULL) free((void *)rp);
            if (0) {
                /* if failed, free subraster */
                delete_subraster(mctx, sp);
                sp = (subraster *)NULL;
            }  /* signal error to caller */
            goto end_of_job;
        }          /* and quit */
    /* ------------------------------------------------------------
    concat  lp || sp || rp  components
    ------------------------------------------------------------ */
    /* --- concat lp||sp||rp to obtain final result --- */
    if (lp != NULL)              /* ignore \left. */
        /* concat lp||sp and free sp,lp */
        sp = rastcat(mctx, lp, sp, 3);
    if (sp != NULL)              /* succeeded or ignored \left. */
        if (rp != NULL)            /* ignore \right. */
            /* concat sp||rp and free sp,rp */
            sp = rastcat(mctx, sp, rp, 3);
    /* --- back to caller --- */
end_of_job:
    /* signal if right delim scripted */
    mctx->isdelimscript = isrightscript;
    return (sp);
} /* --- end-of-function rastleft() --- */


/* ==========================================================================
 * Function:    rastright ( expression, size, basesp, ildelim, arg2, arg3 )
 * Purpose: ...\right handler, intercepts an unexpected/unbalanced \right
 * --------------------------------------------------------------------------
 * Arguments:   expression (I)  char **  to first char of null-terminated
 *              string beginning with a \right
 *              to be rasterized
 *      size (I)    int containing 0-7 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding leading left{
 *              (unused, but passed for consistency)
 *      ildelim (I) int containing rdelims[] index of
 *              right delimiter
 *      arg2 (I)    int unused
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to subraster corresponding to subexpr,
 *              or NULL for any parsing error
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
subraster *rastright(mimetex_ctx *mctx, char **expression, int size, subraster *basesp,
                     int ildelim, int arg2, int arg3)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    subraster *sp = NULL;  /*rasterize \right subexpr's*/
    if (sp != NULL) {          /* returning entire expression */
        /* set flag to replace left half*/
        mctx->isreplaceleft = 1;
    }
    return (sp);
} /* --- end-of-function rastright() --- */


/* ==========================================================================
 * Function:    rastmiddle ( expression, size, basesp,  arg1, arg2, arg3 )
 * Purpose: \middle handler, returns subraster corresponding to
 *      entire expression with \middle delimiter(s) sized to fit.
 * --------------------------------------------------------------------------
 * Arguments:   expression (I/O) char **  to first char of null-terminated
 *              string immediately following \middle to be
 *              rasterized, and returning ptr immediately
 *              to terminating null.
 *      size (I)    int containing 0-7 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding \middle
 *              (unused, but passed for consistency)
 *      arg1 (I)    int unused
 *      arg2 (I)    int unused
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to subraster corresponding to expression,
 *              or NULL for any parsing error
 *              (expression ptr unchanged if error occurs)
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
subraster *rastmiddle(mimetex_ctx *mctx, char **expression, int size, subraster *basesp,
                      int arg1, int arg2, int arg3)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /*rasterize \middle subexpr's*/
    subraster *sp = NULL, *subsp[32];
    char *exprptr = *expression; /* local copy of ptr to expression */
    char delim[32][132]; /* delimiters following \middle's */
    char subexpr[MAXSUBXSZ+1];
    char *subptr = NULL; /*subexpression between \middle's*/
    /* height, above & below baseline */
    int height = 0, habove = 0, hbelow = 0;
    int idelim, ndelims = 0,    /* \middle count (max 32) */
        family = CMSYEX; /* delims from CMSY10 or CMEX10 */
    /* ------------------------------------------------------------
    initialization
    ------------------------------------------------------------ */
    /* expressn preceding 1st \middle */
    subsp[0] = mctx->leftexpression;
    /* set first null */
    subsp[1] = NULL;
    /* ------------------------------------------------------------
    accumulate subrasters between consecutive \middle\delim...\middle\delim...'s
    ------------------------------------------------------------ */
    while (ndelims < 30) {           /* max of 31 \middle's */
        /* --- maintain max height above,below baseline --- */
        if (subsp[ndelims] != NULL) {      /*exprssn preceding current \middle*/
            /* #rows above baseline */
            int baseline = (subsp[ndelims])->baseline;
            /* tot #rows (height) */
            height = ((subsp[ndelims])->image)->height;
            /* max #rows above baseline */
            habove = max2(habove, baseline);
            hbelow = max2(hbelow, height - baseline);
        } /* max #rows below baseline */
        /* --- get delimter after \middle --- */
        /*skip space betwn \middle & \delim*/
        skipwhite(exprptr);
        /* \delim after \middle */
        exprptr = texchar(mctx, exprptr, delim[ndelims]);
        if (*(delim[ndelims]) == '\000')   /* \middle at end-of-expression */
            /* ignore it and consider job done */
            break;
        /* count another \middle\delim */
        ndelims++;
        /* --- get subexpression between \delim and next \middle --- */
        /* no subexpresion yet */
        subsp[ndelims] = NULL;
        if (*exprptr == '\000')        /* end-of-expression after \delim */
            /* so we have all subexpressions */
            break;
        if ((subptr = strtexchr(exprptr, "\\middle")) /* find next \middle */
                ==   NULL) {              /* no more \middle's */
            /*get entire remaining expression*/
            strncpy(subexpr, exprptr, MAXSUBXSZ);
            /* make sure it's null-terminated */
            subexpr[MAXSUBXSZ] = '\000';
            exprptr += strlen(exprptr);
        }  /* push exprptr to terminating '\0'*/
        else {                /* have another \middle */
            /* #chars between \delim...\middle*/
            int sublen = (int)(subptr - exprptr);
            /* get subexpression */
            memcpy(subexpr, exprptr, min2(sublen, MAXSUBXSZ));
            /* and null-terminate it */
            subexpr[min2(sublen, MAXSUBXSZ)] = '\000';
            exprptr += (sublen + strlen("\\middle"));
        } /* push exprptr past \middle*/
        /* --- rasterize subexpression --- */
        /* rasterize subexpresion */
        subsp[ndelims] = rasterize(mctx, subexpr, size);
    } /* --- end-of-while(1) --- */
    /* ------------------------------------------------------------
    construct \middle\delim's and concatanate them between subexpressions
    ------------------------------------------------------------ */
    if (ndelims < 1          /* no delims */
            || (height = habove + hbelow) < 1)  /* or no subexpressions? */
        /* just flush \middle directive */
        goto end_of_job;
    for (idelim = 0; idelim <= ndelims; idelim++) {
        /* --- first add on subexpression preceding delim --- */
        if (subsp[idelim] != NULL) {   /* have subexpr preceding delim */
            if (sp == NULL) {            /* this is first piece */
                /* so just use it */
                sp = subsp[idelim];
                if (idelim == 0)
                    sp = subrastcpy(mctx, sp);
            } /* or copy mctx->leftexpression */
            else sp = rastcat(mctx, sp, subsp[idelim], (idelim > 0 ? 3 : 1));
        } /* or concat it */
        /* --- now construct delimiter --- */
        if (*(delim[idelim]) != '\000') {  /* have delimter */
            subraster *delimsp = get_delim(mctx, delim[idelim], height, family);
            if (delimsp != NULL) {      /* rasterized delim */
                /* set baseline */
                delimsp->baseline = habove;
                if (sp == NULL)          /* this is first piece */
                    /* so just use it */
                    sp = delimsp;
                else sp = rastcat(mctx, sp, delimsp, 3);
            }
        } /*or concat to existing pieces*/
    } /* --- end-of-for(idelim) --- */
    /* --- back to caller --- */
end_of_job:
    if (0)   /* now handled above */
        for (idelim = 1; idelim <= ndelims; idelim++) /* free subsp[]'s (not 0) */
            if (subsp[idelim] != NULL)      /* have allocated subraster */
                /* so free it */
                delete_subraster(mctx, subsp[idelim]);
    if (sp != NULL) {          /* returning entire expression */
        /* height of returned subraster */
        int newht = (sp->image)->height;
        /* guess new baseline */
        sp->baseline = min2(newht - 1, newht / 2 + 5);
        /* set flag to replace left half*/
        mctx->isreplaceleft = 1;
        *expression += strlen(*expression);
    } /* and push to terminating null*/
    return (sp);
} /* --- end-of-function rastmiddle() --- */


/* ==========================================================================
 * Function:    rastflags ( expression, size, basesp,  flag, value, arg3 )
 * Purpose: sets an internal flag, e.g., for \rm, or sets an internal
 *      value, e.g., for \mctx->unitlength=<value>, and returns NULL
 *      so nothing is displayed
 * --------------------------------------------------------------------------
 * Arguments:   expression (I)  char **  to first char of null-terminated
 *              LaTeX expression (unused/unchanged)
 *      size (I)    int containing base font size (not used,
 *              just stored in subraster)
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding "flags" directive
 *              (unused but passed for consistency)
 *      flag (I)    int containing #define'd symbol specifying
 *              internal flag to be set
 *      value (I)   int containing new value of flag
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) NULL so nothing is displayed
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
subraster *rastflags(mimetex_ctx *mctx, char **expression, int size, subraster *basesp,
                     int flag, int value, int arg3)
{
    /* value from expression, if needed */
    char valuearg[1024] = "NOVALUE";
    int argvalue = NOVALUE,     /* atoi(valuearg) */
        isdelta = 0,        /* true if + or - precedes valuearg */
        valuelen = 0; /* strlen(valuearg) */
    /*convert ascii {valuearg} to double*/
    double  dblvalue = (-99.), strtod();
    /* \displaystyle set at mctx->recurlevel */
    static  int displaystylelevel = (-99);
    /* ------------------------------------------------------------
    set flag or value
    ------------------------------------------------------------ */
    switch (flag) {
    default:
        /* unrecognized flag */
        break;
    case ISFONTFAM:
        if (isthischar((*(*expression)), WHITEMATH))  /* \rm followed by white */
            /* skip leading ~ after \rm */
            (*expression)++;
        /* set font family */
        mctx->fontnum = value;
        break;
    case ISSTRING:
        mctx->isstring = value;
        break;
        /* set string/image mode */
    case ISDISPLAYSTYLE:          /* set \displaystyle mode */
        /* \displaystyle set at mctx->recurlevel */
        displaystylelevel = mctx->recurlevel;
        mctx->isdisplaystyle = value;
        break;
    case ISOPAQUE:
        mctx->istransparent = value;
        break;
        /* set transparent/opaque */
    case ISREVERSE:           /* reverse video */
        if (value == 1 || value == NOVALUE) {
            mctx->fgred = 255 - mctx->fgred;
            mctx->fggreen = 255 - mctx->fggreen;
            mctx->fgblue = 255 - mctx->fgblue;
        }
        if (value == 2 || value == NOVALUE) {
            mctx->bgred = 255 - mctx->bgred;
            mctx->bggreen = 255 - mctx->bggreen;
            mctx->bgblue = 255 - mctx->bgblue;
        }
        if (value == 2 || value == NOVALUE)
            mctx->isblackonwhite = !mctx->isblackonwhite;
        if (mctx->gammacorrection > 0.0001)    /* have gamma correction */
            /* use reverse video gamma instead */
            mctx->gammacorrection = REVERSEGAMMA;
        break;
    case ISFONTSIZE:          /* set mctx->fontsize */
    case ISDISPLAYSIZE:           /* set mctx->displaysize */
    case ISCONTENTTYPE:           /*enable/disable content-type lines*/
    case ISAAALGORITHM:           /* set anti-aliasing algorithm */
    case ISWEIGHT:            /* set font weight */
    case ISCENTERWT:          /* set lowpass center pixel weight */
    case ISADJACENTWT:            /* set lowpass adjacent weight */
    case ISCORNERWT:          /* set lowpass corner weight */
    case ISCOLOR:             /* set red(1),green(2),blue(3) */
    case ISSMASH:             /* set (minimum) "smash" margin */
    case ISGAMMA:             /* set gamma correction */
        if (value != NOVALUE) {      /* passed a fixed value to be set */
            /* set given fixed int value */
            argvalue = value;
            dblvalue = (double)value;
        } /* or maybe interpreted as double */
        else {              /* get value from expression */
            *expression = texsubexpr(mctx, *expression, valuearg, 1023, "{", "}", 0, 0);
            if (*valuearg != '\000')     /* guard against empty string */
                if (!isalpha(*valuearg))    /* and against alpha string args */
                    if (!isthischar(*valuearg, "?")) { /*leading ? is query for value*/
                        /* leading + or - */
                        isdelta = isthischar(*valuearg, "+-");
                        if (memcmp(valuearg, "--", 2) == 0) { /* leading -- signals...*/
                            /* ...not delta */
                            isdelta = 0;
                            strcpy(valuearg, valuearg + 1);
                        }
                        switch (flag) {         /* convert to double or int */
                        default:
                            argvalue = atoi(valuearg);
                            /* convert to int */
                            break;
                        case ISGAMMA:
                            dblvalue = strtod(valuearg, NULL);
                            break;
                        } /* or to double */
                    } /* --- end-of-if(*valuearg!='?') --- */
        } /* --- end-of-if(value==NOVALUE) --- */
        switch (flag) {
        default:
            break;
        case ISCOLOR:         /* set color */
            /* convert arg to lower case */
            slower(valuearg);
            if (argvalue == 1 || strstr(valuearg, "red")) {
                mctx->fggreen = mctx->fgblue = (mctx->isblackonwhite ? 0 : 255);
                mctx->fgred = (mctx->isblackonwhite ? 255 : 0);
            }
            if (argvalue == 2 || strstr(valuearg, "green")) {
                mctx->fgred = mctx->fgblue = (mctx->isblackonwhite ? 0 : 255);
                mctx->fggreen = (mctx->isblackonwhite ? 255 : 0);
            }
            if (argvalue == 3 || strstr(valuearg, "blue")) {
                mctx->fgred = mctx->fggreen = (mctx->isblackonwhite ? 0 : 255);
                mctx->fgblue = (mctx->isblackonwhite ? 255 : 0);
            }
            if (argvalue == 0 || strstr(valuearg, "black"))
                mctx->fgred = mctx->fggreen = mctx->fgblue = (mctx->isblackonwhite ? 0 : 255);
            if (argvalue == 7 || strstr(valuearg, "white"))
                mctx->fgred = mctx->fggreen = mctx->fgblue = (mctx->isblackonwhite ? 255 : 0);
            break;
        case ISFONTSIZE:          /* set mctx->fontsize */
            if (argvalue != NOVALUE) {   /* got a value */
                int largestsize = LARGESTSIZE;
                mctx->fontsize = (isdelta ? mctx->fontsize + argvalue : argvalue);
                mctx->fontsize = max2(0, min2(mctx->fontsize, largestsize));
                mctx->shrinkfactor = shrinkfactors[mctx->fontsize];
                if (mctx->isdisplaystyle == 1  /* displaystyle enabled but not set*/
                        || (1 && mctx->isdisplaystyle == 2) /* displaystyle enabled and set */
                        || (0 && mctx->isdisplaystyle == 0))/*\textstyle disabled displaystyle*/
                    if (displaystylelevel != mctx->recurlevel)   /*respect \displaystyle*/
                        if (!mctx->ispreambledollars)  {   /* respect $$...$$'s */
                            if (mctx->fontsize >= mctx->displaysize)
                                /* forced */
                                mctx->isdisplaystyle = 2;
                            else mctx->isdisplaystyle = 1;
                        }
                /*displaystylelevel = (-99);*/
            } /* reset \displaystyle level */
            else {              /* embed font size in expression */
                /* convert size */
                sprintf(valuearg, "%d", mctx->fontsize);
                /* ought to be 1 */
                valuelen = strlen(valuearg);
                if (*expression != '\000') { /* ill-formed expression */
                    /*back up buff*/
                    *expression = (char *)(*expression - valuelen);
                    memcpy(*expression, valuearg, valuelen);
                }
            } /*and put in size*/
            break;
        case ISDISPLAYSIZE:       /* set mctx->displaysize */
            if (argvalue != NOVALUE)     /* got a value */
                mctx->displaysize = (isdelta ? mctx->displaysize + argvalue : argvalue);
            break;
        case ISSMASH:         /* set (minimum) "smash" margin */
            if (argvalue != NOVALUE) {   /* got a value */
                /* set value */
                mctx->smashmargin = argvalue;
                /* hard-coded isdelta */
                if (arg3 != NOVALUE) isdelta = arg3;
                mctx->issmashdelta = (isdelta ? 1 : 0);
            } /* and set delta flag */
            /*sanity*/
            mctx->smashmargin = max2((isdelta ? -5 : 0), min2(mctx->smashmargin, 32));
            /* signal explicit \smash directive*/
            mctx->isexplicitsmash = 1;
            break;
        case ISAAALGORITHM:       /* set anti-aliasing algorithm */
            if (argvalue != NOVALUE) {   /* got a value */
                if (argvalue >= 0) {   /* non-negative to set algorithm */
                    /* set algorithm number */
                    mctx->aaalgorithm = argvalue;
                    mctx->aaalgorithm = max2(0, min2(mctx->aaalgorithm, 4));
                } /* bounds check */
                else mctx->maxfollow = abs(argvalue);
            } /* or mctx->maxfollow=abs(negative#) */
            break;
        case ISWEIGHT:            /* set font weight number */
            value = (argvalue == NOVALUE ? NOVALUE : /* don't have a value */
                     (isdelta ? mctx->weightnum + argvalue : argvalue));
            if (value >= 0 && value < mctx->maxaaparams) { /* in range */
                /* reset mctx->weightnum index */
                mctx->weightnum   = value;
                mctx->minadjacent = aaparams[mctx->weightnum].minadjacent;
                mctx->maxadjacent = aaparams[mctx->weightnum].maxadjacent;
                mctx->cornerwt    = aaparams[mctx->weightnum].cornerwt;
                mctx->adjacentwt  = aaparams[mctx->weightnum].adjacentwt;
                mctx->centerwt    = aaparams[mctx->weightnum].centerwt;
                mctx->fgalias     = aaparams[mctx->weightnum].fgalias;
                mctx->fgonly      = aaparams[mctx->weightnum].fgonly;
                mctx->bgalias     = aaparams[mctx->weightnum].bgalias;
                mctx->bgonly      = aaparams[mctx->weightnum].bgonly;
            }
            break;
        case ISCENTERWT:          /* set lowpass center pixel weight */
            if (argvalue != NOVALUE)     /* got a value */
                /* set lowpass center weight */
                mctx->centerwt = argvalue;
            break;
        case ISADJACENTWT:        /* set lowpass adjacent weight */
            if (argvalue != NOVALUE)     /* got a value */
                /* set lowpass adjacent weight */
                mctx->adjacentwt = argvalue;
            break;
        case ISCORNERWT:          /* set lowpass corner weight */
            if (argvalue != NOVALUE)     /* got a value */
                /* set lowpass corner weight */
                mctx->cornerwt = argvalue;
            break;
        case ISGAMMA:         /* set gamma correction */
            if (dblvalue >= 0.0)         /* got a value */
                /* set gamma correction */
                mctx->gammacorrection = dblvalue;
            break;
        } /* --- end-of-switch() --- */
        break;
    case PNMPARAMS:           /*set mctx->fgalias,mctx->fgonly,mctx->bgalias,mctx->bgonly*/
        *expression = texsubexpr(mctx, *expression, valuearg, 1023, "{", "}", 0, 0);
        /* ought to be 1-4 */
        valuelen = strlen(valuearg);
        if (valuelen > 0 && isthischar(toupper(valuearg[0]), "TY1")) mctx->fgalias = 1;
        if (valuelen > 0 && isthischar(toupper(valuearg[0]), "FN0")) mctx->fgalias = 0;
        if (valuelen > 1 && isthischar(toupper(valuearg[1]), "TY1")) mctx->fgonly = 1;
        if (valuelen > 1 && isthischar(toupper(valuearg[1]), "FN0")) mctx->fgonly = 0;
        if (valuelen > 2 && isthischar(toupper(valuearg[2]), "TY1")) mctx->bgalias = 1;
        if (valuelen > 2 && isthischar(toupper(valuearg[2]), "FN0")) mctx->bgalias = 0;
        if (valuelen > 3 && isthischar(toupper(valuearg[3]), "TY1")) mctx->bgonly = 1;
        if (valuelen > 3 && isthischar(toupper(valuearg[3]), "FN0")) mctx->bgonly = 0;
        break;
    case UNITLENGTH:
        if (value != NOVALUE)        /* passed a fixed value to be set */
            /* set given fixed value */
            mctx->unitlength = (double)(value);
        else {              /* get value from expression */
            *expression = texsubexpr(mctx, *expression, valuearg, 1023, "{", "}", 0, 0);
            if (*valuearg != '\000')     /* guard against empty string */
                mctx->unitlength = strtod(valuearg, NULL);
        } /* convert to double */
        break;
    } /* --- end-of-switch(flag) --- */
    /*just set value, nothing to display*/
    return (NULL);
} /* --- end-of-function rastflags() --- */


/* ==========================================================================
 * Function:    rastspace(expression, size, basesp,  width, isfill, isheight)
 * Purpose: returns a blank/space subraster width wide,
 *      with baseline and height corresponding to basep
 * --------------------------------------------------------------------------
 * Arguments:   expression (I)  char **  to first char of null-terminated
 *              LaTeX expression (unused/unchanged)
 *      size (I)    int containing base font size (not used,
 *              just stored in subraster)
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding space, whose baseline
 *              and height params are transferred to space
 *      width (I)   int containing #bits/pixels for space width
 *      isfill (I)  int containing true to \hfill complete
 *              expression out to width
 *              (Kludge: isfill=99 signals \hspace*
 *              for negative space)
 *      isheight (I)    int containing true (but not NOVALUE)
 *              to treat width arg as height
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to empty/blank subraster
 *              or NULL for any error
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
subraster *rastspace(mimetex_ctx *mctx, char **expression, int size, subraster *basesp,
                     int width, int isfill, int isheight)
{
    /* subraster for space */
    subraster *spacesp = NULL;
    /* for negative space */
    raster  *bp = NULL;
    /* if fail, free unneeded subraster*/
    /* height,baseline of base symbol */
    int baseht = 1, baseln = 0;
    /*default #bits per pixel, 1=bitmap*/
    int pixsz = 1;
    /* defaults for negative hspace */
    int isstar = 0, minspace = 0;
    /* parse for optional {width} */
    char widtharg[256];
    subraster *rightsp = NULL;
    /* ------------------------------------------------------------
    initialization
    ------------------------------------------------------------ */
    if (isfill > 1) {
        /* large fill signals \hspace* */
        isstar = 1;
        isfill = 0;
    }
    /* novalue means false */
    if (isfill == NOVALUE) isfill = 0;
    /* novalue means false */
    if (isheight == NOVALUE) isheight = 0;
    /* reset default minspace */
    minspace = (isstar ? (-1) : 0);
    /* ------------------------------------------------------------
    determine width if not given (e.g., \hspace{width}, \hfill{width})
    ------------------------------------------------------------ */
    if (width == 0) {            /* width specified in expression */
        double dwidth;
        /* test {width} before using it */
        int widthval;
        /* \hspace allows negative */
        int minwidth = (isfill || isheight ? 1 : -600);
        /* --- check if optional [minspace] given for negative \hspace --- */
        if (*(*expression) == '[') {   /* [minspace] if leading char is [ */
            /* ---parse [minspace], bump expression past it, interpret as double--- */
            *expression = texsubexpr(mctx, *expression, widtharg, 127, "[", "]", 0, 0);
            if (*widtharg != '\000')         /* got [minspace] */
                /* in pixels */
                minspace = iround(mctx->unitlength * strtod(widtharg, NULL));
        } /* --- end-of-if(*(*expression)=='[') --- */
        /* set default width */
        width = 1;
        *expression = texsubexpr(mctx, *expression, widtharg, 255, "{", "}", 0, 0);
        /* scaled width value */
        dwidth = mctx->unitlength * strtod(widtharg, NULL);
        widthval =                /* convert {width} to integer */
            (int)(dwidth + (dwidth >= 0.0 ? 0.5 : (-0.5)));
        if (widthval >= minwidth && widthval <= 600) /* sanity check */
            /* replace deafault width */
            width = widthval;
    } /* --- end-of-if(width==0) --- */
    /* ------------------------------------------------------------
    first check for negative space
    ------------------------------------------------------------ */
    if (width < 0) {             /* have negative hspace */
        if (mctx->leftexpression != (subraster *)NULL)   /* can't backspace */
            if ((spacesp = new_subraster(mctx, 0, 0, 0)) /* get new subraster for backspace */
                    !=   NULL) {              /* and if we succeed... */
                /*#pixels wanted,actually backspaced*/
                int nback = (-width), pback;
                if ((bp = backspace_raster(mctx, mctx->leftexpression->image, nback, &pback, minspace, 0))
                        !=    NULL) {            /* and if backspace succeeds... */
                    /* save backspaced image */
                    spacesp->image = bp;
                    /*spacesp->type = mctx->leftexpression->type;*/ /* copy original type */
                    /* need to propagate blanks */
                    spacesp->type = mctx->blanksignal;
                    /* copy original font size */
                    spacesp->size = mctx->leftexpression->size;
                    /* and baseline */
                    spacesp->baseline = mctx->leftexpression->baseline;
                    /* wanted more than we got */
                    mctx->blanksymspace += -(nback - pback);
                    mctx->isreplaceleft = 1;
                }       /*signal to replace entire expressn*/
                else {               /* backspace failed */
                    /* free unneeded envelope */
                    delete_subraster(mctx, spacesp);
                    spacesp = (subraster *)NULL;
                }
            }   /* and signal failure */
        goto end_of_job;
    } /* --- end-of-if(width<0) --- */
    /* ------------------------------------------------------------
    see if width is "absolute" or fill width
    ------------------------------------------------------------ */
    if (isfill               /* called as \hfill{} */
            &&   !isheight) {           /* parameter conflict */
        if (mctx->leftexpression != NULL)   /* if we have left half */
            /*reduce left width from total*/
            width -= (mctx->leftexpression->image)->width;
        if ((rightsp = rasterize(mctx, *expression, size)) /* rasterize right half */
                != NULL)                 /* succeeded */
            width -= (rightsp->image)->width;
    } /* reduce right width from total */
    /* ------------------------------------------------------------
    construct blank subraster, and return it to caller
    ------------------------------------------------------------ */
    /* --- get parameters from base symbol --- */
    if (basesp != (subraster *)NULL) {   /* we have base symbol for space */
        /* height of base symbol */
        baseht = (basesp->image)->height;
        baseln =  basesp->baseline;
    }   /* and its baseline */
    /* --- flip params for height --- */
    if (isheight) {              /* width is actually height */
        /* use given width as height */
        baseht = width;
        width = 1;
    }            /* and set default width */
    /* --- generate and init space subraster --- */
    if (width > 0)           /*make sure we have positive width*/
        if ((spacesp = new_subraster(mctx, width, baseht, pixsz)) /*generate space subraster*/
                !=   NULL)                 /* and if we succeed... */
        {
        /* --- ...re-init subraster parameters --- */
            /*propagate base font size forward*/
            spacesp->size = size;
            /* need to propagate blanks (???) */
            if (1)spacesp->type = mctx->blanksignal;
            spacesp->baseline = baseln;
        }   /* ditto baseline */
    /* ------------------------------------------------------------
    concat right half if \hfill-ing
    ------------------------------------------------------------ */
    if (rightsp != NULL) {           /* we have a right half after fill */
        spacesp = (spacesp == NULL ? rightsp :  /* no space, so just use right half*/
                   /* or cat right half after space */
                   rastcat(mctx, spacesp, rightsp, 3));
        /* need to propagate blanks */
        spacesp->type = mctx->blanksignal;
        *expression += strlen((*expression));
    } /* push expression to its null */
end_of_job:
    return (spacesp);
} /* --- end-of-function rastspace() --- */


/* ==========================================================================
 * Function:    rastnewline ( expression, size, basesp,  arg1, arg2, arg3 )
 * Purpose: \\ handler, returns subraster corresponding to
 *      left-hand expression preceding \\ above right-hand expression
 * --------------------------------------------------------------------------
 * Arguments:   expression (I/O) char **  to first char of null-terminated
 *              string immediately following \\ to be
 *              rasterized, and returning ptr immediately
 *              to terminating null.
 *      size (I)    int containing 0-7 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding \\
 *              (unused, but passed for consistency)
 *      arg1 (I)    int unused
 *      arg2 (I)    int unused
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to subraster corresponding to expression,
 *              or NULL for any parsing error
 *              (expression ptr unchanged if error occurs)
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
subraster *rastnewline(mimetex_ctx *mctx, char **expression, int size, subraster *basesp,
                       int arg1, int arg2, int arg3)
{
    subraster *newlsp = NULL;
    /*rasterize right half of expression*/
    subraster *rightsp = NULL;
    char spacexpr[129]/*, *xptr=spacexpr*/; /*for \\[vspace]*/
    /* convert ascii param to double */
    /* #pixels between lines */
    int vspace = size + 2;
    /* ------------------------------------------------------------
    obtain optional [vspace] argument immediately following \\ command
    ------------------------------------------------------------ */
    /* --- check if [vspace] given --- */
    if (*(*expression) == '[') {     /*have [vspace] if leading char is [*/
        /* ---parse [vspace] and bump expression past it, interpret as double--- */
        *expression = texsubexpr(mctx, *expression, spacexpr, 127, "[", "]", 0, 0);
        /* couldn't get [vspace] */
        if (*spacexpr == '\000') goto end_of_job;
        /* vspace in pixels */
        vspace = iround(mctx->unitlength * strtod(spacexpr, NULL));
    } /* --- end-of-if(*(*expression)=='[') --- */
    /* nothing preceding \\ */
    if (mctx->leftexpression == NULL) goto end_of_job;
    /* ------------------------------------------------------------
    rasterize right half of expression and stack left half above it
    ------------------------------------------------------------ */
    /* --- rasterize right half --- */
    if ((rightsp = rasterize(mctx, *expression, size)) /* rasterize right half */
            /* quit if failed */
            == NULL) goto end_of_job;
    /* --- stack left half above it --- */
    /*newlsp = rastack(rightsp,mctx->leftexpression,1,vspace,0,3);*//*right under left*/
    /*right under left*/
    newlsp = rastack(mctx, rightsp, mctx->leftexpression, 1, vspace, 0, 1);
    /* --- back to caller --- */
end_of_job:
    if (newlsp != NULL) {          /* returning entire expression */
        /* height of returned subraster */
        int newht = (newlsp->image)->height;
        /* guess new baseline */
        newlsp->baseline = min2(newht - 1, newht / 2 + 5);
        /* so set flag to replace left half*/
        mctx->isreplaceleft = 1;
        *expression += strlen(*expression);
    } /* and push to terminating null*/
    /* 1st line over 2nd, or null=error*/
    return (newlsp);
} /* --- end-of-function rastnewline() --- */


/* ==========================================================================
 * Function:    rastarrow ( expression, size, basesp,  drctn, isBig, arg3 )
 * Purpose: returns left/right arrow subraster (e.g., for \longrightarrow)
 * --------------------------------------------------------------------------
 * Arguments:   expression (I)  char **  to first char of null-terminated
 *              LaTeX expression (unused/unchanged)
 *      size (I)    int containing base font size (not used,
 *              just stored in subraster)
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding space, whose baseline
 *              and height params are transferred to space
 *      drctn (I)   int containing +1 for right, -1 for left,
 *              or 0 for leftright
 *      isBig (I)   int containing 0 for ---> or 1 for ===>
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to left/right arrow subraster
 *              or NULL for any error
 * --------------------------------------------------------------------------
 * Notes:     o An optional argument [width] may *immediately* follow
 *      the \longxxx to explicitly set the arrow's width in pixels.
 *      For example, \longrightarrow calculates a default width
 *      (as usual in LaTeX), whereas \longrightarrow[50] explicitly
 *      draws a 50-pixel long arrow.  This can be used, e.g.,
 *      to draw commutative diagrams in conjunction with
 *      \array (and maybe with \stackrel and/or \relstack, too).
 *        o In case you really want to render, say, [f]---->[g], just
 *      use an intervening space, i.e., [f]\longrightarrow~[g].
 *      In text mode use two spaces {\rm~[f]\longrightarrow~~[g]}.
 * ======================================================================= */
/* --- entry point --- */
subraster *rastarrow(mimetex_ctx *mctx, char **expression, int size, subraster *basesp,
                     int drctn, int isBig, int arg3)
{
    subraster *arrowsp = NULL;
    /* parse for optional [width] */
    char widtharg[256];
    /* and _^limits after [width]*/
    char sub[1024], super[1024];
    /*rasterize limits*/
    subraster *subsp = NULL, *supsp = NULL;
    /*space below arrow*/
    subraster *rastack(), *spacesp = NULL;
    /* width, height for \longxxxarrow */
    int width = 10 + 8 * size,  height;
    /*true to handle limits internally*/
    int islimits = 1;
    /* font size for limits */
    int limsize = size - 1;
    /* #empty rows below arrow */
    int vspace = 1;
    /*default #bits per pixel, 1=bitmap*/
    int pixsz = 1;
    /* ------------------------------------------------------------
    construct longleft/rightarrow subraster, with limits, and return it to caller
    ------------------------------------------------------------ */
    /* --- check for optional width arg and replace default width --- */
    if (*(*expression) == '[') {     /*check for []-enclosed optional arg*/
        /* test [width] before using it */
        int widthval;
        *expression = texsubexpr(mctx, *expression, widtharg, 255, "[", "]", 0, 0);
        widthval =              /* convert [width] to integer */
            (int)((mctx->unitlength * strtod(widtharg, NULL)) + 0.5);
        if (widthval >= 2 && widthval <= 600)  /* sanity check */
            width = widthval;
    }       /* replace deafault width */
    /* --- now parse for limits, and bump expression past it(them) --- */
    if (islimits) {              /* handling limits internally */
        /* parse for limits */
        *expression = texscripts(mctx, *expression, sub, super, 3);
        if (*sub != '\000')          /*have a subscript following arrow*/
            /* so try to rasterize subscript */
            subsp = rasterize(mctx, sub, limsize);
        if (*super != '\000')        /*have superscript following arrow*/
            supsp = rasterize(mctx, super, limsize);
    } /*so try to rasterize superscript*/
    /* --- set height based on width --- */
    /* height based on width */
    height = min2(17, max2(9, (width + 2) / 6));
    /* always force odd height */
    height = 1 + (height / 2) * 2;
    /* --- generate arrow subraster --- */
    if ((arrowsp = arrow_subraster(mctx, width, height, pixsz, drctn, isBig)) /*build arrow*/
            /* and quit if we failed */
            ==   NULL) goto end_of_job;
    /* --- add space below arrow --- */
    if (vspace > 0)              /* if we have space below arrow */
        if ((spacesp = new_subraster(mctx, width, vspace, pixsz)) /*allocate required space*/
                !=   NULL)                /* and if we succeeded */
            if ((arrowsp = rastack(mctx, spacesp, arrowsp, 2, 0, 1, 3)) /* space below arrow */
                    /* and quit if we failed */
                    ==   NULL) goto end_of_job;
    /* --- init arrow subraster parameters --- */
    /*propagate base font size forward*/
    arrowsp->size = size;
    /* set baseline at bottom of arrow */
    arrowsp->baseline = height + vspace - 1;
    /* --- add limits above/below arrow, as necessary --- */
    if (subsp != NULL)           /* stack subscript below arrow */
        if ((arrowsp = rastack(mctx, subsp, arrowsp, 2, 0, 1, 3)) /* subscript below arrow */
                /* quit if failed */
                ==   NULL) goto end_of_job;
    if (supsp != NULL)           /* stack superscript above arrow */
        if ((arrowsp = rastack(mctx, arrowsp, supsp, 1, vspace, 1, 3)) /*supsc above arrow*/
                /* quit if failed */
                ==   NULL) goto end_of_job;
    /* --- return arrow (or NULL) to caller --- */
end_of_job:
    return (arrowsp);
} /* --- end-of-function rastarrow() --- */


/* ==========================================================================
 * Function:    rastuparrow ( expression, size, basesp,  drctn, isBig, arg3 )
 * Purpose: returns an up/down arrow subraster (e.g., for \longuparrow)
 * --------------------------------------------------------------------------
 * Arguments:   expression (I)  char **  to first char of null-terminated
 *              LaTeX expression (unused/unchanged)
 *      size (I)    int containing base font size (not used,
 *              just stored in subraster)
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding space, whose baseline
 *              and height params are transferred to space
 *      drctn (I)   int containing +1 for up, -1 for down,
 *              or 0 for updown
 *      isBig (I)   int containing 0 for ---> or 1 for ===>
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to up/down arrow subraster
 *              or NULL for any error
 * --------------------------------------------------------------------------
 * Notes:     o An optional argument [height] may *immediately* follow
 *      the \longxxx to explicitly set the arrow's height in pixels.
 *      For example, \longuparrow calculates a default height
 *      (as usual in LaTeX), whereas \longuparrow[25] explicitly
 *      draws a 25-pixel high arrow.  This can be used, e.g.,
 *      to draw commutative diagrams in conjunction with
 *      \array (and maybe with \stackrel and/or \relstack, too).
 *        o In case you really want to render, say, [f]---->[g], just
 *      use an intervening space, i.e., [f]\longuparrow~[g].
 *      In text use two spaces {\rm~[f]\longuparrow~~[g]}.
 * ======================================================================= */
/* --- entry point --- */
subraster *rastuparrow(mimetex_ctx *mctx, char **expression, int size, subraster *basesp,
                       int drctn, int isBig, int arg3)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* subraster for arrow */
    subraster *arrowsp = NULL;
    /* parse for optional [height] */
    char heightarg[256];
    /* and _^limits after [width]*/
    char sub[1024], super[1024];
    /*rasterize limits*/
    subraster *subsp = NULL, *supsp = NULL;
    /* height, width for \longxxxarrow */
    int height = 8 + 2 * size,  width;
    /*true to handle limits internally*/
    int islimits = 1;
    /* font size for limits */
    int limsize = size - 1;
    /*default #bits per pixel, 1=bitmap*/
    int pixsz = 1;
    /* ------------------------------------------------------------
    construct blank subraster, and return it to caller
    ------------------------------------------------------------ */
    /* --- check for optional height arg and replace default height --- */
    if (*(*expression) == '[') {     /*check for []-enclosed optional arg*/
        /* test height before using it */
        int heightval;
        *expression = texsubexpr(mctx, *expression, heightarg, 255, "[", "]", 0, 0);
        heightval =             /* convert [height] to integer */
            (int)((mctx->unitlength * strtod(heightarg, NULL)) + 0.5);
        if (heightval >= 2 && heightval <= 600) /* sanity check */
            height = heightval;
    }     /* replace deafault height */
    /* --- now parse for limits, and bump expression past it(them) --- */
    if (islimits) {              /* handling limits internally */
        /* parse for limits */
        *expression = texscripts(mctx, *expression, sub, super, 3);
        if (*sub != '\000')          /*have a subscript following arrow*/
            /* so try to rasterize subscript */
            subsp = rasterize(mctx, sub, limsize);
        if (*super != '\000')        /*have superscript following arrow*/
            supsp = rasterize(mctx, super, limsize);
    } /*so try to rasterize superscript*/
    /* --- set width based on height --- */
    /* width based on height */
    width = min2(17, max2(9, (height + 2) / 4));
    /* always force odd width */
    width = 1 + (width / 2) * 2;
    /* --- generate arrow subraster --- */
    if ((arrowsp = uparrow_subraster(mctx, width, height, pixsz, drctn, isBig)) /*build arr*/
            /* and quit if we failed */
            ==   NULL) goto end_of_job;
    /* --- init arrow subraster parameters --- */
    /*propagate base font size forward*/
    arrowsp->size = size;
    /* set baseline at bottom of arrow */
    arrowsp->baseline = height - 1;
    /* --- add limits above/below arrow, as necessary --- */
    if (supsp != NULL) {         /* cat superscript to left of arrow*/
        int supht = (supsp->image)->height, /* superscript height */
                    /* baseline difference to center */
                    deltab = (1 + abs(height - supht)) / 2;
        /* force script baseline to bottom */
        supsp->baseline = supht - 1;
        if (supht <= height)       /* arrow usually taller than script*/
            /* so bottom of script goes here */
            arrowsp->baseline -= deltab;
        /* else bottom of arrow goes here */
        else  supsp->baseline -= deltab;
        if ((arrowsp = rastcat(mctx, supsp, arrowsp, 3)) /* superscript left of arrow */
                ==   NULL) goto end_of_job;
    }  /* quit if failed */
    if (subsp != NULL) {         /* cat subscript to right of arrow */
        int subht = (subsp->image)->height, /* subscript height */
                    /* baseline difference to center */
                    deltab = (1 + abs(height - subht)) / 2;
        /* reset arrow baseline to bottom */
        arrowsp->baseline = height - 1;
        /* force script baseline to bottom */
        subsp->baseline = subht - 1;
        if (subht <= height)       /* arrow usually taller than script*/
            /* so bottom of script goes here */
            arrowsp->baseline -= deltab;
        /* else bottom of arrow goes here */
        else  subsp->baseline -= deltab;
        if ((arrowsp = rastcat(mctx, arrowsp, subsp, 3)) /* subscript right of arrow */
                ==   NULL) goto end_of_job;
    }  /* quit if failed */
    /* --- return arrow (or NULL) to caller --- */
end_of_job:
    /* reset arrow baseline to bottom */
    arrowsp->baseline = height - 1;
    return (arrowsp);
} /* --- end-of-function rastuparrow() --- */


/* ==========================================================================
 * Function:    rastoverlay (expression, size, basesp, overlay, offset2, arg3)
 * Purpose: overlays one raster on another
 * --------------------------------------------------------------------------
 * Arguments:   expression (I/O) char **  to first char of null-terminated
 *              string immediately following overlay \cmd to
 *              be rasterized, and returning ptr immediately
 *              following last character processed.
 *      size (I)    int containing 0-7 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding overlay \cmd
 *              (unused, but passed for consistency)
 *      overlay (I) int containing 1 to overlay / (e.g., \not)
 *              or NOVALUE to pick up 2nd arg from expression
 *      offset2 (I) int containing #pixels to horizontally offset
 *              overlay relative to underlying symbol,
 *              positive(right) or negative or 0,
 *              or NOVALUE to pick up optional [offset] arg
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to subraster corresponding to composite,
 *              or NULL for any parsing error
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
subraster *rastoverlay(mimetex_ctx *mctx, char **expression, int size, subraster *basesp,
                       int overlay, int offset2, int arg3)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    char expr1[512], expr2[512]; /* base, overlay */
    subraster *sp1 = NULL, *sp2 = NULL; /*rasterize 1=base, 2=overlay*/
    /*subraster for composite overlay*/
    subraster *overlaysp = NULL;
    /* ------------------------------------------------------------
    Obtain base, and maybe overlay, and rasterize them
    ------------------------------------------------------------ */
    /* --- check for optional offset2 arg  --- */
    if (offset2 == NOVALUE)          /* only if not explicitly specified*/
        if (*(*expression) == '[') {        /*check for []-enclosed optional arg*/
            /* test before using it */
            int offsetval;
            *expression = texsubexpr(mctx, *expression, expr2, 511, "[", "]", 0, 0);
            /* convert [offset2] to int */
            offsetval = (int)(strtod(expr2, NULL) + 0.5);
            if (abs(offsetval) <= 25)        /* sanity check */
                offset2 = offsetval;
        }        /* replace deafault */
    /* novalue means no offset */
    if (offset2 == NOVALUE) offset2 = 0;
    /* --- parse for base, bump expression past it, and rasterize it --- */
    *expression = texsubexpr(mctx, *expression, expr1, 511, "{", "}", 0, 0);
    /* nothing to overlay, so quit */
    if (*expr1 == '\000') goto end_of_job;
    if ((sp1 = rasterize(mctx, expr1, size)) /* rasterize base expression */
            /* quit if failed to rasterize */
            ==   NULL) goto end_of_job;
    /*in case we return with no overlay*/
    overlaysp = sp1;
    /* --- get overlay expression, and rasterize it --- */
    if (overlay == NOVALUE) {        /* get overlay from input stream */
        *expression = texsubexpr(mctx, *expression, expr2, 511, "{", "}", 0, 0);
        if (*expr2 != '\000')        /* have an overlay */
            sp2 = rasterize(mctx, expr2, size);
    }    /* so rasterize overlay expression */
    else
    /* specific overlay */
        switch (overlay) {
        default:
            break;
        case 1:             /* e.g., \not overlays slash */
            /* rasterize overlay expression */
            sp2 = rasterize(mctx, "/", size + 1);
            offset2 = max2(1, size - 3);  /* push / right a bit */
            offset2 = 0;
            break;
        case 2:             /* e.g., \Not draws diagonal */
            /* no overlay required */
            sp2 = NULL;
            if (overlaysp != NULL) {       /* check that we have raster */
                /* raster to be \Not-ed */
                raster *rp = overlaysp->image;
                /* raster dimensions */
                int width = rp->width, height = rp->height;
                if (0)             /* diagonal within bounding box */
                    /* just draw diagonal */
                    line_raster(mctx, rp, 0, width - 1, height - 1, 0, 1);
                else {            /* construct "wide" diagonal */
                    /* desired extra margin width */
                    int margin = 3;
                    /*alloc it*/
                    sp2 = new_subraster(mctx, width + margin, height + margin, 1);
                    if (sp2 != NULL)        /* allocated successfully */
                        line_raster(mctx, sp2->image, 0, width + margin - 1, height + margin - 1, 0, 1);
                }
            }
            break;
        case 3:             /* e.g., \sout for strikeout */
            /* no overlay required */
            sp2 = NULL;
            if (overlaysp != NULL) {       /* check that we have raster */
                /* raster to be \Not-ed */
                raster *rp = overlaysp->image;
                /* raster dimensions */
                int width = rp->width, height = rp->height;
                /* we'll ignore descenders */
                int baseline = overlaysp->baseline;
                int midrow = max2(0, min2(height - 1, offset2 + ((baseline + 1) / 2)));
                if (1)             /* strikeout within bounding box */
                    line_raster(mctx, rp, midrow, 0, midrow, width - 1, 1);
            } /*draw strikeout*/
            break;
        } /* --- end-of-switch(overlay) --- */
    /*return sp1 if failed to rasterize*/
    if (sp2 == NULL) goto end_of_job;
    /* ------------------------------------------------------------
    construct composite overlay
    ------------------------------------------------------------ */
    overlaysp = rastcompose(mctx, sp1, sp2, offset2, 0, 3);
end_of_job:
    return (overlaysp);
} /* --- end-of-function rastoverlay() --- */


/* ==========================================================================
 * Function:    rastfrac ( expression, size, basesp,  isfrac, arg2, arg3 )
 * Purpose: \frac,\atop handler, returns a subraster corresponding to
 *      expression (immediately following \frac,\atop) at font size
 * --------------------------------------------------------------------------
 * Arguments:   expression (I/O) char **  to first char of null-terminated
 *              string immediately following \frac to be
 *              rasterized, and returning ptr immediately
 *              following last character processed.
 *      size (I)    int containing 0-7 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding \frac
 *              (unused, but passed for consistency)
 *      isfrac (I)  int containing true to draw horizontal line
 *              between numerator and denominator,
 *              or false not to draw it (for \atop).
 *      arg2 (I)    int unused
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to subraster corresponding to fraction,
 *              or NULL for any parsing error
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
subraster *rastfrac(mimetex_ctx *mctx, char **expression, int size, subraster *basesp,
                    int isfrac, int arg2, int arg3)
{
    /* parsed numer, denom */
    char numer[MAXSUBXSZ+1], denom[MAXSUBXSZ+1];
    /*rasterize numer, denom*/
    subraster *numsp = NULL, *densp = NULL;
    subraster *fracsp = NULL; /* subraster for numer/denom */
    int width = 0,          /* width of constructed raster */
        numheight = 0; /* height of numerator */
    /* height,baseline of base symbol */
    int baseht = 0, baseln = 0;
    /*int   istweak = 1;*/          /*true to tweak baseline alignment*/
    /* thickness of fraction line */
    int lineheight = 1;
    /*vertical space between components*/
    int vspace = (size > 2 ? 2 : 1);
    /* ------------------------------------------------------------
    Obtain numerator and denominator, and rasterize them
    ------------------------------------------------------------ */
    /* --- parse for numerator,denominator and bump expression past them --- */
    *expression = texsubexpr(mctx, *expression, numer, 0, "{", "}", 0, 0);
    *expression = texsubexpr(mctx, *expression, denom, 0, "{", "}", 0, 0);
    if (*numer == '\000' && *denom == '\000')  /* missing both components of frac */
        /* nothing to do, so quit */
        goto end_of_job;
    /* --- rasterize numerator, denominator --- */
    if (*numer != '\000')            /* have a numerator */
        if ((numsp = rasterize(mctx, numer, size - 1))  /* so rasterize numer at size-1 */
                /* and quit if failed */
                ==   NULL) goto end_of_job;
    if (*denom != '\000')            /* have a denominator */
        if ((densp = rasterize(mctx, denom, size - 1))  /* so rasterize denom at size-1 */
                ==   NULL) {               /* failed */
            if (numsp != NULL)       /* already rasterized numerator */
                /* so free now-unneeded numerator */
                delete_subraster(mctx, numsp);
            goto end_of_job;
        }          /* and quit */
    /* --- if one componenet missing, use a blank space for it --- */
    if (numsp == NULL)           /* no numerator given */
        /* missing numerator */
        numsp = rasterize(mctx, "[?]", size - 1);
    if (densp == NULL)           /* no denominator given */
        /* missing denominator */
        densp = rasterize(mctx, "[?]", size - 1);
    /* --- check that we got both components --- */
    if (numsp == NULL || densp == NULL) { /* some problem */
        /*delete numerator (if it existed)*/
        delete_subraster(mctx, numsp);
        /*delete denominator (if it existed)*/
        delete_subraster(mctx, densp);
        goto end_of_job;
    }          /* and quit */
    /* --- get height of numerator (to determine where line belongs) --- */
    /* get numerator's height */
    numheight = (numsp->image)->height;
    /* ------------------------------------------------------------
    construct raster with numerator stacked over denominator
    ------------------------------------------------------------ */
    /* --- construct raster with numer/denom --- */
    if ((fracsp = rastack(mctx, densp, numsp, 0, 2 * vspace + lineheight, 1, 3))/*numer/denom*/
            ==  NULL) {             /* failed to construct numer/denom */
        /* so free now-unneeded numerator */
        delete_subraster(mctx, numsp);
        /* and now-unneeded denominator */
        delete_subraster(mctx, densp);
        goto end_of_job;
    }          /* and then quit */
    /* --- determine width of constructed raster --- */
    /*just get width of embedded image*/
    width = (fracsp->image)->width;
    /* --- initialize subraster parameters --- */
    /* propagate font size forward */
    fracsp->size = size;
    /*default baseline*/
    fracsp->baseline = (numheight + vspace + lineheight) + (size + 2);
    /* signal \frac image */
    fracsp->type = FRACRASTER;
    if (basesp != (subraster *)NULL) {   /* we have base symbol for frac */
        /* height of base symbol */
        baseht = (basesp->image)->height;
        /* and its baseline */
        baseln =  basesp->baseline;
    } /* --- end-of-if(basesp!=NULL) --- */
    /* ------------------------------------------------------------
    draw horizontal line between numerator and denominator
    ------------------------------------------------------------ */
    /* signal that we have a \frac */
    mctx->fraccenterline = numheight + vspace;
    if (isfrac)                  /*line for \frac, but not for \atop*/
        rule_raster(mctx, fracsp->image, mctx->fraccenterline, 0, width, lineheight, 0);
    /* ------------------------------------------------------------
    return final result to caller
    ------------------------------------------------------------ */
end_of_job:
    if (mctx->msgfp != NULL && mctx->msglevel >= 99) {
        fprintf(mctx->msgfp, "rastfrac> returning %s\n", (fracsp == NULL ? "null" : "..."));
        if (fracsp != NULL)        /* have a constructed raster */
            type_raster(mctx, fracsp->image, mctx->msgfp);
    } /* display constructed raster */
    return (fracsp);
} /* --- end-of-function rastfrac() --- */


/* ==========================================================================
 * Function:    rastackrel ( expression, size, basesp,  base, arg2, arg3 )
 * Purpose: \stackrel handler, returns a subraster corresponding to
 *      stacked relation
 * --------------------------------------------------------------------------
 * Arguments:   expression (I/O) char **  to first char of null-terminated
 *              string immediately following \stackrel to be
 *              rasterized, and returning ptr immediately
 *              following last character processed.
 *      size (I)    int containing 0-7 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding \stackrel
 *              (unused, but passed for consistency)
 *      base (I)    int containing 1 if upper/first subexpression
 *              is base relation, or 2 if lower/second is
 *      arg2 (I)    int unused
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to subraster corresponding to stacked
 *              relation, or NULL for any parsing error
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
subraster *rastackrel(mimetex_ctx *mctx, char **expression, int size, subraster *basesp,
                      int base, int arg2, int arg3)
{
    /* parsed upper, lower */
    char upper[MAXSUBXSZ+1], lower[MAXSUBXSZ+1];
    /* rasterize upper, lower */
    subraster *upsp = NULL, *lowsp = NULL;
    subraster *relsp = NULL;  /* subraster for upper/lower */
    int upsize  = (base == 1 ? size : size - 1), /* font size for upper component */
                  /* font size for lower component */
                  lowsize = (base == 2 ? size : size - 1);
    /*vertical space between components*/
    int vspace = 1;
    /*free work areas in case of error*/
    /* ------------------------------------------------------------
    Obtain numerator and denominator, and rasterize them
    ------------------------------------------------------------ */
    /* --- parse for numerator,denominator and bump expression past them --- */
    *expression = texsubexpr(mctx, *expression, upper, 0, "{", "}", 0, 0);
    *expression = texsubexpr(mctx, *expression, lower, 0, "{", "}", 0, 0);
    if (*upper == '\000' || *lower == '\000')  /* missing either component */
        /* nothing to do, so quit */
        goto end_of_job;
    /* --- rasterize upper, lower --- */
    if (*upper != '\000')            /* have upper component */
        if ((upsp = rasterize(mctx, upper, upsize))   /* so rasterize upper component */
                /* and quit if failed */
                ==   NULL) goto end_of_job;
    if (*lower != '\000')            /* have lower component */
        if ((lowsp = rasterize(mctx, lower, lowsize)) /* so rasterize lower component */
                ==   NULL) {               /* failed */
            if (upsp != NULL)            /* already rasterized upper */
                /* so free now-unneeded upper */
                delete_subraster(mctx, upsp);
            goto end_of_job;
        }          /* and quit */
    /* ------------------------------------------------------------
    construct stacked relation raster
    ------------------------------------------------------------ */
    /* --- construct stacked relation --- */
    if ((relsp = rastack(mctx, lowsp, upsp, 3 - base, vspace, 1, 3)) /* stacked relation */
            /* quit if failed */
            ==   NULL) goto end_of_job;
    /* --- initialize subraster parameters --- */
    /* propagate font size forward */
    relsp->size = size;
    /* ------------------------------------------------------------
    return final result to caller
    ------------------------------------------------------------ */
end_of_job:
    return (relsp);
} /* --- end-of-function rastackrel() --- */


/* ==========================================================================
 * Function:    rastmathfunc ( expression, size, basesp,  base, arg2, arg3 )
 * Purpose: \log, \lim, etc handler, returns a subraster corresponding
 *      to math functions
 * --------------------------------------------------------------------------
 * Arguments:   expression (I/O) char **  to first char of null-terminated
 *              string immediately following \mathfunc to be
 *              rasterized, and returning ptr immediately
 *              following last character processed.
 *      size (I)    int containing 0-7 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding \mathfunc
 *              (unused, but passed for consistency)
 *      mathfunc (I)    int containing 1=arccos, 2=arcsin, etc.
 *      islimits (I)    int containing 1 if function may have
 *              limits underneath, e.g., \lim_{n\to\infty}
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to subraster corresponding to mathfunc,
 *              or NULL for any parsing error
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
subraster *rastmathfunc(mimetex_ctx *mctx, char **expression, int size, subraster *basesp,
                        int mathfunc, int islimits, int arg3)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /*func as {\rm func}, limits*/
    char func[MAXTOKNSZ+1], limits[MAXSUBXSZ+1];
    char funcarg[MAXTOKNSZ+1]; /* optional func arg */
    /*rasterize func,limits*/
    subraster *funcsp = NULL, *limsp = NULL;
    subraster *mathfuncsp = NULL; /* subraster for mathfunc/limits */
    /* font size for limits */
    int limsize = size - 1;
    /*vertical space between components*/
    int vspace = 1;
    /* --- table of function names by mathfunc number --- */
    /* number of names in table */
    static  int  numnames = 34;
    static  char *funcnames[] = {
        "error",
        /*  0 index is illegal/error bucket*/
        "arccos",  "arcsin",  "arctan", /*  1 -  3 */
        "arg",     "cos",     "cosh",   /*  4 -  6 */
        "cot",     "coth",    "csc",    /*  7 -  9 */
        "deg",     "det",     "dim",    /* 10 - 12 */
        "exp",     "gcd",     "hom",    /* 13 - 15 */
        "inf",     "ker",     "lg", /* 16 - 18 */
        "lim",     "liminf",  "limsup", /* 19 - 21 */
        "ln",      "log",     "max",    /* 22 - 24 */
        "min",     "Pr",      "sec",    /* 25 - 27 */
        "sin",     "sinh",    "sup",    /* 28 - 30 */
        "tan",     "tanh",      /* 31 - 32 */
        /* --- extra mimetex funcnames --- */
        "tr",
        /* 33 */
        "pmod"
        /* 34 */
    } ;
    /* ------------------------------------------------------------
    set up and rasterize function name in \rm
    ------------------------------------------------------------ */
    /* check index bounds */
    if (mathfunc < 0 || mathfunc > numnames) mathfunc = 0;
    switch (mathfunc) {          /* check for special processing */
    default:
    /* no special processing */
        /* init string with {\rm~ */
        strcpy(func, "{\\rm~");
        /* concat function name */
        strcat(func, funcnames[mathfunc]);
        /* and add terminating } */
        strcat(func, "}");
        break;
    case 34:              /* \pmod{x} --> (mod x) */
        /* --- parse for \pmod{arg} argument --- */
        *expression = texsubexpr(mctx, *expression, funcarg, 2047, "{", "}", 0, 0);
        /* init with {\left({\rm~mod} */
        strcpy(func, "{\\({\\rm~mod}");
        /* concat space */
        strcat(func, "\\hspace2");
        /* and \pmodargument */
        strcat(func, funcarg);
        /* and add terminating \right)} */
        strcat(func, "\\)}");
        break;
    } /* --- end-of-switch(mathfunc) --- */
    if ((funcsp = rasterize(mctx, func, size)) /* rasterize function name */
            /* and quit if failed */
            ==   NULL) goto end_of_job;
    /* just return funcsp if no limits */
    mathfuncsp = funcsp;
    /* treat any subscript normally */
    if (!islimits) goto end_of_job;
    /* ------------------------------------------------------------
    Obtain limits, if permitted and if provided, and rasterize them
    ------------------------------------------------------------ */
    /* --- parse for subscript limits, and bump expression past it(them) --- */
    *expression = texscripts(mctx, *expression, limits, limits, 1);
    /* no limits, nothing to do, quit */
    if (*limits == '\000') goto end_of_job;
    /* --- rasterize limits --- */
    if ((limsp = rasterize(mctx, limits, limsize)) /* rasterize limits */
            /* and quit if failed */
            ==   NULL) goto end_of_job;
    /* ------------------------------------------------------------
    construct func atop limits
    ------------------------------------------------------------ */
    /* --- construct func atop limits --- */
    if ((mathfuncsp = rastack(mctx, limsp, funcsp, 2, vspace, 1, 3)) /* func atop limits */
            /* quit if failed */
            ==   NULL) goto end_of_job;
    /* --- initialize subraster parameters --- */
    /* propagate font size forward */
    mathfuncsp->size = size;
    /* ------------------------------------------------------------
    return final result to caller
    ------------------------------------------------------------ */
end_of_job:
    return (mathfuncsp);
} /* --- end-of-function rastmathfunc() --- */


/* ==========================================================================
 * Function:    rastsqrt ( expression, size, basesp,  arg1, arg2, arg3 )
 * Purpose: \sqrt handler, returns a subraster corresponding to
 *      expression (immediately following \sqrt) at font size
 * --------------------------------------------------------------------------
 * Arguments:   expression (I/O) char **  to first char of null-terminated
 *              string immediately following \sqrt to be
 *              rasterized, and returning ptr immediately
 *              following last character processed.
 *      size (I)    int containing 0-7 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding \accent
 *              (unused, but passed for consistency)
 *      arg1 (I)    int unused
 *      arg2 (I)    int unused
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to subraster corresponding to expression,
 *              or NULL for any parsing error
 *              (expression ptr unchanged if error occurs)
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
subraster *rastsqrt(mimetex_ctx *mctx, char **expression, int size, subraster *basesp,
                    int arg1, int arg2, int arg3)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    char    subexpr[MAXSUBXSZ+1], /*parse subexpr to be sqrt-ed*/
    /* optional \sqrt[rootarg]{...} */
    rootarg[MAXSUBXSZ+1];
    /* rasterize subexpr */
    subraster *subsp = NULL;
    subraster *sqrtsp = NULL, /* subraster with the sqrt */
            *rootsp = NULL; /* optionally preceded by [rootarg]*/
    int sqrtheight = 0, sqrtwidth = 0, surdwidth = 0, /* height,width of sqrt */
        rootheight = 0, rootwidth = 0,  /* height,width of rootarg raster */
        subheight = 0, subwidth = 0, pixsz = 0; /* height,width,pixsz of subexpr */
    /*space between subexpr and overbar*/
    int overspace = 2;
    /* free work areas */
    /* ------------------------------------------------------------
    Obtain subexpression to be sqrt-ed, and rasterize it
    ------------------------------------------------------------ */
    /* --- first check for optional \sqrt[rootarg]{...} --- */
    if (*(*expression) == '[') {     /*check for []-enclosed optional arg*/
        *expression = texsubexpr(mctx, *expression, rootarg, 0, "[", "]", 0, 0);
        if (*rootarg != '\000')          /* got rootarg */
            if ((rootsp = rasterize(mctx, rootarg, size - 1)) /*rasterize it at smaller size*/
                    != NULL) {             /* rasterized successfully */
                /* get height of rootarg */
                rootheight = (rootsp->image)->height;
                rootwidth  = (rootsp->image)->width;
            } /* and its width */
    } /* --- end-of-if(**expression=='[') --- */
    /* --- parse for subexpr to be sqrt-ed, and bump expression past it --- */
    *expression = texsubexpr(mctx, *expression, subexpr, 0, "{", "}", 0, 0);
    if (*subexpr == '\000')          /* couldn't get subexpression */
        /* nothing to do, so quit */
        goto end_of_job;
    /* --- rasterize subexpression to be accented --- */
    if ((subsp = rasterize(mctx, subexpr, size))   /*rasterize subexpr at original size*/
            /* quit if failed */
            ==   NULL) goto end_of_job;
    /* ------------------------------------------------------------
    determine height and width of sqrt raster to be constructed
    ------------------------------------------------------------ */
    /* --- first get height and width of subexpr --- */
    /* height of subexpr */
    subheight = (subsp->image)->height;
    /* and its width */
    subwidth  = (subsp->image)->width;
    /* pixsz remains constant */
    pixsz     = (subsp->image)->pixsz;
    /* --- determine height and width of sqrt to contain subexpr --- */
    /* subexpr + blank line + overbar */
    sqrtheight = subheight + overspace;
    /* width of surd */
    surdwidth  = SQRTWIDTH(sqrtheight, (rootheight < 1 ? 2 : 1));
    /* total width */
    sqrtwidth  = subwidth + surdwidth + 1;
    /* ------------------------------------------------------------
    construct sqrt (with room to move in subexpr) and embed subexpr in it
    ------------------------------------------------------------ */
    /* --- construct sqrt --- */
    if ((sqrtsp = accent_subraster(mctx, SQRTACCENT,
                                   (rootheight < 1 ? sqrtwidth : (-sqrtwidth)), sqrtheight, pixsz))
            /* quit if failed to build sqrt */
            ==   NULL) goto end_of_job;
    /* --- embed subexpr in sqrt at lower-right corner--- */
    rastput(mctx, sqrtsp->image, subsp->image, overspace, sqrtwidth - subwidth, 1);
    /* adjust baseline */
    sqrtsp->baseline = subsp->baseline + overspace;
    /* --- "embed" rootarg at upper-left --- */
    if (rootsp != NULL) {            /*have optional \sqrt[rootarg]{...}*/
        /* --- allocate full raster to contain sqrtsp and rootsp --- */
        int fullwidth = sqrtwidth + rootwidth - min2(rootwidth, max2(0, surdwidth - 4)),
                        fullheight = sqrtheight + rootheight - min2(rootheight, 3 + size);
        subraster *fullsp = new_subraster(mctx, fullwidth, fullheight, pixsz);
        if (fullsp != NULL)            /* allocated successfully */
        {
        /* --- embed sqrtsp exactly at lower-right corner --- */
            rastput(mctx, fullsp->image, sqrtsp->image, /* exactly at lower-right corner*/
                    fullheight - sqrtheight, fullwidth - sqrtwidth, 1);
            /* --- embed rootsp near upper-left, nestled above leading surd --- */
            rastput(mctx, fullsp->image, rootsp->image,
                    0, max2(0, surdwidth - rootwidth - 2 - size), 0);
            /* --- replace sqrtsp with fullsp --- */
            /* free original sqrtsp */
            delete_subraster(mctx, sqrtsp);
            /* and repoint it to fullsp instead*/
            sqrtsp = fullsp;
            sqrtsp->baseline = fullheight - (subheight - subsp->baseline);
        }
    } /* --- end-of-if(rootsp!=NULL) --- */
    /* --- initialize subraster parameters --- */
    /* propagate font size forward */
    sqrtsp->size = size;
    /* ------------------------------------------------------------
    free unneeded component subrasters and return final result to caller
    ------------------------------------------------------------ */
end_of_job:
    /* free unneeded subexpr */
    if (subsp != NULL) delete_subraster(mctx, subsp);
    return (sqrtsp);
} /* --- end-of-function rastsqrt() --- */


/* ==========================================================================
 * Function:    rastaccent (expression,size,basesp,accent,isabove,isscript)
 * Purpose: \hat, \vec, \etc handler, returns a subraster corresponding
 *      to expression (immediately following \accent) at font size
 * --------------------------------------------------------------------------
 * Arguments:   expression (I/O) char **  to first char of null-terminated
 *              string immediately following \accent to be
 *              rasterized, and returning ptr immediately
 *              following last character processed.
 *      size (I)    int containing 0-7 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding \accent
 *              (unused, but passed for consistency)
 *      accent (I)  int containing HATACCENT or VECACCENT, etc,
 *              between numerator and denominator,
 *              or false not to draw it (for \over).
 *      isabove (I) int containing true if accent is above
 *              expression to be accented, or false
 *              if accent is below (e.g., underbrace)
 *      isscript (I)    int containing true if sub/superscripts
 *              allowed (for under/overbrace), or 0 if not.
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to subraster corresponding to expression,
 *              or NULL for any parsing error
 *              (expression ptr unchanged if error occurs)
 * --------------------------------------------------------------------------
 * Notes:     o Also handles \overbrace{}^{} and \underbrace{}_{} by way
 *      of isabove and isscript args.
 * ======================================================================= */
/* --- entry point --- */
subraster *rastaccent(mimetex_ctx *mctx, char **expression, int size, subraster *basesp,
                      int accent, int isabove, int isscript)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /*parse subexpr to be accented*/
    char subexpr[MAXSUBXSZ+1];
    char *script = NULL;  /* \under,overbrace allow scripts */
    char subscript[MAXTOKNSZ+1], supscript[MAXTOKNSZ+1]; /* parsed scripts */

    /*rasterize subexpr,script*/
    subraster *subsp = NULL, *scrsp = NULL;
    /* stack accent, subexpr, script */
    subraster *accsubsp = NULL;
    /*raster for the accent itself*/
    subraster *accent_subraster(), *accsp = NULL;
    int accheight = 0, accwidth = 0,    /* height, width of accent */
        subheight = 0, subwidth = 0, pixsz = 0; /* height,width,pixsz of subexpr */
    /*vertical space between accent,sub*/
    int vspace = 0;
    /* ------------------------------------------------------------
    Obtain subexpression to be accented, and rasterize it
    ------------------------------------------------------------ */
    /* --- parse for subexpr to be accented, and bump expression past it --- */
    *expression = texsubexpr(mctx, *expression, subexpr, 0, "{", "}", 0, 0);
    if (*subexpr == '\000')          /* couldn't get subexpression */
        /* nothing to do, so quit */
        goto end_of_job;
    /* --- rasterize subexpression to be accented --- */
    if ((subsp = rasterize(mctx, subexpr, size))   /*rasterize subexpr at original size*/
            /* quit if failed */
            ==   NULL) goto end_of_job;
    /* ------------------------------------------------------------
    determine desired accent width and height
    ------------------------------------------------------------ */
    /* --- first get height and width of subexpr --- */
    /* height of subexpr */
    subheight = (subsp->image)->height;
    /* and its width is overall width */
    subwidth  = (subsp->image)->width;
    /* original pixsz remains constant */
    pixsz     = (subsp->image)->pixsz;
    /* --- determine desired width, height of accent --- */
    /* same width as subexpr */
    accwidth = subwidth;
    /* default for bars */
    accheight = 4;
    switch (accent) {
    default:
        /* default okay */
        break;
    case DOTACCENT:
    case DDOTACCENT:
        /* default for dots */
        accheight = (size < 4 ? 3 : 4);
        break;
    case VECACCENT:
        /* set 1-pixel vertical space */
        vspace = 1;
    case HATACCENT:
        /* default */
        accheight = 7;
        /* unless small width */
        if (subwidth < 10) accheight = 5;
        /* or large */
        else if (subwidth > 25) accheight = 9;
        break;
    } /* --- end-of-switch(accent) --- */
    /*never higher than accented subexpr*/
    accheight = min2(accheight, subheight);
    /* ------------------------------------------------------------
    construct accent, and construct subraster with accent over (or under) subexpr
    ------------------------------------------------------------ */
    /* --- first construct accent --- */
    if ((accsp = accent_subraster(mctx, accent, accwidth, accheight, pixsz)) /* accent */
            /* quit if failed to build accent */
            ==   NULL) goto end_of_job;
    /* --- now stack accent above (or below) subexpr, and free both args --- */
    accsubsp = (isabove ? rastack(mctx, subsp, accsp, 1, vspace, 1, 3)/*accent above subexpr*/
                /*accent below subexpr*/
                : rastack(mctx, accsp, subsp, 2, vspace, 1, 3));
    if (accsubsp == NULL) {          /* failed to stack accent */
        /* free unneeded subsp */
        delete_subraster(mctx, subsp);
        /* and unneeded accsp */
        delete_subraster(mctx, accsp);
        goto end_of_job;
    }          /* and quit */
    /* ------------------------------------------------------------
    look for super/subscript (annotation for over/underbrace)
    ------------------------------------------------------------ */
    /* --- first check whether accent permits accompanying annotations --- */
    /* no annotations for this accent */
    if (!isscript) goto end_of_job;
    /* --- now get scripts if there actually are any --- */
    *expression = texscripts(mctx, *expression, subscript, supscript, (isabove ? 2 : 1));
    /*select above^ or below_ script*/
    script = (isabove ? supscript : subscript);
    /* no accompanying script */
    if (*script == '\000') goto end_of_job;
    /* --- rasterize script annotation at size-2 --- */
    if ((scrsp = rasterize(mctx, script, size - 2)) /* rasterize script at size-2 */
            /* quit if failed */
            ==   NULL) goto end_of_job;
    /* --- stack annotation above (or below) accent, and free both args --- */
    accsubsp = (isabove ? rastack(mctx, accsubsp, scrsp, 1, 0, 1, 3) /* accent above base */
                /* accent below base */
                : rastack(mctx, scrsp, accsubsp, 2, 0, 1, 3));
    /* ------------------------------------------------------------
    return final result to caller
    ------------------------------------------------------------ */
end_of_job:
    if (accsubsp != NULL)          /* initialize subraster parameters */
        /* propagate font size forward */
        accsubsp->size = size;
    return (accsubsp);
} /* --- end-of-function rastaccent() --- */


/* ==========================================================================
 * Function:    rastfont (expression,size,basesp,ifontnum,arg2,arg3)
 * Purpose: \cal{}, \scr{}, \etc handler, returns subraster corresponding
 *      to char(s) within {}'s rendered at size
 * --------------------------------------------------------------------------
 * Arguments:   expression (I/O) char **  to first char of null-terminated
 *              string immediately following \font to be
 *              rasterized, and returning ptr immediately
 *              following last character processed.
 *      size (I)    int containing 0-7 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding \accent
 *              (unused, but passed for consistency)
 *      ifontnum (I)    int containing 1 for \cal{}, 2 for \scr{}
 *      arg2 (I)    int unused
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to subraster corresponding to chars
 *              between {}'s, or NULL for any parsing error
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
subraster *rastfont(mimetex_ctx *mctx, char **expression, int size, subraster *basesp,
                    int ifontnum, int arg2, int arg3)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    char fontchars[MAXSUBXSZ+1], /* chars to render in font */
    /* turn \cal{AB} into \calA\calB */
    subexpr[MAXSUBXSZ+1];
    /* run thru fontchars one at a time*/
    char    *pfchars = fontchars, fchar = '\0';
    /* fontinfo[ifontnum].name */
    char    *name = NULL;
    int family = 0, /* fontinfo[ifontnum].family */
        istext = 0, /* fontinfo[ifontnum].istext */
        klass = 0; /* fontinfo[ifontnum].class */
    subraster *fontsp = NULL; /* rasterize chars in font */
    /* turn off smash in text mode */
    int oldsmashmargin = mctx->smashmargin;
    /* ------------------------------------------------------------
    first get font name and class to determine type of conversion desired
    ------------------------------------------------------------ */
    /*math if out-of-bounds*/
    if (ifontnum <= 0 || ifontnum > nfontinfo) ifontnum = 0;
    /* font name */
    name   = fontinfo[ifontnum].name;
    /* font family */
    family = fontinfo[ifontnum].family;
    /*true in text mode (respect space)*/
    istext = fontinfo[ifontnum].istext;
    /* font class */
    klass  = fontinfo[ifontnum].klass;
    if (istext) {                /* text (respect blanks) */
        /* needed for \text{if $n-m$ even} */
        mctx->mathsmashmargin = mctx->smashmargin;
        mctx->smashmargin = 0;
    }           /* don't smash internal blanks */
    /* ------------------------------------------------------------
    now convert \font{abc} --> {\font~abc}, or convert ABC to \calA\calB\calC
    ------------------------------------------------------------ */
    if (1 || klass < 0) {        /* not character-by-character */
        /* ---
        if \font not immediately followed by { then it has no arg, so just set flag
        ------------------------------------------------------------ */
        if (*(*expression) != '{') {        /* no \font arg, so just set flag */
            if (mctx->msgfp != NULL && mctx->msglevel >= 99)
                fprintf(mctx->msgfp, "rastfont> \\%s rastflags() for font#%d\n", name, ifontnum);
            fontsp = rastflags(mctx, expression, size, basesp, ISFONTFAM, ifontnum, arg3);
            goto end_of_job;
        } /* --- end-of-if(*(*expression)!='{') --- */
        /* ---
        convert \font{abc} --> {\font~abc}
        ------------------------------------------------------------ */
        /* --- parse for {fontchars} arg, and bump expression past it --- */
        *expression = texsubexpr(mctx, *expression, fontchars, 0, "{", "}", 0, 0);
        if (mctx->msgfp != NULL && mctx->msglevel >= 99)
            fprintf(mctx->msgfp, "rastfont> \\%s fontchars=\"%s\"\n", name, fontchars);
        /* --- convert all fontchars at the same time --- */
        /* start off with opening { */
        strcpy(subexpr, "{");
        /* followed by font name */
        strcat(subexpr, name);
        /* followed by whitespace */
        strcat(subexpr, "~");
        /* followed by all the chars */
        strcat(subexpr, fontchars);
        /* terminate with closing } */
        strcat(subexpr, "}");
    } /* --- end-of-if(klass<0) --- */
    else {                  /* character-by-character */
        /* ---
        convert ABC to \calA\calB\calC
        ------------------------------------------------------------ */
        /* true if prev char converted */
        int    isprevchar = 0;
        /* --- parse for {fontchars} arg, and bump expression past it --- */
        *expression = texsubexpr(mctx, *expression, fontchars, 0, "{", "}", 0, 0);
        if (mctx->msgfp != NULL && mctx->msglevel >= 99)
            fprintf(mctx->msgfp, "rastfont> \\%s fontchars=\"%s\"\n", name, fontchars);
        /* --- convert fontchars one at a time --- */
        /* start off with opening {\rm */
        strcpy(subexpr, "{\\rm~");
        /* nope, just start off with { */
        strcpy(subexpr, "{");
        for (pfchars = fontchars; (fchar = *pfchars) != '\000'; pfchars++) {
            if (isthischar(fchar, WHITEMATH)) { /* some whitespace */
                if (0 || istext)       /* and we're in a text mode font */
                    strcat(subexpr, "\\;");
            }    /* so respect whitespace */
            else {                /* char to be displayed in font */
                /* #chars in subexpr before fchar */
                int exprlen = 0;
                /* set true if fchar in font class */
                int isinclass = 0;
                /* --- class: 1=upper, 2=alpha, 3=alnum, 4=lower, 5=digit, 9=all --- */
                switch (klass) {           /* check if fchar is in font class */
                default:
                    /* no chars in unrecognized class */
                    break;
                case 1:
                    if (isupper((int)fchar)) isinclass = 1;
                    break;
                case 2:
                    if (isalpha((int)fchar)) isinclass = 1;
                    break;
                case 3:
                    if (isalnum((int)fchar)) isinclass = 1;
                    break;
                case 4:
                    if (islower((int)fchar)) isinclass = 1;
                    break;
                case 5:
                    if (isdigit((int)fchar)) isinclass = 1;
                    break;
                case 9:
                    isinclass = 1;
                    break;
                }
                if (isinclass) {           /* convert current char to \font */
                    /* by prefixing it with font name */
                    strcat(subexpr, name);
                    isprevchar = 1;
                }     /* and set flag to signal separator*/
                else {            /* current char not in \font */
                    if (isprevchar)        /* extra separator only after \font*/
                        if (isalpha(fchar))   /* separator only before alpha */
                            /* need separator after \font */
                            strcat(subexpr, "~");
                    isprevchar = 0;
                }     /* reset flag for next char */
                /* #chars so far */
                exprlen = strlen(subexpr);
                /*fchar immediately after \fontname*/
                subexpr[exprlen] = fchar;
                subexpr[exprlen+1] = '\000';
            }    /* replace terminating '\0' */
        } /* --- end-of-for(pfchars) --- */
        /* add closing } */
        strcat(subexpr, "}");
    } /* --- end-of-if/else(class<0) --- */
    /* ------------------------------------------------------------
    rasterize subexpression containing chars to be rendered at font
    ------------------------------------------------------------ */
    if (mctx->msgfp != NULL && mctx->msglevel >= 99)
        fprintf(mctx->msgfp, "rastfont> subexpr=\"%s\"\n", subexpr);
    if ((fontsp = rasterize(mctx, subexpr, size))  /* rasterize chars in font */
            /* and quit if failed */
            ==   NULL) goto end_of_job;
    /* ------------------------------------------------------------
    back to caller with chars rendered in font
    ------------------------------------------------------------ */
end_of_job:
    /* restore smash */
    mctx->smashmargin = oldsmashmargin;
    /* this one probably not necessary */
    /* mctx->mathsmashmargin = SMASHMARGIN; */
    if (istext && fontsp != NULL)      /* raster contains text mode font */
        /* signal nosmash */
        fontsp->type = mctx->blanksignal;
    /* chars rendered in font */
    return (fontsp);
} /* --- end-of-function rastfont() --- */


/* ==========================================================================
 * Function:    rastbegin ( expression, size, basesp, arg1, arg2, arg3 )
 * Purpose: \begin{}...\end{}  handler, returns a subraster corresponding
 *      to array expression within environment, i.e., rewrites
 *      \begin{}...\end{} as mimeTeX equivalent, and rasterizes that.
 * --------------------------------------------------------------------------
 * Arguments:   expression (I/O) char **  to first char of null-terminated
 *              string immediately following \begin to be
 *              rasterized, and returning ptr immediately
 *              following last character processed.
 *      size (I)    int containing 0-7 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding \begin
 *              (unused, but passed for consistency)
 *      arg1 (I)    int unused
 *      arg2 (I)    int unused
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to subraster corresponding to array
 *              expression, or NULL for any parsing error
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
subraster *rastbegin(mimetex_ctx *mctx, char **expression, int size, subraster *basesp,
                     int arg1, int arg2, int arg3)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    char subexpr[MAXSUBXSZ+1], /* \begin{} environment params*/
    /* ptrs */
    *exprptr = NULL, *begptr = NULL, *endptr = NULL, *braceptr = NULL;
    /*tokens we're looking for*/
    char    *begtoken = "\\begin{", *endtoken = "\\end{";
    /* mdelims[ienviron] */
    char    *delims = (char *)NULL;
    /* rasterize environment */
    subraster *sp = NULL;
    /* environs[] index */
    int ienviron = 0;
    /* #\begins nested beneath this one*/
    int nbegins = 0;
    /* #chars in environ, subexpr */
    int envlen = 0, sublen = 0;
    /* \begin...\end nesting level */
    static  int blevel = 0;
    static  char *mdelims[] = {
        NULL, NULL, NULL, NULL,
        "()", "[]", "{}", "||", "==",   /* for pbBvVmatrix */
        NULL, NULL, NULL, NULL, "{.", NULL, NULL, NULL, NULL, NULL, NULL,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
    };
    static  char *environs[] = {        /* types of environments we process*/
        "eqnarray",
        /* 0 eqnarray environment */
        "array",
        /* 1 array environment */
        "matrix",
        /* 2 array environment */
        "tabular",
        /* 3 array environment */
        "pmatrix",
        /* 4 ( ) */
        "bmatrix",
        /* 5 [ ] */
        "Bmatrix",
        /* 6 { } */
        "vmatrix",
        /* 7 | | */
        "Vmatrix",
        /* 8 || || */
        "gather",
        /* 9 gather environment */
        "align",
        /* 10 align environment */
        "verbatim",
        /* 11 verbatim environment */
        "picture",
        /* 12 picture environment */
        "cases",
        /* 13 cases environment */
        "equation",
        /* 14 for \begin{equation} */
        NULL
    /* trailer */
    };
    /* ------------------------------------------------------------
    determine type of environment we're beginning
    ------------------------------------------------------------ */
    /* --- first bump nesting level --- */
    /* count \begin...\begin...'s */
    blevel++;
    /* --- \begin must be followed by {type_of_environment} --- */
    exprptr = texsubexpr(mctx, *expression, subexpr, 0, "{", "}", 0, 0);
    /* no environment given */
    if (*subexpr == '\000') goto end_of_job;
    while ((delims = strchr(subexpr, '*')) != NULL) /* have environment* */
        /* treat it as environment */
        strcpy(delims, delims + 1);
    /* --- look up environment in our table --- */
    for (ienviron = 0; ; ienviron++)     /* search table till NULL */
        if (environs[ienviron] == NULL)    /* found NULL before match */
            /* so quit */
            goto end_of_job;
        else
        /* see if we have an exact match */
            if (memcmp(environs[ienviron], subexpr, strlen(subexpr)) == 0) /*match*/
                /* leave loop with ienviron index */
                break;
    /* --- accumulate any additional params for this environment --- */
    /* reset subexpr to empty string */
    *subexpr = '\000';
    /* mdelims[] string for ienviron */
    delims = mdelims[ienviron];
    if (delims != NULL) {            /* add appropriate opening delim */
        /* start with \ for (,[,{,|,= */
        strcpy(subexpr, "\\");
        /* then add opening delim */
        strcat(subexpr, delims);
        subexpr[2] = '\000';
    }      /* remove extraneous closing delim */
    switch (ienviron) {
    default:
        /* environ not implemented yet */
        goto end_of_job;
    case 0:               /* \begin{eqnarray} */
        /* set default rcl for eqnarray */
        strcpy(subexpr, "\\array{rcl$");
        break;
    case 1:
    case 2:
    case 3:     /* \begin{array} followed by {lcr} */
        /*start with mimeTeX \array{ command*/
        strcpy(subexpr, "\\array{");
        /* bump to next non-white char */
        skipwhite(exprptr);
        if (*exprptr == '{') {       /* assume we have {lcr} argument */
            /*add on lcr*/
            exprptr = texsubexpr(mctx, exprptr, subexpr + 7, 0, "{", "}", 0, 0);
            /* quit if no lcr */
            if (*(subexpr + 7) == '\000') goto end_of_job;
            strcat(subexpr, "$");
        }      /* add terminating $ to lcr */
        break;
    case 4:
    case 5:
    case 6:     /* \begin{pmatrix} or b,B,v,Vmatrix */
    case 7:
    case 8:
        /*start with mimeTeX \array{ command*/
        strcat(subexpr, "\\array{");
        break;
    case 9:               /* gather */
        /* center equations */
        strcat(subexpr, "\\array{c$");
        break;
    case 10:              /* align */
        /* a&=b & c&=d & etc */
        strcat(subexpr, "\\array{rclrclrclrclrclrcl$");
        break;
    case 11:              /* verbatim */
        /* {\rm ...} */
        strcat(subexpr, "{\\rm ");
        /*strcat(subexpr,"\\\\{\\rm ");*/   /* \\{\rm } doesn't work in context */
        break;
    case 12:              /* picture */
        /* picture environment */
        strcat(subexpr, "\\picture");
        /* bump to next non-white char */
        skipwhite(exprptr);
        if (*exprptr == '(') {       /*assume we have (width,height) arg*/
            /*add on arg*/
            exprptr = texsubexpr(mctx, exprptr, subexpr + 8, 0, "(", ")", 0, 1);
            if (*(subexpr + 8) == '\000') goto end_of_job;
        } /* quit if no arg */
        /* opening {  after (width,height) */
        strcat(subexpr, "{");
        break;
    case 13:              /* cases */
        /* a&b \\ c&d etc */
        strcat(subexpr, "\\array{ll$");
        break;
    case 14:              /* \begin{equation} */
        /* just enclose expression in {}'s */
        strcat(subexpr, "{");
        break;
    } /* --- end-of-switch(ienviron) --- */
    /* ------------------------------------------------------------
    locate matching \end{...}
    ------------------------------------------------------------ */
    /* --- first \end following \begin --- */
    if ((endptr = strstr(exprptr, endtoken)) /* find 1st \end following \begin */
            /* and quit if no \end found */
            ==   NULL) goto end_of_job;
    /* --- find matching endptr by pushing past any nested \begin's --- */
    /* start after first \begin{...} */
    begptr = exprptr;
    while (1) {              /*break when we find matching \end*/
        /* --- first, set ptr to closing } terminating current \end{...} --- */
        if ((braceptr = strchr(endptr + 1, '}'))   /* find 1st } following \end{ */
                /* and quit if no } found */
                ==   NULL) goto end_of_job;
        /* -- locate next nested \begin --- */
        if ((begptr = strstr(begptr, begtoken))  /* find next \begin{...} */
                /*no more, so we have matching \end*/
                ==   NULL) break;
        /* push ptr past token */
        begptr += strlen(begtoken);
        /* past endptr, so not nested */
        if (begptr >= endptr) break;
        /* --- have nested \begin, so push forward to next \end --- */
        /* count another nested \begin */
        nbegins++;
        if ((endptr = strstr(endptr + strlen(endtoken), endtoken)) /* find next \end */
                /* and quit if none found */
                ==   NULL) goto end_of_job;
    } /* --- end-of-while(1) --- */
    /* --- push expression past closing } of \end{} --- */
    /* resume processing after } */
    *expression = braceptr + 1;
    /* ------------------------------------------------------------
    add on everything (i.e., the ...'s) between \begin{}[{}] ... \end{}
    ------------------------------------------------------------ */
    /* --- add on everything, completing subexpr for \begin{}...\end{} --- */
    /* #chars in "preamble" */
    sublen = strlen(subexpr);
    /* #chars between \begin{}{}...\end */
    envlen = (int)(endptr - exprptr);
    /*concatanate environ after subexpr*/
    memcpy(subexpr + sublen, exprptr, envlen);
    /* and null-terminate */
    subexpr[sublen+envlen] = '\000';
    if (2 > 1)               /* always... */
        /* ...followed by terminating } */
        strcat(subexpr, "}");
    /* --- add terminating \right), etc, if necessary --- */
    if (delims != (char *)NULL) {        /* need closing delim */
        /* start with \ for ),],},|,= */
        strcat(subexpr, "\\");
        strcat(subexpr, delims + 1);
    }      /* add appropriate closing delim */
    /* ------------------------------------------------------------
    change nested \begin...\end to {\begin...\end} so \array{} can handle them
    ------------------------------------------------------------ */
    if (nbegins > 0)             /* have nested begins */
        if (blevel < 2) {           /* only need to do this once */
            /* start at beginning of subexpr */
            begptr = subexpr;
            while ((begptr = strstr(begptr, begtoken)) != NULL) { /* have \begin{...} */
                /* \begin --> {\begin */
                strchange(0, begptr, "{");
                begptr += strlen(begtoken);
            } /* continue past {\begin */
            /* start at beginning of subexpr */
            endptr = subexpr;
            while ((endptr = strstr(endptr, endtoken)) != NULL) /* have \end{...} */
                if ((braceptr = strchr(endptr + 1, '}')) /* find 1st } following \end{ */
                        /* and quit if no } found */
                        ==   NULL) goto end_of_job;
                else {              /* found terminating } */
                    /* \end{...} --> \end{...}} */
                    strchange(0, braceptr, "}");
                    endptr = braceptr + 1;
                }       /* continue past \end{...} */
        } /* --- end-of-if(nbegins>0) --- */
    /* ------------------------------------------------------------
    post process as necessary
    ------------------------------------------------------------ */
    switch (ienviron) {
    default:
        /* no post-processing required */
        break;
    case 10:              /* align */
        /* tag all &='s */
        strreplace(subexpr, "&=", "#*@*#=", 0);
        /* tag all &<'s */
        strreplace(subexpr, "&<", "#*@*#<", 0);
        /* tag all &\lt's */
        strreplace(subexpr, "&\\lt", "#*@*#<", 0);
        /* tag all &\leq's */
        strreplace(subexpr, "&\\leq", "#*@*#\\leq", 0);
        /* tag all &>'s */
        strreplace(subexpr, "&>", "#*@*#>", 0);
        /* tag all &\gt's */
        strreplace(subexpr, "&\\gt", "#*@*#>", 0);
        /* tag all &\geq's */
        strreplace(subexpr, "&\\geq", "#*@*#\\geq", 0);
        if (nbegins < 1)             /* don't modify nested arrays */
            /* add space */
            strreplace(subexpr, "&", "\\hspace{10}&\\hspace{10}", 0);
        /*restore and xlate tagged &='s*/
        strreplace(subexpr, "#*@*#=", "& = &", 0);
        /*restore, xlate tagged &<'s*/
        strreplace(subexpr, "#*@*#<", "& \\lt &", 0);
        /*xlate tagged &\leq's*/
        strreplace(subexpr, "#*@*#\\leq", "& \\leq &", 0);
        /*restore, xlate tagged &>'s*/
        strreplace(subexpr, "#*@*#>", "& \\gt &", 0);
        /*xlate tagged &\geq's*/
        strreplace(subexpr, "#*@*#\\geq", "& \\geq &", 0);
        break;
    case 11:              /* verbatim */
        /* xlate \n newline to latex \\ */
        strreplace(subexpr, "\n", "\\\\", 0);
        /*strcat(subexpr,"\\\\");*/     /* add final latex \\ newline */
        break;
    case 12:              /* picture */
        /*remove \put's (not really needed)*/
        strreplace(subexpr, "\\put ", " ", 0);
        /*remove \put's (not really needed)*/
        strreplace(subexpr, "\\put(", "(", 0);
        /* actually an ellipse */
        strreplace(subexpr, "\\oval", "\\circle", 0);
        break;
    } /* --- end-of-switch(ienviron) --- */
    /* ------------------------------------------------------------
    return rasterized mimeTeX equivalent of \begin{}...\end{} environment
    ------------------------------------------------------------ */
    /* --- debugging output --- */
    if (mctx->msgfp != NULL && mctx->msglevel >= 99)
        fprintf(mctx->msgfp, "rastbegin> subexpr=%s\n", subexpr);
    /* --- rasterize mimeTeX equivalent of \begin{}...\end{} environment --- */
    /* rasterize subexpr */
    sp = rasterize(mctx, subexpr, size);
end_of_job:
    /* decrement \begin nesting level */
    blevel--;
    /* back to caller with sp or NULL */
    return (sp);
} /* --- end-of-function rastbegin() --- */


/* ==========================================================================
 * Function:    rastarray ( expression, size, basesp, arg1, arg2, arg3 )
 * Purpose: \array handler, returns a subraster corresponding to array
 *      expression (immediately following \array) at font size
 * --------------------------------------------------------------------------
 * Arguments:   expression (I/O) char **  to first char of null-terminated
 *              string immediately following \array to be
 *              rasterized, and returning ptr immediately
 *              following last character processed.
 *      size (I)    int containing 0-7 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding \array
 *              (unused, but passed for consistency)
 *      arg1 (I)    int unused
 *      arg2 (I)    int unused
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to subraster corresponding to array
 *              expression, or NULL for any parsing error
 * --------------------------------------------------------------------------
 * Notes:     o Summary of syntax...
 *          \array{3,lcrBC$a&b&c\\d&e&f\\etc}
 *        o The 3,lcrBC$ part is an optional "preamble".  The lcr means
 *      what you think, i.e., "horizontal" left,center,right
 *      justification down corresponding column.  The new BC means
 *      "vertical" baseline,center justification across corresponding
 *      row.  The leading 3 specifies the font size 0-4 to be used.
 *      You may also specify +1,-1,+2,-2, etc, which is used as an
 *      increment to the current font size, e.g., -1,lcr$ uses
 *      one font size smaller than current.  Without a leading
 *      + or -, the font size is "absolute".
 *        o The preamble can also be just lcrBC$ without a leading
 *      size-part, or just 3$ without a trailing lcrBC-part.
 *      The default size is whatever is current, and the
 *      default justification is c(entered) and B(aseline).
 * ======================================================================= */
/* --- entry point --- */
subraster *rastarray(mimetex_ctx *mctx, char **expression, int size, subraster *basesp,
                     int arg1, int arg2, int arg3)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    char subexpr[MAXSUBXSZ+1], *exprptr, /*parse array subexpr*/
         subtok[MAXTOKNSZ+1], *subptr = subtok, /* &,\\ inside { } not a delim*/
         token[MAXTOKNSZ+1],  *tokptr = token, /* subexpr token to rasterize */
         *preptr = token; /*process optional size,lcr preamble*/
    /* need escaped rowdelim */
    char *coldelim = "&", *rowdelim = "\\";
    /* max #rows, cols */
    int maxarraysz = 63;
    int justify[65] = {
        /* -1,0,+1 = l,c,r */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    int hline[65] = {
        /* hline above row? */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    int vline[65] = {
        /*vline left of col?*/
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    int colwidth[65] = {
        /*widest tokn in col*/
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    int rowheight[65] = {
        /* "highest" in row */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    int fixcolsize[65] = {
        /*1=fixed col width*/
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    int fixrowsize[65] = {
        /*1=fixed row height*/
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    int rowbaseln[65] = {
        /* baseline for row */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    int vrowspace[65] = {
        /*extra //[len]space*/
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    int rowcenter[65] = {
        /*true = vcenter row*/
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    /* --- propagate global values across arrays --- */
    static int gjustify[65] = {
        /* -1,0,+1 = l,c,r */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    static int gcolwidth[65] = {
        /*widest tokn in col*/
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    static int growheight[65] = {
        /* "highest" in row */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    static int gfixcolsize[65] = {
        /*1=fixed col width*/
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    static int gfixrowsize[65] = {
        /*1=fixed row height*/
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },
    growcenter[65] = {
        /*true = vcenter row*/
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    int rowglobal = 0, colglobal = 0,   /* true to set global values */
        rowpropagate = 0, colpropagate = 0; /* true if propagating values */
    int irow, nrows = 0, icol, ncols[65], /*#rows in array, #cols in each row*/
        maxcols = 0; /* max# cols in any single row */
    int itoken, ntokens = 0,    /* index, total #tokens in array */
        subtoklen = 0, /* strlen of {...} subtoken */
        istokwhite = 1,/* true if token all whitespace */
        nnonwhite = 0; /* #non-white tokens */
    int isescape = 0, wasescape = 0,     /* current,prev chars escape? */
        ischarescaped = 0,      /* is current char escaped? */
        nescapes = 0; /* #consecutive escapes */
    subraster *toksp[1025];   /* rasterize tokens */
    subraster *arraysp = NULL; /* subraster for entire array */
    /* raster for entire array */
    raster  *arrayrp = NULL;
    /* free toksp[] workspace at eoj */
    int rowspace = 2, colspace = 4, /* blank space between rows, cols */
                                 /*space to accommodate hline,vline*/
                                 hspace = 1, vspace = 1;
    int width = 0, height = 0,  /* width,height of array */
                            /*upper-left corner for cell in it*/
                            leftcol = 0, toprow = 0;
    /* token signals hline */
    char    *hlchar = "\\hline", *hdchar = "\\hdash";
    /* extract \hline from token */
    char    hltoken[1025];
    /*flag, token must be \hl or \hd*/
    int ishonly = 0, hltoklen, minhltoklen = 3;
    /* true for new row */
    int isnewrow = 1;
    /*default #bits per pixel, 1=bitmap*/
    int pixsz = 1;
    /* ------------------------------------------------------------
    Macros to determine extra raster space required for vline/hline
    ------------------------------------------------------------ */
#define vlinespace(icol) \
    ( vline[icol] == 0?  0 :    /* no vline so no space needed */   \
      ( icol<1 || icol>=maxcols? vspace+(colspace+1)/2 : vspace ) )
#define hlinespace(irow) \
    ( hline[irow] == 0?  0 :    /* no hline so no space needed */   \
      ( irow<1 || irow>=nrows? hspace+(rowspace+1)/2 : hspace ) )
    /* ------------------------------------------------------------
    Obtain array subexpression
    ------------------------------------------------------------ */
    /* --- parse for array subexpression, and bump expression past it --- */
    /* set two leading blanks */
    subexpr[1] = *subexpr = ' ';
    *expression = texsubexpr(mctx, *expression, subexpr + 2, 0, "{", "}", 0, 0);
    if (mctx->msglevel >= 29 && mctx->msgfp != NULL) /* debugging, display array */
        fprintf(mctx->msgfp, "rastarray> %.256s\n", subexpr + 2);
    if (*(subexpr + 2) == '\000')    /* couldn't get subexpression */
        /* nothing to do, so quit */
        goto end_of_job;
    /* XXX: IS IT OK TO INITIALIZE THESE ARRAYS EVERY TIME? */
    for (icol = 0; icol <= maxarraysz; icol++) /* for each array[] index */
        gjustify[icol]    = gcolwidth[icol]   = growheight[icol] =
                                                    gfixcolsize[icol] = gfixrowsize[icol] = growcenter[icol] = 0;
    /* ------------------------------------------------------------
    process optional size,lcr preamble if present
    ------------------------------------------------------------ */
    /* --- reset size, get lcr's, and push exprptr past preamble --- */
    /* reset size and get lcr's */
    exprptr = preamble(mctx, subexpr + 2, &size, preptr);
    /* --- init with global values --- */
    for (icol = 0; icol <= maxarraysz; icol++) { /* propagate global values... */
        /* -1,0,+1 = l,c,r */
        justify[icol] = gjustify[icol];
        /* column width */
        colwidth[icol] = gcolwidth[icol];
        /* row height */
        rowheight[icol] = growheight[icol];
        /* 1=fixed col width */
        fixcolsize[icol] = gfixcolsize[icol];
        /* 1=fixed row height */
        fixrowsize[icol] = gfixrowsize[icol];
        rowcenter[icol] = growcenter[icol];
    } /* true = vcenter row */
    /* --- process lcr's, etc in preamble --- */
    /* debugging flag */
    itoken = 0;
    if (mctx->msglevel >= 29 && mctx->msgfp != NULL) /* debugging, display preamble */
        if (*preptr != '\000')          /* if we have one */
            fprintf(mctx->msgfp, "rastarray> preamble= \"%.256s\"\nrastarray> preamble: ",
                    preptr);
    /* init lcr counts */
    irow = icol = 0;
    while (*preptr != '\000') {       /* check preamble text for lcr */
        /* current preamble character */
        char  prepchar = *preptr;
        /*1,2,or 0*/
        int   prepcase = (islower(prepchar) ? 1 : (isupper(prepchar) ? 2 : 0));
        if (irow < maxarraysz && icol < maxarraysz)
            switch (/*tolower*/(prepchar)) {
            default:
                /* just flush unrecognized chars */
                break;
            case 'l':
                /*left-justify this column*/
                justify[icol] = (-1);
                if (colglobal) gjustify[irow] = justify[irow];
                break;
            case 'c':
                /* center this column */
                justify[icol] = (0);
                if (colglobal) gjustify[irow] = justify[irow];
                break;
            case 'r':
                /* right-justify this col */
                justify[icol] = (+1);
                if (colglobal) gjustify[irow] = justify[irow];
                break;
            case '|':
                vline[icol] += 1;
                /* solid vline left of col */
                break;
            case '.':
                vline[icol] = (-1);
                /*dashed vline left of col */
                break;
            case 'b':
                prepchar = 'B';
                /* alias for B */
                prepcase = 2;
            case 'B':
                /* baseline-justify row */
                break;
            case 'v':
                prepchar = 'C';
                /* alias for C */
                prepcase = 2;
            case 'C':
                /* vertically center row */
                rowcenter[irow] = 1;
                if (rowglobal) growcenter[irow] = rowcenter[irow];
                break;
            case 'g':
                colglobal = 1;
                prepcase = 0;
                /* set global col values */
                break;
            case 'G':
                rowglobal = 1;
                prepcase = 0;
                /* set global row values */
                break;
            case '#':
                colglobal = rowglobal = 1;
                break;
            } /* set global col,row vals */
        if (mctx->msglevel >= 29 && mctx->msgfp != NULL) /* debugging */
            fprintf(mctx->msgfp, " %c[%d]", prepchar,
                    (prepcase == 1 ? icol + 1 : (prepcase == 2 ? irow + 1 : 0)));
        /* check next char for lcr */
        preptr++;
        /* #lcr's processed (debugging only)*/
        itoken++;
        /* --- check for number or +number specifying colwidth or rowheight --- */
        if (prepcase != 0) {           /* only check upper,lowercase */
            int  ispropagate = (*preptr == '+' ? 1 : 0); /* leading + propagates width/ht */
            if (ispropagate) {            /* set row or col propagation */
                /* propagating col values */
                if (prepcase == 1) colpropagate = 1;
                else if (prepcase == 2) rowpropagate = 1;
            } /*propagating row values*/
            if (!colpropagate && prepcase == 1) {
                /* reset colwidth */
                colwidth[icol] = 0;
                fixcolsize[icol] = 0;
            }     /* reset width flag */
            if (!rowpropagate && prepcase == 2) {
                /* reset row height */
                rowheight[irow] = 0;
                fixrowsize[irow] = 0;
            }     /* reset height flag */
            /* bump past leading + */
            if (ispropagate) preptr++;
            if (isdigit(*preptr)) {       /* digit follows character */
                /* preptr set to 1st char after num*/
                char *endptr = NULL;
                /* interpret number */
                int size = (int)(strtol(preptr, &endptr, 10));
                char *whchars = "?wh";   /* debugging width/height labels */
                /* skip over all digits */
                preptr = endptr;
                if (size == 0 || (size >= 3 && size <= 500)) { /* sanity check */
                    /* icol,irow...maxarraysz index */
                    int index;
                    if (prepcase == 1)       /* lowercase signifies colwidth */
                        for (index = icol; index <= maxarraysz; index++) { /*propagate col size*/
                            /* set colwidth to fixed size */
                            colwidth[index] = size;
                            /* set fixed width flag */
                            fixcolsize[index] = (size > 0 ? 1 : 0);
                            /* and propagate justification */
                            justify[index] = justify[icol];
                            if (colglobal) {       /* set global values */
                                /* set global col width */
                                gcolwidth[index] = colwidth[index];
                                /*set global width flag*/
                                gfixcolsize[index] = fixcolsize[index];
                                gjustify[index] = justify[icol];
                            } /* set global col justify */
                            if (!ispropagate) break;
                        }   /* don't propagate */
                    else
                    /* uppercase signifies rowheight */
                        for (index = irow; index <= maxarraysz; index++) { /*propagate row size*/
                            /* set rowheight to size */
                            rowheight[index] = size;
                            /* set fixed height flag */
                            fixrowsize[index] = (size > 0 ? 1 : 0);
                            /* and propagate row center */
                            rowcenter[index] = rowcenter[irow];
                            if (rowglobal) {       /* set global values */
                                /* set global row height */
                                growheight[index] = rowheight[index];
                                /*set global height flag*/
                                gfixrowsize[index] = fixrowsize[index];
                                growcenter[index] = rowcenter[irow];
                            } /*set global row center*/
                            if (!ispropagate) break;
                        }   /* don't propagate */
                } /* --- end-of-if(size>=3&&size<=500) --- */
                if (mctx->msglevel >= 29 && mctx->msgfp != NULL) /* debugging */
                    fprintf(mctx->msgfp, ":%c=%d/fix#%d", whchars[prepcase],
                            (prepcase == 1 ? colwidth[icol] : rowheight[irow]),
                            (prepcase == 1 ? fixcolsize[icol] : fixrowsize[irow]));
            } /* --- end-of-if(isdigit()) --- */
        } /* --- end-of-if(prepcase!=0) --- */
        /* bump col if lowercase lcr */
        if (prepcase == 1) icol++;
        /* bump row if uppercase BC */
        else if (prepcase == 2) irow++;
    } /* --- end-of-while(*preptr!='\000') --- */
    if (mctx->msglevel >= 29 && mctx->msgfp != NULL) /* debugging, emit final newline */
        if (itoken > 0)             /* if we have preamble */
            fprintf(mctx->msgfp, "\n");
    /* ------------------------------------------------------------
    tokenize and rasterize components  a & b \\ c & d \\ etc  of subexpr
    ------------------------------------------------------------ */
    /* --- rasterize tokens one at a time, and maintain row,col counts --- */
    /* start with top row */
    nrows = 0;
    ncols[nrows] = 0;           /* no tokens/cols in top row yet */
    while (1) {              /* scan chars till end */
        /* --- local control flags --- */
        int   iseox = (*exprptr == '\000'),   /* null signals end-of-expression */
                      iseor = iseox,          /* \\ or eox signals end-of-row */
                              /* & or eor signals end-of-col */
                              iseoc = iseor;
        /* --- check for escapes --- */
        /* is current char escape? */
        isescape = isthischar(*exprptr, ESCAPE);
        /*prev char esc?*/
        wasescape = (!isnewrow && isthischar(*(exprptr - 1), ESCAPE));
        /* # preceding consecutive escapes */
        nescapes = (wasescape ? nescapes + 1 : 0);
        /* is current char escaped? */
        ischarescaped = (nescapes % 2 == 0 ? 0 : 1);
        /* ------------------------------------------------------------
        check for {...} subexpression starting from where we are now
        ------------------------------------------------------------ */
        if (*exprptr == '{'            /* start of {...} subexpression */
                &&   !ischarescaped) {        /* if not escaped \{ */
            /*entire subexpr*/
            subptr = texsubexpr(mctx, exprptr, subtok, 4095, "{", "}", 1, 1);
            /* #chars in {...} */
            subtoklen = strlen(subtok);
            /* copy {...} to accumulated token */
            memcpy(tokptr, exprptr, subtoklen);
            /* bump tokptr to end of token */
            tokptr  += subtoklen;
            /* and bump exprptr past {...} */
            exprptr += subtoklen;
            /* signal non-empty token */
            istokwhite = 0;
            /* continue with char after {...} */
            continue;
        } /* --- end-of-if(*exprptr=='{') --- */
        /* ------------------------------------------------------------
        check for end-of-row(\\) and/or end-of-col(&)
        ------------------------------------------------------------ */
        /* --- check for (escaped) end-of-row delimiter --- */
        if (isescape && !ischarescaped)    /* current char is escaped */
            if (isthischar(*(exprptr + 1), rowdelim) /* next char is rowdelim */
                    ||   *(exprptr + 1) == '\000') { /* or a pathological null */
                /* so set end-of-row flag */
                iseor = 1;
                wasescape = isescape = nescapes = 0;
            } /* reset flags for new row */
        /* --- check for end-of-col delimiter --- */
        if (iseor             /* end-of-row signals end-of-col */
                || (!ischarescaped && isthischar(*exprptr, coldelim))) /*or unescaped coldel*/
            /* so set end-of-col flag */
            iseoc = 1;
        /* ------------------------------------------------------------
        rasterize completed token
        ------------------------------------------------------------ */
        if (iseoc) {               /* we have a completed token */
            /* first, null-terminate token */
            *tokptr = '\000';
            /* --- check first token in row for [len] and/or \hline or \hdash --- */
            /*init for token not only an \hline*/
            ishonly = 0;
            if (ncols[nrows] == 0) {     /*\hline must be first token in row*/
                tokptr = token;
                skipwhite(tokptr);
                /* skip whitespace after // */
                /* --- first check for optional [len] --- */
                if (*tokptr == '[') {          /* have [len] if leading char is [ */
                    /* ---parse [len] and bump tokptr past it, interpret as double--- */
                    char lenexpr[128];
                    /* chars between [...] as int */
                    int len;
                    tokptr = texsubexpr(mctx, tokptr, lenexpr, 127, "[", "]", 0, 0);
                    if (*lenexpr != '\000') {    /* got [len] expression */
                        /* len in pixels */
                        len = iround(mctx->unitlength * strtod(lenexpr, NULL));
                        if (len >= (-63) && len <= 255) { /* sanity check */
                            /* extra vspace before this row */
                            vrowspace[nrows] = len;
                            /* flush [len] from token */
                            strcpy(token, tokptr);
                            tokptr = token;
                            skipwhite(tokptr);
                        }
                    } /* reset ptr, skip white */
                } /* --- end-of-if(*tokptr=='[') --- */
                /* --- now check for \hline or \hdash --- */
                /* extract first char from token */
                tokptr = texchar(mctx, tokptr, hltoken);
                /* length of first char */
                hltoklen = strlen(hltoken);
                if (hltoklen >= minhltoklen) {     /*token must be at least \hl or \hd*/
                    if (memcmp(hlchar, hltoken, hltoklen) == 0) /* we have an \hline */
                        /* bump \hline count for row */
                        hline[nrows] += 1;
                    else if (memcmp(hdchar, hltoken, hltoklen) == 0) /*we have an \hdash*/
                        hline[nrows] = (-1);
                }   /* set \hdash flag for row */
                if (hline[nrows] != 0) {       /* \hline or \hdash prefixes token */
                    /* flush whitespace after \hline */
                    skipwhite(tokptr);
                    if (*tokptr == '\000'  /* end-of-expression after \hline */
                            ||   isthischar(*tokptr, coldelim)) { /* or unescaped coldelim */
                        /* so token contains \hline only */
                        istokwhite = 1;
                        if (iseox) ishonly = 1;
                    } /* ignore entire row at eox */
                    else
                    /* token contains more than \hline */
                        strcpy(token, tokptr);
                } /* so flush \hline from token */
            } /* --- end-of-if(ncols[nrows]==0) --- */
            /* --- rasterize completed token --- */
            toksp[ntokens] = (istokwhite ? NULL : /* don't rasterize empty token */
                              /* rasterize non-empty token */
                              rasterize(mctx, token, size));
            if (toksp[ntokens] != NULL)      /* have a rasterized token */
                /* bump rasterized token count */
                nnonwhite++;
            /* --- maintain colwidth[], rowheight[] max, and rowbaseln[] --- */
            if (toksp[ntokens] != NULL) {    /* we have a rasterized token */
                /* --- update max token "height" in current row, and baseline --- */
                int twidth = ((toksp[ntokens])->image)->width,  /* width of token */
                             theight = ((toksp[ntokens])->image)->height, /* height of token */
                                       tbaseln = (toksp[ntokens])->baseline,  /* baseline of token */
                                                 rheight = rowheight[nrows], /* current max height for row */
                                                           /* current baseline for max height */
                                                           rbaseln = rowbaseln[nrows];
                if (0 || fixrowsize[nrows] == 0)   /* rowheight not fixed */
                    rowheight[nrows] = /*max2( rheight,*/( /* current (max) rowheight */
                                                             max2(rbaseln + 1, tbaseln + 1)   /* max height above baseline */
                                                             /* plus max below */
                                                             + max2(rheight - rbaseln - 1, theight - tbaseln - 1));
                /*max space above baseline*/
                rowbaseln[nrows] = max2(rbaseln, tbaseln);
                /* --- update max token width in current column --- */
                /* current column index */
                icol = ncols[nrows];
                if (0 || fixcolsize[icol] == 0)    /* colwidth not fixed */
                    /*widest token in col*/
                    colwidth[icol] = max2(colwidth[icol], twidth);
            } /* --- end-of-if(toksp[]!=NULL) --- */
            /* --- bump counters --- */
            if (!ishonly)            /* don't count only an \hline */
                if (ncols[nrows] < maxarraysz) {   /* don't overflow arrays */
                    /* bump total token count */
                    ntokens++;
                    ncols[nrows] += 1;
                }      /* and bump #cols in current row */
            /* --- get ready for next token --- */
            /* reset ptr for next token */
            tokptr = token;
            /* next token starts all white */
            istokwhite = 1;
        } /* --- end-of-if(iseoc) --- */
        /* ------------------------------------------------------------
        bump row as necessary
        ------------------------------------------------------------ */
        if (iseor) {               /* we have a completed row */
            /* max# cols in array */
            maxcols = max2(maxcols, ncols[nrows]);
            if (ncols[nrows] > 0 || hline[nrows] == 0) /*ignore row with only \hline*/
                if (nrows < maxarraysz)        /* don't overflow arrays */
                    /* bump row count */
                    nrows++;
            /* no cols in this row yet */
            ncols[nrows] = 0;
            if (!iseox) {            /* don't have a null yet */
                /* bump past extra \ in \\ delim */
                exprptr++;
                iseox = (*exprptr == '\000');
            } /* recheck for pathological \null */
            /* signal start of new row */
            isnewrow = 1;
        } /* --- end-of-if(iseor) --- */
        else
            /* no longer first col of new row */
            isnewrow = 0;
        /* ------------------------------------------------------------
        quit when done, or accumulate char in token and proceed to next char
        ------------------------------------------------------------ */
        /* --- quit when done --- */
        /* null terminator signalled done */
        if (iseox) break;
        /* --- accumulate chars in token --- */
        if (!iseoc) {              /* don't accumulate delimiters */
            /* accumulate non-delim char */
            *tokptr++ = *exprptr;
            if (!isthischar(*exprptr, WHITESPACE))  /* this token isn't empty */
                istokwhite = 0;
        }       /* so reset flag to rasterize it */
        /* --- ready for next char --- */
        /* bump ptr */
        exprptr++;
    } /* --- end-of-while(*exprptr!='\000') --- */
    /* --- make sure we got something to do --- */
    if (nnonwhite < 1)           /* completely empty array */
        /* NULL back to caller */
        goto end_of_job;
    /* ------------------------------------------------------------
    determine dimensions of array raster and allocate it
    ------------------------------------------------------------ */
    /* --- adjust colspace --- */
    /* temp kludge */
    colspace = 2 + 2 * size;
    /* --- reset propagated sizes at boundaries of array --- */
    /* reset explicit 0's at edges */
    colwidth[maxcols] = rowheight[nrows] = 0;
    /* --- determine width of array raster --- */
    /* empty space between cols */
    width = colspace * (maxcols - 1);
    if (mctx->msglevel >= 29 && mctx->msgfp != NULL) /* debugging */
        fprintf(mctx->msgfp, "rastarray> %d cols,  widths: ", maxcols);
    for (icol = 0; icol <= maxcols; icol++) { /* and for each col */
        /*width of this col (0 for maxcols)*/
        width += colwidth[icol];
        /*plus space for vline, if present*/
        width += vlinespace(icol);
        if (mctx->msglevel >= 29 && mctx->msgfp != NULL) /* debugging */
            fprintf(mctx->msgfp, " %d=%2d+%d", icol + 1, colwidth[icol], (vlinespace(icol)));
    }
    /* --- determine height of array raster --- */
    /* empty space between rows */
    height = rowspace * (nrows - 1);
    if (mctx->msglevel >= 29 && mctx->msgfp != NULL) /* debugging */
        fprintf(mctx->msgfp, "\nrastarray> %d rows, heights: ", nrows);
    for (irow = 0; irow <= nrows; irow++) { /* and for each row */
        /*height of this row (0 for nrows)*/
        height += rowheight[irow];
        height += vrowspace[irow];      /*plus extra //[len], if present*/
        /*plus space for hline, if present*/
        height += hlinespace(irow);
        if (mctx->msglevel >= 29 && mctx->msgfp != NULL) /* debugging */
            fprintf(mctx->msgfp, " %d=%2d+%d", irow + 1, rowheight[irow], (hlinespace(irow)));
    }
    /* --- allocate subraster and raster for array --- */
    if (mctx->msglevel >= 29 && mctx->msgfp != NULL) /* debugging */
        fprintf(mctx->msgfp, "\nrastarray> tot width=%d(colspc=%d) height=%d(rowspc=%d)\n",
                width, colspace, height, rowspace);
    if ((arraysp = new_subraster(mctx, width, height, pixsz)) /* allocate new subraster */
            /* quit if failed */
            ==   NULL)  goto end_of_job;
    /* --- initialize subraster parameters --- */
    /* image */
    arraysp->type = IMAGERASTER;
    /* not applicable for image */
    arraysp->symdef = NULL;
    /*is a little above center good?*/
    arraysp->baseline = min2(height / 2 + 5, height - 1);
    /* size (probably unneeded) */
    arraysp->size = size;
    /* raster embedded in subraster */
    arrayrp = arraysp->image;
    /* ------------------------------------------------------------
    embed tokens/cells in array
    ------------------------------------------------------------ */
    /* start with first token */
    itoken = 0;
    /* start at top row of array */
    toprow = 0;
    for (irow = 0; irow <= nrows; irow++) { /*tokens were accumulated row-wise*/
        /* --- initialization for row --- */
        /* baseline for this row */
        int   baseline = rowbaseln[irow];
        if (hline[irow] != 0) {        /* need hline above this row */
            /* row for hline */
            int hrow = (irow < 1 ? 0 : toprow - rowspace / 2);
            /* row for bottom hline */
            if (irow >= nrows) hrow = height - 1;
            rule_raster(mctx, arrayrp, hrow, 0, width, 1, (hline[irow] < 0 ? 1 : 0));
        } /* hline */
        /*just needed \hline for irow=nrows*/
        if (irow >= nrows) break;
        toprow += vrowspace[irow];        /* extra //[len] space above irow */
        /* check for large negative [-len] */
        if (toprow < 0) toprow = 0;
        /* space for hline above irow */
        toprow += hlinespace(irow);
        /* start at leftmost column */
        leftcol = 0;
        for (icol = 0; icol < ncols[irow]; icol++) { /* go through cells in this row */
            /* token that belongs in this cell */
            subraster *tsp = toksp[itoken];
            /* --- first adjust leftcol for vline to left of icol, if present ---- */
            /* space for vline to left of col */
            leftcol += vlinespace(icol);
            /* --- now rasterize cell ---- */
            if (tsp != NULL) {           /* have a rasterized cell token */
                /* --- local parameters --- */
                int cwidth = colwidth[icol],  /* total column width */
                    twidth = (tsp->image)->width, /* token width */
                    theight = (tsp->image)->height, /* token height */
                    tokencol = 0,         /*H offset (init for left justify)*/
                    tokenrow = baseline - tsp->baseline; /*V offset (init for baseline)*/
                /* --- adjust leftcol for vline to left of icol, if present ---- */
                /*leftcol += vlinespace(icol);*/  /* space for vline to left of col */
                /* --- reset justification (if not left-justified) --- */
                if (justify[icol] == 0)        /* but user wants it centered */
                    tokencol = (cwidth - twidth + 1) / 2; /* so split margin left/right */
                else if (justify[icol] == 1)   /* or user wants right-justify */
                    /* so put entire margin at left */
                    tokencol = cwidth - twidth;
                /* --- reset vertical centering (if not baseline-aligned) --- */
                if (rowcenter[irow])       /* center cells in row vertically */
                    /* center row */
                    tokenrow = (rowheight[irow] - theight) / 2;
                /* --- embed token raster at appropriate place in array raster --- */
                rastput(mctx, arrayrp, tsp->image,  /* overlay cell token in array */
                        toprow + tokenrow,    /*with aligned baseline or centered*/
                        /* and justified as requested */
                        leftcol + tokencol, 1);
            } /* --- end-of-if(tsp!=NULL) --- */
            /* bump index for next cell */
            itoken++;
            leftcol += colwidth[icol] + colspace /*move leftcol right for next col*/
                       /* + vlinespace(icol) */ ; /*don't add space for vline to left of col*/
        } /* --- end-of-for(icol) --- */
        /* move toprow down for next row */
        toprow += rowheight[irow] + rowspace;
    } /* --- end-of-for(irow) --- */
    /* ------------------------------------------------------------
    draw vlines as necessary
    ------------------------------------------------------------ */
    /* start at leftmost column */
    leftcol = 0;
    for (icol = 0; icol <= maxcols; icol++) { /* check each col for a vline */
        if (vline[icol] != 0) {        /* need vline to left of this col */
            /* column for vline */
            int vcol = (icol < 1 ? 0 : leftcol - colspace / 2);
            /*column for right edge vline*/
            if (icol >= maxcols) vcol = width - 1;
            rule_raster(mctx, arrayrp, 0, vcol, 1, height, (vline[icol] < 0 ? 2 : 0));
        } /* vline */
        /* space for vline to left of col */
        leftcol += vlinespace(icol);
        if (icol < maxcols)            /* don't address past end of array */
            /*move leftcol right for next col*/
            leftcol += colwidth[icol] + colspace;
    } /* --- end-of-for(icol) --- */
    /* ------------------------------------------------------------
    free workspace and return final result to caller
    ------------------------------------------------------------ */
end_of_job:
    /* --- free workspace --- */
    if (ntokens > 0)           /* if we have workspace to free */
        while (--ntokens >= 0)       /* free each token subraster */
            if (toksp[ntokens] != NULL)    /* if we rasterized this cell */
                /* then free it */
                delete_subraster(mctx, toksp[ntokens]);
    /* --- return final result to caller --- */
    return (arraysp);
} /* --- end-of-function rastarray() --- */


/* ==========================================================================
 * Function:    rastpicture ( expression, size, basesp, arg1, arg2, arg3 )
 * Purpose: \picture handler, returns subraster corresponding to picture
 *      expression (immediately following \picture) at font size
 * --------------------------------------------------------------------------
 * Arguments:   expression (I/O) char **  to first char of null-terminated
 *              string immediately following \picture to be
 *              rasterized, and returning ptr immediately
 *              following last character processed.
 *      size (I)    int containing 0-7 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding \picture
 *              (unused, but passed for consistency)
 *      arg1 (I)    int unused
 *      arg2 (I)    int unused
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to subraster corresponding to picture
 *              expression, or NULL for any parsing error
 * --------------------------------------------------------------------------
 * Notes:     o Summary of syntax...
 *        \picture(width,height){(x,y){pic_elem}~(x,y){pic_elem}~etc}
 *        o
 * ======================================================================= */
/* --- entry point --- */
subraster *rastpicture(mimetex_ctx *mctx, char **expression, int size, subraster *basesp,
                       int arg1, int arg2, int arg3)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    char picexpr[2049], *picptr = picexpr, /* picture {expre} */
         putexpr[256], *putptr, *multptr, /*[multi]put (x,y[;xinc,yinc;num])*/
         pream[96], *preptr,     /* optional put preamble */
         picelem[1025]; /* picture element following put */
    subraster *picelemsp = NULL, /* rasterize picture elements */
              *picturesp = NULL, /* subraster for entire picture */
              *oldworkingbox = mctx->workingbox; /* save working box on entry */
    /* raster for entire picture */
    raster  *picturerp = NULL;
    /* pixels are one bit each */
    int pixsz = 1;
    double x = 0.0, y = 0.0,       /* x,y-coords for put,multiput*/
           xinc = 0.0, yinc = 0.0; /* x,y-incrementss for multiput*/
    int width = 0,  height = 0, /* #pixels width,height of picture */
        ewidth = 0, eheight = 0,    /* pic element width,height */
        ix = 0, xpos = 0, iy = 0, ypos = 0,   /* mimeTeX x,y pixel coords */
        num = 1, inum; /* number reps, index of element */
    /* center or lowerleft put position*/
    int iscenter = 0;
    int *oldworkingparam = mctx->workingparam, /* save working param on entry */
        origin = 0; /* x,yinc ++=00 +-=01 -+=10 --=11 */
    /* ------------------------------------------------------------
    First obtain (width,height) arguments immediately following \picture command
    ------------------------------------------------------------ */
    /* --- parse for (width,height) arguments, and bump expression past it --- */
    *expression = texsubexpr(mctx, *expression, putexpr, 254, "(", ")", 0, 0);
    /* couldn't get (width,height) */
    if (*putexpr == '\000') goto end_of_job;
    /* --- now interpret width,height returned in putexpr --- */
    if ((putptr = strchr(putexpr, ',')) != NULL) /* look for ',' in width,height*/
        /* found it, so replace ',' by '\0'*/
        *putptr = '\000';
    /*width pixels*/
    width = height = iround(mctx->unitlength * strtod(putexpr, NULL));
    if (putptr != NULL)              /* 2nd arg, if present, is height */
        /*in pixels*/
        height = iround(mctx->unitlength * strtod(putptr + 1, NULL));
    /* ------------------------------------------------------------
    Then obtain entire picture {...} subexpression following (width,height)
    ------------------------------------------------------------ */
    /* --- parse for picture subexpression, and bump expression past it --- */
    *expression = texsubexpr(mctx, *expression, picexpr, 2047, "{", "}", 0, 0);
    /* couldn't get {pic_elements} */
    if (*picexpr == '\000') goto end_of_job;
    /* ------------------------------------------------------------
    allocate subraster and raster for complete picture
    ------------------------------------------------------------ */
    /* --- sanity check on width,height args --- */
    if (width < 2 ||  width > 600
            ||  height < 2 || height > 600) goto end_of_job;
    /* --- allocate and initialize subraster for constructed picture --- */
    if ((picturesp = new_subraster(mctx, width, height, pixsz)) /*allocate new subraster*/
            /* quit if failed */
            ==   NULL)  goto end_of_job;
    /* set mctx->workingbox to our picture */
    mctx->workingbox = picturesp;
    /* --- initialize picture subraster parameters --- */
    /* image */
    picturesp->type = IMAGERASTER;
    /* not applicable for image */
    picturesp->symdef = NULL;
    /* is a little above center good? */
    picturesp->baseline = height / 2 + 2;
    /* size (probably unneeded) */
    picturesp->size = size;
    /* raster embedded in subraster */
    picturerp = picturesp->image;
    if (mctx->msgfp != NULL && mctx->msglevel >= 29) /* debugging */
        fprintf(mctx->msgfp, "picture> width,height=%d,%d\n", width, height);
    /* ------------------------------------------------------------
    parse out each picture element, rasterize it, and place it in picture
    ------------------------------------------------------------ */
    while (*picptr != '\000') {      /* until we run out of pic_elems */
        /* ------------------------------------------------------------
        first obtain leading \[multi]put(x,y[;xinc,yinc;num]) args for pic_elem
        ------------------------------------------------------------ */
        /* --- init default values in case not explicitly supplied in args --- */
        x = y = 0.0;
        xinc = yinc = 0.0;
        /* init default values */
        num = 1;
        /* center, origin */
        iscenter = origin = 0;
        /* --- get (pream$x,y;xinc,yinc;num ) args and bump picptr past it --- */
        while (*picptr != '\000')          /* skip invalid chars preceding ( */
            /* found opening ( */
            if (*picptr == '(') break;
            /* else skip invalid char */
            else picptr++;
        picptr = texsubexpr(mctx, picptr, putexpr, 254, "(", ")", 0, 0);
        /* couldn't get (x,y) */
        if (*putexpr == '\000') goto end_of_job;
        /* --- first look for $-terminated or for any non-digit preamble --- */
        /* init preamble as empty string */
        *pream = '\000';
        if ((putptr = strchr(putexpr, '$')) != NULL) { /*check for $ pream terminator*/
            /* replace $ by '\0', bump past $ */
            *putptr++ = '\000';
            strninit(pream, putexpr, 92);
        } /* copy leading preamble from put */
        else {                /* look for any non-digit preamble */
            /* #chars in preamble */
            int npream = 0;
            for (preptr = pream, putptr = putexpr; ; npream++, putptr++)
                if (*putptr == '\000'        /* end-of-putdata signalled */
                        ||   !isalpha((int)(*putptr))   /* or found non-alpha char */
                        /* or preamble too long */
                        ||   npream > 92) break;
                /* copy alpha char to preamble */
                else *preptr++ = *putptr;
            *preptr = '\000';
        }       /* null-terminate preamble */
        /* --- interpret preamble --- */
        for (preptr = pream; ; preptr++)   /* examine each preamble char */
            /* end-of-preamble signalled */
            if (*preptr == '\000') break;
            else switch (tolower(*preptr)) { /* check lowercase preamble char */
                default:
                    /* unrecognized flag */
                    break;
                case 'c':
                    iscenter = 1;
                    /* center pic_elem at x,y coords */
                    break;
                } /* --- end-of-switch --- */
        /* --- interpret x,y;xinc,yinc;num following preamble --- */
        if (*putptr != '\000') {       /*check for put data after preamble*/
            /* --- first squeeze preamble out of put expression --- */
            /* squeeze out preamble */
            if (*pream != '\000') strcpy(putexpr, putptr);
            /* --- interpret x,y --- */
            if ((multptr = strchr(putexpr, ';')) != NULL) /*semicolon signals multiput*/
                /* replace semicolon by '\0' */
                *multptr = '\000';
            if ((putptr = strchr(putexpr, ',')) != NULL) /* comma separates x,y */
                /* replace comma by '\0'  */
                *putptr = '\000';
            if (*putexpr != '\000')       /* leading , may be placeholder */
                /* x coord in pixels*/
                x = mctx->unitlength * strtod(putexpr, NULL);
            if (putptr != NULL)       /* 2nd arg, if present, is y coord */
                /* in pixels */
                y = mctx->unitlength * strtod(putptr + 1, NULL);
            /* --- interpret xinc,yinc,num if we have a multiput --- */
            if (multptr != NULL) {        /* found ';' signalling multiput */
                if ((preptr = strchr(multptr + 1, ';')) != NULL) /* ';' preceding num arg*/
                    /* replace ';' by '\0' */
                    *preptr = '\000';
                if ((putptr = strchr(multptr + 1, ',')) != NULL) /* ',' between xinc,yinc*/
                    /* replace ',' by '\0' */
                    *putptr = '\000';
                if (*(multptr + 1) != '\000')   /* leading , may be placeholder */
                    /* xinc in pixels */
                    xinc = mctx->unitlength * strtod(multptr + 1, NULL);
                if (putptr != NULL)         /* 2nd arg, if present, is yinc */
                    /* in user pixels */
                    yinc = mctx->unitlength * strtod(putptr + 1, NULL);
                /*explicit num val or 999*/
                num = (preptr == NULL ? 999 : atoi(preptr + 1));
            } /* --- end-of-if(multptr!=NULL) --- */
        } /* --- end-of-if(*preptr!='\000') --- */
        if (mctx->msgfp != NULL && mctx->msglevel >= 29) /* debugging */
            fprintf(mctx->msgfp,
                    "picture> pream;x,y;xinc,yinc;num=\"%s\";%.2f,%.2f;%.2f,%.2f;%d\n",
                    pream, x, y, xinc, yinc, num);
        /* ------------------------------------------------------------
        now obtain {...} picture element following [multi]put, and rasterize it
        ------------------------------------------------------------ */
        /* --- parse for {...} picture element and bump picptr past it --- */
        picptr = texsubexpr(mctx, picptr, picelem, 1023, "{", "}", 0, 0);
        /* couldn't get {pic_elem} */
        if (*picelem == '\000') goto end_of_job;
        if (mctx->msgfp != NULL && mctx->msglevel >= 29) /* debugging */
            fprintf(mctx->msgfp, "picture> picelem=\"%.50s\"\n", picelem);
        /* --- rasterize picture element --- */
        /* init origin as working param */
        origin = 0;
        /* and point working param to it */
        mctx->workingparam = &origin;
        /* rasterize picture element */
        picelemsp = rasterize(mctx, picelem, size);
        /* failed to rasterize, skip elem */
        if (picelemsp == NULL) continue;
        /* width of element, in pixels */
        ewidth  = (picelemsp->image)->width;
        /* height of element, in pixels */
        eheight = (picelemsp->image)->height;
        /* origin set to (.5,.5) for center*/
        if (origin == 55) iscenter = 1;
        if (mctx->msgfp != NULL && mctx->msglevel >= 29) { /* debugging */
            fprintf(mctx->msgfp, "picture> ewidth,eheight,origin,num=%d,%d,%d,%d\n",
                    ewidth, eheight, origin, num);
            if (mctx->msglevel >= 999) type_raster(mctx, picelemsp->image, mctx->msgfp);
        }
        /* ------------------------------------------------------------
        embed element in picture (once, or multiple times if requested)
        ------------------------------------------------------------ */
        for (inum = 0; inum < num; inum++) { /* once, or for num repetitions */
            /* --- set x,y-coords for this iteration --- */
            ix = iround(x);
            /* round x,y to nearest integer */
            iy = iround(y);
            if (iscenter) {          /* place center of element at x,y */
                /* x picture coord to center elem */
                xpos = ix - ewidth / 2;
                ypos = height - iy - eheight / 2;
            } /* y pixel coord to center elem */
            else {              /* default places lower-left at x,y*/
                /* set x pixel coord for left */
                xpos = ix;
                if (origin == 10 || origin == 11)  /* x,yinc's are -+ or -- */
                    /* so set for right instead */
                    xpos = ix - ewidth;
                /* set y pixel coord for lower */
                ypos = height - iy - eheight;
                if (origin == 1 || origin == 11) /* x,yinc's are +- or -- */
                    ypos = height - iy;
            }     /* so set for upper instead */
            if (mctx->msgfp != NULL && mctx->msglevel >= 29) /* debugging */
                fprintf(mctx->msgfp,
                        "picture> inum,x,y,ix,iy,xpos,ypos=%d,%.2f,%.2f,%d,%d,%d,%d\n",
                        inum, x, y, ix, iy, xpos, ypos);
            /* --- embed token raster at xpos,ypos, and quit if out-of-bounds --- */
            if (!rastput(mctx, picturerp, picelemsp->image, ypos, xpos, 0)) break;
            /* --- apply increment --- */
            /* quit if both increments zero */
            if (xinc == 0. && yinc == 0.) break;
            x += xinc;
            /* increment coords for next iter */
            y += yinc;
        } /* --- end-of-for(inum) --- */
        /* --- free picture element subraster after embedding it in picture --- */
        /* done with subraster, so free it */
        delete_subraster(mctx, picelemsp);
    } /* --- end-of-while(*picptr!=0) --- */
    /* ------------------------------------------------------------
    return picture constructed from pic_elements to caller
    ------------------------------------------------------------ */
end_of_job:
    /* restore original working box */
    mctx->workingbox = oldworkingbox;
    /* restore original working param */
    mctx->workingparam = oldworkingparam;
    /* return our picture to caller */
    return (picturesp);
} /* --- end-of-function rastpicture() --- */


/* ==========================================================================
 * Function:    rastline ( expression, size, basesp, arg1, arg2, arg3 )
 * Purpose: \line handler, returns subraster corresponding to line
 *      parameters (xinc,yinc){xlen}
 * --------------------------------------------------------------------------
 * Arguments:   expression (I/O) char **  to first char of null-terminated
 *              string immediately following \line to be
 *              rasterized, and returning ptr immediately
 *              following last character processed.
 *      size (I)    int containing 0-7 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding \line
 *              (unused, but passed for consistency)
 *      arg1 (I)    int unused
 *      arg2 (I)    int unused
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to subraster corresponding to line
 *              requested, or NULL for any parsing error
 * --------------------------------------------------------------------------
 * Notes:     o Summary of syntax...
 *        \line(xinc,yinc){xlen}
 *        o if {xlen} not given, then it's assumed xlen = |xinc|
 * ======================================================================= */
/* --- entry point --- */
subraster *rastline(mimetex_ctx *mctx, char **expression, int size, subraster *basesp,
                    int arg1, int arg2, int arg3)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /*line(xinc,yinc){xlen}*/
    char  linexpr[257], *xptr = linexpr;
    /* subraster for line */
    subraster *linesp = NULL;
    /*char  *origexpression = *expression;*/ /*original expression after \line*/
    /* pixels are one bit each */
    int pixsz = 1;
    /* line thickness */
    int thickness = 1;
    double  xinc = 0.0, yinc = 0.0, /* x,y-increments for line, */
                       /* x,y lengths for line */
                       xlen = 0.0, ylen = 0.0;
    int width = 0,  height = 0, /* #pixels width,height of line */
                             /*alloc width,height plus thickness*/
                             rwidth = 0, rheight = 0;
    int istop = 0,  isright = 0,    /* origin at bot-left if x,yinc>=0 */
                              /* x,yinc: ++=00 +-=01 -+=10 --=11 */
                              origin = 0;
    /* ------------------------------------------------------------
    obtain (xinc,yinc) arguments immediately following \line command
    ------------------------------------------------------------ */
    /* --- parse for (xinc,yinc) arguments, and bump expression past it --- */
    *expression = texsubexpr(mctx, *expression, linexpr, 253, "(", ")", 0, 0);
    /* couldn't get (xinc,yinc) */
    if (*linexpr == '\000') goto end_of_job;
    /* --- now interpret xinc,yinc;thickness returned in linexpr --- */
    if ((xptr = strchr(linexpr, ';')) != NULL) { /* look for ';' after xinc,yinc */
        /* terminate linexpr at ; */
        *xptr = '\000';
        thickness = (int)strtol(xptr + 1, NULL, 10);
    } /* get int thickness */
    if ((xptr = strchr(linexpr, ',')) != NULL) /* look for ',' in xinc,yinc */
        /* found it, so replace ',' by '\0'*/
        *xptr = '\000';
    if (*linexpr != '\000')          /* check against missing 1st arg */
        /* xinc in user units */
        xinc = xlen = strtod(linexpr, NULL);
    if (xptr != NULL)            /* 2nd arg, if present, is yinc */
        /* in user units */
        yinc = ylen = strtod(xptr + 1, NULL);
    /* ------------------------------------------------------------
    obtain optional {xlen} following (xinc,yinc), and calculate ylen
    ------------------------------------------------------------ */
    /* --- check if {xlen} given --- */
    if (*(*expression) == '{') {     /*have {xlen} if leading char is { */
        /* --- parse {xlen} and bump expression past it, interpret as double --- */
        *expression = texsubexpr(mctx, *expression, linexpr, 253, "{", "}", 0, 0);
        /* couldn't get {xlen} */
        if (*linexpr == '\000') goto end_of_job;
        /* xlen in user units */
        xlen = strtod(linexpr, NULL);
        /* --- set other values accordingly --- */
        /* if xlen negative, flip xinc sign*/
        if (xlen  < 0.0) xinc = -xinc;
        /* set ylen from xlen and slope*/
        if (xinc != 0.0) ylen = xlen * yinc / xinc;
        /* can't have xlen if xinc=0 */
        else xlen  = 0.0;
    } /* --- end-of-if(*(*expression)=='{') --- */
    /* ------------------------------------------------------------
    calculate width,height, etc, based on xlen,ylen, etc
    ------------------------------------------------------------ */
    /* --- force lengths positive --- */
    /* force xlen positive */
    xlen = absval(xlen);
    /* force ylen positive */
    ylen = absval(ylen);
    /* --- calculate corresponding lengths in pixels --- */
    /*scale by mctx->unitlength and round,*/
    width   = max2(1, iround(mctx->unitlength * xlen));
    /* and must be at least 1 pixel */
    height  = max2(1, iround(mctx->unitlength * ylen));
    rwidth  = width  + (ylen < 0.001 ? 0 : max2(0, thickness - 1));
    rheight = height + (xlen < 0.001 ? 0 : max2(0, thickness - 1));
    /* --- set origin corner, x,yinc's: ++=0=(0,0) +-=1=(0,1) -+=10=(1,0) --- */
    /*negative xinc, so corner is (1,?)*/
    if (xinc < 0.0) isright = 1;
    /*negative yinc, so corner is (?,1)*/
    if (yinc < 0.0) istop = 1;
    /* interpret 0=(0,0), 11=(1,1), etc*/
    origin = isright * 10 + istop;
    if (mctx->msgfp != NULL && mctx->msglevel >= 29) /* debugging */
        fprintf(mctx->msgfp, "rastline> width,height,origin;x,yinc=%d,%d,%d;%g,%g\n",
                width, height, origin, xinc, yinc);
    /* ------------------------------------------------------------
    allocate subraster and raster for line
    ------------------------------------------------------------ */
    /* --- sanity check on width,height,thickness args --- */
    if (width < 1 ||  width > 600
            ||  height < 1 || height > 600
            ||  thickness < 1 || thickness > 25) goto end_of_job;
    /* --- allocate and initialize subraster for constructed line --- */
    if ((linesp = new_subraster(mctx, rwidth, rheight, pixsz)) /* alloc new subraster */
            /* quit if failed */
            ==   NULL)  goto end_of_job;
    /* --- initialize line subraster parameters --- */
    /* image */
    linesp->type = IMAGERASTER;
    /* not applicable for image */
    linesp->symdef = NULL;
    linesp->baseline = height / 2 + 2   /* is a little above center good? */
                       /* account for line thickness too */
                       + (rheight - height) / 2;
    /* size (probably unneeded) */
    linesp->size = size;
    /* ------------------------------------------------------------
    draw the line
    ------------------------------------------------------------ */
    line_raster(mctx, linesp->image,
    /* embedded raster image */
                (istop ?   0 : height - 1), /* row0, from bottom or top */
                (isright ?  width - 1 : 0), /* col0, from left or right */
                (istop ?   height - 1 : 0), /* row1, to top or bottom */
                (isright ? 0 :  width - 1), /* col1, to right or left */
                /* line thickness (usually 1 pixel)*/
                thickness);
    /* ------------------------------------------------------------
    return constructed line to caller
    ------------------------------------------------------------ */
end_of_job:
    if (mctx->workingparam != NULL)          /* caller wants origin */
        /* return origin corner to caller */
        *mctx->workingparam = origin;
    /* return line to caller */
    return (linesp);
} /* --- end-of-function rastline() --- */


/* ==========================================================================
 * Function:    rastrule ( expression, size, basesp, arg1, arg2, arg3 )
 * Purpose: \rule handler, returns subraster corresponding to rule
 *      parameters [lift]{width}{height}
 * --------------------------------------------------------------------------
 * Arguments:   expression (I/O) char **  to first char of null-terminated
 *              string immediately following \rule to be
 *              rasterized, and returning ptr immediately
 *              following last character processed.
 *      size (I)    int containing 0-7 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding \rule
 *              (unused, but passed for consistency)
 *      arg1 (I)    int unused
 *      arg2 (I)    int unused
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to subraster corresponding to rule
 *              requested, or NULL for any parsing error
 * --------------------------------------------------------------------------
 * Notes:     o Summary of syntax...
 *        \rule[lift]{width}{height}
 *        o if [lift] not given, then bottom of rule on baseline
 *        o if width=0 then you get an invisible strut 1 (one) pixel wide
 * ======================================================================= */
/* --- entry point --- */
subraster *rastrule(mimetex_ctx *mctx, char **expression, int size, subraster *basesp,
                    int arg1, int arg2, int arg3)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* rule[lift]{wdth}{hgt} */
    char rulexpr[257];
    /* subraster for rule */
    subraster *rulesp = NULL;
    /* pixels are one bit each */
    int pixsz = 1;
    /* default rule parameters */
    int lift = 0, width = 0, height = 0;
    /* convert ascii params to doubles */
    double dval;
    /* alloc width, height plus lift */
    int rwidth = 0, rheight = 0;
    /* draw rule in rulesp->image */
    /* ------------------------------------------------------------
    Obtain lift,width,height
    ------------------------------------------------------------ */
    /* --- check for optional lift arg  --- */
    if (*(*expression) == '[') {     /*check for []-enclosed optional arg*/
        *expression = texsubexpr(mctx, *expression, rulexpr, 255, "[", "]", 0, 0);
        /* convert [lift] to int */
        dval = (int)(strtod(rulexpr, NULL) + 0.5);
        if (dval <= 99 && dval >= (-99))     /* sanity check */
            lift = iround(mctx->unitlength * dval);
    } /* scale by mctx->unitlength and round */
    /* --- parse for width --- */
    *expression = texsubexpr(mctx, *expression, rulexpr, 255, "{", "}", 0, 0);
    /* quit if args missing */
    if (*rulexpr == '\000') goto end_of_job;
    /* convert {width} to int */
    dval = (int)(strtod(rulexpr, NULL) + 0.5);
    if (dval <= 500 && dval >= 0)        /* sanity check */
        /* scale by mctx->unitlength and round*/
        width = max2(0, iround(mctx->unitlength * dval));
    /* --- parse for height --- */
    *expression = texsubexpr(mctx, *expression, rulexpr, 255, "{", "}", 0, 0);
    /* quit if args missing */
    if (*rulexpr == '\000') goto end_of_job;
    /* convert {height} to int */
    dval = (int)(strtod(rulexpr, NULL) + 0.5);
    if (dval <= 500 && dval > 0)         /* sanity check */
        /* scale by mctx->unitlength and round*/
        height = max2(1, iround(mctx->unitlength * dval));
    /* --- raster width,height in pixels --- */
    /* raster must be at least 1 pixel*/
    rwidth  = max2(1, width);
    rheight = height + (lift >= 0 ? lift :   /* raster height plus lift */
                        /* may need empty space above rule */
                        (-lift < height ? 0 : -lift - height + 1));
    /* ------------------------------------------------------------
    allocate subraster and raster for rule
    ------------------------------------------------------------ */
    /* --- sanity check on width,height,thickness args --- */
    if (rwidth < 1 ||  rwidth > 600
            ||  rheight < 1 || rheight > 600) goto end_of_job;
    /* --- allocate and initialize subraster for constructed rule --- */
    if ((rulesp = new_subraster(mctx, rwidth, rheight, pixsz)) /* alloc new subraster */
            /* quit if failed */
            ==   NULL)  goto end_of_job;
    /* --- initialize line subraster parameters --- */
    /* image */
    rulesp->type = IMAGERASTER;
    /* not applicable for image */
    rulesp->symdef = NULL;
    /*adjust baseline for lift*/
    rulesp->baseline = rheight - 1 + (lift >= 0 ? 0 : lift);
    /* size (probably unneeded) */
    rulesp->size = size;
    /* ------------------------------------------------------------
    draw the rule
    ------------------------------------------------------------ */
    rule_raster(mctx, rulesp->image,
    /* embedded raster image */
                (-lift < height ? 0 : rheight - height), /* topmost row for top-left corner*/
                0,
                /* leftmost col for top-left corner*/
                width,
                /* rule width */
                height,
                /* rule height */
                /* rule type */
                (width > 0 ? 0 : 4));
    /* ------------------------------------------------------------
    return constructed rule to caller
    ------------------------------------------------------------ */
end_of_job:
    /* return rule to caller */
    return (rulesp);
} /* --- end-of-function rastrule() --- */


/* ==========================================================================
 * Function:    rastcircle ( expression, size, basesp, arg1, arg2, arg3 )
 * Purpose: \circle handler, returns subraster corresponding to ellipse
 *      parameters (xdiam[,ydiam])
 * --------------------------------------------------------------------------
 * Arguments:   expression (I/O) char **  to first char of null-terminated
 *              string immediately following \circle to be
 *              rasterized, and returning ptr immediately
 *              following last character processed.
 *      size (I)    int containing 0-7 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding \circle
 *              (unused, but passed for consistency)
 *      arg1 (I)    int unused
 *      arg2 (I)    int unused
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to subraster corresponding to ellipse
 *              requested, or NULL for any parsing error
 * --------------------------------------------------------------------------
 * Notes:     o Summary of syntax...
 *        \circle(xdiam[,ydiam])
 *        o
 * ======================================================================= */
/* --- entry point --- */
subraster *rastcircle(mimetex_ctx *mctx, char **expression, int size, subraster *basesp,
                      int arg1, int arg2, int arg3)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /*circle(xdiam[,ydiam])*/
    char circexpr[512], *xptr = circexpr;
    /* default to draw all quadrants */
    char *qptr = NULL, quads[256] = "1234";
    /* ;theta0,theta1 instead of ;quads*/
    double  theta0 = 0.0, theta1 = 0.0;
    /* subraster for ellipse */
    subraster *circsp = NULL;
    /* pixels are one bit each */
    int pixsz = 1;
    double xdiam = 0.0, ydiam = 0.0;   /* x,y major/minor axes/diameters */
    /* #pixels width,height of ellipse */
    int width = 0,  height = 0;
    /* drawn lines are one pixel thick */
    int thickness = 1;
    /* force origin centered */
    int origin = 55;
    /* ------------------------------------------------------------
    obtain (xdiam[,ydiam]) arguments immediately following \circle command
    ------------------------------------------------------------ */
    /* --- parse for (xdiam[,ydiam]) args, and bump expression past it --- */
    *expression = texsubexpr(mctx, *expression, circexpr, 500, "(", ")", 0, 0);
    /* couldn't get (xdiam[,ydiam])*/
    if (*circexpr == '\000') goto end_of_job;
    /* --- now interpret xdiam[,ydiam] returned in circexpr --- */
    if ((qptr = strchr(circexpr, ';')) != NULL) { /* semicolon signals quads data */
        /* replace semicolon by '\0' */
        *qptr = '\000';
        /* save user-requested quads */
        strninit(quads, qptr + 1, 128);
        if ((qptr = strchr(quads, ',')) != NULL) { /* have theta0,theta1 instead */
            /* replace , with null */
            *qptr = '\000';
            /* theta0 precedes , */
            theta0 = strtod(quads, NULL);
            /* theta1 follows , */
            theta1 = strtod(qptr + 1, NULL);
            qptr = NULL;
        }          /* signal thetas instead of quads */
        else
            qptr = quads;
    }         /* set qptr arg for circle_raster()*/
    else
    /* no ;quads at all */
        /* default to all 4 quadrants */
        qptr = quads;
    if ((xptr = strchr(circexpr, ',')) != NULL) /* look for ',' in xdiam[,ydiam]*/
        /* found it, so replace ',' by '\0'*/
        *xptr = '\000';
    /* xdiam=ydiam in user units */
    xdiam = ydiam = strtod(circexpr, NULL);
    if (xptr != NULL)            /* 2nd arg, if present, is ydiam */
        /* in user units */
        ydiam = strtod(xptr + 1, NULL);
    /* ------------------------------------------------------------
    calculate width,height, etc
    ------------------------------------------------------------ */
    /* --- calculate width,height in pixels --- */
    /*scale by mctx->unitlength and round,*/
    width  = max2(1, iround(mctx->unitlength * xdiam));
    /* and must be at least 1 pixel */
    height = max2(1, iround(mctx->unitlength * ydiam));
    if (mctx->msgfp != NULL && mctx->msglevel >= 29) /* debugging */
        fprintf(mctx->msgfp, "rastcircle> width,height;quads=%d,%d,%s\n",
                width, height, (qptr == NULL ? "default" : qptr));
    /* ------------------------------------------------------------
    allocate subraster and raster for complete picture
    ------------------------------------------------------------ */
    /* --- sanity check on width,height args --- */
    if (width < 1 ||  width > 600
            ||  height < 1 || height > 600) goto end_of_job;
    /* --- allocate and initialize subraster for constructed ellipse --- */
    if ((circsp = new_subraster(mctx, width, height, pixsz)) /* allocate new subraster */
            /* quit if failed */
            ==   NULL)  goto end_of_job;
    /* --- initialize ellipse subraster parameters --- */
    /* image */
    circsp->type = IMAGERASTER;
    /* not applicable for image */
    circsp->symdef = NULL;
    /* is a little above center good? */
    circsp->baseline = height / 2 + 2;
    /* size (probably unneeded) */
    circsp->size = size;
    /* ------------------------------------------------------------
    draw the ellipse
    ------------------------------------------------------------ */
    if (qptr != NULL)            /* have quads */
        circle_raster(mctx, circsp->image,
        /* embedded raster image */
                      0, 0,               /* row0,col0 are upper-left corner */
                      height - 1, width - 1,  /* row1,col1 are lower-right */
                      thickness,
                      /* line thickness is 1 pixel */
                      /* "1234" quadrants to be drawn */
                      qptr);
    else
    /* have theta0,theta1 */
        circle_recurse(mctx, circsp->image,
        /* embedded raster image */
                       0, 0,               /* row0,col0 are upper-left corner */
                       height - 1, width - 1,  /* row1,col1 are lower-right */
                       thickness,
                       /* line thickness is 1 pixel */
                       /* theta0,theta1 arc to be drawn */
                       theta0, theta1);
    /* ------------------------------------------------------------
    return constructed ellipse to caller
    ------------------------------------------------------------ */
end_of_job:
    if (mctx->workingparam != NULL)          /* caller wants origin */
        /* return center origin to caller */
        *mctx->workingparam = origin;
    /* return ellipse to caller */
    return (circsp);
} /* --- end-of-function rastcircle() --- */


/* ==========================================================================
 * Function:    rastbezier ( expression, size, basesp, arg1, arg2, arg3 )
 * Purpose: \bezier handler, returns subraster corresponding to bezier
 *      parameters (col0,row0)(col1,row1)(colt,rowt)
 * --------------------------------------------------------------------------
 * Arguments:   expression (I/O) char **  to first char of null-terminated
 *              string immediately following \bezier to be
 *              rasterized, and returning ptr immediately
 *              following last character processed.
 *      size (I)    int containing 0-7 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding \bezier
 *              (unused, but passed for consistency)
 *      arg1 (I)    int unused
 *      arg2 (I)    int unused
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to subraster corresponding to bezier
 *              requested, or NULL for any parsing error
 * --------------------------------------------------------------------------
 * Notes:     o Summary of syntax...
 *        \bezier(col1,row1)(colt,rowt)
 *        o col0=0,row0=0 assumed, i.e., given by
 *      \picture(){~(col0,row0){\bezier(col1,row1)(colt,rowt)}~}
 * ======================================================================= */
/* --- entry point --- */
subraster *rastbezier(mimetex_ctx *mctx, char **expression, int size, subraster *basesp,
                      int arg1, int arg2, int arg3)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* subraster for bezier */
    subraster *bezsp = NULL;
    /*\bezier(r,c)(r,c)(r,c)*/
    char    bezexpr[129], *xptr = bezexpr;
    /* convert ascii params to doubles */
    double  r0 = 0.0, c0 = 0.0, r1 = 0.0, c1 = 0.0, rt = 0.0, ct = 0.0, /* bezier points */
            rmid = 0.0, cmid = 0.0, /* coords at parameterized midpoint*/
            rmin = 0.0, cmin = 0.0, /* minimum r,c */
            rmax = 0.0, cmax = 0.0, /* maximum r,c */
            rdelta = 0.0, cdelta = 0.0, /* rmax-rmin, cmax-cmin */
            r = 0.0, c = 0.0; /* some point */
    /* 0=r0,c0 1=r1,c1 2=rt,ct */
    int iarg = 0;
    /* dimensions of bezier raster */
    int width = 0, height = 0;
    /* pixels are one bit each */
    int pixsz = 1;
    /*int   thickness = 1;*/        /* drawn lines are one pixel thick */
    /*c's,r's reset to lower-left origin*/
    int origin = 0;
    /* draw bezier in bezsp->image */
    /* ------------------------------------------------------------
    obtain (c1,r1)(ct,rt) args immediately following \bezier command
    ------------------------------------------------------------ */
    for (iarg = 1; iarg <= 2; iarg++) {  /* 0=c0,r0 1=c1,r1 2=ct,rt */
        /* --- parse for (r,c) args, and bump expression past them all --- */
        *expression = texsubexpr(mctx, *expression, bezexpr, 127, "(", ")", 0, 0);
        /* couldn't get (r,c)*/
        if (*bezexpr == '\000') goto end_of_job;
        /* --- now interpret (r,c) returned in bezexpr --- */
        /* init x-coord=col, y-coord=row */
        c = r = 0.0;
        if ((xptr = strchr(bezexpr, ',')) != NULL) { /* comma separates row,col */
            /* found it, so replace ',' by '\0'*/
            *xptr = '\000';
            r = mctx->unitlength * strtod(xptr + 1, NULL);
        } /* row=y-coord in pixels */
        /* col=x-coord in pixels */
        c = mctx->unitlength * strtod(bezexpr, NULL);
        /* --- store r,c --- */
        switch (iarg) {
        case 0:
            r0 = r;
            c0 = c;
            break;
        case 1:
            r1 = r;
            c1 = c;
            break;
        case 2:
            rt = r;
            ct = c;
            break;
        }
    } /* --- end-of-for(iarg) --- */
    /* --- determine midpoint and maximum,minimum points --- */
    /* y-coord at middle of bezier */
    rmid = 0.5 * (rt + 0.5 * (r0 + r1));
    /* x-coord at middle of bezier */
    cmid = 0.5 * (ct + 0.5 * (c0 + c1));
    /* lowest row */
    rmin = min3(r0, r1, rmid);
    /* leftmost col */
    cmin = min3(c0, c1, cmid);
    /* highest row */
    rmax = max3(r0, r1, rmid);
    /* rightmost col */
    cmax = max3(c0, c1, cmid);
    /* height */
    rdelta = rmax - rmin;
    /* width */
    cdelta = cmax - cmin;
    /* --- rescale coords so we start at 0,0 --- */
    r0 -= rmin;
    /* rescale r0,c0 */
    c0 -= cmin;
    r1 -= rmin;
    /* rescale r1,c1 */
    c1 -= cmin;
    rt -= rmin;
    /* rescale rt,ct */
    ct -= cmin;
    /* --- flip rows so 0,0 becomes lower-left corner instead of upper-left--- */
    /* map 0-->height-1, height-1-->0 */
    r0 = rdelta - r0 + 1;
    r1 = rdelta - r1 + 1;
    rt = rdelta - rt + 1;
    /* --- determine width,height of raster needed for bezier --- */
    /* round width up */
    width  = (int)(cdelta + 0.9999) + 1;
    /* round height up */
    height = (int)(rdelta + 0.9999) + 1;
    if (mctx->msgfp != NULL && mctx->msglevel >= 29) /* debugging */
        fprintf(mctx->msgfp, "rastbezier> width,height,origin=%d,%d,%d; c0,r0=%g,%g; "
                "c1,r1=%g,%g\n rmin,mid,max=%g,%g,%g; cmin,mid,max=%g,%g,%g\n",
                width, height, origin, c0, r0, c1, r1, rmin, rmid, rmax, cmin, cmid, cmax);
    /* ------------------------------------------------------------
    allocate raster
    ------------------------------------------------------------ */
    /* --- sanity check on width,height args --- */
    if (width < 1 ||  width > 600
            ||  height < 1 || height > 600) goto end_of_job;
    /* --- allocate and initialize subraster for constructed bezier --- */
    if ((bezsp = new_subraster(mctx, width, height, pixsz)) /* allocate new subraster */
            /* quit if failed */
            ==   NULL)  goto end_of_job;
    /* --- initialize bezier subraster parameters --- */
    /* image */
    bezsp->type = IMAGERASTER;
    /* not applicable for image */
    bezsp->symdef = NULL;
    /* is a little above center good? */
    bezsp->baseline = height / 2 + 2;
    /* size (probably unneeded) */
    bezsp->size = size;
    /* ------------------------------------------------------------
    draw the bezier
    ------------------------------------------------------------ */
    bezier_raster(mctx, bezsp->image,
    /* embedded raster image */
                  r0, c0,             /* row0,col0 are lower-left corner */
                  r1, c1,             /* row1,col1 are upper-right */
                  /* bezier tangent point */
                  rt, ct);
    /* ------------------------------------------------------------
    return constructed bezier to caller
    ------------------------------------------------------------ */
end_of_job:
    if (mctx->workingparam != NULL)          /* caller wants origin */
        /* return center origin to caller */
        *mctx->workingparam = origin;
    /* return bezier to caller */
    return (bezsp);
} /* --- end-of-function rastbezier() --- */


/* ==========================================================================
 * Function:    rastraise ( expression, size, basesp, arg1, arg2, arg3 )
 * Purpose: \raisebox{lift}{subexpression} handler, returns subraster
 *      containing subexpression with its baseline "lifted" by lift
 *      pixels, scaled by \mctx->unitlength, or "lowered" if lift arg
 *      negative
 * --------------------------------------------------------------------------
 * Arguments:   expression (I/O) char **  to first char of null-terminated
 *              string immediately following \raisebox to be
 *              rasterized, and returning ptr immediately
 *              following last character processed.
 *      size (I)    int containing 0-7 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding \raisebox
 *              (unused, but passed for consistency)
 *      arg1 (I)    int unused
 *      arg2 (I)    int unused
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to subraster corresponding to \raisebox
 *              requested, or NULL for any parsing error
 * --------------------------------------------------------------------------
 * Notes:     o Summary of syntax...
 *        \raisebox{lift}{subexpression}
 *        o
 * ======================================================================= */
/* --- entry point --- */
subraster *rastraise(mimetex_ctx *mctx, char **expression, int size, subraster *basesp,
                     int arg1, int arg2, int arg3)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* args */
    char subexpr[MAXSUBXSZ+1], *liftexpr = subexpr;
    /* rasterize subexpr to be raised */
    subraster *raisesp = NULL;
    int lift = 0;           /* amount to raise/lower baseline */
    /* ------------------------------------------------------------
    obtain {lift} argument immediately following \raisebox command
    ------------------------------------------------------------ */
    /* --- parse for {lift} arg, and bump expression past it --- */
    *expression = texsubexpr(mctx, *expression, liftexpr, 0, "{", "}", 0, 0);
    /* couldn't get {lift} */
    if (*liftexpr == '\000') goto end_of_job;
    /*{lift} to integer*/
    lift = (int)((mctx->unitlength * strtod(liftexpr, NULL)) + 0.0);
    /* sanity check */
    if (abs(lift) > 200) lift = 0;
    /* ------------------------------------------------------------
    obtain {subexpr} argument after {lift}, and rasterize it
    ------------------------------------------------------------ */
    /* --- parse for {subexpr} arg, and bump expression past it --- */
    *expression = texsubexpr(mctx, *expression, subexpr, 0, "{", "}", 0, 0);
    /* --- rasterize subexpression to be raised/lowered --- */
    if ((raisesp = rasterize(mctx, subexpr, size)) /* rasterize subexpression */
            /* and quit if failed */
            ==   NULL) goto end_of_job;
    /* ------------------------------------------------------------
    raise/lower baseline and return it to caller
    ------------------------------------------------------------ */
    /* --- raise/lower baseline --- */
    /* new baseline (no height checks) */
    raisesp->baseline += lift;
    /* --- return raised subexpr to caller --- */
end_of_job:
    /* return raised subexpr to caller */
    return (raisesp);
} /* --- end-of-function rastraise() --- */


/* ==========================================================================
 * Function:    rastrotate ( expression, size, basesp, arg1, arg2, arg3 )
 * Purpose: \rotatebox{degrees}{subexpression} handler, returns subraster
 *      containing subexpression rotated by degrees (counterclockwise
 *      if degrees positive)
 * --------------------------------------------------------------------------
 * Arguments:   expression (I/O) char **  to first char of null-terminated
 *              string immediately following \rotatebox to be
 *              rasterized, and returning ptr immediately
 *              following last character processed.
 *      size (I)    int containing 0-7 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding \rotatebox
 *              (unused, but passed for consistency)
 *      arg1 (I)    int unused
 *      arg2 (I)    int unused
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to subraster corresponding to \rotatebox
 *              requested, or NULL for any parsing error
 * --------------------------------------------------------------------------
 * Notes:     o Summary of syntax...
 *        \rotatebox{degrees}{subexpression}
 *        o
 * ======================================================================= */
/* --- entry point --- */
subraster *rastrotate(mimetex_ctx *mctx, char **expression, int size, subraster *basesp,
                      int arg1, int arg2, int arg3)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* args */
    char subexpr[MAXSUBXSZ+1], *degexpr = subexpr;
    /* subraster for rotated subexpr */
    subraster*rotsp = NULL;
    /* rotate subraster->image 90 degs */
    raster *rotrp = NULL;
    /* delete intermediate rasters */
    /* baseline of rasterized image */
    int baseline = 0;
    double degrees = 0.0, ipart, fpart; /* degrees to be rotated */
    /* positive ipart, isneg=1 if neg */
    int idegrees = 0, isneg = 0;
    /* degrees is n90 multiples of 90 */
    int n90 = 0, isn90 = 1;
    /* ------------------------------------------------------------
    obtain {degrees} argument immediately following \rotatebox command
    ------------------------------------------------------------ */
    /* --- parse for {degrees} arg, and bump expression past it --- */
    *expression = texsubexpr(mctx, *expression, degexpr, 0, "{", "}", 0, 0);
    /* couldn't get {degrees} */
    if (*degexpr == '\000') goto end_of_job;
    /* degrees to be rotated */
    degrees = strtod(degexpr, NULL);
    if (degrees < 0.0) {         /* clockwise rotation desired */
        /* flip sign so degrees positive */
        degrees = -degrees;
        isneg = 1;
    }            /* and set flag to indicate flip */
    /* integer and fractional parts */
    fpart = modf(degrees, &ipart);
    /* degrees mod 360 */
    ipart = (double)(((int)degrees) % 360);
    /* restore fractional part */
    degrees = ipart + fpart;
    if (isneg)               /* if clockwise rotation requested */
        /* do equivalent counterclockwise */
        degrees = 360.0 - degrees;
    /* integer degrees */
    idegrees = (int)(degrees + 0.5);
    /* degrees is n90 multiples of 90 */
    n90 = idegrees / 90;
    /*true if degrees is multiple of 90*/
    isn90 = (90 * n90 == idegrees);
    /* forced true for time being */
    isn90 = 1;
    /* ------------------------------------------------------------
    obtain {subexpr} argument after {degrees}, and rasterize it
    ------------------------------------------------------------ */
    /* --- parse for {subexpr} arg, and bump expression past it --- */
    *expression = texsubexpr(mctx, *expression, subexpr, 0, "{", "}", 0, 0);
    /* --- rasterize subexpression to be rotated --- */
    if ((rotsp = rasterize(mctx, subexpr, size))   /* rasterize subexpression */
            /* and quit if failed */
            ==   NULL) goto end_of_job;
    /* --- return unmodified image if no rotation requested --- */
    /* don't bother rotating image */
    if (abs(idegrees) < 2) goto end_of_job;
    /* --- extract params for image to be rotated --- */
    /* unrotated rasterized image */
    rotrp = rotsp->image;
    /* and baseline of that image */
    baseline = rotsp->baseline;
    /* ------------------------------------------------------------
    rotate by multiples of 90 degrees
    ------------------------------------------------------------ */
    if (isn90)               /* rotation by multiples of 90 */
        if (n90 > 0) {              /* do nothing for 0 degrees */
            /* rasrot() rotates clockwise */
            n90 = 4 - n90;
            while (n90 > 0) {          /* still have remaining rotations */
                /* rotate raster image */
                raster *nextrp = rastrot(mctx, rotrp);
                /* something's terribly wrong */
                if (nextrp == NULL) break;
                /* free previous raster image */
                delete_raster(mctx, rotrp);
                /* and replace it with rotated one */
                rotrp = nextrp;
                n90--;
            }              /* decrement remaining count */
        } /* --- end-of-if(isn90) --- */
    /* ------------------------------------------------------------
    requested rotation not multiple of 90 degrees
    ------------------------------------------------------------ */
    if (!isn90) {                /* explicitly construct rotation */
        /* not yet implemented */
        ;
    }
    /* ------------------------------------------------------------
    re-populate subraster envelope with rotated image
    ------------------------------------------------------------ */
    /* --- re-init various subraster parameters, embedding raster in it --- */
    if (rotrp != NULL) {         /* rotated raster constructed okay */
        /* signal constructed image */
        rotsp->type = IMAGERASTER;
        /* raster we just constructed */
        rotsp->image = rotrp;
        /* --- now try to guess pleasing baseline --- */
        if (idegrees > 2) {       /* leave unchanged if unrotated */
            if (strlen(subexpr) < 3      /* we rotated a short expression */
                    ||   abs(idegrees - 180) < 3)   /* or just turned it upside-down */
                /* so set with nothing descending */
                baseline = rotrp->height - 1;
            else
            /* rotated a long expression */
                baseline = (65 * (rotrp->height - 1)) / 100;
        } /* roughly center long expr */
        rotsp->baseline = baseline;
    }    /* set baseline as calculated above*/
    /* --- return rotated subexpr to caller --- */
end_of_job:
    /*return rotated subexpr to caller*/
    return (rotsp);
} /* --- end-of-function rastrotate() --- */


/* ==========================================================================
 * Function:    rastreflect ( expression, size, basesp, arg1, arg2, arg3 )
 * Purpose: \reflectbox[axis]{subexpression} handler, returns subraster
 *      containing subexpression reflected horizontally (i.e., around
 *      vertical axis, |_ becomes _|) if [axis] not given or axis=1,
 *      or reflected vertically if axis=2 given.
 * --------------------------------------------------------------------------
 * Arguments:   expression (I/O) char **  to first char of null-terminated
 *              string immediately following \reflectbox to
 *              be rasterized, and returning ptr immediately
 *              following last character processed.
 *      size (I)    int containing 0-7 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding \reflectbox
 *              (unused, but passed for consistency)
 *      arg1 (I)    int unused
 *      arg2 (I)    int unused
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to subraster corresponding to \reflectbox
 *              requested, or NULL for any parsing error
 * --------------------------------------------------------------------------
 * Notes:     o Summary of syntax...
 *        \reflectbox[axis]{subexpression}
 *        o
 * ======================================================================= */
/* --- entry point --- */
subraster *rastreflect(mimetex_ctx *mctx, char **expression, int size, subraster *basesp,
                       int arg1, int arg2, int arg3)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* args */
    char subexpr[MAXSUBXSZ+1], *axisexpr = subexpr;
    /* subraster for reflected subexpr */
    subraster *refsp = NULL;
    /* reflect subraster->image */
    raster *refrp = NULL;
    /* default horizontal reflection */
    int axis = 1;
    /* baseline of rasterized image */
    int baseline = 0;
    /* ------------------------------------------------------------
    obtain [axis] argument immediately following \reflectbox command, if given
    ------------------------------------------------------------ */
    /* --- check for optional [axis] arg  --- */
    if (*(*expression) == '[') {     /*check for []-enclosed optional arg*/
        *expression = texsubexpr(mctx, *expression, axisexpr, 255, "[", "]", 0, 0);
        /* convert [axis] to int */
        axis = atoi(axisexpr);
        if (axis < 1 || axis > 2)    /* check axis input */
            axis = 1;
    }           /* back to default if illegal */
    /* ------------------------------------------------------------
    obtain {subexpr} argument after optional [axis], and rasterize it
    ------------------------------------------------------------ */
    /* --- parse for {subexpr} arg, and bump expression past it --- */
    *expression = texsubexpr(mctx, *expression, subexpr, 0, "{", "}", 0, 0);
    /* --- rasterize subexpression to be reflected --- */
    if ((refsp = rasterize(mctx, subexpr, size))   /* rasterize subexpression */
            /* and quit if failed */
            ==   NULL) goto end_of_job;
    /* --- return unmodified image if no reflection requested --- */
    /* don't bother reflecting image */
    if (axis < 1 || axis > 2) goto end_of_job;
    /* --- extract params for image to be reflected --- */
    /* unreflected rasterized image */
    refrp = refsp->image;
    /* and baseline of that image */
    baseline = refsp->baseline;
    /* ------------------------------------------------------------
    reflect image and adjust its parameters
    ------------------------------------------------------------ */
    /* --- reflect image --- */
    /* reflect raster image */
    refrp = rastref(mctx, refsp->image, axis);
    /* failed to reflect image */
    if (refrp == NULL) goto end_of_job;
    /* free original raster image */
    delete_raster(mctx, refsp->image);
    /*and replace it with reflected one*/
    refsp->image = refrp;
    /* --- adjust parameters --- */
    if (axis == 2)           /* for vertical reflection */
        /* set with nothing descending */
        baseline = refrp->height - 1;
    /* reset baseline of reflected image*/
    refsp->baseline = baseline;
    /* --- return reflected subexpr to caller --- */
end_of_job:
    /*back to caller with reflected expr*/
    return (refsp);
} /* --- end-of-function rastreflect() --- */


/* ==========================================================================
 * Function:    rastfbox ( expression, size, basesp, arg1, arg2, arg3 )
 * Purpose: \fbox{subexpression} handler, returns subraster
 *      containing subexpression with frame box drawn around it
 * --------------------------------------------------------------------------
 * Arguments:   expression (I/O) char **  to first char of null-terminated
 *              string immediately following \fbox to be
 *              rasterized, and returning ptr immediately
 *              following last character processed.
 *      size (I)    int containing 0-7 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding \fbox
 *              (unused, but passed for consistency)
 *      arg1 (I)    int unused
 *      arg2 (I)    int unused
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to subraster corresponding to \fbox
 *              requested, or NULL for any parsing error
 * --------------------------------------------------------------------------
 * Notes:     o Summary of syntax...
 *        \fbox[width][height]{subexpression}
 *        o
 * ======================================================================= */
/* --- entry point --- */
subraster *rastfbox(mimetex_ctx *mctx, char **expression, int size, subraster *basesp,
                    int arg1, int arg2, int arg3)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* args */
    char subexpr[MAXSUBXSZ+1], widtharg[512];
    /* rasterize subexpr to be framed */
    subraster *framesp = NULL;
    /* framed image raster */
    raster *bp = NULL;
    /* interpret [width][height] */
    /*extra frame width, line thickness*/
    int fwidth = 6, fthick = 1;
    int width = (-1), height = (-1),    /* optional [width][height] args */
        iscompose = 0; /* set true if optional args given */
    /* ------------------------------------------------------------
    obtain optional [width][height] arguments immediately following \fbox
    ------------------------------------------------------------ */
    /* --- first check for optional \fbox[width] --- */
    if (*(*expression) == '[') {     /* check for []-enclosed width arg */
        *expression = texsubexpr(mctx, *expression, widtharg, 511, "[", "]", 0, 0);
        if (*widtharg != '\000') {       /* got widtharg */
            width = max2(1, iround(mctx->unitlength * strtod(widtharg, NULL)));
            height = 1;
            fwidth = 2;
            iscompose = 1;
        }
    } /* --- end-of-if(**expression=='[') --- */
    if (width > 0)           /* found leading [width], so... */
        if (*(*expression) == '[') {        /* check for []-enclosed height arg */
            *expression = texsubexpr(mctx, *expression, widtharg, 511, "[", "]", 0, 0);
            if (*widtharg != '\000') {       /* got widtharg */
                height = max2(1, iround(mctx->unitlength * strtod(widtharg, NULL)));
                fwidth = 0;
            }            /* no extra border */
        } /* --- end-of-if(**expression=='[') --- */
    /* ------------------------------------------------------------
    obtain {subexpr} argument
    ------------------------------------------------------------ */
    /* --- parse for {subexpr} arg, and bump expression past it --- */
    *expression = texsubexpr(mctx, *expression, subexpr, 0, "{", "}", 0, 0);
    /* --- rasterize subexpression to be framed --- */
    if (width < 0 || height < 0) {   /* no explicit dimensions given */
        if ((framesp = rasterize(mctx, subexpr, size)) /* rasterize subexpression */
                ==   NULL) goto end_of_job;
    }  /* and quit if failed */
    else {
        /* compose subexpr with empty box */
        char composexpr[8192];
        sprintf(composexpr, "\\compose{\\hspace{%d}\\vspace{%d}}{%.8000s}",
                width, height, subexpr);
        if ((framesp = rasterize(mctx, composexpr, size)) /* rasterize subexpression */
                ==   NULL) goto end_of_job;
    }  /* and quit if failed */
    /* ------------------------------------------------------------
    draw frame, reset params, and return it to caller
    ------------------------------------------------------------ */
    /* --- draw border --- */
    if ((bp = border_raster(mctx, framesp->image, -fwidth, -fwidth, fthick, 1))
            /* draw border and quit if failed */
            ==   NULL) goto end_of_job;
    /* --- replace original image and raise baseline to accommodate frame --- */
    /* replace image with framed one */
    framesp->image = bp;
    if (!iscompose)              /* simple border around subexpr */
        /* so just raise baseline */
        framesp->baseline += fwidth;
    else
        /* set at bottom */
        framesp->baseline = (framesp->image)->height - 1;
    /* --- return framed subexpr to caller --- */
end_of_job:
    /* return framed subexpr to caller */
    return (framesp);
} /* --- end-of-function rastfbox() --- */


/* ==========================================================================
 * Function:    rasttoday ( expression, size, basesp, arg1, arg2, arg3 )
 * Purpose: handle \today
 * --------------------------------------------------------------------------
 * Arguments:   expression (I/O) char **  to first char of null-terminated
 *              string immediately following \today,
 *              and returning ptr immediately
 *              following last character processed.
 *      size (I)    int containing 0-7 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding \today
 *              (unused, but passed for consistency)
 *      arg1 (I)    int unused
 *      arg2 (I)    int unused
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) subraster ptr to date stamp
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
subraster *rasttoday(mimetex_ctx *mctx, char **expression, int size, subraster *basesp,
                     int arg1, int arg2, int arg3)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    char optarg[2050];    /* optional [+/-tzdelta,ifmt] args */
    /* timestamp to be rasterized */
    char *today = optarg;
    /* rasterize timestamp */
    subraster *todaysp = NULL;
    /* default timestamp() args */
    int ifmt = 1, tzdelta = 0;
    /* ------------------------------------------------------------
    Get optional args \today[+/-tzdelta,ifmt]
    ------------------------------------------------------------ */
    /* --- check for optional \today[+/-tzdelta,ifmt] --- */
    if (*(*expression) == '[') {     /* check for []-enclosed value */
        *expression = texsubexpr(mctx, *expression, optarg, 2047, "[", "]", 0, 0);
        if (*optarg != '\000') {     /* got optional arg */
            char *comma = strchr(optarg, ','); /* comma between +/-tzdelta,ifmt */
            /* #optional args between []'s */
            int iarg, nargs = (comma == NULL ? 1 : 2);
            /* null-terminate first arg */
            if (comma != NULL) *comma = '\000';
            for (iarg = 1; iarg <= nargs; iarg++) { /* process one or both args */
                /* choose 1st or 2nd arg */
                char *arg = (iarg == 1 ? optarg : comma + 1);
                if (isthischar(*arg, "+-"))    /* leading +/- signals tzdelta */
                    /* so interpret arg as tzdelta */
                    tzdelta = atoi(arg);
                else ifmt = atoi(arg);
            }  /* else interpret args as ifmt */
        } /* --- end-of-if(*optarg!='\0') --- */
    } /* --- end-of-if(**expression=='[') --- */
    /* ------------------------------------------------------------
    Get timestamp and rasterize it
    ------------------------------------------------------------ */
    /* rasterize timestamp as text */
    strcpy(today, "\\text{");
    /* get timestamp */
    strcat(today, timestamp(tzdelta, ifmt));
    /* terminate \text{} braces */
    strcat(today, "}");
    /* rasterize timestamp */
    todaysp = rasterize(mctx, today, size);
    /* --- return timestamp raster to caller --- */
    /*end_of_job:*/
    /* return timestamp to caller */
    return (todaysp);
} /* --- end-of-function rasttoday() --- */


/* ==========================================================================
 * Function:    rastcalendar ( expression, size, basesp, arg1, arg2, arg3 )
 * Purpose: handle \calendar
 * --------------------------------------------------------------------------
 * Arguments:   expression (I/O) char **  to first char of null-terminated
 *              string immediately following \calendar
 *              and returning ptr immediately
 *              following last character processed.
 *      size (I)    int containing 0-7 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding \calendar
 *              (unused, but passed for consistency)
 *      arg1 (I)    int unused
 *      arg2 (I)    int unused
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) subraster ptr to rendered one-month calendar
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
subraster *rastcalendar(mimetex_ctx *mctx, char **expression, int size, subraster *basesp,
                        int arg1, int arg2, int arg3)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* optional [year,month] args */
    char optarg[2050];
    /* calendar to be rasterized */
    char *calstr = NULL;
    /* rasterize calendar string */
    subraster *calendarsp = NULL;
    /* default calendar() args */
    int year = 0, month = 0, day = 0, argval = 0;
    /* ------------------------------------------------------------
    Get optional args \today[+/-tzdelta,ifmt]
    ------------------------------------------------------------ */
    /* --- check for optional \calendar[year,month] --- */
    if (*(*expression) == '[') {     /* check for []-enclosed value */
        *expression = texsubexpr(mctx, *expression, optarg, 2047, "[", "]", 0, 0);
        if (*optarg != '\000') {     /* got optional arg */
            char *comma = strchr(optarg, ','), /* comma between year,month */
                          /* second comma before day */
                          *comma2 = NULL;
            /* #optional args between []'s */
            int iarg, nargs = (comma == NULL ? 1 : 2);
            if (comma != NULL) {
                /*null-terminate first arg*/
                *comma = '\000';
                if ((comma2 = strchr(comma + 1, ',')) != NULL) { /* have third arg */
                    /* null-term 2nd arg, bump count */
                    *comma2 = '\000';
                    nargs++;
                }
            }
            for (iarg = 1; iarg <= nargs; iarg++) { /* process one or both args */
                /*get arg*/
                char *arg = (iarg == 1 ? optarg : (iarg == 2 ? comma + 1 : comma2 + 1));
                /* interpret arg as integer */
                argval = atoi(arg);
                if (iarg < 3) {        /* first two args are month,year */
                    /* year value */
                    if (argval > 1972 && argval < 2100) year = argval;
                    else if (argval >= 1 && argval <= 12) month = argval;
                } /*or month*/
                else
                /* only 3rd arg can be day */
                    if (argval >= 1 && argval <= 31) day = argval;
            } /* day value */
        } /* --- end-of-if(*optarg!='\0') --- */
    } /* --- end-of-if(**expression=='[') --- */
    /* ------------------------------------------------------------
    Get calendar string and rasterize it
    ------------------------------------------------------------ */
    if (mctx->msgfp != NULL && mctx->msglevel >= 9)
        fprintf(mctx->msgfp, "rastcalendar> year=%d, month=%d, day=%d\n",
                year, month, day);
    /* get calendar string */
    calstr = calendar(year, month, day);
    /* rasterize calendar string */
    calendarsp = rasterize(mctx, calstr, size);
    /* --- return calendar raster to caller --- */
    /*end_of_job:*/
    /* return calendar to caller */
    return (calendarsp);
} /* --- end-of-function rastcalendar() --- */



/* ==========================================================================
 * Function:    rastnoop ( expression, size, basesp, nargs, arg2, arg3 )
 * Purpose: no op -- flush \escape without error
 * --------------------------------------------------------------------------
 * Arguments:   expression (I/O) char **  to first char of null-terminated
 *              string immediately following \escape to be
 *              flushed, and returning ptr immediately
 *              following last character processed.
 *      size (I)    int containing 0-5 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding \escape
 *              (unused, but passed for consistency)
 *      nargs (I)   int containing number of {}-args after
 *              \escape to be flushed along with it
 *      arg2 (I)    int unused
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) NULL subraster ptr
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
subraster *rastnoop(mimetex_ctx *mctx, char **expression, int size, subraster *basesp,
                    int nargs, int arg2, int arg3)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /*dummy args eaten by \escape*/
    char subexpr[MAXSUBXSZ+1];
    /* rasterize subexpr */
    subraster *noopsp = NULL;
    /* --- flush accompanying args if necessary --- */
    if (nargs != NOVALUE             /* not unspecified */
            &&   nargs > 0)             /* and args to be flushed */
        while (--nargs >= 0)       /* count down */
            /*flush arg*/
            *expression = texsubexpr(mctx, *expression, subexpr, 0, "{", "}", 0, 0);
    /* --- return null ptr to caller --- */
    /*end_of_job:*/
    /* return NULL ptr to caller */
    return (noopsp);
} /* --- end-of-function rastnoop() --- */



