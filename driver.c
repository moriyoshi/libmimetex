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

#ifndef FORMLEVEL
#define FORMLEVEL LOGLEVEL        /*mctx.msglevel if called from html form*/
#endif

/* --- anti-aliasing flags (needed by GetPixel() as well as main()) --- */
#ifdef AA               /* if anti-aliasing requested */
#define ISAAVALUE 1           /* turn flag on */
#else
#define ISAAVALUE 0           /* else turn flag off */
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

static int iscaching = ISCACHING;  /* true if caching images */
static char cachepath[256] = CACHEPATH;  /* relative path to cached files */
static int isemitcontenttype = 1;  /* true to emit mime content-type */
static int isnomath = 0;       /* true to inhibit math mode */
static int seclevel        = SECURITY;    /* security level */
static int inputseclevel   = INPUTSECURITY; /* \input{} security level */
static int counterseclevel = COUNTERSECURITY; /* \counter{} security level */
static int environseclevel = ENVIRONSECURITY; /* \environ{} security level */
static char pathprefix[256] = { '\000' }; /*prefix for \input,\counter paths*/
static int exitstatus = 0;
static char exprprefix[256] = "\000";  /* prefix prepended to expressions */
static int ninputcmds = 0;     /* # of \input commands processed */
static int errorstatus = ERRORSTATUS;  /* exit status if error encountered*/
static int isplusblank = -1;  /*interpret +'s in query as blanks?*/
static int tzdelta = 0;

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
 * Function:    urlprune ( url, n )
 * Purpose: Prune http://abc.def.ghi.com/etc into abc.def.ghi.com
 *      (if n=2 only ghi.com is returned, or if n=-1 only "ghi")
 * --------------------------------------------------------------------------
 * Arguments:   url (I)     char * to null-terminated string
 *              containing url to be pruned
 *      n (i)       int containing number of levels retained
 *              in pruned url.  If n<0 its abs() is used,
 *              but the topmost level (usually .com, .org,
 *              etc) is omitted.  That is, if n=2 would
 *              return "ghi.com" then n=-1 returns "ghi".
 *              n=0 retains all levels.
 * --------------------------------------------------------------------------
 * Returns: ( char * )  pointer to (static) null-terminated string
 *              containing pruned url with the first n
 *              top-level domain, e.g., for n=2,
 *              http://abc.def.ghi.com/etc returns ghi.com,
 *              or an empty string "\000" for any error
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
static char *urlprune(char *url, int n)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* pruned url returned to caller */
    static  char pruned[1024];
    char    *purl = /*NULL*/pruned;     /* ptr to pruned, init for error */
    /* delimiter separating components */
    char    *delim = NULL;
    /* lowercase a string */
    char    *strnlower();
    /*true to truncate .com from pruned*/
    int istruncate = (n < 0 ? 1 : 0);
    /* number of dots found in url */
    int ndots = 0;
    /* ------------------------------------------------------------
    prune the url
    ------------------------------------------------------------ */
    /* --- first check input --- */
    /* init for error */
    *pruned = '\000';
    /* missing input, so return NULL */
    if (isempty(url)) goto end_of_job;
    /* flip n positive */
    if (n < 0) n = (-n);
    /* retain all levels of url */
    if (n == 0) n = 999;
    /* --- preprocess url --- */
    /* copy url to our static buffer */
    strninit(pruned, url, 999);
    /* lowercase it and... */
    strlower(pruned);
    trimwhite(pruned);
    /*remove leading/trailing whitespace*/
    /* --- first remove leading http:// --- */
    if ((delim = strstr(pruned, "://")) != NULL) /* found http:// or ftp:// etc */
        if (((int)(delim - pruned)) <= 8) {    /* make sure it's a prefix */
            strcpy(pruned, delim + 3);  /* squeeze out leading http:// */
            trimwhite(pruned);
        }        /*remove leading/trailing whitespace*/
    /* --- next remove leading www. --- */
    if ((delim = strstr(pruned, "www.")) != NULL) /* found www. */
        if (((int)(delim - pruned)) == 0) {    /* make sure it's the leading chars*/
            /* squeeze out leading www. */
            strcpy(pruned, delim + 4);
            trimwhite(pruned);
        }        /*remove leading/trailing whitespace*/
    /* --- finally remove leading / and everything following it --- */
    if ((delim = strchr(pruned, '/')) != NULL) /* found first / */
        *delim = '\000';          /* null-terminate url at first / */
    /* nothing left in url */
    if (isempty(pruned)) goto end_of_job;
    /* --- count dots from back of url --- */
    /*ptr to '\000' terminating pruned*/
    delim = pruned + strlen(pruned);
    while (((int)(delim - pruned)) > 0) {    /* don't back up before first char */
        /* ptr to preceding character */
        delim--;
        /* not a dot, so keep looking */
        if (*delim != '.') continue;
        /* count another dot found */
        ndots++;
        if (istruncate) {              /* remove trailing .com */
            /* don't truncate any more dots */
            istruncate = 0;
            /* truncate pruned url */
            *delim = '\000';
            ndots = 0;
        }            /* and reset dot count */
        if (ndots >= n) {              /* have all requested levels */
            /* squeeze out any leading levels */
            strcpy(pruned, delim + 1);
            break;
        }                /* and we're done */
    } /* --- end-of-while() --- */
    /*completed okay, return pruned url*/
    purl = pruned;
end_of_job:
    /* back with pruned url */
    return (purl);
} /* --- end-of-function urlprune() --- */

/* ==========================================================================
 * Function:    urlncmp ( url1, url2, n )
 * Purpose: Compares the n topmost levels of two urls
 * --------------------------------------------------------------------------
 * Arguments:   url1 (I)    char * to null-terminated string
 *              containing url to be compared with url2
 *      url2 (I)    char * to null-terminated string
 *              containing url to be compared with url1
 *      n (I)       int containing number of top levels
 *              to compare, or 0 to compare them all.
 *              n<0 compares that many top levels excluding
 *              the last, i.e., for n=-1, xxx.com and xxx.org
 *              would be considered a match
 * --------------------------------------------------------------------------
 * Returns: ( int )     1 if url's match, or
 *              0 if not.
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
static int urlncmp(char *url1, char *url2, int n)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    char *prune = NULL, /* prune url's */
         prune1[1024], prune2[1024]; /* pruned copies of url1,url2 */
    /* true if url's match */
    int ismatch = 0;
    /* ------------------------------------------------------------
    prune url's and compare the pruned results
    ------------------------------------------------------------ */
    /* --- check input --- */
    if (isempty(url1)            /*make sure both url1,url2 supplied*/
            /* missing input, so return 0 */
            ||   isempty(url2)) goto end_of_job;
    /* --- prune url's --- */
    /* ptr to pruned version of url1 */
    prune = urlprune(url1, n);
    /* some problem with url1 */
    if (isempty(prune)) goto end_of_job;
    /* local copy of pruned url1 */
    strninit(prune1, prune, 999);
    /* ptr to pruned version of url2 */
    prune = urlprune(url2, n);
    /* some problem with url2 */
    if (isempty(prune)) goto end_of_job;
    /* local copy of pruned url2 */
    strninit(prune2, prune, 999);
    /* --- compare pruned url's --- */
    if (strcmp(prune1, prune2) == 0)     /* pruned url's are identical */
        /* signal match to caller */
        ismatch = 1;
end_of_job:
    /*back with #matching url components*/
    return (ismatch);
} /* --- end-of-function urlncmp() --- */

/* ==========================================================================
 * Functions:   int  unescape_url ( char *url, int isescape )
 *      char x2c ( char *what )
 * Purpose: unescape_url replaces 3-character sequences %xx in url
 *          with the single character represented by hex xx.
 *      x2c returns the single character represented by hex xx
 *          passed as a 2-character sequence in what.
 * --------------------------------------------------------------------------
 * Arguments:   url (I)     char * containing null-terminated
 *              string with embedded %xx sequences
 *              to be converted.
 *      isescape (I)    int containing 1 to _not_ unescape
 *              \% sequences (0 would be NCSA default)
 *      what (I)    char * whose first 2 characters are
 *              interpreted as ascii representations
 *              of hex digits.
 * --------------------------------------------------------------------------
 * Returns: ( int )     unescape_url always returns 0.
 *      ( char )    x2c returns the single char
 *              corresponding to hex xx passed in what.
 * --------------------------------------------------------------------------
 * Notes:     o These two functions were taken verbatim from util.c in
 *   ftp://ftp.ncsa.uiuc.edu/Web/httpd/Unix/ncsa_httpd/cgi/ncsa-default.tar.Z
 *        o Not quite "verbatim" -- I added the "isescape logic" 4-Dec-03
 *      so unescape_url() can be safely applied to input which may or
 *      may not have been url-encoded.  (Note: currently, all calls
 *      to unescape_url() pass iescape=0, so it's not used.)
 *        o Added +++'s to blank xlation on 24-Sep-06
 *        o Added ^M,^F,etc to blank xlation 0n 01-Oct-06
 * ======================================================================= */
