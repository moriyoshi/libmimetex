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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "mimetex.h"
#include "gifsave.h"
#include "md5.h"

/* --- check whether or not to perform http_referer check --- */
#ifdef REFERER              /* only specified referers allowed */
#undef NOREFMAXLEN
#define NOREFMAXLEN NOREFSAFELEN
#else
/* all http_referer's allowed */
#define REFERER NULL
#endif

#ifdef INPUTREFERER             /*http_referer's permitted to \input*/
#ifndef INPUTSECURITY         /* so we need to permit \input{} */
#define INPUTSECURITY (99999)   /* make sure SECURITY<INPUTSECURITY */
#endif
#else
/* no INPUTREFERER list supplied */
#define INPUTREFERER NULL     /* so init it as NULL pointer */
#endif

/* --- check top levels of http_referer against server_name --- */
#ifdef REFLEVELS            /* #topmost levels to check */
#undef NOREFMAXLEN
#define NOREFMAXLEN NOREFSAFELEN
#else
#ifdef NOREFCHECK
#define REFLEVELS 0         /* don't match host and referer */
#else
#define REFLEVELS 3         /* default matches abc.def.com */
#endif
#endif

/* --- max query_string length if no http_referer supplied --- */
#ifndef NOREFMAXLEN
#define NOREFMAXLEN 9999      /* default to any length query */
#endif
#ifndef NOREFSAFELEN
#define NOREFSAFELEN 24       /* too small for hack exploit */
#endif

#ifndef LOGFILE
#define LOGFILE "mimetex.log"     /* default log file */
#endif
#ifndef CACHELOG
#define CACHELOG "mimetex.log"    /* default caching log file */
#endif

#ifdef SS
#define ISSUPERSAMPLING 1
#ifndef AAALGORITHM
#define AAALGORITHM 1       /* default supersampling algorithm */
#endif
#ifndef AA                /* anti-aliasing not explicitly set */
#define AA              /* so define it ourselves */
#endif
#ifndef SSFONTS           /* need supersampling fonts */
#define SSFONTS
#endif
#else
#define ISSUPERSAMPLING 0
#ifndef AAALGORITHM
#define AAALGORITHM 3 /*2*/     /* default lowpass algorithm */
#endif
#endif
/* --- image caching (cache images if given -DCACHEPATH=\"path\") --- */
#ifndef CACHEPATH
#define ISCACHING 0           /* no caching */
#define CACHEPATH "\000"      /* same directory as mimetex.cgi */
#else
#define ISCACHING 1           /* caching if -DCACHEPATH="path" */
#endif
/* --- \input paths (prepend prefix if given -DPATHPREFIX=\"prefix\") --- */
#define PATHPREFIX "\000"     /* paths relative mimetex.cgi */
/* --- treat +'s in query string as blanks? --- */
#ifdef PLUSBLANK            /* + always interpreted as blank */
#define ISPLUSBLANK 1
#else
#ifdef PLUSNOTBLANK           /* + never interpreted as blank */
#define ISPLUSBLANK 0
#else
/* program tries to determine */
#define ISPLUSBLANK (-1)
#endif
#endif
/* --- skip argv[]'s preceding ARGSIGNAL when parsing command-line args --- */
#ifdef NOARGSIGNAL
#define ARGSIGNAL NULL
#endif
#ifndef ARGSIGNAL
#define ARGSIGNAL "++"
#endif
/* --- security and logging (inhibit message logging, etc) --- */
#ifndef SECURITY
#define SECURITY 999          /* default highest security level */
#endif
#if !defined(NODUMPENVP) && !defined(DUMPENVP)
#define DUMPENVP          /* assume char *envp[] available */
#endif
/* --- check whether or not \input, \counter, \environment permitted --- */
#ifdef DEFAULTSECURITY          /* default security specified */
#define EXPLICITDEFSECURITY       /* don't override explicit default */
#else
/* defualt security not specified */
#define DEFAULTSECURITY (8)       /* so set default security level */
#endif
#ifndef INPUTPATH           /* \input{} paths permitted for... */
#define INPUTPATH NULL        /* ...any referer */
#endif
#ifndef INPUTSECURITY           /* \input{} security not specified */
#ifdef INPUTOK            /* but INPUTOK flag specified */
#define INPUTSECURITY (99999)   /* so enable \input{} */
#ifndef EXPLICITDEFSECURITY     /* don't override explicit default */
#undef  DEFAULTSECURITY       /* but we'll override our default */
#define DEFAULTSECURITY (99999)   /*let -DINPUTOK enable \counter,etc*/
#endif
#else
/* else no \input{} specified */
#define INPUTSECURITY DEFAULTSECURITY /* set default \input security */
#endif
#endif
#ifndef COUNTERSECURITY         /*\counter{} security not specified*/
#ifdef COUNTEROK          /* but COUNTEROK flag specified */
#define COUNTERSECURITY (99999) /* so enable \counter{} */
#else
/* else no \counter{} specified */
#define COUNTERSECURITY DEFAULTSECURITY /*set default \counter security*/
#endif
#endif
#ifndef ENVIRONSECURITY         /* \environ security not specified */
#ifdef ENVIRONOK          /* but ENVIRONOK flag specified */
#define ENVIRONSECURITY (99999) /* so enable \environ */
#else
/* else no \environ specified */
#define ENVIRONSECURITY DEFAULTSECURITY /*set default \environ security*/
#endif
#endif

#ifndef ERRORSTATUS         /* exit(ERRORSTATUS) for any error */
#define ERRORSTATUS 0         /* default doesn't signal errors */
#endif

/* --- black on white background (default), or white on black --- */
#ifdef WHITE
#define ISBLACKONWHITE 0      /* white on black background */
#else
#define ISBLACKONWHITE 1      /* black on white background */
#endif

/* --- colors --- */
#define BGRED   (ISBLACKONWHITE?255:0)
#define BGGREEN (ISBLACKONWHITE?255:0)
#define BGBLUE  (ISBLACKONWHITE?255:0)
#define FGRED   (ISBLACKONWHITE?0:255)
#define FGGREEN (ISBLACKONWHITE?0:255)
#define FGBLUE  (ISBLACKONWHITE?0:255)

#ifndef FORMLEVEL
#define FORMLEVEL LOGLEVEL        /*msglevel if called from html form*/
#endif

extern  char **environ;     /* for \environment directive */

/* ------------------------------------------------------------
messages (used mostly by main() and also by rastmessage())
------------------------------------------------------------ */
static  char *copyright1 =      /* copyright, gnu/gpl notice */
    "+-----------------------------------------------------------------------+\n"
    "|mimeTeX vers " VERSION
    ", Copyright(c) 2002-2009, John Forkosh Associates, Inc|\n"
    "+-----------------------------------------------------------------------+\n"
    "| mimeTeX is free software, licensed to you under terms of the GNU/GPL, |\n"
    "|           and comes with absolutely no warranty whatsoever.           |";
static  char *copyright2 =
    "|          See http://www.forkosh.com/mimetex.html for details.         |\n"
    "+-----------------------------------------------------------------------+";
static  int maxmsgnum = 3,      /* maximum msgtable[] index */
            invmsgnum = 0,          /* general invalid message */
            refmsgnum = 3; /* urlncmp() failed to validate */
static  char *msgtable[] = {        /* messages referenced by [index] */
    "\\red\\small\\rm\\fbox{\\array{"
    /* [0] is invalid_referer_msg */
    "Please~read~www.forkosh.com/mimetex.html\\\\and~install~mimetex.cgi~"
    "on~your~own~server.\\\\Thank~you,~John~Forkosh}}",
    "\\red\\small\\rm\\fbox{\\array{"
    /* [1] */
    "Please~provide~your~{\\tiny~HTTP-REFERER}~to~access~the~public\\\\"
    "mimetex~server.~~Or~please~read~~www.forkosh.com/mimetex.html\\\\"
    "and~install~mimetex.cgi~on~your~own~server.~~Thank~you,~John~Forkosh}}",
    "\\red\\small\\rm\\fbox{\\array{"
    /* [2] */
    "The~public~mimetex~server~is~for~testing.~~For~production,\\\\"
    "please~read~~www.forkosh.com/mimetex.html~~and~install\\\\"
    "mimetex.cgi~on~your~own~server.~~Thank~you,~John~Forkosh}}",
    "\\red\\small\\rm\\fbox{\\array{"
    /* [3] */
    "Only~SERVER_NAME~may~use~mimetex~on~this~server.\\\\"
    "Please~read~~www.forkosh.com/mimetex.html~~and~install\\\\"
    "mimetex.cgi~on~your~own~server.~~Thank~you,~John~Forkosh}}",
    NULL
} ;               /* trailer */

static int daemonlevel = 0;    /* incremented in main() */
static int isss = ISSUPERSAMPLING; /* supersampling flag for main() */
static int iscaching = ISCACHING;  /* true if caching images */
static char cachepath[256] = CACHEPATH;  /* relative path to cached files */
static int isemitcontenttype = 1;  /* true to emit mime content-type */
static int isnomath = 0;       /* true to inhibit math mode */

static char *md5str(char *instr)
{
    static char outstr[64];
    unsigned char md5sum[16];
    md5_context ctx;
    int j;
    md5_starts(&ctx);
    md5_update(&ctx, (uint8_t *)instr, strlen(instr));
    md5_finish(&ctx, md5sum);
    for (j = 0; j < 16; j++)
        sprintf(outstr + j*2, "%02x", md5sum[j]);
    outstr[32] = '\000';
    return (outstr);
}

