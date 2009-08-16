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

#include <string.h>
#include "mimetex_priv.h"

/* ==========================================================================
 * Function:    type_bytemap ( bp, grayscale, width, height, fp )
 * Purpose: Emit an ascii dump representing bp, on fp.
 * --------------------------------------------------------------------------
 * Arguments:   bp (I)      intbyte * to bytemap for which an
 *              ascii dump is to be constructed.
 *      grayscale (I)   int containing #gray shades, 256 for 8-bit
 *      width (I)   int containing #cols in bytemap
 *      height (I)  int containing #rows in bytemap
 *      fp (I)      File ptr to output device (defaults to
 *              stdout if passed as NULL).
 * --------------------------------------------------------------------------
 * Returns: ( int )     1 if completed successfully,
 *              or 0 otherwise (for any error).
 * --------------------------------------------------------------------------
 * Notes:
 * ======================================================================= */
/* --- entry point --- */
int type_bytemap(intbyte *bp, int grayscale,
                 int width, int height, FILE *fp)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* max columns for display */
    static  int display_width = 72;
    int byte_width = 3,         /* cols to display byte (ff+space) */
        maxbyte = 0; /* if maxbyte<16, set byte_width=2 */
    int white_byte = 0,         /* show dots for white_byte's */
        black_byte = grayscale - 1; /* show stars for black_byte's */
    char scanline[133]; /* ascii image for one scan line */
    int scan_width, /* #chars in scan (<=display_width)*/
        scan_cols; /* #cols in scan (hicol-locol+1) */
    int ibyte,              /* bp[] index */
        irow, locol, hicol = (-1); /* height index, width indexes */
    /* ------------------------------------------------------------
    initialization
    ------------------------------------------------------------ */
    /* --- redirect null fp --- */
    /* default fp to stdout if null */
    if (fp == (FILE *)NULL) fp = stdout;
    /* --- check for ascii string --- */
    if (isstring) {              /* bp has ascii string, not raster */
        /* #chars in ascii string */
        width = strlen((char *)bp);
        height = 1;
    }            /* default */
    /* --- see if we can get away with byte_width=1 --- */
    for (ibyte = 0; ibyte < width*height; ibyte++) { /* check all bytes */
        /* current byte value */
        int byteval = (int)bp[ibyte];
        if (byteval < black_byte)        /* if it's less than black_byte */
            maxbyte = max2(maxbyte, byteval);
    } /* then find max non-black value */
    if (maxbyte < 16)            /* bytevals will fit in one column */
        /* so reset display byte_width */
        byte_width = 1;
    /* ------------------------------------------------------------
    display ascii dump of bitmap image (in segments if display_width < rp->width)
    ------------------------------------------------------------ */
    while ((locol = hicol + 1) < width) { /*start where prev segment left off*/
        /* --- set hicol for this pass (locol set above) --- */
        /* show as much as display allows */
        hicol += display_width / byte_width;
        /* but not more than bytemap */
        if (hicol >= width) hicol = width - 1;
        /* #cols in this scan */
        scan_cols = hicol - locol + 1;
        /* #chars in this scan */
        scan_width = byte_width * scan_cols;
        /* separator */
        if (locol > 0 && !isstring) fprintf(fp, "----------\n");
        /* ------------------------------------------------------------
        display all scan lines for this local...hicol segment range
        ------------------------------------------------------------ */
        for (irow = 0; irow < height; irow++) { /* all scan lines for col range */
            /* --- allocations and declarations --- */
            /* first bp[] byte in this scan */
            int  lobyte = irow * width + locol;
            /* sprintf() buffer for byte */
            char scanbyte[32];
            /* --- set chars in scanline[] based on bytes in bytemap bp[] --- */
            /* blank out scanline */
            memset(scanline, ' ', scan_width);
            for (ibyte = 0; ibyte < scan_cols; ibyte++) { /* set chars for each col */
                /* value of current byte */
                int byteval = (int)bp[lobyte+ibyte];
                /* dot-fill scanbyte */
                memset(scanbyte, '.', byte_width);
                if (byteval == black_byte)   /* but if we have a black byte */
                    /* star-fill scanbyte instead */
                    memset(scanbyte, '*', byte_width);
                if (byte_width > 1)          /* don't blank out single char */
                    /* blank-fill rightmost character */
                    scanbyte[byte_width-1] = ' ';
                if (byteval != white_byte    /* format bytes that are non-white */
                        &&   byteval != black_byte)     /* and that are non-black */
                    /*hex-format*/
                    sprintf(scanbyte, "%*x ", max2(1, byte_width - 1), byteval);
                memcpy(scanline + ibyte*byte_width, scanbyte, byte_width);
            } /*in line*/
            /* --- display completed scan line --- */
            fprintf(fp, "%.*s\n", scan_width, scanline);
        } /* --- end-of-for(irow) --- */
    } /* --- end-of-while(hicol<width) --- */
    /* ------------------------------------------------------------
    Back to caller with 1=okay, 0=failed.
    ------------------------------------------------------------ */
    return (1);
} /* --- end-of-function type_bytemap() --- */