/* --- entry point --- */
static int unescape_url(char *url, int isescape)
{
    int x = 0, y = 0, prevescape = 0, gotescape = 0;
    /* true to xlate plus to blank */
    int xlateplus = (isplusblank == 1 ? 1 : 0);
    /* replace + with blank, if needed */
    int strreplace();
    char x2c();
    static char *hex = "0123456789ABCDEFabcdef";
    /* ---
     * xlate ctrl chars to blanks
     * ------------------------------------------------------------ */
    if (1) {                 /* xlate ctrl chars to blanks */
        char *ctrlchars = "\n\t\v\b\r\f\a\015";
        /*initial segment with ctrlchars*/
        int  seglen = strspn(url, ctrlchars);
        /* total length of url string */
        int  urllen = strlen(url);
        /* --- first, entirely remove ctrlchars from beginning and end --- */
        if (seglen > 0) {          /*have ctrlchars at start of string*/
            /* squeeze out initial ctrlchars */
            strcpy(url, url + seglen);
            urllen -= seglen;
        }     /* string is now shorter */
        while (--urllen >= 0)          /* now remove ctrlchars from end */
            if (isthischar(url[urllen], ctrlchars))  /* ctrlchar at end */
                /* re-terminate string before it */
                url[urllen] = '\000';
            /* or we're done */
            else break;
        /* length of url string */
        urllen++;
        /* --- now, replace interior ctrlchars with ~ blanks --- */
        while ((seglen = strcspn(url, ctrlchars)) < urllen) /*found a ctrlchar*/
            /* replace ctrlchar with ~ */
            url[seglen] = '~';
    } /* --- end-of-if(1) --- */
    /* ---
     * xlate +'s to blanks if requested or if deemed necessary
     * ------------------------------------------------------------ */
    if (isplusblank == (-1)) {   /*determine whether or not to xlate*/
        char *searchfor[] = { " ", "%20", "%2B", "%2b", "+++", "++",
                              "+=+", "+-+", NULL
                            };
        int  isearch = 0,         /* searchfor[] index */
                       /*#occurrences*/
                       nfound[11] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
        /* --- locate occurrences of searchfor[] strings in url --- */
        for (isearch = 0; searchfor[isearch] != NULL; isearch++) {
            /* start search at beginning */
            char *psearch = url;
            /* init #occurrences count */
            nfound[isearch] = 0;
            while ((psearch = strstr(psearch, searchfor[isearch])) != NULL) {
                /* count another occurrence */
                nfound[isearch] += 1;
                psearch += strlen(searchfor[isearch]);
            } /*resume search after it*/
        } /* --- end-of-for(isearch) --- */
        /* --- apply some common-sense logic --- */
        if (nfound[0] + nfound[1] > 0)     /* we have actual " "s or "%20"s */
            /* so +++'s aren't blanks */
            isplusblank = xlateplus = 0;
        if (nfound[2] + nfound[3] > 0) {   /* we have "%2B" for +++'s */
            if (isplusblank != 0)        /* and haven't disabled xlation */
                /* so +++'s are blanks */
                isplusblank = xlateplus = 1;
            else
            /* we have _both_ "%20" and "%2b" */
                xlateplus = 0;
        }      /* tough call */
        if (nfound[4] + nfound[5] > 0  /* we have multiple ++'s */
                ||   nfound[6] + nfound[7] > 0)   /* or we have a +=+ or +-+ */
            if (isplusblank != 0)        /* and haven't disabled xlation */
                /* so xlate +++'s to blanks */
                xlateplus = 1;
    } /* --- end-of-if(isplusblank==-1) --- */
    if (xlateplus > 0) {         /* want +'s xlated to blanks */
        char *xlateto[] = { "", " ", " ", " + ", " ", " ", " ", " ", " " };
        while (xlateplus > 0) {        /* still have +++'s to xlate */
            /* longest +++ string */
            char plusses[99] = "++++++++++++++++++++";
            /* null-terminate +++'s */
            plusses[xlateplus] = '\000';
            /* xlate +++'s */
            strreplace(url, plusses, xlateto[xlateplus], 0);
            /* next shorter +++ string */
            xlateplus--;
        } /* --- end-of-while(xlateplus>0) --- */
    } /* --- end-of-if(xlateplus) --- */
    /* don't iterate this xlation */
    isplusblank = 0;
    /* ---
     * xlate %nn to corresponding char
     * ------------------------------------------------------------ */
    for (; url[y]; ++x, ++y) {
        gotescape = prevescape;
        prevescape = (url[x] == '\\');
        if ((url[x] = url[y]) == '%')
            if (!isescape || !gotescape)
                if (isthischar(url[y+1], hex)
                        && isthischar(url[y+2], hex)) {
                    url[x] = x2c(&url[y+1]);
                    y += 2;
                }
    }
    url[x] = '\0';
    return 0;
} /* --- end-of-function unescape_url() --- */

/* ==========================================================================
 * Function:    rasteditfilename ( filename )
 * Purpose:    edits filename to remove security problems,
 *        e.g., removes all ../'s and ..\'s.
 * --------------------------------------------------------------------------
 * Arguments:    filename (I)    char * to null-terminated string containing
 *                name of file to be edited
 * --------------------------------------------------------------------------
 * Returns:    ( char * )    pointer to edited filename,
 *                or empty string "\000" if any problem
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
static char *rasteditfilename(char *filename)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    static    char editname[2050];        /*edited filename returned to caller*/
    int    isprefix = (*pathprefix=='\000'?0:1); /* true if paths have prefix */
    /* ------------------------------------------------------------
    edit filename
    ------------------------------------------------------------ */
    /* --- first check filename arg --- */
    *editname = '\000';            /* init edited name as empty string*/
    if (filename == (char *)NULL)
        /* no filename arg */
        goto end_of_job;
    if (*filename == '\000')
        /* filename is an empty string */
        goto end_of_job;
    /* --- init edited filename --- */
    strcpy(editname, filename);        /* init edited name as input name */
    compress(editname, ' ');            /* remove embedded blanks */
    /* --- remove leading or embedded ....'s --- */
    while (strreplace(editname,"....",NULL,0) > 0);  /* squeeze out ....'s */
    /* --- remove leading / and \ and dots (and blanks) --- */
    if (*editname != '\000') {
        /* still have chars in filename */
        while (isthischar(*editname, " ./\\"))
            /* absolute paths invalid so flush leading / or \ (or ' ')*/
            strcpy(editname, editname + 1);
    }
    if (*editname == '\000')
        /* no chars left in filename */
        goto end_of_job;
    /* --- remove leading or embedded ../'s and ..\'s --- */
    while (strreplace(editname, "../", NULL, 0) > 0);  /* squeeze out ../'s */
    while (strreplace(editname, "..\\", NULL, 0) > 0); /* and ..\'s */
    while (strreplace(editname, "../", NULL, 0) > 0);  /* and ../'s again */
    /* --- prepend path prefix (if compiled with -DPATHPREFIX) --- */
    if (isprefix && *editname!='\000')    /* filename is preceded by prefix */
        strchange(0, editname, pathprefix);    /* so prepend prefix */
end_of_job:
    /* back with edited filename */
    return ( editname );
}
/* --- end-of-function rasteditfilename() --- */

/* ========================================================================== * Function:    sanitize_pathname ( filename )
 * Purpose: edits filename to remove security problems,
 *      e.g., removes all ../'s and ..\'s.
 * --------------------------------------------------------------------------
 * Arguments:   filename (I)    char * to null-terminated string containing
 *              name of file to be edited
 * --------------------------------------------------------------------------
 * Returns: ( char * )  pointer to edited filename,
 *              or empty string "\000" if any problem
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
char    *sanitize_pathname(char *filename)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    int strreplace();

    /*edited filename returned to caller*/
    static  char editname[2050];
    /* prepend pathprefix if necessary */
    /* true if paths have prefix */
    int isprefix = (*pathprefix == '\000' ? 0 : 1);
    /* ------------------------------------------------------------
    edit filename
    ------------------------------------------------------------ */
    /* --- first check filename arg --- */
    /* init edited name as empty string*/
    *editname = '\000';
    /* no filename arg */
    if (filename == (char *)NULL) goto end_of_job;
    /* filename is an empty string */
    if (*filename == '\000') goto end_of_job;
    /* --- init edited filename --- */
    /* init edited name as input name */
    strcpy(editname, filename);
    /* remove embedded blanks */
    compress(editname, ' ');
    /* --- remove leading or embedded ....'s --- */
    /* squeeze out ....'s */
    while (strreplace(editname, "....", NULL, 0) > 0) ;
    /* --- remove leading / and \ and dots (and blanks) --- */
    if (*editname != '\000')         /* still have chars in filename */
        while (isthischar(*editname, " ./\\"))  /* absolute paths invalid */
            strcpy(editname, editname + 1);  /* so flush leading / or \ (or ' ')*/
    /* no chars left in filename */
    if (*editname == '\000') goto end_of_job;
    /* --- remove leading or embedded ../'s and ..\'s --- */
    while (strreplace(editname, "../", NULL, 0) > 0) ; /* squeeze out ../'s */
    /* and ..\'s */
    while (strreplace(editname, "..\\", NULL, 0) > 0) ;
    while (strreplace(editname, "../", NULL, 0) > 0) ; /* and ../'s again */
    /* --- prepend path prefix (if compiled with -DPATHPREFIX) --- */
    if (isprefix && *editname != '\000') /* filename is preceded by prefix */
        /* so prepend prefix */
        strchange(0, editname, pathprefix);
end_of_job:
    /* back with edited filename */
    return (editname);
} /* --- end-of-function sanitize_pathname() --- */

/* ==========================================================================
 * Function:    rastopenfile ( filename, mode )
 * Purpose: Opens filename[.tex] in mode, returning FILE *
 * --------------------------------------------------------------------------
 * Arguments:   filename (I/O)  char * to null-terminated string containing
 *              name of file to open (preceded by path
 *              relative to mimetex executable)
 *              If fopen() fails, .tex appeneded,
 *              and returned if that fopen() succeeds
 *      mode (I)    char * to null-terminated string containing
 *              fopen() mode
 * --------------------------------------------------------------------------
 * Returns: ( FILE * )  pointer to opened file, or NULL if error
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
FILE    *rastopenfile(mimetex_ctx *mctx, char *filename, char *mode)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    char *sanitize_pathname();

    FILE    *fp = (FILE *)NULL; /*file pointer to opened filename*/
    char    texfile[2050] = "\000",     /* local, edited copy of filename */
            amode[512] = "r"; /* test open mode if arg mode=NULL */
    /* true of mode!=NULL */
    int ismode = 0;
    /* ------------------------------------------------------------
    Check mode and open file
    ------------------------------------------------------------ */
    /* --- edit filename --- */
    /*edited copy of filename*/
    strncpy(texfile, sanitize_pathname(filename), 2047);
    /* make sure it's null terminated */
    texfile[2047] = '\000';
    /* --- check mode --- */
    if (mode != (char *)NULL)        /* caller passed mode arg */
        if (*mode != '\000') {          /* and it's not an empty string */
            /* so flip mode flag true */
            ismode = 1;
            /* and replace "r" with caller's */
            strncpy(amode, mode, 254);
            /* make sure it's null terminated */
            amode[254] = '\000';
            compress(amode, ' ');
        }      /* remove embedded blanks */
    /* --- open filename or filename.tex --- */
    if (strlen(texfile) > 1)         /* make sure we got actual filename*/
        if ((fp = fopen(texfile, amode))   /* try opening given filename */
                ==   NULL) {              /* failed to open given filename */
            /* signal possible filename error */
            strcpy(filename, texfile);
            /* but first try adding .tex */
            strcat(texfile, ".tex");
            if ((fp = fopen(texfile, amode)) /* now try opening filename.tex */
                    !=   NULL)              /* filename.tex succeeded */
                strcpy(filename, texfile);
        }   /* replace caller's filename */
    /* --- close file if only opened to check name --- */
    if (!ismode && fp != NULL)       /* no mode, so just checking */
        /* close file, fp signals success */
        fclose(fp);
    /* --- return fp or NULL to caller --- */
    /*end_of_job:*/
    if (mctx->msglevel >= 9 && mctx->msgfp != NULL) { /* debuging */
        fprintf(mctx->msgfp, "rastopenfile> returning fopen(%s,%s) = %s\n",
                filename, amode, (fp == NULL ? "NULL" : "Okay"));
        fflush(mctx->msgfp);
    }
    /* return fp or NULL to caller */
    return (fp);
} /* --- end-of-function rastopenfile() --- */