/* ==========================================================================
 * Function:    rastenviron ( expression, size, basesp, arg1, arg2, arg3 )
 * Purpose: handle \environment
 * --------------------------------------------------------------------------
 * Arguments:   expression (I/O) char **  to first char of null-terminated
 *              string immediately following \environment
 *              and returning ptr immediately
 *              following last character processed (in this
 *              case, \environment takes no arguments, so
 *              expression is returned unchanged).
 *      size (I)    int containing 0-7 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding \environment
 *              (unused, but passed for consistency)
 *      arg1 (I)    int unused
 *      arg2 (I)    int unused
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) subraster ptr to rendered environment image
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
subraster *rastenviron(char **expression, int size, subraster *basesp,
                       int arg1, int arg2, int arg3)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    char *strwrap();
    char *strdetex();
    char *texsubexpr();
    char *mimeprep();
    int unescape_url();
    subraster *rasterize();

    /* optional [...] args (for future)*/
    char optarg[255];
    char environstr[8192] = "\000",  /* string for all environment vars */
         environvar[1024] = "\000"; /* one environment variable */
    /* removes/replaces any math chars */
    /* ptr to preprocessed environvar */
    char *environptr = NULL;
    /*is \environ permitted*/
    int isenviron = (seclevel <= environseclevel ? 1 : 0);
    int maxvarlen = 512,        /* max chars in environment var */
        maxenvlen = 6400,       /* max chars in entire string */
       wraplen = 48; /* strwrap() wrap lines at 48 chars*/
    /* environ[] index */
    int ienv = 0;
    /* rasterize environment string */
    subraster *environsp = NULL;
    /* ------------------------------------------------------------
    Get args
    ------------------------------------------------------------ */
    /* --- check for optional \environment args --- */
    if (1)               /* there aren't any args (yet) */
        if (*(*expression) == '[') {        /* check for []-enclosed value */
            *expression = texsubexpr(*expression, optarg, 250, "[", "]", 0, 0);
            if (*optarg != '\000') {
                /* got optional arg, so process it */
                ;
                /* interpret \environment[wraplen] */
                wraplen = atoi(optarg);
                /* set minimum */
                if (wraplen < 1) wraplen = 8;
            } /* --- end-of-if(*optarg!='\0') --- */
        } /* --- end-of-if(**expression=='[') --- */
    /* ------------------------------------------------------------
    emit error message for unauthorized users trying to use \environ
    ------------------------------------------------------------ */
    if (!isenviron) {            /* environseclevel > seclevel */
        sprintf(environstr,
                "\\ \\text{[\\backslash environment\\ not permitted]}\\ ");
        /* rasterize error message */
        goto rasterize_environ;
    } /* --- end-of-if(!isenviron) --- */
    /* ------------------------------------------------------------
    Accumulate environment variables and rasterize string containing them
    ------------------------------------------------------------ */
    /* reset environment string */
    *environstr = '\000';
    /*init string*/
    strcat(environstr, "\\nocaching\\fbox{\\normalsize\\text{");
    for (ienv = 0; ; ienv++) {       /* loop over environ[] strings */
        /* null terminates list */
        if (environ[ienv] == (char *)NULL) break;
        /* double-check empty string */
        if (*(environ[ienv]) == '\000') break;
        /* max length displayed */
        strninit(environvar, environ[ienv], maxvarlen);
        if (strlen(environ[ienv]) > maxvarlen)   /* we truncated the variable */
            /* so add an ellipsis */
            strcat(environvar, "...");
        /* convert all %xx's to chars */
        unescape_url(environvar, 0);
        environptr = strdetex(environvar, 1); /* remove/replace any math chars */
        strninit(environvar, environptr, maxvarlen); /*de-tex'ed/nomath environvar*/
        /* wrap long lines */
        environptr = strwrap(environvar, wraplen, -6);
        /* line-wrapped environvar */
        strninit(environvar, environptr, maxvarlen);
        /* preprocess environvar string */
        mimeprep(environvar);
        if (strlen(environstr) + strlen(environvar) > maxenvlen) break;
        sprintf(environstr + strlen(environstr), /* display environment string */
                " %2d. %s\\\\\n", ienv + 1, environvar);
        if (msgfp != NULL && msglevel >= 9)
            fprintf(msgfp, "rastenviron> %2d. %.256s\n",
                    ienv + 1,/*environ[ienv]*/environvar);
        /* don't overflow buffer */
        if (strlen(environstr) >= 7200) break;
    } /* --- end-of-for(ienv) --- */
    /* end {\text{...}} mode */
    strcat(environstr, "}}");
rasterize_environ:
    /* rasterize environment string */
    environsp = rasterize(environstr, size);
    /* --- return environment raster to caller --- */
    /*end_of_job:*/
    /* return environment to caller */
    return (environsp);
} /* --- end-of-function rastenviron() --- */

/* ==========================================================================
 * Function:    rastmessage ( expression, size, basesp, arg1, arg2, arg3 )
 * Purpose: handle \message
 * --------------------------------------------------------------------------
 * Arguments:   expression (I/O) char **  to first char of null-terminated
 *              string immediately following \message
 *              and returning ptr immediately
 *              following last character processed.
 *      size (I)    int containing 0-7 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding \mesasge
 *              (unused, but passed for consistency)
 *      arg1 (I)    int unused
 *      arg2 (I)    int unused
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) subraster ptr to rendered message image
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
static subraster *rastmessage(char **expression, int size, subraster *basesp,
                              int arg1, int arg2, int arg3)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    char    *texsubexpr();
    subraster *rasterize();
    int strreplace();
    char    *urlprune();
    char    *strdetex();

    char amsg[256] = "\000";
    /* default message number */
    int imsg = 0;
    char    msg[4096];
    /* rasterize requested message */
    subraster *messagesp = NULL;
    /* #topmost levels to match */
    int reflevels = REFLEVELS;
    /* remove math chars from messages */
    char    *http_host    = getenv("HTTP_HOST"), /* http host for mimeTeX */
            *server_name  = getenv("SERVER_NAME"), /* server hosting mimeTeX */
            *referer_match = (!isempty(http_host) ? http_host : /*match http_host*/
                              (!isempty(server_name) ? server_name : (NULL))); /* or server_name */
    /* ------------------------------------------------------------
    obtain message {amsg} argument
    ------------------------------------------------------------ */
    /* --- parse for {amsg} arg, and bump expression past it --- */
    *expression = texsubexpr(*expression, amsg, 255, "{", "}", 0, 0);
    /* --- interpret argument --- */
    if (*amsg != '\000') {       /* got amsg arg */
        /* interpret as an int */
        imsg = atoi(amsg);
        if (imsg < 0               /* if too small */
                ||   imsg > maxmsgnum)        /* or too big */
            imsg = 0;
    }             /* default to first message */
    /* --- retrieve requested message --- */
    /* local copy of message */
    strninit(msg, msgtable[imsg], 4095);
    /* --- process as necessary --- */
    if (imsg == refmsgnum) {         /* urlncmp() failed to validate */
        if (reflevels > 0)             /* have #levels to validate */
            strreplace(msg, "SERVER_NAME",   /* replace SERVER_NAME */
                       /*with referer_match*/
                       strdetex(urlprune(referer_match, reflevels), 1), 0);
    } /* --- end-of-switch(imsg) --- */
    /* --- rasterize requested message --- */
    /* rasterize message string */
    messagesp = rasterize(msg, size);
    /* --- return message raster to caller --- */
    /*end_of_job:*/
    /* return message to caller */
    return (messagesp);
} /* --- end-of-function rastmessage() --- */



/* ==========================================================================
 * Function:    main() driver for mimetex.c
 * Purpose: emits a mime xbitmap or gif image of a LaTeX math expression
 *      entered either as
 *          (1) html query string from a browser (most typical), or
 *          (2) a query string from an html <form method="get">
 *          whose <input name="formdata"> (mostly for demo), or
 *          (3) command-line arguments (mostly to test).
 *      If no input supplied, expression defaults to "f(x)=x^2",
 *      treated as test (input method 3).
 *         If args entered on command-line (or if no input supplied),
 *      output is (usually) human-viewable ascii raster images on
 *      stdout rather than the usual mime xbitmaps or gif images.
 * --------------------------------------------------------------------------
 * Command-Line Arguments:
 *      When running mimeTeX from the command-line, rather than
 *      from a browser, syntax is
 *           ./mimetex  [-d ]       dump gif to stdout
 *              [expression expression, e.g., x^2+y^2,
 *              |-f input_file] or read expression from file
 *              [-m msglevel]   verbosity of debugging output
 *              [-s fontsize]   default fontsize, 0-5
 *      -d   Rather than ascii debugging output, mimeTeX dumps the
 *           actual gif (or xbitmap) to stdout, e.g.,
 *          ./mimetex  -d  x^2+y^2  > expression.gif
 *           creates a gif file containing an image of x^2+y^2
 *      -f   Reads expression from input_file, and automatically
 *           assumes -d switch.  The input_file may contain the
 *           expression on one line or spread out over many lines.
 *           MimeTeX will concatanate all lines from input_file
 *           to construct one long expression.  Blanks, tabs, and
 *           newlines will just be ignored.
 *      -m   0-99, controls verbosity level for debugging output
 *           (usually used only while testing code).
 *      -s   Font size, 0-5.  As usual, the font size can
 *           also be specified in the expression by a leading
 *           preamble terminated by $, e.g., 3$f(x)=x^2 displays
 *           f(x)=x^2 at font size 3.  Default font size is 2.
 * --------------------------------------------------------------------------
 * Exits:   0=success, 1=some error
 * --------------------------------------------------------------------------
 * Notes:     o For an executable that emits mime xbitmaps, compile as
 *           cc -DXBITMAP mimetex.c -lm -o mimetex.cgi
 *      or, alternatively, for an executable that emits gif images
 *           cc -DGIF mimetex.c gifsave.c -lm -o mimetex.cgi
 *      or for gif images with anti-aliasing
 *           cc -DGIF -DAA mimetex.c gifsave.c -lm -o mimetex.cgi
 *      See Notes at top of file for other compile-line -D options.
 *        o Move executable to your cgi-bin directory and either
 *      point your browser to it directly in the form
 *           http://www.yourdomain.com/cgi-bin/mimetex.cgi?3$f(x)=x^2
 *      or put a tag in your html document of the form
 *           <img src="../cgi-bin/mimetex.cgi?3$f(x)=x^2"
 *             border=0 align=absmiddle>
 *      where f(x)=x^2 (or any other expression) will be displayed
 *      either as a mime xbitmap or gif image (as per -D flag).
 * ======================================================================= */

/* ------------------------------------------------------------
header files and other data
------------------------------------------------------------ */
/* --- (additional) standard headers --- */
/* --- other data --- */
/* ------------------------------------------------------------
globals for gif and png callback functions
------------------------------------------------------------ */
static raster *bitmap_raster = NULL; /* use 0/1 bitmap image or */
static intbyte *colormap_raster = NULL;  /* anti-aliased color indexes */
/* --- anti-aliasing flags (needed by GetPixel() as well as main()) --- */
#ifdef AA               /* if anti-aliasing requested */
#define ISAAVALUE 1           /* turn flag on */
#else
#define ISAAVALUE 0           /* else turn flag off */
#endif

static int isaa = ISAAVALUE;   /* set anti-aliasing flag */

/* ------------------------------------------------------------
logging data structure, and default data to be logged
------------------------------------------------------------ */
/* --- logging data structure --- */
typedef struct logdata {
    /* ------------------------------------------------------------
    environment variable name, max #chars to display, min msglevel to display
    ------------------------------------------------------------ */
    /* environment variable name */
    char  *name;
    /* max #chars to display */
    int   maxlen;
    /* min msglevel to display data */
    int   msglevel;
} logdata ; /* --- end-of-logdata_struct --- */

/* --- data logged by mimeTeX --- */
static logdata mimelog[] = {
    /* ------ variable ------ maxlen msglevel ----- */
    { "QUERY_STRING",         999,    4 },
    { "REMOTE_ADDR",          999,    3 },
    { "HTTP_REFERER",         999,    3 },
    { "REQUEST_URI",          999,    5 },
    { "HTTP_USER_AGENT",      999,    3 },
    { "HTTP_X_FORWARDED_FOR", 999,    3 },
    { NULL, -1, -1 }            /* trailer record */
}; /* --- end-of-mimelog[] --- */


