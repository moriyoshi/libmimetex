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

#include <string.h>
#include "mimetex.h"

/* ==========================================================================
 * Function:    type_pbmpgm ( rp, ptype, file )
 * Purpose: Write pbm or pgm image of rp to file
 * --------------------------------------------------------------------------
 * Arguments:   rp (I)      ptr to raster struct for which
 *              a pbm/pgm file is to be written.
 *      ptype (I)   int containing 1 for pbm, 2 for pgm, or
 *              0 to determine ptype from values in rp
 *      file (I)    ptr to null-terminated char string
 *              containing name of fuke to be written
 *              (see notes below).
 * --------------------------------------------------------------------------
 * Returns: ( int )     total #bytes written,
 *              or 0 for any error.
 * --------------------------------------------------------------------------
 * Notes:     o (a) If file==NULL, output is written to stdout;
 *      (b) if *file=='\000' then file is taken as the
 *          address of an output buffer to which output
 *          is written (and is followed by a terminating
 *          '\0' which is not counted in #bytes returned);
 *      (c) otherwise file is the filename (opened and
 *          closed internally) to which output is written,
 *          except that any final .ext extension is replaced
 *          by .pbm or .pgm depending on ptype.
 * ======================================================================= */
/* --- entry point --- */
int type_pbmpgm(raster *rp, int ptype, char *file)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* completion flag, total #bytes written */
    int isokay = 0, nbytes = 0;
    /*height(row), width(col) indexes in raster*/
    int irow = 0, jcol = 0;
    int pixmin = 9999, pixmax = (-9999), /* min, max pixel value in raster */
        ngray = 0; /* #gray scale values */
    FILE
    /* *fopen(), */