/* ==========================================================================
 * Function:    rastreadfile ( filename, islock, tag, value )
 * Purpose: Read filename, returning value as string
 *      between <tag>...</tag> or entire file if tag=NULL passed.
 * --------------------------------------------------------------------------
 * Arguments:   filename (I)    char * to null-terminated string containing
 *              name of file to read (preceded by path
 *              relative to mimetex executable)
 *      islock (I)  int containing 1 to lock file while reading
 *              (hopefully done by opening in "r+" mode)
 *      tag (I)     char * to null-terminated string containing
 *              html-like tagname.  File contents between
 *              <tag> and </tag> will be returned, or
 *              entire file if tag=NULL passed.
 *      value (O)   char * returning value between <tag>...</tag>
 *              or entire file if tag=NULL.
 * --------------------------------------------------------------------------
 * Returns: ( int )     1=okay, 0=some error
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int rastreadfile(mimetex_ctx *mctx, char *filename, int islock, char *tag, char *value)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* pointer to opened filename */
    FILE    *fp = (FILE *)NULL;
    char    texfile[1024] = "\000",     /* local copy of input filename */
            text[MAXLINESZ+1]; /* line from input file */
    char    *tagp, tag1[1024], tag2[1024];  /* left <tag> and right <tag/> */
    /* #chars in value, max allowed */
    int vallen = 0, maxvallen = MAXFILESZ;
    /* status returned, 1=okay */
    int status = (-1);
    /* tag we're looking for */
    int tagnum = 0;
    /*int   islock = 1;*/           /* true to lock file */
    /* ------------------------------------------------------------
    Open file
    ------------------------------------------------------------ */
    /* --- first check output arg --- */
    /* no output buffer supplied */
    if (value == (char *)NULL) goto end_of_job;
    /* init buffer with empty string */
    *value = '\000';
    /* --- open filename or filename.tex --- */
    if (filename != (char *)NULL) {      /* make sure we got filename arg */
        /* local copy of filename */
        strncpy(texfile, filename, 1023);
        /* make sure it's null terminated */
        texfile[1023] = '\000';
        fp = rastopenfile(mctx, texfile, (islock ? "r+" : "r"));
    } /* try opening it */
    /* --- check that file opened --- */
    if (fp == (FILE *)NULL) {        /* failed to open file */
        sprintf(value, "{\\normalsize\\rm[file %s?]}", texfile);
        goto end_of_job;
    }          /* return error message to caller */
    /* file opened successfully */
    status = 0;
    /* start at beginning of file */
    if (islock) rewind(fp);
    /* ------------------------------------------------------------
    construct <tag>'s
    ------------------------------------------------------------ */
    if (tag != (char *)NULL)         /* caller passed tag arg */
        if (*tag != '\000') {           /* and it's not an empty string */
            strcpy(tag1, "<");
            strcpy(tag2, "</"); /* begin with < and </ */
            strcat(tag1, tag);
            /* followed by caller's tag */
            strcat(tag2, tag);
            strcat(tag1, ">");
            /* ending both tags with > */
            strcat(tag2, ">");
            compress(tag1, ' ');
            /* remove embedded blanks */
            compress(tag2, ' ');
            tagnum = 1;
        }           /* signal that we have tag */
    /* ------------------------------------------------------------
    Read file, concatnate lines
    ------------------------------------------------------------ */
    while (fgets(text, MAXLINESZ - 1, fp) != (char *)NULL) { /*read input till eof*/
        switch (tagnum) {              /* look for left- or right-tag */
        case 0:
            status = 1;
            /* no tag to look for */
            break;
        case 1:             /* looking for opening left <tag> */
            /*haven't found it yet*/
            if ((tagp = strstr(text, tag1)) == NULL) break;
            /* shift out preceding text */
            strcpy(text, tagp + strlen(tag1));
            /*now looking for closing right tag*/
            tagnum = 2;
        case 2:             /* looking for closing right </tag> */
            /*haven't found it yet*/
            if ((tagp = strstr(text, tag2)) == NULL) break;
            /* terminate line at tag */
            *tagp = '\000';
            /* done after this line */
            tagnum = 3;
            /* successfully read tag */
            status = 1;
            break;
        } /* ---end-of-switch(tagnum) --- */
        if (tagnum != 1) {             /* no tag or left tag already found*/
            /* #chars in current line */
            int textlen = strlen(text);
            /* quit before overflow */
            if (vallen + textlen > maxvallen) break;
            /* concat line to end of value */
            strcat(value, text);
            /* bump length */
            vallen += textlen;
            if (tagnum > 2) break;
        }       /* found right tag, so we're done */
    } /* --- end-of-while(fgets()!=NULL) --- */
    /* okay if no tag or we found tag */
    if (tagnum < 1 || tagnum > 2) status = 1;
    /* close input file after reading */
    fclose(fp);
    /* --- return value and status to caller --- */
end_of_job:
    /* return status to caller */
    return (status);
} /* --- end-of-function rastreadfile() --- */


/* ==========================================================================
 * Function:    rastwritefile ( filename, tag, value, isstrict )
 * Purpose: Re/writes filename, replacing string between <tag>...</tag>
 *      with value, or writing entire file as value if tag=NULL.
 * --------------------------------------------------------------------------
 * Arguments:   filename (I)    char * to null-terminated string containing
 *              name of file to write (preceded by path
 *              relative to mimetex executable)
 *      tag (I)     char * to null-terminated string containing
 *              html-like tagname.  File contents between
 *              <tag> and </tag> will be replaced, or
 *              entire file written if tag=NULL passed.
 *      value (I)   char * containing string replacing value
 *              between <tag>...</tag> or replacing entire
 *              file if tag=NULL.
 *      isstrict (I)    int containing 1 to only rewrite existing
 *              files, or 0 to create new file if necessary.
 * --------------------------------------------------------------------------
 * Returns: ( int )     1=okay, 0=some error
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int rastwritefile(mimetex_ctx *mctx, char *filename, char *tag, char *value, int isstrict)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    char *timestamp();
    FILE *rastopenfile();
    int rastreadfile();

    /* pointer to opened filename */
    FILE *fp = (FILE *)NULL;
    char texfile[1024] = "\000";     /* local copy of input filename */
    char filebuff[MAXFILESZ+1] = "\000"; /* entire contents of file */
    char tag1[1024], tag2[1024];     /* left <tag> and right <tag/> */
    int istag = 0,
        isnewfile = 0,          /* true if writing new file */
        status = 0; /* status returned, 1=okay */
    /* true to update <timestamp> tag */
    int istimestamp = 0;
    /* ------------------------------------------------------------
    check args
    ------------------------------------------------------------ */
    /* --- check filename and value --- */
    if (filename == (char *)NULL         /* quit if no filename arg supplied*/
            /* or no value arg supplied */
            ||   value == (char *)NULL) goto end_of_job;
    if (strlen(filename) < 2         /* quit if unreasonable filename */
            /* or empty value string supplied */
            ||   *value == '\000') goto end_of_job;
    /* --- establish filename[.tex] --- */
    /* local copy of input filename */
    strncpy(texfile, filename, 1023);
    /* make sure it's null terminated */
    texfile[1023] = '\000';
    if (rastopenfile(mctx, texfile, NULL)      /* unchanged or .tex appended */
            == (FILE *)NULL) {          /* can't open, so write new file */
        /* fail if new files not permitted */
        if (isstrict) goto end_of_job;
        isnewfile = 1;
    }            /* signal we're writing new file */
    /* --- check whether tag supplied by caller --- */
    if (tag != (char *)NULL)         /* caller passed tag argument */
        if (*tag != '\000') {           /* and it's not an empty string */
            /* so flip tag flag true */
            istag = 1;
            strcpy(tag1, "<");
            strcpy(tag2, "</"); /* begin tags with < and </ */
            strcat(tag1, tag);
            /* followed by caller's tag */
            strcat(tag2, tag);
            strcat(tag1, ">");
            /* ending both tags with > */
            strcat(tag2, ">");
            compress(tag1, ' ');
            compress(tag2, ' ');
        } /* remove embedded blanks */
    /* ------------------------------------------------------------
    read existing file if just rewriting a single tag
    ------------------------------------------------------------ */
    /* --- read original file if only replacing a tag within it --- */
    /* init as empty file */
    *filebuff = '\000';
    if (!isnewfile)              /* if file already exists */
        if (istag)                  /* and just rewriting one tag */
            if (rastreadfile(mctx, texfile, 1, NULL, filebuff) /* read entire existing file */
                    /* signal error if failed to read */
                    <=   0) goto end_of_job;
    /* ------------------------------------------------------------
    construct new file data if needed (entire file replaced by value if no tag)
    ------------------------------------------------------------ */
    if (istag) {             /* only replacing tag in file */
        /* --- find <tag> and </tag> in file --- */
        /*tag,buff lengths*/
        int    tlen1 = strlen(tag1),  tlen2 = strlen(tag2), flen;
        char   *tagp1 = (isnewfile ? NULL : strstr(filebuff, tag1)), /* <tag> in file*/
                        *tagp2 = (isnewfile ? NULL : strstr(filebuff, tag2)); /*</tag> in file*/
        /* --- if adding new <tag> just concatanate at end of file --- */
        if (tagp1 == (char *)NULL) {        /* add new tag to file */
            /* --- preprocess filebuff --- */
            if (tagp2 != (char *)NULL)         /* apparently have ...</tag> */
                strcpy(filebuff, tagp2 + tlen2);   /* so get rid of leading ...</tag> */
            if ((flen = strlen(filebuff))  /* #chars currently in buffer */
                    > 0)                  /* we have non-empty buffer */
                if (!isthischar(*(filebuff + flen - 1), "\n\r")) /*no newline at end of file*/
                    /* so add one before new tag */
                    if (0)strcat(filebuff, "\n");
            /* --- add new tag --- */
            /* add opening <tag> */
            strcat(filebuff, tag1);
            /* then value */
            strcat(filebuff, value);
            strcat(filebuff, tag2);       /* finally closing </tag> */
            /* newline at end of file */
            strcat(filebuff, "\n");
        } /* --- end-of-if(tagp1==NULL) --- */
        else {                 /* found existing opening <tag> */
            if (tagp2 == NULL) {           /* apparently have <tag>... */
                /* so get rid of trailing ... */
                *(tagp1 + tlen1) = '\000';
                /* then concatanate value */
                strcat(filebuff, value);
                strcat(filebuff, tag2);
            }      /* and finally closing </tag> */
            else
            /* else have <tag>...<tag/> */
                if ((flen = ((int)(tagp2 - tagp1)) - tlen1) /* len of .'s in <tag>...</tag> */
                        >=   0)              /* usually <tag> precedes </tag> */
                    /* change ...'s to value */
                    strchange(flen, tagp1 + tlen1, value);
                else {               /* weirdly, </tag> precedes <tag> */
                    char fbuff[4096];         /* field buff for <tag>value</tag> */
                    if ((flen = ((int)(tagp1 - tagp2)) + tlen1) /* strlen(</tag>...<tag>) */
                            /* must be internal error */
                            <=   0) goto end_of_job;
                    /* set opening <tag> */
                    strcpy(fbuff, tag1);
                    /* then value */
                    strcat(fbuff, value);
                    strcat(fbuff, tag2);      /* finally closing </tag> */
                    strchange(flen, tagp2, fbuff);
                }    /* replace original </tag>...<tag> */
        } /* --- end-of-if/else(tagp1==NULL) --- */
    } /* --- end-of-if(istag) --- */
    /* ------------------------------------------------------------
    rewrite file and return to caller
    ------------------------------------------------------------ */
    /* --- first open file for write --- */
    if ((fp = rastopenfile(mctx, texfile, "w"))  /* open for write */
            /* signal error if can't open */
            == (FILE *)NULL) goto end_of_job;
    /* --- rewrite and close file --- */
    if (fputs((istag ? filebuff : value), fp)    /* write filebuff or value */
            /* signal success if succeeded */
            !=  EOF) status = 1;
    /* close output file after writing */
    fclose(fp);
    /* --- modify timestamp --- */
    if (status > 0)              /*forget timestamp if write failed*/
        if (istimestamp)            /* if we're updating timestamp */
            if (istag)                 /* only log time in tagged file */
                if (strstr(tag, "timestamp") == (char *)NULL) { /* but avoid recursion */
                    /* field buff <timestamp> value */
                    char fbuff[2048];
                    /* tag modified */
                    strcpy(fbuff, tag);
                    /* spacer */
                    strcat(fbuff, " modified at ");
                    /* start with timestamp */
                    strcat(fbuff, timestamp(tzdelta, 0));
                    status = rastwritefile(mctx, filename, "timestamp", fbuff, 1);
                }
    /* --- return status to caller --- */