/* ==========================================================================
 * Function:    GetPixel ( int x, int y )
 * Purpose: callback for GIF_CompressImage() returning the
 *      pixel at column x, row y
 * --------------------------------------------------------------------------
 * Arguments:   x (I)       int containing column=0...width-1
 *              of desired pixel
 *      y (I)       int containing row=0...height-1
 *              of desired pixel
 * --------------------------------------------------------------------------
 * Returns: ( int )     0 or 1, if pixel at x,y is off or on
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
static int GetPixel(int x, int y)
{
    /* pixel index for x,y-coords*/
    int ipixel = y * bitmap_raster->width + x;
    /* value of pixel */
    int pixval = 0;
    if (!isaa)               /* use bitmap if not anti-aliased */
        /*pixel = 0 or 1*/
        pixval = (int)getlongbit(bitmap_raster->pixmap, ipixel);
    else
    /* else use anti-aliased grayscale*/
        /* colors[] index number */
        pixval = (int)(colormap_raster[ipixel]);
    if (msgfp != NULL && msglevel >= 9999) { /* dump pixel */
        fprintf(msgfp, "GetPixel> x=%d, y=%d  pixel=%d\n", x, y, pixval);
        fflush(msgfp);
    }
    return pixval;
} /* --- end-of-function GetPixel() --- */

/* --- entry point --- */
int main(int argc, char *argv[], char *envp[])
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    char *mimeprep();
    int unescape_url();
    int emitcache();
    char *timestamp();
    char *strdetex();
    int logger();
    int ismonth();
    int aacolormap();
    subraster *rasterize();
    raster *border_raster();
    int delete_subraster();
    int type_raster();
    int type_bytemap();
    int urlncmp();
    int strreplace();
    char *urlprune();
    int type_pbmpgm();
    int isstrstr();
    int aalowpass(), aapnm();

    /* --- expression to be emitted --- */
    /* input TeX expression */
    static  char exprbuffer[MAXEXPRSZ+1] = "f(x)=x^2";
    /* ptr to expression */
    char *expression = exprbuffer;
    /* default font size */
    int size = NORMALSIZE;
    /* getenv("QUERY_STRING") result */
    char *query = getenv("QUERY_STRING");
    /* preprocess expression */
    /* convert %xx's to ascii chars */
    /* emit cached image if it exists */
    int isquery = 0,            /* true if input from QUERY_STRING */
        isqempty = 0,           /* true if query string empty */
        isqforce = 0,           /* true to force query emulation */
        isqlogging = 0,         /* true if logging in query mode */
        isformdata = 0,         /* true if input from html form */
        isinmemory = 1,         /* true to generate image in memory*/
        isdumpimage = 0,        /* true to dump image on stdout */
        isdumpbuffer = 0;       /* true to dump to memory buffer */
    /* --- rasterization --- */
    /* rasterize expression */
    subraster *sp = NULL;
    /* put a border around raster */
    raster *bp = NULL;
    /* for clean-up at end-of-job */
    /* --- http_referer --- */
    /* http_referer must contain this */
    char    *referer = REFERER;
    /*http_referer's permitted to \input*/
    char    *inputreferer = INPUTREFERER;
    /* cmp http_referer,server_name */
    int reflevels = REFLEVELS;
    /* prune referer_match */
    struct  {
        /* http_referer can't contain this */
        char *referer;
        int msgnum;
    } denyreferer[] = {       /* referer table to deny access to */
#ifdef DENYREFERER
#include DENYREFERER      /* e.g.,  {"",1},  for no referer */
#endif
        { NULL, -999 }
    /* trailer */
    };
    char    *http_referer = getenv("HTTP_REFERER"), /* referer using mimeTeX */
            *http_host    = getenv("HTTP_HOST"), /* http host for mimeTeX */
            *server_name  = getenv("SERVER_NAME"), /* server hosting mimeTeX */
            *referer_match = (!isempty(http_host) ? http_host : /*match http_host*/
                             (!isempty(server_name) ? server_name : (NULL))); /* or server_name */
    int ishttpreferer = (isempty(http_referer) ? 0 : 1);
    /* true for inavlid referer */
    int isinvalidreferer = 0;
    /*max query_string len if no referer*/
    int norefmaxlen = NOREFMAXLEN;
    /* --- gif --- */
    char *gif_outfile = (char *)NULL;   /* gif output defaults to stdout */
    char gif_buffer[MAXGIFSZ] = "\000";  /* or gif written in memory buffer */
    char cachefile[256] = "\000";    /* full path and name to cache file*/
    /* max-age is two hours */
    int maxage = 7200;
    /*Vertical-Align:baseline-(height-1)*/
    int valign = (-9999);
    /* --- pbm/pgm (-g switch) --- */
    int ispbmpgm = 0;           /* true to write pbm/pgm file */
    /* entry point, graphic format */
    int ptype = 0;
    /* output file defaults to stdout */
    char *pbm_outfile = (char *)NULL;
    /* --- anti-aliasing --- */
    intbyte *bytemap_raster = NULL,     /* anti-aliased bitmap */
            colors[256]; /* grayscale vals in bytemap */
    int grayscale = 256; /* 0-255 grayscales in 8-bit bytes */
    int ncolors = 2;        /* #colors (2=b&w) */
    /*patternnumcount[] index diagnostic*/
    int ipattern;
    /* --- messages --- */
    char logfile[256] = LOGFILE,     /*log queries if msglevel>=LOGLEVEL*/
         cachelog[256] = CACHELOG;   /* cached image log in cachepath/ */
    /* name program executed as */
    char    *progname = (argc > 0 ? argv[0] : "noname");
    char    *dashes =           /* separates logfile entries */
        "--------------------------------------------------------------------------";
    /*msg to invalid referer*/
    char    *invalid_referer_msg = msgtable[invmsgnum];
    /*referer isn't host*/
    char    *invalid_referer_match = msgtable[refmsgnum];
    char    contenttype[2048] = "\000"; /* content-type:, etc buffer */
    /* ------------------------------------------------------------
    initialization
    ------------------------------------------------------------ */
    /* --- run optional system command string --- */
#ifdef SYSTEM
    system(SYSTEM);
