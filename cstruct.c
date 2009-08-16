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

#include "mimetex_priv.h"

/* ==========================================================================
 * Function:    cstruct_chardef ( cp, fp, col1 )
 * Purpose: Emit a C struct of cp on fp, starting in col1.
 * --------------------------------------------------------------------------
 * Arguments:   cp (I)      ptr to chardef struct for which
 *              a C struct is to be generated.
 *      fp (I)      File ptr to output device (defaults to
 *              stdout if passed as NULL).
 *      col1 (I)    int containing 0...65; output lines
 *              are preceded by col1 blanks.
 * --------------------------------------------------------------------------
 * Returns: ( int )     1 if completed successfully,
 *              or 0 otherwise (for any error).
 * --------------------------------------------------------------------------
 * Notes:
 * ======================================================================= */
/* --- entry point --- */
int cstruct_chardef(chardef *cp, FILE *fp, int col1)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* field within output line */
    char    field[64];
    int cstruct_raster(),   /* emit a raster */
    /* emit a string and comment */
    emit_string();
    /* ------------------------------------------------------------
    emit   charnum, location, name  /  hirow, hicol,  lorow, locol
    ------------------------------------------------------------ */
    /* --- charnum, location, name --- */
    /*char#,location*/
    sprintf(field, "{ %3d,%5d,\n", cp->charnum, cp->location);
    emit_string(fp, col1, field, "character number, location");
    /* --- toprow, topleftcol,   botrow, botleftcol  --- */
    sprintf(field, "  %3d,%2d,  %3d,%2d,\n",    /* format... */
            cp->toprow, cp->topleftcol,           /* toprow, topleftcol, */
            /* and botrow, botleftcol */
            cp->botrow, cp->botleftcol);
    emit_string(fp, col1, field, "topleft row,col, and botleft row,col");
    /* ------------------------------------------------------------
    emit raster and chardef's closing brace, and then return to caller
    ------------------------------------------------------------ */
    /* emit raster */
    cstruct_raster(&cp->image, fp, col1 + 4);
    /* emit closing brace */
    emit_string(fp, 0, "  }", NULL);
    /* back to caller with 1=okay, 0=failed */
    return (1);
} /* --- end-of-function cstruct_chardef() --- */


/* ==========================================================================
 * Function:    cstruct_raster ( rp, fp, col1 )
 * Purpose: Emit a C struct of rp on fp, starting in col1.
 * --------------------------------------------------------------------------
 * Arguments:   rp (I)      ptr to raster struct for which
 *              a C struct is to be generated.
 *      fp (I)      File ptr to output device (defaults to
 *              stdout if passed as NULL).
 *      col1 (I)    int containing 0...65; output lines
 *              are preceded by col1 blanks.
 * --------------------------------------------------------------------------
 * Returns: ( int )     1 if completed successfully,
 *              or 0 otherwise (for any error).
 * --------------------------------------------------------------------------
 * Notes:
 * ======================================================================= */
/* --- entry point --- */
int cstruct_raster(raster *rp, FILE *fp, int col1)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* field within output line */
    char    field[64];
    /* type cast for pixmap string */
    char    typecast[64] = "(pixbyte *)";
    /* to emit raster bitmap */
    int hex_bitmap();
    /* emit a string and comment */
    int emit_string();
    /* ------------------------------------------------------------
    emit width and height
    ------------------------------------------------------------ */
    sprintf(field, "{ %2d,  %3d,%2d,%2d, %s\n", /* format width,height,pixsz */
            rp->width, rp->height, rp->format, rp->pixsz, typecast);
    emit_string(fp, col1, field, "width,ht, fmt,pixsz,map...");
    /* ------------------------------------------------------------
    emit bitmap and closing brace, and return to caller
    ------------------------------------------------------------ */
    /* emit bitmap */
    hex_bitmap(rp, fp, col1 + 2, 1);
    /* emit closing brace */
    emit_string(fp, 0, " }", NULL);
    /* back to caller with 1=okay, 0=failed */
    return (1);
} /* --- end-of-function cstruct_raster() --- */