end_of_job:
    /* return status to caller */
    return (status);
} /* --- end-of-function rastwritefile() --- */



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
static subraster *rastenviron(mimetex_ctx *mctx,
                              char **expression, int size, subraster *basesp,
                              int arg1, int arg2, int arg3)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
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
            *expression = texsubexpr(mctx, *expression, optarg, 250, "[", "]", 0, 0);
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
        environptr = strwrap(mctx, environvar, wraplen, -6);
        /* line-wrapped environvar */
        strninit(environvar, environptr, maxvarlen);
        /* preprocess environvar string */
        mimeprep(mctx, environvar);
        if (strlen(environstr) + strlen(environvar) > maxenvlen) break;
        sprintf(environstr + strlen(environstr), /* display environment string */
                " %2d. %s\\\\\n", ienv + 1, environvar);
        if (mctx->msgfp != NULL && mctx->msglevel >= 9)
            fprintf(mctx->msgfp, "rastenviron> %2d. %.256s\n",
                    ienv + 1,/*environ[ienv]*/environvar);
        /* don't overflow buffer */
        if (strlen(environstr) >= 7200) break;
    } /* --- end-of-for(ienv) --- */
    /* end {\text{...}} mode */
    strcat(environstr, "}}");
rasterize_environ:
    /* rasterize environment string */
    environsp = rasterize(mctx, environstr, size);
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
static subraster *rastmessage(mimetex_ctx *mctx, char **expression, int size,
                              subraster *basesp, int arg1, int arg2, int arg3)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
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
    *expression = texsubexpr(mctx, *expression, amsg, 255, "{", "}", 0, 0);
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
    messagesp = rasterize(mctx, msg, size);
    /* --- return message raster to caller --- */
    /*end_of_job:*/
    /* return message to caller */
    return (messagesp);
} /* --- end-of-function rastmessage() --- */

/* ==========================================================================
 * Function:    rastcounter ( expression, size, basesp, arg1, arg2, arg3 )
 * Purpose: \counter[value]{filename} handler, returns subraster
 *      containing image of counter value read from filename
 *      (or optional [value]), and increments counter
 * --------------------------------------------------------------------------
 * Arguments:   expression (I/O) char **  to first char of null-terminated
 *              string immediately following \counter to be
 *              rasterized, and returning ptr immediately
 *              following last character processed.
 *      size (I)    int containing 0-7 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding \counter
 *              (unused, but passed for consistency)
 *      arg1 (I)    int unused
 *      arg2 (I)    int unused
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to subraster corresponding to \counter
 *              requested, or NULL for any parsing error
 * --------------------------------------------------------------------------
 * Notes:     o Summary of syntax...
 *        \counter[value][logfile]{filename:tag}
 *        o :tag is optional
 * ======================================================================= */
/* --- entry point --- */
static subraster *rastcounter(mimetex_ctx *mctx, char **expression, int size,
                              subraster *basesp, int arg1, int arg2, int arg3)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    char filename[1024] = "\000", /* counter file */
         logfile[1024] = "\000", tag[1024] = "\000"; /*optional log file and tag*/
    /* rasterized counter image */
    subraster *countersp = NULL;
    FILE *logfp = NULL; /* counter and log file pointers */
    int status = 0,
        iscounter = (seclevel <= counterseclevel ? 1 : 0), /*is \counter permitted*/
        isstrict = 1; /* true to only write to existing files */
    char text[MAXFILESZ] = "1_", /* only line in counter file without tags */
         *delim = NULL,      /* delimiter in text */
         utext[128] = "1_",  /* default delimiter */
         *udelim = utext + 1;
         /* underscore delimiter */
    int counter = 1,        /* atoi(text) (after _ removed, if present) */
        value = 1,      /* optional [value] argument */
        gotvalue = 0,       /* set true if [value] supplied */
        isdelta = 0,        /* set true if [+value] or [-value] is delta*/
        ordindex = (-1); /* ordinal[] index to append ordinal suffix */
    /*--- ordinal suffixes based on units digit of counter ---*/
    static  char *ordinal[] = {
        "th", "st", "nd", "rd", "th", "th", "th", "th", "th", "th"
    };
    /* log vars*/
    static  char *logvars[] = {"REMOTE_ADDR", "HTTP_REFERER", NULL};
    /* logvars[commentvar] replaced by comment */
    static  int  commentvar = 1;
    /* ------------------------------------------------------------
    first obtain optional [value][logfile] args immediately following \counter
    ------------------------------------------------------------ */
    /* --- first check for optional \counter[value] --- */
    if (*(*expression) == '[') {     /* check for []-enclosed value */
        *expression = texsubexpr(mctx, *expression, text, 1023, "[", "]", 0, 0);
        if (*text != '\000')         /* got counter value (or logfile) */
            if (strlen(text) >= 1) {        /* and it's not an empty string */
                if (isthischar(*text, "+-0123456789"))  /* check for leading +-digit */
                    /* signal we got optional value */
                    gotvalue = 1;
                else
                /* not +-digit, so must be logfile */
                    strcpy(logfile, text);
            }     /* so just copy it */
    } /* --- end-of-if(**expression=='[') --- */
    /* --- next check for optional \counter[][logfile] --- */
    if (*(*expression) == '[') {     /* check for []-enclosed logfile */
        *expression = texsubexpr(mctx, *expression, filename, 1023, "[", "]", 0, 0);
        if (*filename != '\000')         /* got logfile (or counter value) */
            if (strlen(filename) >= 1) {    /* and it's not an empty string */
                if (!(isthischar(*text, "+-0123456789")) /* not a leading +-digit */
                        ||   gotvalue)            /* or we already got counter value */
                    /* so just copy it */
                    strcpy(logfile, filename);
                else {            /* leading +-digit must be value */
                    /* copy value to text line */
                    strcpy(text, filename);
                    gotvalue = 1;
                }
            }     /* and signal we got optional value*/
    } /* --- end-of-if(**expression=='[') --- */
    /* --- evaluate [value] if present --- */
    if (gotvalue) {              /*leading +-digit should be in text*/
        /* signal adding */
        if (*text == '+') isdelta = (+1);
        /* signal subtracting */
        if (*text == '-') isdelta = (-1);
        /*abs(value)*/
        value = (int)(strtod((isdelta == 0 ? text : text + 1), &udelim) + 0.1);
        /* set negative value if needed */
        if (isdelta == (-1)) value = (-value);
        /* re-init counter */
        counter = value;
    } /* --- end-of-if(gotvalue) --- */
    /* ------------------------------------------------------------
    obtain counter {filename} argument
    ------------------------------------------------------------ */
    /* --- parse for {filename} arg, and bump expression past it --- */
    *expression = texsubexpr(mctx, *expression, filename, 1023, "{", "}", 0, 0);
    /* --- check for counter filename:tag --- */
    if (*filename != '\000')         /* got filename */
        if ((delim = strchr(filename, ':')) /* look for : in filename:tag */
                != (char *)NULL) {             /* found it */
            /* null-terminate filename at : */
            *delim = '\000';
            strcpy(tag, delim + 1);
        }      /* and stuff after : is tag */
    /* ------------------------------------------------------------
    emit error message for unauthorized users trying to use \counter{}
    ------------------------------------------------------------ */
    if (!iscounter) {            /* counterseclevel > seclevel */
        sprintf(text,
                "\\ \\text{[\\backslash counter\\lbrace %.128s\\rbrace\\ not permitted]}\\ ",
                (isempty(filename) ? "???" : filename));
        /* rasterize error message */
        goto rasterize_counter;
    } /* --- end-of-if(!iscounter) --- */
    /* ------------------------------------------------------------
    Read and parse file, increment and rewrite counter (with optional underscore)
    ------------------------------------------------------------ */
    if (strlen(filename) > 1) {      /* make sure we got {filename} arg */
        /* --- read and interpret first (and only) line from counter file --- */
        if (!gotvalue || (isdelta != 0))   /*if no [count] arg or if delta arg*/
            if ((status = rastreadfile(mctx, filename, 1, tag, text)) > 0) { /*try reading file*/
                /* underscore delim from file */
                char *vdelim = NULL;
                /* value and delim from file */
                double fileval  = strtod(text, &vdelim);
                /* integerized */
                counter = (int)(fileval < 0.0 ? fileval - 0.1 : fileval + 0.1);
                counter += value;         /* bump count by 1 or add/sub delta*/
                if (!gotvalue) udelim = vdelim;
            }  /* default to file's current delim */
        /* --- check for ordinal suffix --- */
        if (udelim != (char *)NULL)        /* have some delim after value */
            if (*udelim == '_') {     /* underscore signals ordinal */
                /* abs(counter) */
                int abscount = (counter >= 0 ? counter : (-counter));
                /* least significant digit */
                ordindex = abscount % 10;
                if (abscount >= 10)        /* counter is 10 or greater */
                    if ((abscount / 10) % 10 == 1)  /* and the last two are 10-19 */
                        ordindex = 0;
            }     /* use th for 11,12,13 rather than st,nd,rd */
        /* --- rewrite counter file --- */
        if (status >= 0) {         /* file was read okay */
            /*build image of incremented counter*/
            sprintf(text, "%d", counter);
            /* tack on _ */
            if (ordindex >= 0) strcat(text, "_");
            /* and newline */
            if (*tag == '\000') strcat(text, "\n");
            status = rastwritefile(mctx, filename, tag, text, isstrict);
        } /*rewrite counter*/
    } /* --- end-of-if(strlen(filename)>1) --- */
    /* ------------------------------------------------------------
    log counter request
    ------------------------------------------------------------ */
    if (strlen(logfile) > 1) {       /* optional [logfile] given */
        char   comment[1024] = "\000",     /* embedded comment, logfile:comment*/
                               /* check for : signalling comment */
                               *commptr = strchr(logfile, ':');
        /* logfile must exist if isstrict */
        int    islogokay = 1;
        if (commptr != NULL) {          /* have embedded comment */
            /* comment follows : */
            strcpy(comment, commptr + 1);
            *commptr = '\000';
        }        /* null-terminate actual logfile */
        /* edit log file name */
        strcpy(logfile, rasteditfilename(logfile));
        /* given an invalid file name */
        if (*logfile == '\000') islogokay = 0;
        else if (isstrict) {            /*okay, but only write if it exists*/
            if ((logfp = fopen(logfile, "r")) == (FILE *)NULL) /*doesn't already exist*/
                /* so don't write log file */
                islogokay = 0;
            else fclose(logfp);
        }         /* close file opened for test read */
        if (islogokay)              /* okay to write logfile */
            if ((logfp = fopen(logfile, "a"))  /* open logfile */
                    != (FILE *)NULL) {            /* opened successfully for append */
                /* logvars[] index */
                int  ilog = 0;
                /* first emit timestamp */
                fprintf(logfp, "%s  ", timestamp(tzdelta, 0));
                /* emit counter filename */
                if (*tag == '\000') fprintf(logfp, "%s", filename);
                /* or tag if we have one */
                else fprintf(logfp, "<%s>", tag);
                /* emit counter value */
                fprintf(logfp, "=%d", counter);
                if (status < 1)           /* read or re-write failed */
                    /* emit error */
                    fprintf(logfp, "(%s %d)", "error status", status);
                for (ilog = 0; logvars[ilog] != NULL; ilog++) /* log till end-of-table */
                    if (ilog == commentvar       /* replace with comment... */
                            &&   commptr != NULL)       /* ...if available */
                        /* log embedded comment */
                        fprintf(logfp, "  %.256s", comment);
                    else {
                        /*getenv(variable) to be logged*/
                        char *logval = getenv(logvars[ilog]);
                        fprintf(logfp, "  %.64s",    /* log variable */
                                (logval != NULL ? logval : "<unknown>"));
                    } /* emit value or <unknown> */
                /* terminating newline */
                fprintf(logfp, "\n");
                /* close logfile */
                fclose(logfp);
            } /* --- end-of-if(islogokay&&logfp!=NULL) --- */
    } /* --- end-of-if(strlen(logfile)>1) --- */
    /* ------------------------------------------------------------
    construct counter expression and rasterize it
    ------------------------------------------------------------ */
    /* --- construct expression --- */
    /*sprintf(text,"%d",counter);*/     /* start with counter */
    /* comma-separated counter value */
    strcpy(text, dbltoa(((double)counter), 0));
    if (ordindex >= 0) {         /* need to tack on ordinal suffix */
        /* start with ^ and {\underline{\rm */
        strcat(text, "^{\\underline{\\rm~");
        /* then st,nd,rd, or th */
        strcat(text, ordinal[ordindex]);
        strcat(text, "}}");
    }        /* finish with }} */
    /* --- rasterize it --- */