#endif
    /* --- set global variables --- */
    strcpy(pathprefix, PATHPREFIX);
    /* for command-line mode output */
    msgfp = stdout;
    /* set supersampling flag */
    isss = issupersampling;
    /* true to emit mime content-type */
    isemitcontenttype = 1;
    /* reset content-type:, etc. cache */
    *contenttype = '\000';
    /* true to inhibit math mode */
    isnomath = 0;
    /* overall security level */
    seclevel = SECURITY;
    /* security level for \input{} */
    inputseclevel = INPUTSECURITY;
    /* security level for \counter{} */
    counterseclevel = COUNTERSECURITY;
    /* security level for \environ */
    environseclevel = ENVIRONSECURITY;
    /* reset count of \input commands */
    ninputcmds = 0;
    exitstatus = 0;
    errorstatus = ERRORSTATUS;  /* reset exit/error status */
    /* true if caching images */
    iscaching = ISCACHING;
    if (iscaching) {             /* images are being cached */
        /* relative path to cached files */
        strcpy(cachepath, CACHEPATH);
        if (*cachepath == '%') {       /* leading % signals cache headers */
            /* signal caching mime content-type*/
            strcpy(cachepath, cachepath + 1);
        }
    }  /* and squeeze out leading % char */
    /* signal that image not in memory */
    gifSize = 0;
    fgred = FGRED;
    fggreen = FGGREEN;
    /* default foreground colors */
    fgblue = FGBLUE;
    bgred = BGRED;
    bggreen = BGGREEN;
    /* default background colors */
    bgblue = BGBLUE;
    /* set shrinkfactor */
    shrinkfactor = shrinkfactors[NORMALSIZE];
    for (ipattern = 1; ipattern <= 51; ipattern++)
        patternnumcount0[ipattern] = patternnumcount1[ipattern] = 0;
    /* ---
     * check QUERY_STRING query for expression overriding command-line arg
     * ------------------------------------------------------------ */
    if (query != NULL)           /* check query string from environ */
        if (strlen(query) >= 1) {          /* caller gave us a query string */
            /* so use it as expression */
            strncpy(expression, query, MAXEXPRSZ);
            /* make sure it's null terminated */
            expression[MAXEXPRSZ] = '\000';
            if (0)               /*true to remove leading whitespace*/
                while (isspace(*expression) && *expression != '\000')
                    /* squeeze out white space */
                    strcpy(expression, expression + 1);
            isquery = 1;
        }          /* and set isquery flag */
    if (!isquery) {              /* empty query string */
        char *host = getenv("HTTP_HOST"), /* additional getenv("") results */
                     *name = getenv("SERVER_NAME"), *addr = getenv("SERVER_ADDR");
        if (host != NULL || name != NULL || addr != NULL) { /* assume http query */
            /* set flag to signal query */
            isquery = 1;
            /* signal error */
            if (exitstatus == 0) exitstatus = errorstatus;
            strcpy(expression,
            /* and give user an error message */
                   "\\red\\small\\rm\\fbox{\\begin{gather}\\LaTeX~expression~not~supplied"
                   "\\\\i.e.,~no~?query\\_string~given~to~mimetex.cgi\\end{gather}}");
        }
        /* signal empty query string */
        isqempty = 1;
    } /* --- end-of-if(!isquery) --- */
    /* ---
     * process command-line input args (if not a query)
     * ------------------------------------------------------------ */
    if (!isquery /* don't have an html query string */
            || (argc > 1)) { /* or have command-line args */
        char   *argsignal = ARGSIGNAL,     /* signals start of mimeTeX args */
               stopsignal[32] = "--"; /* default Unix end-of-args signal */
        int    iarg = 0, argnum = 0,   /*argv[] index for command-line args*/
               exprarg = 0,            /* argv[] index for expression */
               infilearg = 0,          /* argv[] index for infile */
               nswitches = 0,          /* number of -switches */
               isstopsignal = 0,       /* true after stopsignal found */
               isstrict = 1,  /* true for strict arg checking */
               nargs = 0, nbadargs = 0,    /* number of arguments, bad ones */
               maxbadargs = (isstrict ? 0 : 1),    /*assume query if too many bad args*/
               isgoodargs = 0; /* true to accept command-line args*/
        if (argsignal != NULL) {
            /* if compiled with -DARGSIGNAL */
            while (argc > ++iarg) {
                /* check each argv[] for argsignal */
                if (!strcmp(argv[iarg], argsignal)) { /* check for exact match */
                    /* got it, start parsing next arg */
                    argnum = iarg;
                    /* stop looking for argsignal */
                    break;
                }
            }
        }
        while (argc > ++argnum) {       /* check for switches and values, */
            /* count another command-line arg */
            nargs++;
            if (strcmp(argv[argnum], stopsignal) == 0) { /* found stopsignal */
                /* so set stopsignal flag */
                isstopsignal = 1;
                continue;
            }         /* and get expression after it */
            if (!isstopsignal            /* haven't seen stopsignal switch */
                    &&   *argv[argnum] == '-') {    /* and have some '-' switch */
                /* ptr to char(s) following - */
                char *field = argv[argnum] + 1;
                /* single char following '-' */
                char flag = tolower(*field);
                /* #chars following - */
                int  arglen = strlen(field);
                argnum++;
                /* arg following flag/switch is usually its value */
                /* another switch on command line */
                nswitches++;
                if (isstrict &&            /* if strict checking then... */
                        !isthischar(flag, "g") && arglen != 1) { /*must be single-char switch*/
                    /* so ignore longer -xxx switch */
                    nbadargs++;
                    argnum--;
                } else             /* process single-char -x switch */
                    switch (flag) {       /* see what user wants to tell us */
                        /* --- ignore uninterpreted flag --- */
                    default:
                        nbadargs++;
                        argnum--;
                        break;
                        /* --- adjustable program parameters (not checking input) --- */
                    case 'b':
                        isdumpimage++;
                        isdumpbuffer++;
                        argnum--;
                        break;
                    case 'd':
                        isdumpimage++;
                        argnum--;
                        break;
                    case 'e':
                        isdumpimage++;
                        gif_outfile = argv[argnum];
                        break;
                    case 'f':
                        isdumpimage++;
                        infilearg = argnum;
                        break;
                    case 'g':
                        ispbmpgm++;
                        /* -g2 ==> ptype=2 */
                        if (arglen > 1) ptype = atoi(field + 1);
                        /*next arg is -switch*/
                        if (1 || *argv[argnum] == '-') argnum--;
                        else pbm_outfile = argv[argnum];
                        /*next arg is filename*/
                        break;
                    case 'm':
                        if (argnum < argc) msglevel = atoi(argv[argnum]);
                        break;
                    case 'o':
                        istransparent = (istransparent ? 0 : 1);
                        argnum--;
                        break;
                    case 'q':
                        isqforce = 1;
                        argnum--;
                        break;
                    case 's':
                        if (argnum < argc) size = atoi(argv[argnum]);
                        break;
                    } /* --- end-of-switch(flag) --- */
            } /* --- end-of-if(*argv[argnum]=='-') --- */
            else
            /* expression if arg not a -flag */
                if (infilearg == 0) {      /* no infile arg yet */
                    /* 2nd expression invalid */
                    if (exprarg != 0) nbadargs++;
                    /* but take last expression */
                    exprarg = argnum;
                    /*infilearg = (-1);*/
                }   /* and set infilearg */
                /* infile and expression invalid */
                else nbadargs++;
        } /* --- end-of-while(argc>++argnum) --- */
        if (msglevel >= 999 && msgfp != NULL) { /* display command-line info */
            fprintf(msgfp, "argc=%d, progname=%s, #args=%d, #badargs=%d\n",
                    argc, progname, nargs, nbadargs);
            fprintf(msgfp, "cachepath=\"%.50s\" pathprefix=\"%.50s\"\n",
                    cachepath, pathprefix);
        }
        /* ---
         * decide whether command-line input overrides query_string
         * ------------------------------------------------------------ */
        if (isdumpimage > 2) nbadargs++;    /* duplicate/conflicting -switch */
        isgoodargs = (!isstrict         /*good if not doing strict checking*/
                      || !isquery               /* or if no query, must use args */
                      /* bad args imply query */
                      || (nbadargs < nargs && nbadargs <= maxbadargs));
        /* ---
         * take expression from command-line args
         * ------------------------------------------------------------ */
        if (isgoodargs && exprarg > 0       /* good expression on command line */
                &&   infilearg <= 0)           /* and not given in input file */
            if (!isquery               /* no conflict if no query_string */
                    ||   nswitches > 0) {         /* explicit -switch(es) also given */
                /*expr from command-line*/
                strncpy(expression, argv[exprarg], MAXEXPRSZ);
                /* make sure it's null terminated */
                expression[MAXEXPRSZ] = '\000';
                isquery = 0;
            }         /* and not from a query_string */
        /* ---
         * or read expression from input file
         * ------------------------------------------------------------ */
        if (isgoodargs && infilearg > 0) {  /* have a good -f arg */
            /* open input file for read */
            FILE *infile = fopen(argv[infilearg], "r");
            if (infile != (FILE *)NULL) {      /* opened input file successfully */
                /* line from file */
                char instring[MAXLINESZ+1];
                /* total #bytes read from file */
                int  exprsz = 0;
                /* file input, not a query_string */
                isquery = 0;
                /* start expresion as empty string */
                *expression = '\000';
                while (fgets(instring, MAXLINESZ, infile) != (char *)NULL) /*till eof*/
                    if (exprsz + strlen(instring) < MAXEXPRSZ) {   /* have room for line */
                        /* concat line to end of expression*/
                        strcat(expression, instring);
                        exprsz += strlen(instring);
                    }   /* update expression buffer length */
                fclose(infile);
            }    /*close input file after reading expression*/
        } /* --- end-of-if(infilearg>0) --- */
        /* ---
         * xlate +++'s to blanks only if query
         * ------------------------------------------------------------ */
        /* don't xlate +++'s to blanks */
        if (!isquery) isplusblank = 0;
        /* ---
         * check if emulating query (for testing)
         * ------------------------------------------------------------ */
        /* emulate query string processing */
        if (isqforce) isquery = 1;
        /* ---
         * check if emitting pbm/pgm graphic
         * ------------------------------------------------------------ */
        if (isgoodargs && ispbmpgm > 0)     /* have a good -g arg */
            if (1 && gif_outfile != NULL)      /* had an -e switch with file */
                if (*gif_outfile != '\000') { /* make sure string isn't empty */
                    pbm_outfile = gif_outfile;   /* use -e switch file for pbm/pgm */
                    /* reset gif output file */
                    gif_outfile = (char *)NULL;
                    /*isdumpimage--;*/
                }     /* and decrement -e count */
    } /* --- end-of-if(!isquery) --- */
    /* ---
     * check for <form> input
     * ------------------------------------------------------------ */
    if (isquery) {               /* must be <form method="get"> */
        if (!memcmp(expression, "formdata", 8)) { /*must be <input name="formdata"> */
            /* find equal following formdata */
            char *delim = strchr(expression, '=');
            if (delim != (char *)NULL)   /* found unescaped equal sign */
                /* so shift name= out of expression*/
                strcpy(expression, delim + 1);
            while ((delim = strchr(expression, '+')) != NULL) /*unescaped plus sign*/
                /* is "shorthand" for blank space */
                *delim = ' ';
            /*unescape_url(expression,1);*/ /* convert unescaped %xx's to chars */
            /* convert all %xx's to chars */
            unescape_url(expression, 0);
            /* repeat */
            unescape_url(expression, 0);
            /* msglevel for forms */
            if (0) msglevel = FORMLEVEL;
            isformdata = 1;
        }           /* set flag to signal form data */
        else
        /* --- query, but not <form> input --- */
            unescape_url(expression, 0);
    }   /* convert _all_ %xx's to chars */
    /* ---
     * check queries for prefixes/suffixes/embedded that might cause problems
     * ------------------------------------------------------------ */
    /* --- expression whose last char is \ --- */
    if (lastchar(expression) == '\\')    /* last char is backslash */
        /* assume "\ " lost the final space*/
        strcat(expression, " ");
    /* ---
     * check queries for embedded prefixes signalling special processing
     * ------------------------------------------------------------ */
    if (isquery) {               /* only check queries */
        /* --- check for msglevel=###$ prefix --- */
        if (!memcmp(expression, "msglevel=", 9)) { /* query has msglevel prefix */
            /* find $ delim following msglevel*/
            char *delim = strchr(expression, '$');
            if (delim != (char *)NULL) {    /* check that we found delim */
                /* replace delim with null */
                *delim = '\000';
                if (seclevel <= 9)       /* permit msglevel specification */
                    /* interpret ### in msglevel###$ */
                    msglevel = atoi(expression + 9);
                strcpy(expression, delim + 1);
            }
        } /* shift out prefix and delim */
        /* --- next check for logfile=xxx$ prefix (must follow msglevel) --- */
        if (!memcmp(expression, "logfile=", 8)) { /* query has logfile= prefix */
            /* find $ delim following logfile=*/
            char *delim = strchr(expression, '$');
            if (delim != (char *)NULL) {    /* check that we found delim */
                /* replace delim with null */
                *delim = '\000';
                if (seclevel <= 3)       /* permit logfile specification */
                    /* interpret xxx in logfile=xxx$ */
                    strcpy(logfile, expression + 8);
                strcpy(expression, delim + 1);
            }
        } /* shift out prefix and delim */
    } /* --- end-of-if(isquery) --- */
    /* ---
     * log query (e.g., for debugging)
     * ------------------------------------------------------------ */
    if (isquery)                 /* only log query_string's */
        if (msglevel >= LOGLEVEL        /* check if logging */
                &&   seclevel <= 5)            /* and if logging permitted */
            if (logfile != NULL)       /* if a logfile is given */
                if (*logfile != '\000') {         /*and if it's not an empty string*/
                    if ((msgfp = fopen(logfile, "a"))  /* open logfile for append */
                            !=   NULL) {            /* ignore logging if can't open */
                        /* --- default logging --- */
                        /* log query */
                        logger(msgfp, msglevel, expression, mimelog);
                        /* --- additional debug logging (argv and environment) --- */
                        if (msglevel >= 9) {        /* log environment */
                            /*char name[999],*value;*/
                            int i;
                            fprintf(msgfp, "Command-line arguments...\n");
                            if (argc < 1)            /* no command-line args */
                                fprintf(msgfp, "  ...argc=%d, no argv[] variables\n", argc);
                            else
                                for (i = 0; i < argc; i++)  /* display all argv[]'s */
                                    fprintf(msgfp, "  argv[%d] = \"%s\"\n", i, argv[i]);
#ifdef DUMPENVP         /* char *envp[] available for dump */
                            fprintf(msgfp, "Environment variables (using envp[])...\n");
                            if (envp == (char **)NULL)   /* envp not provided */
                                fprintf(msgfp, "  ...envp[] environment variables not available\n");
                            else
                                for (i = 0; ; i++)      /* display all envp[]'s */
                                    if (envp[i] == (char *)NULL) break;
                                    else fprintf(msgfp, "  envp[%d] = \"%s\"\n", i, envp[i]);
#endif
/* --- DUMPENVP ---*/
#ifdef DUMPENVIRON  /* skip what should be redundant output */
                            fprintf(msgfp, "Environment variables (using environ)...\n");
                            if (environ == (char **)NULL)    /* environ not provided */
                                fprintf(msgfp, "  ...extern environ variables not available\n");
                            else
                                for (i = 0; ; i++)      /*display environ[] and getenv()'s*/
                                    if (environ[i] == (char *)NULL) break;
                                    else {
                                        /* set up name for getenv() arg */
                                        strcpy(name, environ[i]);
                                        if ((value = strchr(name, '=')) != NULL) { /* = delimits name */
                                            /* got it, so null-terminate name */
                                            *value = '\000';
                                            value = getenv(name);
                                        } /* and look up name using getenv() */
                                        /* missing = delim in environ[i] */
                                        else strcpy(name, "NULL");
                                        fprintf(msgfp, "environ[%d]: \"%s\"\n\tgetenv(%s) = \"%s\"\n",
                                                i, environ[i], name, (value == NULL ? "NULL" : value));
                                    } /* --- end-of-if/else --- */
#endif
/* --- DUMPENVIRON ---*/
                        } /* --- end-of-if(msglevel>=9) --- */
                        /* --- close log file if no longer needed --- */
                        if (msglevel < DBGLEVEL) {      /* logging, but not debugging */
                            /* so log separator line, */
                            fprintf(msgfp, "%s\n", dashes);
                            /* close logfile immediately, */
                            fclose(msgfp);
                            msgfp = NULL;
                        }         /* and reset msgfp pointer */
                        else
                            /* set query logging flag */
                            isqlogging = 1;
                    } /* --- end-of-if(msglevel>=LOGLEVEL) --- */
                    else
                    /* couldn't open logfile */
                        msglevel = 0;
                }            /* can't emit messages */
    /* ---
     * prepend prefix to submitted expression
     * ------------------------------------------------------------ */
    if (1 || isquery) {
        /* queries or command-line */
        if (*exprprefix != '\000') {        /* we have a prefix string */
            /* #chars in prefix */
            int npref = strlen(exprprefix);
            /*make room*/
            memmove(expression + npref + 1, expression, strlen(expression) + 1);
            /* copy prefix into expression */
            memcpy(expression, exprprefix, npref);
            /* followed by { */
            expression[npref] = '{';
            strcat(expression, "}");
        }       /* and terminating } to balance { */
    }
    /* ---
     * check if http_referer is allowed to use this image and to use \input{}
     * ------------------------------------------------------------ */
    if (isquery) {           /* not relevant if "interactive" */
        /* --- check -DREFERER=\"comma,separated,list\" of valid referers --- */
        if (referer != NULL) {          /* compiled with -DREFERER=\"...\" */
            if (strcmp(referer, "month") != 0)     /* but it's *only* "month" signal */
                if (ishttpreferer)            /* or called "standalone" */
                    if (!isstrstr(http_referer, referer, 0)) { /* invalid http_referer */
                        /* so give user error message */
                        expression = invalid_referer_msg;
                        isinvalidreferer = 1;
                    }
        }     /* and signal invalid referer */
        else
        /* compiled without -DREFERER= */
            if (reflevels > 0) {       /*match referer unless -DREFLEVELS=0*/
                /* --- check topmost levels of http_referer against http_host --- */
                if (ishttpreferer             /* have http_referer */
                        &&   !isempty(referer_match))    /* and something to match it with */
                    if (!urlncmp(http_referer, referer_match, reflevels)) { /*match failed*/
                        /* init error message */
                        strcpy(exprbuffer, invalid_referer_match);
                        strreplace(exprbuffer, "SERVER_NAME", /* and then replace SERVER_NAME */
                                   /*with referer_match*/
                                   strdetex(urlprune(referer_match, reflevels), 1), 0);
                        isinvalidreferer = 1;
                    }        /* and signal invalid referer */
            } /* --- end-of-if(reflevels>0) --- */
        /* --- check -DINPUTREFERER=\"comma,separated,list\" of \input users --- */
        /* set default input security */
        inputseclevel = INPUTSECURITY;
        if (inputreferer != NULL) {         /* compiled with -DINPUTREFERER= */
            if (http_referer == NULL)          /* but no http_referer given */
                /* unknown user can't \input{} */
                inputseclevel = (-1);
            else
            /*have inputreferer and http_referer*/
                if (!isstrstr(http_referer, inputreferer, 0)) /*http_referer can't \input*/
                    /* this known user can't \input{} */
                    inputseclevel = (-1);
        } /* --- end-of-if(inputreferer!=NULL) --- */
    } /* --- end-of-if(isquery) --- */
    /* ---
     * check if referer contains "month" signal
     * ------------------------------------------------------------ */
    if (isquery)                 /* not relevant if "interactive" */
        if (referer != NULL)            /* nor if compiled w/o -DREFERER= */
            if (!isinvalidreferer)         /* nor if already invalid referer */
                if (strstr(referer, "month") != NULL)  /* month check requested */
                    if (!ismonth(progname)) {        /* not executed as mimetexJan-Dec */
                        /* so give user error message */
                        expression = invalid_referer_msg;
                        isinvalidreferer = 1;
                    }      /* and signal invalid referer */
    /* ---
     * check if http_referer is to be denied access
     * ------------------------------------------------------------ */
    if (isquery)                 /* not relevant if "interactive" */
        if (!isinvalidreferer) {        /* nor if already invalid referer */
            /* denyreferer index, message# */
            int iref = 0, msgnum = (-999);
            for (iref = 0; msgnum < 0; iref++) { /* run through denyreferer[] table */
                /* referer to be denied */
                char *deny = denyreferer[iref].referer;
                /* null signals end-of-table */
                if (deny == NULL) break;
                if (msglevel >= 999 && msgfp != NULL) { /* debugging */
                    fprintf(msgfp, "main> invalid iref=%d: deny=%s http_referer=%s\n",
                            iref, deny, (http_referer == NULL ? "null" : http_referer));
                    fflush(msgfp);
                }
                if (*deny == '\000') {     /* signal to check for no referer */
                    if (http_referer == NULL)      /* http_referer not supplied */
                        msgnum = denyreferer[iref].msgnum;
                } /* so set message# */
                else
                /* have referer to check for */
                    if (http_referer != NULL)     /* and have referer to be checked */
                        if (isstrstr(http_referer, deny, 0)) /* invalid http_referer */
                            /* so set message# */
                            msgnum = denyreferer[iref].msgnum;
            } /* --- end-of-for(iref) --- */
            if (msgnum >= 0) {           /* deny access to this referer */
                /* keep index within bounds */
                if (msgnum > maxmsgnum) msgnum = 0;
                /* set user error message */
                expression = msgtable[msgnum];
                isinvalidreferer = 1;
            }      /* and signal invalid referer */
        } /* --- end-of-if(!isinvalidreferer) --- */
    /* --- also check maximum query_string length if no http_referer given --- */
    if (isquery) {
        /* not relevant if "interactive" */
        if (!isinvalidreferer) {
            /* nor if already invalid referer */
            if (!ishttpreferer) {
                /* no http_referer supplied */
                if (strlen(expression) > norefmaxlen) {   /* query_string too long */
                    if (isempty(referer_match)) {
                        /* no referer_match to display */
                        /* set invalid http_referer message*/
                        expression = invalid_referer_msg;
                    } else {              /* error with referer_match display*/
                        /* init error message */
                        strcpy(exprbuffer, invalid_referer_match);
                        strreplace(exprbuffer, "SERVER_NAME", /* and then replace SERVER_NAME */
                                   strdetex(urlprune(referer_match, reflevels), 1), 0);
                    } /*with host_http*/
                    isinvalidreferer = 1;
                } /* and signal invalid referer */
            }
        }
    }
    /* ---
     * check for image caching
     * ------------------------------------------------------------ */
    if (strstr(expression, "\\counter")  != NULL /* can't cache \counter{} */
            ||   strstr(expression, "\\input")    != NULL /* can't cache \input{} */
            ||   strstr(expression, "\\today")    != NULL /* can't cache \today */
            ||   strstr(expression, "\\calendar") != NULL /* can't cache \calendar */
            ||   strstr(expression, "\\nocach")   != NULL /* no caching requested */
            ||   isformdata             /* don't cache user form input */
       ) {
        /* so turn caching off */
        iscaching = 0;
        maxage = 5;
    }          /* and set max-age to 5 seconds */
    if (isquery)                 /* don't cache command-line images */
        if (iscaching) {            /* image caching enabled */
            /* --- set up path to cached image file --- */
            /* md5 hash of expression */
            char *md5hash = md5str(expression);
            if (md5hash == NULL)       /* failed for some reason */
                /* so turn off caching */
                iscaching = 0;
            else {
                /* start with (relative) path */
                strcpy(cachefile, cachepath);
                /* add md5 hash of expression */
                strcat(cachefile, md5hash);
                /* finish with .gif extension */
                strcat(cachefile, ".gif");
                /* signal GIF_Create() to cache */
                gif_outfile = cachefile;
                /* --- emit mime content-type line --- */
                if (0 && isemitcontenttype) { /* now done in emitcache() */
                    fprintf(stdout, "Cache-Control: max-age=%d\n", maxage);
                    if (abs(valign) < 999)         /* have vertical align */
                        fprintf(stdout, "Vertical-Align: %d\n", valign);
                    fprintf(stdout, "Content-type: image/gif\n\n");
                }
                /* --- emit cached image if it already exists --- */
                if (emitcache(cachefile, maxage, valign, 0) > 0) /* cached image emitted */
                    /* so nothing else to do */
                    goto end_of_job;
                /* --- log caching request --- */
                if (msglevel >= 1             /* check if logging */
                        /*&&   seclevel <= 5*/)      /* and if logging permitted */
                    if (cachelog != NULL)        /* if a logfile is given */
                        if (*cachelog != '\000') {      /*and if it's not an empty string*/
                            char filename[256];     /* construct cachepath/cachelog */
                            /* fopen(filename) */
                            FILE *filefp = NULL;
                            /* start with (relative) path */
                            strcpy(filename, cachepath);
                            /* add cache log filename */
                            strcat(filename, cachelog);
                            if ((filefp = fopen(filename, "a")) /* open cache logfile for append */
                                    !=   NULL) {        /* ignore logging if can't open */
                                /* set true if http_referer logged */
                                int isreflogged = 0;
                                fprintf(filefp, "%s                 %s\n", /* timestamp, md5 file */
                                        /*skip path*/
                                        timestamp(tzdelta, 0), cachefile + strlen(cachepath));
                                /* expression in filename */
                                fprintf(filefp, "%s\n", expression);
                                if (http_referer != NULL)     /* show referer if we have one */
                                    if (*http_referer != '\000') {    /* and if not an empty string*/
                                        /* #chars on line in log file*/
                                        int loglen = strlen(dashes);
                                        /* line to be printed */
                                        char *refp = http_referer;
                                        /* signal http_referer logged*/
                                        isreflogged = 1;
                                        while (1) {                /* printed in parts if needed*/
                                            /* print a part */
                                            fprintf(filefp, "%.*s\n", loglen, refp);
                                            /* no more parts */
                                            if (strlen(refp) <= loglen) break;
                                            refp += loglen;
                                        }
                                    }         /* bump ptr to next part */
                                if (!isreflogged)               /* http_referer not logged */
                                    /* so log dummy referer line */
                                    fprintf(filefp, "http://none\n");
                                /* separator line */
                                fprintf(filefp, "%s\n", dashes);
                                fclose(filefp);
                            }             /* close logfile immediately */
                        } /* --- end-of-if(cachelog!=NULL) --- */
            } /* --- end-of-if/else(md5hash==NULL) --- */
        } /* --- end-of-if(iscaching) --- */
    /* ---
     * emit copyright, gnu/gpl notice (if "interactive")
     * ------------------------------------------------------------ */
    if (!isdumpimage)            /* don't mix ascii with image dump */
        if ((!isquery || isqlogging) && msgfp != NULL) { /* called from command line */
            /* display copyright */
            fprintf(msgfp, "%s\n%s\n", copyright1, copyright2);
            /*revision date*/
            fprintf(msgfp, "Most recent revision: %s\n", REVISIONDATE);
        } /* --- end-of-if(!isquery...) --- */
    /* ------------------------------------------------------------
    rasterize expression and put a border around it
    ------------------------------------------------------------ */
    /* --- preprocess expression, converting LaTeX constructs for mimeTeX  --- */
    if (expression != NULL) {        /* have expression to rasterize */
        expression = mimeprep(expression);
    }  /* preprocess expression */
    /* --- double-check that we actually have an expression to rasterize --- */
    if (expression == NULL) {        /* nothing to rasterize */
        /*signal error to parent*/
        if (exitstatus == 0) exitstatus = errorstatus;
        if ((!isquery || isqlogging) && msgfp != NULL) { /*emit error if not query*/
            if (exitstatus != 0) fprintf(msgfp, "Exit code = %d,\n", exitstatus);
            fprintf(msgfp, "No LaTeX expression to rasterize\n");
        }
        goto end_of_job;
    }            /* and then quit */
    /* --- rasterize expression --- */
    if ((sp = rasterize(expression, size)) == NULL) {  /* failed to rasterize */
        /*signal error to parent*/
        if (exitstatus == 0) exitstatus = errorstatus;
        if ((!isquery || isqlogging) && msgfp != NULL) { /*emit error if not query*/
            if (exitstatus != 0) fprintf(msgfp, "Exit code = %d,\n", exitstatus);
            fprintf(msgfp, "Failed to rasterize %.2048s\n", expression);
        }
        if (isquery) {             /* try to display failed expression*/
            /* buffer for failed expression */
            char errormsg[4096];
            strcpy(errormsg,
            /* init error message */
                   "\\red\\fbox{\\begin{gather}"
                   "{\\rm~mi\\underline{meTeX~failed~to~render~your~expressi}on}\\\\[5]");
            /*render expression as \rm*/
            strcat(errormsg, "{\\rm\\hspace{10}{");
            /*add detexed expression to msg*/
            strcat(errormsg, strdetex(expression, 0));
            /* finish up */
            strcat(errormsg, "}\\hspace{10}}\\end{gather}}");
            if ((sp = rasterize(errormsg, 1)) == NULL)  /*couldn't rasterize errmsg*/
                sp = rasterize(   /* so rasterize generic error */
                         "\\red\\rm~\\fbox{mimeTeX~failed~to~render\\\\your~expression}", 1);
        }
        /* re-check for err message failure*/
        if (sp ==  NULL) goto end_of_job;
    } /* --- end-of-if((sp=rasterize())==NULL) --- */
    /* ---no border requested, but this adjusts width to multiple of 8 bits--- */
    if (issupersampling)             /* no border needed for gifs */
        /* so just extract pixel map */
        bp = sp->image;
    else
    /* for mime xbitmaps must have... */
        /* image width multiple of 8 bits */
        bp = border_raster(sp->image, 0, 0, 0, 1);
    /* global copy for gif,png output */
    sp->image = bitmap_raster = bp;
    if (sp != NULL && bp != NULL) {      /* have raster */
        /* #pixels for Vertical-Align: */
        valign = sp->baseline - (bp->height - 1);
        if (abs(valign) > 255) valign = (-9999);
    } /* sanity check */
    if (ispbmpgm && ptype < 2)       /* -g switch or -g1 switch */
        type_pbmpgm(bp, ptype, pbm_outfile);  /* emit b/w pbm file */
    /* ------------------------------------------------------------
    generate anti-aliased bytemap from (bordered) bitmap
    ------------------------------------------------------------ */
    if (isaa) {              /* we want anti-aliased bitmap */
        /* ---
         * allocate bytemap and colormap as per width*height of bitmap
         * ------------------------------------------------------------ */
        /*#bytes needed in byte,colormap*/
        int   nbytes = (bp->width) * (bp->height);
        if (isss)                  /* anti-aliasing by supersampling */
            /*bytemap in raster*/
            bytemap_raster = (intbyte *)(bitmap_raster->pixmap);
        else
        /* need to allocate bytemap */
            if (aaalgorithm == 0)        /* anti-aliasing not wanted */
                /* so signal no anti-aliasing */
                isaa = 0;
            else
            /* anti-aliasing wanted */
                if ((bytemap_raster = (intbyte *)malloc(nbytes))  /* malloc bytemap */
                        /* reset flag if malloc failed */
                        ==   NULL) isaa = 0;
        if (isaa)                  /* have bytemap, so... */
            if ((colormap_raster = (intbyte *)malloc(nbytes))  /* malloc colormap */
                    /* reset flag if malloc failed */
                    ==   NULL) isaa = 0;
        /* ---
         * now generate anti-aliased bytemap and colormap from bitmap
         * ------------------------------------------------------------ */
        if (isaa) {                /*re-check that we're anti-aliasing*/
            /* ---
             * select anti-aliasing algorithm
             * ------------------------------------------------------------ */
            if (!isss)           /* generate bytemap for lowpass */
                switch (aaalgorithm) {          /* choose antialiasing algorithm */
                default:
                    isaa = 0;
                    /* unrecognized algorithm */
                    break;
                case 1:              /* 1 for aalowpass() */
                    if (aalowpass(bp, bytemap_raster, grayscale) /*my own lowpass filter*/
                            /*failed, so turn off anti-aliasing*/
                            ==   0)  isaa = 0;
                    break;
                case 2:              /*2 for netpbm pnmalias.c algorithm*/
                    if (aapnm(bp, bytemap_raster, grayscale) /* pnmalias.c filter */
                            /*failed, so turn off anti-aliasing*/
                            ==   0)  isaa = 0;
                    break;
                case 3:              /*3 for aapnm() based on aagridnum()*/
                    if (aapnmlookup(bp, bytemap_raster, grayscale) /* pnmalias.c filter */
                            /*failed, so turn off anti-aliasing*/
                            ==   0)  isaa = 0;
                    break;
                case 4:              /* 4 for aalookup() table lookup */
                    if (aalowpasslookup(bp, bytemap_raster, grayscale) /* aalookup() */
                            /*failed, so turn off anti-aliasing*/
                            ==   0)  isaa = 0;
                    break;
                } /* --- end-of-switch(aaalgorithm) --- */
            /* ---
             * emit aalookup() pattern# counts/percents diagnostics
             * ------------------------------------------------------------ */
            if (!isquery && msgfp != NULL && msglevel >= 99) { /*emit patternnumcounts*/
                /* init total w,b center counts */
                int pcount0 = 0, pcount1 = 0;
                for (ipattern = 1; ipattern <= 51; ipattern++) { /*each possible pattern*/
                    if (ipattern > 1)          /* ignore all-white squares */
                        /* bump total white centers */
                        pcount0 += patternnumcount0[ipattern];
                    pcount1 += patternnumcount1[ipattern];
                } /* bump total black centers */
                if (pcount0 + pcount1 > 0)      /* have pcounts (using aalookup) */
                    fprintf(msgfp, "  aalookup() patterns excluding#1 white"
                            " (%%'s are in tenths of a percent)...\n");
                for (ipattern = 1; ipattern <= 51; ipattern++) { /*each possible pattern*/
                    int tot = patternnumcount0[ipattern] + patternnumcount1[ipattern];
                    if (tot > 0)           /* this pattern occurs in image */
                        fprintf(msgfp,
                                "  pattern#%2d: %7d(%6.2f%%) +%7d(%6.2f%%) =%7d(%6.2f%%)\n",
                                ipattern, patternnumcount0[ipattern], (ipattern <= 1 ? 999.99 :
                                                                       1000.*((double)patternnumcount0[ipattern]) / ((double)pcount0)),
                                patternnumcount1[ipattern],
                                1000.*((double)patternnumcount1[ipattern]) / ((double)pcount1),
                                tot, (ipattern <= 1 ? 999.99 :
                                      1000.*((double)tot) / ((double)(pcount0 + pcount1))));
                }
                if (pcount0 + pcount1 > 0) /* true when using aalookup() */
                    fprintf(msgfp,
                            "all patterns: %7d          +%7d          =%7d  total pixels\n",
                            pcount0, pcount1, pcount0 + pcount1);
            }
            /* ---
             * finally, generate colors and colormap
             * ------------------------------------------------------------ */
            if (isaa) {              /* we have bytemap_raster */
                ncolors = aacolormap(bytemap_raster, nbytes, colors, colormap_raster);
                if (ncolors < 2) {     /* failed */
                    /* so turn off anti-aliasing */
                    isaa = 0;
                    ncolors = 2;
                }        /* and reset for black&white */
            } /* --- end-of-if(isaa) --- */
            if (isaa && ispbmpgm && ptype > 1) { /* -g2 switch  */
                /*construct arg for write_pbmpgm()*/
                raster pbm_raster;
                pbm_raster.width  = bp->width;
                pbm_raster.height = bp->height;
                pbm_raster.pixsz  = 8;
                pbm_raster.pixmap = (pixbyte *)bytemap_raster;
                type_pbmpgm(&pbm_raster, ptype, pbm_outfile);
            } /*write grayscale file*/
        } /* --- end-of-if(isaa) --- */
    } /* --- end-of-if(isaa) --- */
    /* ------------------------------------------------------------
    display results on msgfp if called from command line (usually for testing)
    ------------------------------------------------------------ */
    if ((!isquery || isqlogging) || msglevel >= 99)  /*command line or debuging*/
        if (!isdumpimage) {         /* don't mix ascii with image dump */
            /* ---
             * display ascii image of rasterize()'s rasterized bitmap
             * ------------------------------------------------------------ */
            if (!isss) {               /* no bitmap for supersampling */
                fprintf(msgfp, "\nAscii dump of bitmap image...\n");
                type_raster(bp, msgfp);
            }      /* emit ascii image of raster */
            /* ---
             * display anti-aliasing results applied to rasterized bitmap
             * ------------------------------------------------------------ */
            if (isaa) {                /* if anti-aliasing applied */
                /* colors[] index */
                int igray;
                /* --- anti-aliased bytemap image --- */
                if (msgfp != NULL && msglevel >= 9) { /* don't usually emit raw bytemap */
                    fprintf(msgfp, "\nHex dump of anti-aliased bytemap, " /*emit bytemap*/
                            "asterisks denote \"black\" bytes (value=%d)...\n", grayscale - 1);
                    type_bytemap(bytemap_raster, grayscale, bp->width, bp->height, msgfp);
                }
                /* --- colormap image --- */
                fprintf(msgfp, "\nHex dump of colormap indexes, " /* emit colormap */
                        "asterisks denote \"black\" bytes (index=%d)...\n", ncolors - 1);
                type_bytemap(colormap_raster, ncolors, bp->width, bp->height, msgfp);
                /* --- rgb values corresponding to colormap indexes */
                fprintf(msgfp, "\nThe %d colormap indexes denote rgb values...", ncolors);
                for (igray = 0; igray < ncolors; igray++) /* show colors[] values */
                    fprintf(msgfp, "%s%2x-->%3d", (igray % 5 ? "   " : "\n"),
                            igray, (int)(colors[ncolors-1] - colors[igray]));
                /* always needs a final newline */
                fprintf(msgfp, "\n");
            } /* --- end-of-if(isaa) --- */
        } /* --- end-of-if(!isquery||msglevel>=9) --- */
    /* ------------------------------------------------------------
    emit xbitmap or gif image, and exit
    ------------------------------------------------------------ */
    if (isquery               /* called from browser (usual) */
            || (isdumpimage && !ispbmpgm)       /* or to emit gif dump of image */
            ||    msglevel >= 99) {         /* or for debugging */
        /* grayscale index */
        int  igray = 0;
        /* ------------------------------------------------------------
        emit GIF image
        ------------------------------------------------------------ */
        /* --- don't use memory buffer if outout file given --- */
        /* reset memory buffer flag */
        if (gif_outfile != NULL) isinmemory = 0;
        /* --- construct contenttype[] buffer containing mime headers --- */
        if (1) {               /* always construct buffer */
            sprintf(contenttype, "Cache-Control: max-age=%d\n", maxage);
            /*sprintf(contenttype+strlen(contenttype),
               "Expires: Fri, 31 Oct 2003 23:59:59 GMT\n" );*/
            /*sprintf(contenttype+strlen(contenttype),
               "Last-Modified: Wed, 15 Oct 2003 01:01:01 GMT\n");*/
            if (abs(valign) < 999)       /* have Vertical-Align: header info*/
                sprintf(contenttype + strlen(contenttype),
                        "Vertical-Align: %d\n", valign);
            sprintf(contenttype + strlen(contenttype),
                    "Content-type: image/gif\n\n");
        }
        /* --- emit mime content-type line --- */
        if (isemitcontenttype      /* content-type lines wanted */
                &&   !isdumpimage         /* don't mix ascii with image dump */
                &&   !isinmemory          /* done below if in memory */
                &&   !iscaching) {        /* done by emitcache() if caching */
            /* emit content-type: header buffer*/
            fputs(contenttype, stdout);
        }
        /* --- write output to memory buffer, possibly for testing --- */
        if (isinmemory             /* want gif written to memory */
                ||   isdumpbuffer)            /*or dump memory buffer for testing*/
            if (gif_outfile == NULL) {        /* and don't already have a file */
                /* init buffer as empty string */
                *gif_buffer = '\000';
                /* zero out buffer */
                memset(gif_buffer, 0, MAXGIFSZ);
                /* and point outfile to buffer */
                gif_outfile = gif_buffer;
                if (isdumpbuffer)          /* buffer dump test requested */
                    isdumpbuffer = 999;
            }       /* so signal dumping to buffer */
        /* --- initialize gifsave library and colors --- */
        if (msgfp != NULL && msglevel >= 999) {
            fprintf(msgfp, "main> calling GIF_Create(*,%d,%d,%d,8)\n",
                    bp->width, bp->height, ncolors);
            fflush(msgfp);
        }
        while (1) {        /* init gifsave lib, and retry if caching fails */
            int status = GIF_Create(gif_outfile, bp->width, bp->height, ncolors, 8);
            /* continue if succeeded */
            if (status == 0) break;
            /* quit if failed */
            if (iscaching == 0) goto end_of_job;
            /* retry without cache file */
            iscaching = 0;
            /* reset isdumpbuffer signal */
            isdumpbuffer = 0;
            /* force in-memory image generation*/
            if (isquery) isinmemory = 1;
            if (isinmemory) {          /* using memory buffer */
                /* emit images to memory buffer */
                gif_outfile = gif_buffer;
                *gif_outfile = '\000';
            }    /* empty string signals buffer */
            else {                /* or */
                /* emit images to stdout */
                gif_outfile = (char *)NULL;
                if (isemitcontenttype) {     /* content-type lines wanted */
                    fprintf(stdout, "Cache-Control: max-age=%d\n", maxage);
                    fprintf(stdout, "Content-type: image/gif\n\n");
                }
            }
        } /* --- end-of-while(1) --- */
        /* background white if all 255 */
        GIF_SetColor(0, bgred, bggreen, bgblue);
        if (!isaa) {               /* just b&w if not anti-aliased */
            /* foreground black if all 0 */
            GIF_SetColor(1, fgred, fggreen, fgblue);
            colors[0] = '\000';
            colors[1] = '\001';
        } /* and set 2 b&w color indexes */
        else
        /* set grayscales for anti-aliasing */
            /* --- anti-aliased, so call GIF_SetColor() for each colors[] --- */
            for (igray = 1; igray < ncolors; igray++) { /* for colors[] values */
                /*--- gfrac goes from 0 to 1.0, as igray goes from 0 to ncolors-1 ---*/
                double gfrac = ((double)colors[igray]) / ((double)colors[ncolors-1]);
                /* --- r,g,b components go from background to foreground color --- */
                int red  = iround(((double)bgred)  + gfrac * ((double)(fgred - bgred))),
                    green = iround(((double)bggreen) + gfrac * ((double)(fggreen - bggreen))),
                    blue = iround(((double)bgblue) + gfrac * ((double)(fgblue - bgblue)));
                /* --- set color index number igray to rgb values gray,gray,gray --- */
                /*set gray,grayer,...,0=black*/
                GIF_SetColor(igray, red, green, blue);
            } /* --- end-of-for(igray) --- */
        /* --- set gif color#0 (background) transparent --- */
        if (istransparent)             /* transparent background wanted */
            /* set transparent background */
            GIF_SetTransparent(0);
        /*flush debugging output*/
        if (msgfp != NULL && msglevel >= 9) fflush(msgfp);
        /* --- emit compressed gif image (to stdout or cache file) --- */
        /* emit gif */
        GIF_CompressImage(0, 0, -1, -1, GetPixel);
        /* close file */
        GIF_Close();
        if (msgfp != NULL && msglevel >= 9) {
            fprintf(msgfp, "main> created gifSize=%d\n", gifSize);
            fflush(msgfp);
        }
        /* --- may need to emit image from cached file or from memory --- */
        if (isquery                /* have an actual query string */
                ||   isdumpimage          /* or dumping image */
                ||   msglevel >= 99) {        /* or debugging */
            /* no headers if dumping image */
            int maxage2 = (isdumpimage ? (-1) : maxage);
            if (iscaching)            /* caching enabled */
                /*emit cached image (hopefully)*/
                emitcache(cachefile, maxage2, valign, 0);
            else if (isinmemory)          /* or emit image from memory buffer*/
                emitcache(gif_buffer, maxage2, valign, 1);
        } /*emitted from memory buffer*/
        /* --- for testing, may need to write image buffer to file --- */
        if (isdumpbuffer > 99)         /* gif image in memory buffer */
            if (gifSize > 0) {            /* and it's not an empty buffer */
                /* dump to mimetex.gif */
                FILE *dumpfp = fopen("mimetex.gif", "wb");
                if (dumpfp != NULL) {      /* file opened successfully */
                    /*write*/
                    fwrite(gif_buffer, sizeof(unsigned char), gifSize, dumpfp);
                    fclose(dumpfp);
                }     /* and close file */
            } /* --- end-of-if(isdumpbuffer>99) --- */
    } /* --- end-of-if(isquery) --- */
    /* --- exit --- */