*fp = NULL; /* pointer to output file (or NULL) */
    char    outline[1024], outfield[256], /* output line, field */
    /* cr at end-of-line */
    cr[16] = "\n\000";
    /* maximum allowed line length */
    int maxlinelen = 70;
    int pixfrac = 6;    /* use (pixmax-pixmin)/pixfrac as step */
    static char *suffix[] = { NULL, ".pbm", ".pgm" };   /* file.suffix[ptype] */
    static char *magic[] = { NULL, "P1", "P2" };    /*identifying "magic number"*/
    static char *mode[] = { NULL, "w", "w" }; /* fopen() mode[ptype] */
    /* ------------------------------------------------------------
    check input, determine grayscale,  and set up output file if necessary
    ------------------------------------------------------------ */
    /* --- check input args --- */
    /* no input raster provided */
    if (rp == NULL) goto end_of_job;
    if (ptype != 0)              /* we'll determine ptype below */
        /*invalid output graphic format*/
        if (ptype < 1 || ptype > 2) goto end_of_job;
    /* --- determine largest (and smallest) value in pixmap --- */
    for (irow = 0; irow < rp->height; irow++)  /* for each row, top-to-bottom */
        for (jcol = 0; jcol < rp->width; jcol++) { /* for each col, left-to-right */
            /* value of pixel at irow,jcol */
            int pixval = getpixel(rp, irow, jcol);
            /* new minimum */
            pixmin = min2(pixmin, pixval);
            pixmax = max2(pixmax, pixval);
        } /* new maximum */
    ngray = 1 + (pixmax - pixmin);      /* should be 2 for b/w bitmap */
    if (ptype == 0)              /* caller wants us to set ptype */
        /* use grayscale if >2 shades */
        ptype = (ngray >= 3 ? 2 : 1);
    /* --- open output file if necessary --- */
    /*null ptr signals output to stdout*/
    if (file == NULL) fp = stdout;
    else if (*file != '\000') {          /* explicit filename provided, so...*/
        /* file.ext, ptr to last . in fname*/
        char  fname[512], *pdot = NULL;
        /* local copy of file name */
        strncpy(fname, file, 255);
        /* make sure it's null terminated */
        fname[255] = '\000';
        if ((pdot = strrchr(fname, '.')) == NULL) /*no extension on original name*/
            /* so add extension */
            strcat(fname, suffix[ptype]);
        else
        /* we already have an extension */
            /* so replace original extension */
            strcpy(pdot, suffix[ptype]);
        if ((fp = fopen(fname, mode[ptype]))   /* open output file */
                /* quit if failed to open */
                == (FILE *)NULL) goto end_of_job;
    } /* --- ens-of-if(*file!='\0') --- */
    /* ------------------------------------------------------------
    format and write header
    ------------------------------------------------------------ */
    /* --- format header info --- */
    /* initialize line buffer */
    *outline = '\000';
    /* begin file with "magic number" */
    strcat(outline, magic[ptype]);
    /* followed by cr to end line */
    strcat(outline, cr);
    /* format width and height */
    sprintf(outfield, "%d %d", rp->width, rp->height);
    /* add width and height to header */
    strcat(outline, outfield);
    /* followed by cr to end line */
    strcat(outline, cr);
    if (ptype == 2) {            /* need max grayscale value */
        /* format maximum pixel value */
        sprintf(outfield, "%d", pixmax);
        /* add max value to header */
        strcat(outline, outfield);
        strcat(outline, cr);
    }       /* followed by cr to end line */
    /* --- write header to file or memory buffer --- */
    if (fp == NULL)              /* if we have no open file... */
        /* add header to caller's buffer */
        strcat(file, outline);
    else
    /* or if we have an open file... */
        if (fputs(outline, fp)     /* try writing header to open file */
                /* return with error if failed */
                ==   EOF) goto end_of_job;
    /* bump output byte count */
    nbytes += strlen(outline);
    /* ------------------------------------------------------------
    format and write pixels
    ------------------------------------------------------------ */
    /* initialize line buffer */
    *outline = '\000';
    for (irow = 0; irow <= rp->height; irow++) /* for each row, top-to-bottom */
        for (jcol = 0; jcol < rp->width; jcol++)  { /* for each col, left-to-right */
            /* --- format value at irow,jcol--- */
            /* init empty field */
            *outfield = '\000';
            if (irow < rp->height) {       /* check row index */
                /* value of pixel at irow,jcol */
                int pixval = getpixel(rp, irow, jcol);
                if (ptype == 1)              /* pixval must be 1 or 0 */
                    pixval = (pixval > pixmin + ((pixmax - pixmin) / pixfrac) ? 1 : 0);
                sprintf(outfield, "%d ", pixval);
            }   /* format pixel value */
            /* --- write line if this value won't fit on it (or last line) --- */
            if (strlen(outline) + strlen(outfield) + strlen(cr) >= maxlinelen /*won't fit*/
                    ||   irow >= rp->height) {        /* force writing last line */
                /* add cr to end current line */
                strcat(outline, cr);
                if (fp == NULL)              /* if we have no open file... */
                    /* add header to caller's buffer */
                    strcat(file, outline);
                else
                /* or if we have an open file... */
                    if (fputs(outline, fp)     /* try writing header to open file */
                            /* return with error if failed */
                            ==   EOF) goto end_of_job;
                /* bump output byte count */
                nbytes += strlen(outline);
                /* re-initialize line buffer */
                *outline = '\000';
            } /* --- end-of-if(strlen>=maxlinelen) --- */
            /* done after writing last line */
            if (irow >= rp->height) break;
            /* --- concatanate value to line -- */
            /* concatanate value to line */
            strcat(outline, outfield);
        } /* --- end-of-for(jcol,irow) --- */
    /* signal successful completion */
    isokay = 1;
    /* ------------------------------------------------------------
    Back to caller with total #bytes written, or 0=failed.
    ------------------------------------------------------------ */
end_of_job:
    if (fp != NULL             /* output written to an open file */
            &&   fp != stdout)            /* and it's not just stdout */
        /* so close file before returning */
        fclose(fp);
    /*back to caller with #bytes written*/
    return ((isokay ? nbytes : 0));
} /* --- end-of-function type_pbmpgm() --- */