rasterize_counter:
    /* rasterize counter subexpression */
    countersp = rasterize(mctx, text, size);
    /* --- return counter image to caller --- */
    /*end_of_job:*/
    /* return counter image to caller */
    return (countersp);
} /* --- end-of-function rastcounter() --- */

/* ==========================================================================
 * Function:    rastinput ( expression, size, basesp, arg1, arg2, arg3 )
 * Purpose: \input{filename} handler, reads filename and returns
 *      subraster containing image of expression read from filename
 * --------------------------------------------------------------------------
 * Arguments:   expression (I/O) char **  to first char of null-terminated
 *              string immediately following \input to be
 *              rasterized, and returning ptr immediately
 *              following last character processed.
 *      size (I)    int containing 0-7 default font size
 *      basesp (I)  subraster *  to character (or subexpression)
 *              immediately preceding \input
 *              (unused, but passed for consistency)
 *      arg1 (I)    int unused
 *      arg2 (I)    int unused
 *      arg3 (I)    int unused
 * --------------------------------------------------------------------------
 * Returns: ( subraster * ) ptr to subraster corresponding to expression
 *              in filename, or NULL for any parsing error
 * --------------------------------------------------------------------------
 * Notes:     o Summary of syntax...
 *        \input{filename}     reads entire file named filename
 *        \input{filename:tag} reads filename, but returns only
 *        those characters between <tag>...</tag> in that file.
 *        o
 * ======================================================================= */
/* --- entry point --- */
static subraster *rastinput(mimetex_ctx *mctx, char **expression, int size,
                            subraster *basesp, int arg1, int arg2, int arg3)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* args */
    char tag[1024] = "\000", filename[1024] = "\000";
    /* rasterized input image */
    subraster *inputsp = NULL;
    /* read input file */
    int status;
    /* don't reformat (numerical) input */
    int format = 0, npts = 0;
    /*true if \input permitted*/
    int isinput = (seclevel <= inputseclevel ? 1 : 0);
    /* permitted \input{} paths for any user */
    char    *inputpath = NULL;
    /* search for valid inputpath in filename */
    char    subexpr[MAXFILESZ+1] = "\000", /*concatanated lines from input file*/
            *reformat = NULL;
    /* ------------------------------------------------------------
    obtain [tag]{filename} argument
    ------------------------------------------------------------ */
    /* --- parse for optional [tag] or [fmt] arg, bump expression past it --- */
    if (*(*expression) == '[') {     /* check for []-enclosed value */
        /* optional argument field */
        char argfld[MAXTOKNSZ+1];
        *expression = texsubexpr(mctx, *expression, argfld, MAXTOKNSZ - 1, "[", "]", 0, 0);
        if ((reformat = strstr(argfld, "dtoa")) != NULL) { /*dtoa/dbltoa requested*/
            format = 1;         /* signal dtoa()/dbltoa() format */
            if ((reformat = strchr(reformat, '=')) != NULL) /* have dtoa= */
                npts = (int)strtol(reformat + 1, NULL, 0);
        } /* so set npts */
        if (format == 0) {       /* reformat not requested */
            strninit(tag, argfld, 1020);
        }
    }    /* so interpret arg as tag */
    /* --- parse for {filename} arg, and bump expression past it --- */
    *expression = texsubexpr(mctx, *expression, filename, 1020, "{", "}", 0, 0);
    /* --- check for alternate filename:tag --- */
    if (!isempty(filename)           /* got filename */
            /*&& isempty(tag)*/) {          /* but no [tag] */
        /* look for : in filename:tag */
        char *delim = strchr(filename, ':');
        if (delim != (char *)NULL) {      /* found it */
            /* null-terminate filename at : */
            *delim = '\000';
            strninit(tag, delim + 1, 1020);
        }
    }   /* and stuff after : is tag */
    /* --- check filename for an inputpath valid for all users --- */
    if (!isinput                 /* if this user can't \input{} */
            &&   !isempty(filename)         /* and we got a filename */
            &&   !isempty(inputpath))       /* and an inputpath */
        if (isstrstr(filename, inputpath, 0))  /* filename has allowed inputpath */
            /* okay to \input{} this filename */
            isinput = 1;
    /* --- guard against recursive runaway (e.g., file \input's itself) --- */
    if (++ninputcmds > 8)            /* max \input's per expression */
        /* flip flag off after the max */
        isinput = 0;
    /* ------------------------------------------------------------
    Read file (and convert to numeric if [dtoa] option was given)
    ------------------------------------------------------------ */
    if (isinput) {           /* user permitted to use \input{} */
        /* read file */
        status = rastreadfile(mctx, filename, 0, tag, subexpr);
        /* quit if problem */
        if (*subexpr == '\000') goto end_of_job;
        /* --- rasterize input subexpression  --- */
        /* preprocess subexpression */
        mimeprep(mctx, subexpr);
        if (format == 1) {             /* dtoa()/dbltoa() */
            /* interpret subexpr as double */
            double d = strtod(subexpr, NULL);
            if (d != 0.0)            /* conversion to double successful */
                if ((reformat = dbltoa(d, npts)) != NULL) /* reformat successful */
                    strcpy(subexpr, reformat);
        } /*replace subexpr with reformatted*/
    } /* --- end-of-if(isinput) --- */
    /* ------------------------------------------------------------
    emit error message for unauthorized users trying to use \input{}
    ------------------------------------------------------------ */
    else {                  /* inputseclevel > seclevel */
        sprintf(subexpr,
                "\\ \\text{[\\backslash input\\lbrace %.128s\\rbrace\\ not permitted]}\\ ",
                (isempty(filename) ? "???" : filename));
    } /* --- end-of-if/else(isinput) --- */
    /* ------------------------------------------------------------
    Rasterize constructed subexpression
    ------------------------------------------------------------ */
    /* rasterize subexpression */
    inputsp = rasterize(mctx, subexpr, size);
    /* --- return input image to caller --- */
end_of_job:
    /* return input image to caller */
    return (inputsp);
} /* --- end-of-function rastinput() --- */



