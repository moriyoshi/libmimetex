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

#include <stdio.h>
#include "mimetex_priv.h"

/* ==========================================================================
 * Function:    xbitmap_raster ( rp, fp )
 * Purpose: Emit a mime xbitmap representing rp, on fp.
 * --------------------------------------------------------------------------
 * Arguments:   rp (I)      ptr to raster struct for which a mime
 *              xbitmap is to be constructed.
 *      fp (I)      File ptr to output device (defaults to
 *              stdout if passed as NULL).
 * --------------------------------------------------------------------------
 * Returns: ( int )     1 if completed successfully,
 *              or 0 otherwise (for any error).
 * --------------------------------------------------------------------------
 * Notes:
 * ======================================================================= */
/* --- entry point --- */
int xbitmap_raster(raster *rp, FILE *fp)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* dummy title */
    char    *title = "image";
    /* dump bitmap as hex bytes */
    int hex_bitmap();
    /* ------------------------------------------------------------
    emit text to display mime xbitmap representation of rp->bitmap image
    ------------------------------------------------------------ */
    /* --- first redirect null fp --- */
    /* default fp to stdout if null */
    if (fp == (FILE *)NULL) fp = stdout;
    /* --- check for ascii string --- */
    if (isstring)                /* pixmap has string, not raster */
        /* can't handle ascii string */
        return (0);
    /* --- emit prologue strings and hex dump of bitmap for mime xbitmap --- */
    fprintf(fp, "Content-type: image/x-xbitmap\n\n");
    fprintf(fp, "#define %s_width %d\n#define %s_height %d\n",
            title, rp->width, title, rp->height);
    fprintf(fp, "static char %s_bits[] = {\n", title);
    /* emit hex dump of bitmap bytes */
    hex_bitmap(rp, fp, 0, 0);
    /* ending with "};" for C array */
    fprintf(fp, "};\n");
    /* ------------------------------------------------------------
    Back to caller with 1=okay, 0=failed.
    ------------------------------------------------------------ */
    return (1);
} /* --- end-of-function xbitmap_raster() --- */