end_of_job:
    if (!isss)                 /*bytemap raster in sp for supersamp*/
        /*free bytemap_raster*/
        if (bytemap_raster != NULL) free(bytemap_raster);
    /*and colormap_raster*/
    if (colormap_raster != NULL)free(colormap_raster);
    /* free malloced buffer */
    if (0 && gif_buffer != NULL) free(gif_buffer);
    /* and free expression */
    if (1 && sp != NULL) delete_subraster(sp);
    if (msgfp != NULL          /* have message/log file open */
            &&   msgfp != stdout) {       /* and it's not stdout */
        fprintf(msgfp, "mimeTeX> successful end-of-job at %s\n",
                timestamp(tzdelta, 0));
        /* so log separator line */
        fprintf(msgfp, "%s\n", dashes);
        fclose(msgfp);
    }           /* and close logfile */
    /* --- dump memory leaks in debug window if in MS VC++ debug mode --- */
#if defined(_CRTDBG_MAP_ALLOC)
    _CrtDumpMemoryLeaks();
#endif
    /* --- exit() if not running as Windows DLL (see CreateGifFromEq()) --- */
#if !defined(_USRDLL)
    if (errorstatus == 0)        /*user doesn't want errors signalled*/
        /* so reset error status */
        exitstatus = 0;
    exit(exitstatus);