static mathchardef extra_handlers[] = {
    { "\\environment", NOVALUE, NOVALUE, NOVALUE, (HANDLER)(rastenviron) },
    { "\\message", NOVALUE, NOVALUE, NOVALUE, (HANDLER)(rastmessage) },
    { "\\counter", NOVALUE, NOVALUE, NOVALUE, (HANDLER)(rastcounter) },
    { "\\input", NOVALUE, NOVALUE, NOVALUE, (HANDLER)(rastinput) },
    { NULL,     -999,   -999,   -999,       NULL }
};

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
static int type_pbmpgm(raster *rp, int ptype, FILE *fp)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* completion flag, total #bytes written */
    int isokay = 0, nbytes = 0;
    /*height(row), width(col) indexes in raster*/
    int irow = 0, jcol = 0;
    int pixmin = 9999, pixmax = (-9999); /* min, max pixel value in raster */
    char    outline[1024], outfield[256], /* output line, field */
    /* cr at end-of-line */
    cr[16] = "\n\000";
    /* maximum allowed line length */
    int maxlinelen = 70;
    int pixfrac = 6;    /* use (pixmax-pixmin)/pixfrac as step */
    static char *magic[] = { NULL, "P1", "P2" };    /*identifying "magic number"*/
    /* ------------------------------------------------------------
    check input, determine grayscale,  and set up output file if necessary
    ------------------------------------------------------------ */
    /* --- check input args --- */
    /* no input raster provided */
    if (rp == NULL) goto end_of_job;
    /* --- determine largest (and smallest) value in pixmap --- */
    for (irow = 0; irow < rp->height; irow++)  /* for each row, top-to-bottom */
        for (jcol = 0; jcol < rp->width; jcol++) { /* for each col, left-to-right */
            /* value of pixel at irow,jcol */
            int pixval = getpixel(rp, irow, jcol);
            /* new minimum */
            pixmin = min2(pixmin, pixval);
            pixmax = max2(pixmax, pixval);
        } /* new maximum */
    /* --- open output file if necessary --- */
    /*null ptr signals output to stdout*/
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
static int type_bytemap(mimetex_ctx *mctx, intbyte *bp, int grayscale,
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
    if (mctx->isstring) {              /* bp has ascii string, not raster */
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
        if (locol > 0 && !mctx->isstring) fprintf(fp, "----------\n");
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
static int xbitmap_raster(raster *rp, FILE *fp)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* dummy title */
    char    *title = "image";
    /* dump bitmap as hex bytes */
    /* ------------------------------------------------------------
    emit text to display mime xbitmap representation of rp->bitmap image
    ------------------------------------------------------------ */
    /* --- first redirect null fp --- */
    /* default fp to stdout if null */
    if (fp == (FILE *)NULL) fp = stdout;
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
    fclose(fp);
    return (1);
} /* --- end-of-function xbitmap_raster() --- */

struct gif_raster_params {
    mimetex_ctx *mctx;
    int ncolors;
    raster *bitmap; /* use 0/1 bitmap image or */
    intbyte *colormap;  /* anti-aliased color indexes */
};

/* ==========================================================================
 * Function:    gif_raster_get_pixel ( int x, int y )
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
static int gif_raster_get_pixel(void *_ctx, int x, int y)
{
    struct gif_raster_params *ctx = _ctx;

    /* pixel index for x,y-coords*/
    int ipixel = y * ctx->bitmap->width + x;
    /* value of pixel */
    int pixval = 0;
    if (!ctx->colormap)               /* use bitmap if not anti-aliased */
        /*pixel = 0 or 1*/
        pixval = (int)getlongbit(ctx->bitmap->pixmap, ipixel);
    else
    /* else use anti-aliased grayscale*/
        /* colors[] index number */
        pixval = (int)(ctx->colormap[ipixel]);
    if (ctx->mctx->msgfp != NULL && ctx->mctx->msglevel >= 9999) { /* dump pixel */
        fprintf(ctx->mctx->msgfp, "gif_raster_get_pixel> x=%d, y=%d  pixel=%d\n", x, y, pixval);
        fflush(ctx->mctx->msgfp);
    }
    return pixval;
} /* --- end-of-function gif_raster_get_pixel() --- */


static int gif_raster(mimetex_ctx *mctx, int ncolors, raster *bp, intbyte *colormap, intbyte *colors, FILE *fp, void *buffer, int buffer_size)
{
    struct gif_raster_params params = { mctx, ncolors, bp, colormap };
    GIFContext *gctx;
    /* --- initialize gifsave library and colors --- */
    if (mctx->msgfp != NULL && mctx->msglevel >= 999) {
        fprintf(mctx->msgfp, "main> calling GIF_Create(*,%d,%d,%d,8)\n",
                bp->width, bp->height, ncolors);
        fflush(mctx->msgfp);
    }
    if ((gctx = GIF_Create(fp, buffer, buffer_size, bp->width, bp->height, ncolors, 8)) == NULL)
        return 0;
    /* background white if all 255 */
    GIF_SetColor(gctx, 0, mctx->bgred, mctx->bggreen, mctx->bgblue);
    if (ncolors == 2) {               /* just b&w if not anti-aliased */
        /* foreground black if all 0 */
        GIF_SetColor(gctx, 1, mctx->fgred, mctx->fggreen, mctx->fgblue);
        /* and set 2 b&w color indexes */
        colors[0] = '\000';
        colors[1] = '\001';
    } else {
        int igray;
        /* set grayscales for anti-aliasing */
        /* --- anti-aliased, so call GIF_SetColor() for each colors[] --- */
        for (igray = 1; igray < ncolors; igray++) { /* for colors[] values */
            /*--- gfrac goes from 0 to 1.0, as igray goes from 0 to ncolors-1 ---*/
            double gfrac = ((double)colors[igray]) / ((double)colors[ncolors-1]);
            /* --- r,g,b components go from background to foreground color --- */
            int red  = iround(((double)mctx->bgred)  + gfrac * ((double)(mctx->fgred - mctx->bgred))),
                green = iround(((double)mctx->bggreen) + gfrac * ((double)(mctx->fggreen - mctx->bggreen))),
                blue = iround(((double)mctx->bgblue) + gfrac * ((double)(mctx->fgblue - mctx->bgblue)));
            /* --- set color index number igray to rgb values gray,gray,gray --- */
            /*set gray,grayer,...,0=black*/
            GIF_SetColor(gctx, igray, red, green, blue);
        } /* --- end-of-for(igray) --- */
    }        
    /* --- set gif color#0 (background) transparent --- */
    if (mctx->istransparent)             /* transparent background wanted */
        /* set transparent background */
        GIF_SetTransparent(gctx, 0);
    /*flush debugging output*/
    if (mctx->msgfp != NULL && mctx->msglevel >= 9)
        fflush(mctx->msgfp);
    /* --- emit compressed gif image (to stdout or cache file) --- */
    /* emit gif */
    GIF_CompressImage(gctx, 0, 0, -1, -1, gif_raster_get_pixel, &params);
    /* close file */
    GIF_Close(gctx);
    return gctx->gifSize;
}

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
static int ismonth(char *month)
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

/* ------------------------------------------------------------
logging data structure, and default data to be logged
------------------------------------------------------------ */
/* --- logging data structure --- */
typedef struct logdata {
    /* ------------------------------------------------------------
    environment variable name, max #chars to display, min mctx.msglevel to display
    ------------------------------------------------------------ */
    /* environment variable name */
    char  *name;
    /* max #chars to display */
    int   maxlen;
    /* min mctx.msglevel to display data */
    int   msglevel;
} logdata ; /* --- end-of-logdata_struct --- */