#endif
} /* --- end-of-function main() --- */

/* ==========================================================================
 * Function:    CreateGifFromEq ( expression, gifFileName )
 * Purpose: shortcut method to create GIF file for expression,
 *      with antialising and all other capabilities
 * --------------------------------------------------------------------------
 * Arguments:   expression (I)  char *ptr to null-terminated string
 *              containing LaTeX expression to be rendred
 *      gifFileName (I) char *ptr to null-terminated string
 *              containing name of output gif file
 * --------------------------------------------------------------------------
 * Returns: ( int )     exit value from main (0 if successful)
 * --------------------------------------------------------------------------
 * Notes:     o This function is the entry point when mimeTeX is built
 *      as a Win32 DLL rather then a standalone app or CGI
 *        o Contributed to mimeTeX by Shital Shah.  See his homepage
 *        http://www.shitalshah.com
 *        o Shital discusses the mimeTeX Win32 DLL project at
 *        http://www.codeproject.com/dotnet/Eq2Img.asp
 *      and you can download his latest code from
 *        http://www.shitalshah.com/dev/eq2img_all.zip
 * ======================================================================= */
/* --- include function to expose Win32 DLL to outside world --- */
#if defined(_USRDLL)
extern _declspec(dllexport)int _cdecl
CreateGifFromEq(char *expression, char *gifFileName);
#endif
/* --- entry point --- */
int CreateGifFromEq(char *expression, char *gifFileName)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* main() akways returns an int */
    int main();
    /* --- set constants --- */
    /* count of args supplied to main() */
    int argc = 4;
    char    *argv[5] = {    /* command line args to run with -e option */ "MimeTeXWin32DLL", "-e", /* constant args */
                       /*gifFileName, expression,*/ NULL, NULL, NULL
                       };
    /* --- set argv[]'s not computable at load time --- */
    /* args are -e gifFileName */
    argv[2] = gifFileName;
    /* and now  -e gifFileName expression */
    argv[3] = expression;
    /* ------------------------------------------------------------
    Run mimeTeX in command-line mode with -e (export) option, and then return
    ------------------------------------------------------------ */
    return  main(argc, argv
#ifdef DUMPENVP
                 , NULL
#endif
                ) ;
} /* --- end-of-function CreateGifFromEq() --- */


/* ==========================================================================
 * Function:    ismonth ( char *month )
 * Purpose: returns 1 if month contains current month "jan"..."dec".
 * --------------------------------------------------------------------------
 * Arguments:   month (I)   char * containing null-terminated string
 *              in which "jan"..."dec" is (putatively)
 *              contained as a substring.
 * --------------------------------------------------------------------------
 * Returns: ( int )     1 if month contains current month,
 *              0 otherwise
 * --------------------------------------------------------------------------
 * Notes:     o There's a three day "grace period", e.g., Dec 3 mtaches Nov.
 * ======================================================================= */
/* --- entry point --- */
int ismonth(char *month)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /*1 if month contains current month*/
    int isokay = 0;
    /*long  time_val = 0L;*/        /* binary value returned by time() */
    /* binary value returned by time() */
    time_t  time_val = (time_t)(0);
    /* interpret time_val */
    struct tm *tmstruct = (struct tm *)NULL, *localtime();
    /* current month 1-12 and day 1-31 */
    int imonth, mday;
    /* grace period */
    int ngrace = 3;
    char    lcmonth[128] = "\000";
    /* lowercase month */
    int i = 0;
    static  char *months[] = {
        /* month must contain current one */
        "dec", "jan", "feb", "mar", "apr", "may", "jun",
        "jul", "aug", "sep", "oct", "nov", "dec", "jan"
    };
    /* ------------------------------------------------------------
    get current date:time info, and check month
    ------------------------------------------------------------ */
    /* --- lowercase input month --- */
    if (month != NULL)           /* check that we got input */
        for (i = 0; i < 120 && *month != '\000'; i++, month++) /* go thru month chars */
            /* lowerase each char in month */
            lcmonth[i] = tolower(*month);
    /* must be invalid input */
    if (i < 2) goto end_of_job;
    /* null-terminate lcmonth[] */
    lcmonth[i] = '\000';
    /* --- get current date:time --- */
    /* get date and time */
    time((time_t *)(&time_val));
    /* interpret time_val */
    tmstruct = localtime((time_t *)(&time_val));
    /* --- month and day  --- */
    /* 1=jan ... 12=dec */
    imonth = 1 + (int)(tmstruct->tm_mon);
    /* 1-31 */
    mday = (int)(tmstruct->tm_mday);
    if (imonth < 1 || imonth > 12    /* quit if month out-of-range */
            /* or date out of range */
            ||   mday < 0 || mday > 31) goto end_of_job;
    /* --- check input month against current date --- */
    /* current month */
    if (strstr(lcmonth, months[imonth]) != NULL) isokay = 1;
    if (mday <= ngrace)              /* 1-3 within grace period */
        /* last month */
        if (strstr(lcmonth, months[imonth-1]) != NULL) isokay = 1;
    if (mday >= 31 - ngrace)     /* 28-31 within grace period */
        /* next month */
        if (strstr(lcmonth, months[imonth+1]) != NULL) isokay = 1;