/* ==========================================================================
 * Function:    logger ( fp, mctx.msglevel, message, logvars )
 * Purpose: Logs the environment variables specified in logvars
 *      to fp if their mctx.msglevel is >= the passed mctx.msglevel.
 * --------------------------------------------------------------------------
 * Arguments:   fp (I)      FILE * to file containing log
 *      mctx.msglevel (I)    int containing logging message level
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
static int logger(FILE *fp, int msglevel, char *message, logdata *logvars)
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
            if (msglevel >= logvars[ilog].msglevel)   /* check mctx.msglevel for this var */
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
static int readcachefile(char *cachefile, unsigned char *buffer)
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
static int emitcache(char *cachefile, int maxage, int valign, int nbytes)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
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
    if (nbytes > 0) {              /* cachefile is buffer */
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
 *              [-m mctx.msglevel]   verbosity of debugging output
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

/* --- data logged by mimeTeX --- */
static logdata mimelog[] = {
    /* ------ variable ------ maxlen mctx.msglevel ----- */
    { "QUERY_STRING",         999,    4 },
    { "REMOTE_ADDR",          999,    3 },
    { "HTTP_REFERER",         999,    3 },
    { "REQUEST_URI",          999,    5 },
    { "HTTP_USER_AGENT",      999,    3 },
    { "HTTP_X_FORWARDED_FOR", 999,    3 },
    { NULL, -1, -1 }            /* trailer record */
}; /* --- end-of-mimelog[] --- */


/* --- entry point --- */
int main(int argc, char *argv[], char *envp[])
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    static char *suffix[] = { ".gif", ".pbm", ".pgm", ".xbm" };
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
    char *outfile = (char *)NULL;
    char outfilebuf[256];
    char *pdot;
    char cachefile[256] = "\000";    /* full path and name to cache file*/
    /* max-age is two hours */
    int maxage = 7200;
    /*Vertical-Align:baseline-(height-1)*/
    int valign = (-9999);
    /* --- image format (-g switch) --- */
    /* -1=detect by filename 0=gif 1=pbm 2=pgm 3=xbm */
    int ptype = -1;
    /* --- anti-aliasing --- */
    intbyte *bytemap_raster = NULL;    /* anti-aliased bitmap */
    intbyte *colormap_raster = NULL;
    intbyte colors[256]; /* grayscale vals in bytemap */
    int grayscale = 256; /* 0-255 grayscales in 8-bit bytes */
    int ncolors = 2;        /* #colors (2=b&w) */
    /*patternnumcount[] index diagnostic*/
    int ipattern;
    /* --- messages --- */
    char logfile[256] = LOGFILE,     /*log queries if mctx.msglevel>=LOGLEVEL*/
         cachelog[256] = CACHELOG;   /* cached image log in cachepath/ */
    /* name program executed as */
    char    *progname = (argc > 0 ? argv[0] : "noname");
    char    *dashes =           /* separates logfile entries */
        "--------------------------------------------------------------------------";
    /*msg to invalid referer*/
    char    *invalid_referer_msg = msgtable[invmsgnum];
    /*referer isn't host*/
    char    *invalid_referer_match = msgtable[refmsgnum];
    mimetex_ctx mctx;
    /* ------------------------------------------------------------
    initialization
    ------------------------------------------------------------ */
    if (mimetex_ctx_init(&mctx)) {
        fprintf(stderr, "Failed to initialize context\n");
        return 1;
    }

    /* install extra handlers */
    {
        int i;
        for (i = 0; i < sizeof(symtables) / sizeof(*symtables); i++) {
            if (!symtables[i].table) {
                symtables[i].family = NOVALUE;
                symtables[i].table = extra_handlers;
                break;
            }
        }
    }
    /* --- set global variables --- */
    strcpy(pathprefix, PATHPREFIX);
    /* for command-line mode output */
    mctx.msgfp = stdout;
    /* true to emit mime content-type */
    isemitcontenttype = 1;
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
    /* signal that image not in memory */
    /* default foreground colors */

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
                } else {             /* process single-char -x switch */
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
                        outfile = argv[argnum];
                        break;
                    case 'f':
                        isdumpimage++;
                        infilearg = argnum;
                        break;
                    case 'g':
                        /* -g2 ==> ptype=2 */
                        if (arglen > 1) ptype = atoi(field + 1);
                        if (ptype < 0 || ptype > sizeof(suffix) / sizeof(*suffix))
                            ptype = 0;
                        argnum--;
                        break;
                    case 'm':
                        if (argnum < argc) mctx.msglevel = atoi(argv[argnum]);
                        break;
                    case 'o':
                        mctx.istransparent = (mctx.istransparent ? 0 : 1);
                        argnum--;
                        break;
                    case 'q':
                        isqforce = 1;
                        argnum--;
                        break;
                    case 'c':
                        isemitcontenttype = 0;
                        argnum--;
                        break;
                    case 'a':
                        mctx.aaalgorithm = atoi(argv[argnum]);
                        break;
                    case 's':
                        if (argnum < argc) size = atoi(argv[argnum]);
                        break;
                    } /* --- end-of-switch(flag) --- */
                }
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

        if (outfile != NULL) {
            /* local copy of file name */
            strncpy(outfilebuf, outfile, sizeof(outfilebuf));
            /* make sure it's null terminated */
            outfilebuf[sizeof(outfilebuf) - 1] = '\000';
            if ((pdot = strrchr(outfilebuf, '.')) == NULL) {
                /* no extension on original name */
                /* so add extension */
                if (ptype < 0)
                    ptype = 0; /* falls back to the default (gif) */
                strcat(outfilebuf, suffix[ptype]);
            } else {
                if (ptype == -1) {
                    int i;
                    for (i = 0; i < sizeof(suffix) / sizeof(*suffix); i++) {
                        if (!strcmp(pdot, suffix[i]))
                            ptype = i;
                    }
                    if (ptype == -1)
                        ptype = 0; /* falls back to the default (gif) */
                }
            }
            outfile = outfilebuf;
        }

        if (ptype == 1 && mctx.aaalgorithm)
            mctx.aaalgorithm = 0;

        if (mctx.msglevel >= 999 && mctx.msgfp != NULL) { /* display command-line info */
            fprintf(mctx.msgfp, "argc=%d, progname=%s, #args=%d, #badargs=%d\n",
                    argc, progname, nargs, nbadargs);
            fprintf(mctx.msgfp, "cachepath=\"%.50s\" pathprefix=\"%.50s\"\n",
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
            /* mctx.msglevel for forms */
            if (0) mctx.msglevel = FORMLEVEL;
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
        /* --- check for mctx.msglevel=###$ prefix --- */
        if (!memcmp(expression, "mctx.msglevel=", 9)) { /* query has mctx.msglevel prefix */
            /* find $ delim following mctx.msglevel*/
            char *delim = strchr(expression, '$');
            if (delim != (char *)NULL) {    /* check that we found delim */
                /* replace delim with null */
                *delim = '\000';
                if (seclevel <= 9)       /* permit mctx.msglevel specification */
                    /* interpret ### in mctx.msglevel###$ */
                    mctx.msglevel = atoi(expression + 9);
                strcpy(expression, delim + 1);
            }
        } /* shift out prefix and delim */
        /* --- next check for logfile=xxx$ prefix (must follow mctx.msglevel) --- */
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
        if (mctx.msglevel >= LOGLEVEL        /* check if logging */
                &&   seclevel <= 5)            /* and if logging permitted */
            if (logfile != NULL)       /* if a logfile is given */
                if (*logfile != '\000') {         /*and if it's not an empty string*/
                    if ((mctx.msgfp = fopen(logfile, "a"))  /* open logfile for append */
                            !=   NULL) {            /* ignore logging if can't open */
                        /* --- default logging --- */
                        /* log query */
                        logger(mctx.msgfp, mctx.msglevel, expression, mimelog);
                        /* --- additional debug logging (argv and environment) --- */
                        if (mctx.msglevel >= 9) {        /* log environment */
                            /*char name[999],*value;*/
                            int i;
                            fprintf(mctx.msgfp, "Command-line arguments...\n");
                            if (argc < 1)            /* no command-line args */
                                fprintf(mctx.msgfp, "  ...argc=%d, no argv[] variables\n", argc);
                            else
                                for (i = 0; i < argc; i++)  /* display all argv[]'s */
                                    fprintf(mctx.msgfp, "  argv[%d] = \"%s\"\n", i, argv[i]);
#ifdef DUMPENVP         /* char *envp[] available for dump */
                            fprintf(mctx.msgfp, "Environment variables (using envp[])...\n");
                            if (envp == (char **)NULL)   /* envp not provided */
                                fprintf(mctx.msgfp, "  ...envp[] environment variables not available\n");
                            else
                                for (i = 0; ; i++)      /* display all envp[]'s */
                                    if (envp[i] == (char *)NULL) break;
                                    else fprintf(mctx.msgfp, "  envp[%d] = \"%s\"\n", i, envp[i]);
#endif
/* --- DUMPENVP ---*/
#ifdef DUMPENVIRON  /* skip what should be redundant output */
                            fprintf(mctx.msgfp, "Environment variables (using environ)...\n");
                            if (environ == (char **)NULL)    /* environ not provided */
                                fprintf(mctx.msgfp, "  ...extern environ variables not available\n");
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
                                        fprintf(mctx.msgfp, "environ[%d]: \"%s\"\n\tgetenv(%s) = \"%s\"\n",
                                                i, environ[i], name, (value == NULL ? "NULL" : value));
                                    } /* --- end-of-if/else --- */
#endif
/* --- DUMPENVIRON ---*/
                        } /* --- end-of-if(mctx.msglevel>=9) --- */
                        /* --- close log file if no longer needed --- */
                        if (mctx.msglevel < DBGLEVEL) {      /* logging, but not debugging */
                            /* so log separator line, */
                            fprintf(mctx.msgfp, "%s\n", dashes);
                            /* close logfile immediately, */
                            fclose(mctx.msgfp);
                            mctx.msgfp = NULL;
                        }         /* and reset mctx.msgfp pointer */
                        else
                            /* set query logging flag */
                            isqlogging = 1;
                    } /* --- end-of-if(mctx.msglevel>=LOGLEVEL) --- */
                    else
                    /* couldn't open logfile */
                        mctx.msglevel = 0;
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
    if (isquery) { /* not relevant if "interactive" */
        if (!isinvalidreferer) {        /* nor if already invalid referer */
            /* denyreferer index, message# */
            int iref = 0, msgnum = (-999);
            for (iref = 0; msgnum < 0; iref++) { /* run through denyreferer[] table */
                /* referer to be denied */
                char *deny = denyreferer[iref].referer;
                /* null signals end-of-table */
                if (deny == NULL) break;
                if (mctx.msglevel >= 999 && mctx.msgfp != NULL) { /* debugging */
                    fprintf(mctx.msgfp, "main> invalid iref=%d: deny=%s http_referer=%s\n",
                            iref, deny, (http_referer == NULL ? "null" : http_referer));
                    fflush(mctx.msgfp);
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
    }
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
     * emit copyright, gnu/gpl notice (if "interactive")
     * ------------------------------------------------------------ */
    if (!isdumpimage) { /* don't mix ascii with image dump */
        if ((!isquery || isqlogging) && mctx.msgfp != NULL) { /* called from command line */
            /* display copyright */
            fprintf(mctx.msgfp, "%s\n%s\n", copyright1, copyright2);
            /*revision date*/
            fprintf(mctx.msgfp, "Most recent revision: %s\n", REVISIONDATE);
        } /* --- end-of-if(!isquery...) --- */
    }
    /* ------------------------------------------------------------
    rasterize expression and put a border around it
    ------------------------------------------------------------ */
    /* --- preprocess expression, converting LaTeX constructs for mimeTeX  --- */
    if (expression != NULL) {        /* have expression to rasterize */
        expression = mimeprep(&mctx, expression);
    }  /* preprocess expression */
    /* --- double-check that we actually have an expression to rasterize --- */
    if (expression == NULL) {        /* nothing to rasterize */
        /*signal error to parent*/
        if (exitstatus == 0) exitstatus = errorstatus;
        if ((!isquery || isqlogging) && mctx.msgfp != NULL) { /*emit error if not query*/
            if (exitstatus != 0) fprintf(mctx.msgfp, "Exit code = %d,\n", exitstatus);
            fprintf(mctx.msgfp, "No LaTeX expression to rasterize\n");
        }
        goto end_of_job;
    }            /* and then quit */
    /* --- rasterize expression --- */
    if ((sp = rasterize(&mctx, expression, size)) == NULL) {  /* failed to rasterize */
        /*signal error to parent*/
        if (exitstatus == 0) exitstatus = errorstatus;
        if ((!isquery || isqlogging) && mctx.msgfp != NULL) { /*emit error if not query*/
            if (exitstatus != 0) fprintf(mctx.msgfp, "Exit code = %d,\n", exitstatus);
            fprintf(mctx.msgfp, "Failed to rasterize %.2048s\n", expression);
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
            if ((sp = rasterize(&mctx, errormsg, 1)) == NULL)  /*couldn't rasterize errmsg*/
                /* so rasterize generic error */
                sp = rasterize(&mctx,
                         "\\red\\rm~\\fbox{mimeTeX~failed~to~render\\\\your~expression}", 1);
        }
        /* re-check for err message failure*/
        if (sp ==  NULL) goto end_of_job;
    } /* --- end-of-if((sp=rasterize())==NULL) --- */
    /* ---no border requested, but this adjusts width to multiple of 8 bits--- */
    /* for mime xbitmaps must have... */
    /* image width multiple of 8 bits */
    bp = border_raster(&mctx, sp->image, 0, 0, 0, 1);
    /* global copy for gif,png output */
    sp->image = bp;
    if (sp != NULL && bp != NULL) {      /* have raster */
        /* #pixels for Vertical-Align: */
        valign = sp->baseline - (bp->height - 1);
        if (abs(valign) > 255) valign = (-9999);
    } /* sanity check */
    /* ------------------------------------------------------------
    generate anti-aliased bytemap from (bordered) bitmap
    ------------------------------------------------------------ */
    if (mctx.aaalgorithm) {              /* we want anti-aliased bitmap */
        /* ---
         * allocate bytemap and colormap as per width*height of bitmap
         * ------------------------------------------------------------ */
        /*#bytes needed in byte,colormap*/
        int   nbytes = (bp->width) * (bp->height);
        /* anti-aliasing wanted */
        /* malloc bytemap and colormap */
        if ((bytemap_raster = (intbyte *)malloc(nbytes)) == NULL) {
            fprintf(mctx.msgfp, "allocation failure\n");
            goto end_of_job;
        }
        /* have bytemap, so... */
        if ((colormap_raster = (intbyte *)malloc(nbytes)) == NULL) {
            fprintf(mctx.msgfp, "allocation failure\n");
            goto end_of_job;
        }
        /* ---
         * now generate anti-aliased bytemap and colormap from bitmap
         * ------------------------------------------------------------ */
        /* ---
         * select anti-aliasing algorithm
         * ------------------------------------------------------------ */
        /* generate bytemap for lowpass */
        switch (mctx.aaalgorithm) {          /* choose antialiasing algorithm */
        default:
            /* unrecognized algorithm */
            mctx.aaalgorithm = 0;
            break;
        case 1:              /* 1 for aalowpass() */
            /*my own lowpass filter*/
            /*failed, so turn off anti-aliasing*/
            if (aalowpass(&mctx,bp, bytemap_raster, grayscale) == 0) {
                mctx.aaalgorithm = 0;
            }
            break;
        case 2:              /*2 for netpbm pnmalias.c algorithm*/
            if (aapnm(&mctx, bp, bytemap_raster, grayscale) == 0) {
                mctx.aaalgorithm = 0;
            }
            break;
        case 3:              /*3 for aapnm() based on aagridnum()*/
            if (aapnmlookup(&mctx, bp, bytemap_raster, grayscale) == 0) {
                mctx.aaalgorithm = 0;
            }
            break;
        case 4:              /* 4 for aalookup() table lookup */
            if (aalowpasslookup(&mctx, bp, bytemap_raster, grayscale) == 0) {
                mctx.aaalgorithm = 0;
            }
            break;
        } /* --- end-of-switch(aaalgorithm) --- */
        if (mctx.aaalgorithm) {              /* we have bytemap_raster */
            /* ---
             * emit aalookup() pattern# counts/percents diagnostics
             * ------------------------------------------------------------ */
            if (!isquery && mctx.msgfp != NULL && mctx.msglevel >= 99) { /*emit patternnumcounts*/
                /* init total w,b center counts */
                int pcount0 = 0, pcount1 = 0;
                for (ipattern = 1; ipattern <= 51; ipattern++) { /*each possible pattern*/
                    if (ipattern > 1)          /* ignore all-white squares */
                        /* bump total white centers */
                        pcount0 += mctx.patternnumcount0[ipattern];
                    pcount1 += mctx.patternnumcount1[ipattern];
                } /* bump total black centers */
                if (pcount0 + pcount1 > 0)      /* have pcounts (using aalookup) */
                    fprintf(mctx.msgfp, "  aalookup() patterns excluding#1 white"
                            " (%%'s are in tenths of a percent)...\n");
                for (ipattern = 1; ipattern <= 51; ipattern++) { /*each possible pattern*/
                    int tot = mctx.patternnumcount0[ipattern] + mctx.patternnumcount1[ipattern];
                    if (tot > 0)           /* this pattern occurs in image */
                        fprintf(mctx.msgfp,
                                "  pattern#%2d: %7d(%6.2f%%) +%7d(%6.2f%%) =%7d(%6.2f%%)\n",
                                ipattern, mctx.patternnumcount0[ipattern], (ipattern <= 1 ? 999.99 :
                                                                       1000.*((double)mctx.patternnumcount0[ipattern]) / ((double)pcount0)),
                                mctx.patternnumcount1[ipattern],
                                1000.*((double)mctx.patternnumcount1[ipattern]) / ((double)pcount1),
                                tot, (ipattern <= 1 ? 999.99 :
                                      1000.*((double)tot) / ((double)(pcount0 + pcount1))));
                }
                if (pcount0 + pcount1 > 0) /* true when using aalookup() */
                    fprintf(mctx.msgfp,
                            "all patterns: %7d          +%7d          =%7d  total pixels\n",
                            pcount0, pcount1, pcount0 + pcount1);
            }
            if (colormap_raster) {
                /* ---
                 * finally, generate colors and colormap
                 * ------------------------------------------------------------ */
                ncolors = aacolormap(&mctx, bytemap_raster, nbytes, colors, colormap_raster);
                if (ncolors < 2) {     /* failed */
                    /* so turn off anti-aliasing */
                    mctx.aaalgorithm = 0;
                    ncolors = 2;
                }        /* and reset for black&white */
            }
        }
    } /* --- end-of-if(isaa) --- */
    /* ------------------------------------------------------------
    display results on mctx.msgfp if called from command line (usually for testing)
    ------------------------------------------------------------ */
    if ((!isquery || isqlogging) || mctx.msglevel >= 99) {
        /*command line or debuging*/
        if (!isdumpimage) {         /* don't mix ascii with image dump */
            /* ---
             * display ascii image of rasterize()'s rasterized bitmap
             * ------------------------------------------------------------ */
            fprintf(mctx.msgfp, "\nAscii dump of bitmap image...\n");
            type_raster(&mctx, bp, mctx.msgfp);
            /* ---
             * display anti-aliasing results applied to rasterized bitmap
             * ------------------------------------------------------------ */
            if (mctx.aaalgorithm) {                /* if anti-aliasing applied */
                /* colors[] index */
                int igray;
                /* --- anti-aliased bytemap image --- */
                if (mctx.msgfp != NULL && mctx.msglevel >= 9) { /* don't usually emit raw bytemap */
                    fprintf(mctx.msgfp, "\nHex dump of anti-aliased bytemap, " /*emit bytemap*/
                            "asterisks denote \"black\" bytes (value=%d)...\n", grayscale - 1);
                    type_bytemap(&mctx, bytemap_raster, grayscale, bp->width, bp->height, mctx.msgfp);
                }
                if (colormap_raster) {
                    /* --- colormap image --- */
                    fprintf(mctx.msgfp, "\nHex dump of colormap indexes, " /* emit colormap */
                            "asterisks denote \"black\" bytes (index=%d)...\n", ncolors - 1);
                    type_bytemap(&mctx, colormap_raster, ncolors, bp->width, bp->height, mctx.msgfp);
                    /* --- rgb values corresponding to colormap indexes */
                    fprintf(mctx.msgfp, "\nThe %d colormap indexes denote rgb values...", ncolors);
                    for (igray = 0; igray < ncolors; igray++) /* show colors[] values */
                        fprintf(mctx.msgfp, "%s%2x-->%3d", (igray % 5 ? "   " : "\n"),
                                igray, (int)(colors[ncolors-1] - colors[igray]));
                    /* always needs a final newline */
                    fprintf(mctx.msgfp, "\n");
                }
            } /* --- end-of-if(isaa) --- */
        } /* --- end-of-if(!isquery||mctx.msglevel>=9) --- */
    }
    /* ------------------------------------------------------------
    emit xbitmap or gif image, and exit
    ------------------------------------------------------------ */

    if (!isquery) {
        if (outfile) {
            FILE *fp = fopen(outfile, "wb");
            if (fp != NULL) {
                if (ptype == 0) {
                    gif_raster(&mctx, ncolors, bp, colormap_raster, colors, fp, NULL, 0);
                } else if (ptype == 1) {
                    if (ncolors == 2)
                        type_pbmpgm(bp, 1, fp);  /* emit b/w pbm file */
                    else
                        fprintf(mctx.msgfp, "-g1 (pbm) doesn't allow grayscaled images\n");
                } else if (ptype == 2) {
                    /*construct arg for write_pbmpgm()*/
                    raster pbm_raster;
                    pbm_raster.width  = bp->width;
                    pbm_raster.height = bp->height;
                    pbm_raster.pixsz  = 8;
                    pbm_raster.pixmap = (pixbyte *)bytemap_raster;
                    type_pbmpgm(&pbm_raster, 2, fp);
                } else if (ptype == 3) {
                    xbitmap_raster(bp, fp);
                }
            }
        }
    } else {
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
        if (iscaching) {            /* image caching enabled */
            /* --- set up path to cached image file --- */
            /* md5 hash of expression */
            /* relative path to cached files */
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
                /* --- emit cached image if it already exists --- */
                if (emitcache(cachefile, maxage, valign, 0) > 0) /* cached image emitted */
                    /* so nothing else to do */
                    goto end_of_job;
                /* --- log caching request --- */
                if (mctx.msglevel >= 1             /* check if logging */
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

        {
            int gifSize = 0;
            char gif_buffer[MAXGIFSZ] = "\000";  /* or gif written in memory buffer */
            FILE *fp = iscaching ? fopen(cachefile, "wb"): NULL;
            if (fp == NULL)
                gifSize = gif_raster(&mctx, ncolors, bp, colormap_raster, colors, NULL, gif_buffer, sizeof(gif_buffer));
            else
                gif_raster(&mctx, ncolors, bp, colormap_raster, colors, fp, NULL, 0);
            /* --- may need to emit image from cached file or from memory --- */
            if (iscaching)            /* caching enabled */
                /*emit cached image (hopefully)*/
                emitcache(cachefile, maxage, valign, 0);
            else          /* or emit image from memory buffer*/
                emitcache(gif_buffer, maxage, valign, gifSize);
        }
    } /* --- end-of-if(isquery) --- */
    /* --- exit --- */
end_of_job:
    if (bytemap_raster != NULL) free(bytemap_raster);
    /*and colormap_raster*/
    if (colormap_raster != NULL)free(colormap_raster);
    /* and free expression */
    if (1 && sp != NULL) delete_subraster(&mctx, sp);
    if (mctx.msgfp != NULL          /* have message/log file open */
            &&   mctx.msgfp != stdout) {       /* and it's not stdout */
        fprintf(mctx.msgfp, "mimeTeX> successful end-of-job at %s\n",
                timestamp(tzdelta, 0));
        /* so log separator line */
        fprintf(mctx.msgfp, "%s\n", dashes);
        fclose(mctx.msgfp);
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