end_of_job:
    /*1 if month contains current month*/
    return (isokay);
} /* --- end-of-function ismonth() --- */


/* ==========================================================================
 * Function:    logger ( fp, msglevel, message, logvars )
 * Purpose: Logs the environment variables specified in logvars
 *      to fp if their msglevel is >= the passed msglevel.
 * --------------------------------------------------------------------------
 * Arguments:   fp (I)      FILE * to file containing log
 *      msglevel (I)    int containing logging message level
 *      message (I) char * to optional message, or NULL
 *      logvars (I) logdata * to array of environment variables
 *              to be logged
 * --------------------------------------------------------------------------
 * Returns: ( int )     number of variables from logvars
 *              that were actually logged
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int logger(FILE *fp, int msglevel, char *message, logdata *logvars)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* logvars[] index, #vars logged */
    int ilog = 0, nlogged = 0;
    /* timestamp logged */
    char    *timestamp();
    /* getenv(name) to be logged */
    char    *value = NULL;
    /* ------------------------------------------------------------
    Log each variable
    ------------------------------------------------------------ */
    /*emit timestamp before first var*/
    fprintf(fp, "%s\n", timestamp(tzdelta, 0));
    if (message != NULL)             /* optional message supplied */
        /* emit caller-supplied message */
        fprintf(fp, "  MESSAGE = %s\n", message);
    if (logvars != (logdata *)NULL)      /* have logvars */
        for (ilog = 0; logvars[ilog].name != NULL; ilog++)  /* till end-of-table */
            if (msglevel >= logvars[ilog].msglevel)   /* check msglevel for this var */
                if ((value = getenv(logvars[ilog].name)) /* getenv(name) to be logged */
                        != NULL) {               /* check that name exists */
                    fprintf(fp, "  %s = %.*s\n",    /* emit variable name = value */
                            logvars[ilog].name, logvars[ilog].maxlen, value);
                    /* bump #vars logged */
                    nlogged++;
                } /* --- end-of-for(ilog) --- */
    /* back to caller */
    return (nlogged);
} /* --- end-of-function logger() --- */


/* ==========================================================================
 * Function:    emitcache ( cachefile, maxage, valign, isbuffer )
 * Purpose: dumps bytes from cachefile to stdout
 * --------------------------------------------------------------------------
 * Arguments:   cachefile (I)   pointer to null-terminated char string
 *              containing full path to file to be dumped,
 *              or contains buffer of bytes to be dumped
 *      maxage (I)  int containing maxage, in seconds, for
 *              http header, or -1 to not emit headers
 *      valign (I)  int containing Vertical-Align:, in pixels,
 *              for http header, or <= -999 to not emit
 *      isbuffer (I)    1 if cachefile is buffer of bytes to be
 *              dumped
 * --------------------------------------------------------------------------
 * Returns: ( int )     #bytes dumped (0 signals error)
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int emitcache(char *cachefile, int maxage, int valign, int isbuffer)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* read cache file */
    int nbytes = gifSize, readcachefile();
    /* emit cachefile to stdout */
    FILE    *emitptr = stdout;
    /* bytes from cachefile */
    unsigned char buffer[MAXGIFSZ+1];
    /* ptr to buffer */
    unsigned char *buffptr = buffer;
    /* true to emit Vertical-Align: */
    int isvalign = (abs(valign) < 999 ? 1 : 0);
    /* ------------------------------------------------------------
    initialization
    ------------------------------------------------------------ */
    /* --- check that files opened okay --- */
    if (emitptr == (FILE *)NULL)         /* failed to open emit file */
        /* so return 0 bytes to caller */
        goto end_of_job;
    /* --- read the file if necessary --- */
    if (isbuffer) {              /* cachefile is buffer */
        /* so reset buffer pointer */
        buffptr = (unsigned char *)cachefile;
    }
    else {                  /* cachefile is file name */
        if ((nbytes = readcachefile(cachefile, buffer)) /* read the file */
                < 1) goto end_of_job;
    }      /* quit if file not read */
    /* --- first emit http headers if requested --- */
    if (isemitcontenttype            /* content-type lines enabled */
            &&   maxage >= 0)           /* caller wants http headers */
    {
    /* --- emit mime content-type line --- */
        fprintf(emitptr, "Cache-Control: max-age=%d\n", maxage);
        fprintf(emitptr, "Content-Length: %d\n", nbytes);
        if (isvalign)             /* Vertical-Align: header wanted */
            fprintf(emitptr, "Vertical-Align: %d\n", valign);
        fprintf(emitptr, "Content-type: image/gif\n\n");
    }
    /* ------------------------------------------------------------
    set stdout to binary mode (for Windows)
    ------------------------------------------------------------ */
    /* emitptr = fdopen(STDOUT_FILENO,"wb"); */  /* doesn't work portably, */
#ifdef WINDOWS              /* so instead... */
#ifdef HAVE_SETMODE           /* prefer (non-portable) setmode() */
    if (setmode(fileno(stdout), O_BINARY)     /* windows specific call */
            /* handle error */
            == -1) ;/* sets stdout to binary mode */
#else
/* setmode() not available */
#if 1
    /* freopen() stdout binary */
    freopen("CON", "wb", stdout);
#else
    /* fdopen() stdout binary */
    stdout = fdopen(STDOUT_FILENO, "wb");
#endif
#endif
#endif
    /* ------------------------------------------------------------
    emit bytes from cachefile
    ------------------------------------------------------------ */
    /* --- write bytes to stdout --- */
    if (fwrite(buffptr, sizeof(unsigned char), nbytes, emitptr) /* write buffer */
            <    nbytes)                /* failed to write all bytes */
        /* reset total count to 0 */
        nbytes = 0;
end_of_job:
    /* back with #bytes emitted */
    return (nbytes);
} /* --- end-of-function emitcache() --- */


/* ==========================================================================
 * Function:    readcachefile ( cachefile, buffer )
 * Purpose: read cachefile into buffer
 * --------------------------------------------------------------------------
 * Arguments:   cachefile (I)   pointer to null-terminated char string
 *              containing full path to file to be read
 *      buffer (O)  pointer to unsigned char string
 *              returning contents of cachefile
 *              (max 64000 bytes)
 * --------------------------------------------------------------------------
 * Returns: ( int )     #bytes read (0 signals error)
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int readcachefile(char *cachefile, unsigned char *buffer)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /*open cachefile for binary read*/
    FILE    *cacheptr = fopen(cachefile, "rb");
    /* bytes from cachefile */
    unsigned char cachebuff[64];
    int buflen = 32,            /* #bytes we try to read from file */
        nread = 0,          /* #bytes actually read from file */
        maxbytes = MAXGIFSZ,        /* max #bytes returned in buffer */
                                    /* total #bytes read */
        nbytes = 0;
    /* ------------------------------------------------------------
    initialization
    ------------------------------------------------------------ */
    /* --- check that files opened okay --- */
    /*failed to open cachefile*/
    if (cacheptr == (FILE *)NULL) goto end_of_job;
    /* --- check that output buffer provided --- */
    /* no buffer */
    if (buffer == (unsigned char *)NULL) goto end_of_job;
    /* ------------------------------------------------------------
    read bytes from cachefile
    ------------------------------------------------------------ */
    while (1) {
        /* --- read bytes from cachefile --- */
        /* read */
        nread = fread(cachebuff, sizeof(unsigned char), buflen, cacheptr);
        if (nbytes + nread > maxbytes)     /* block too big for buffer */
            /* so truncate it */
            nread = maxbytes - nbytes;
        /* no bytes left in cachefile */
        if (nread < 1) break;
        /* --- store bytes in buffer --- */
        /* copy current block to buffer */
        memcpy(buffer + nbytes, cachebuff, nread);
        /* --- ready to read next block --- */
        /* bump total #bytes emitted */
        nbytes += nread;
        /* no bytes left in cachefile */
        if (nread < buflen) break;
        /* avoid buffer overflow */
        if (nbytes >= maxbytes) break;
    } /* --- end-of-while(1) --- */
end_of_job:
    /* close file if opened */
    if (cacheptr != NULL) fclose(cacheptr);
    /* back with #bytes emitted */
    return (nbytes);
} /* --- end-of-function readcachefile() --- */


