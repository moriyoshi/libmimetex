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
  

/* ==========================================================================
 * Function:    strdetex ( s, mode )
 * Purpose: Removes/replaces any LaTeX math chars in s
 *      so that s can be displayed "verbatim",
 *      e.g., for error messages.
 * --------------------------------------------------------------------------
 * Arguments:   s (I)       char * to null-terminated string
 *              whose math chars are to be removed/replaced
 *      mode (I)    int containing 0 to _not_ use macros (i.e.,
 *              mimeprep won't be called afterwards),
 *              or containing 1 to use macros that will
 *              be expanded by a subsequent call to mimeprep.
 * --------------------------------------------------------------------------
 * Returns: ( char * )  ptr to "cleaned" copy of s
 *              or "" (empty string) for any error.
 * --------------------------------------------------------------------------
 * Notes:     o The returned pointer addresses a static buffer,
 *      so don't call strdetex() again until you're finished
 *      with output from the preceding call.
 * ======================================================================= */
/* --- entry point --- */
char    *strdetex(char *s, int mode)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* copy of s with no math chars */
    static  char sbuff[4096];
    /* replace _ with -, etc */
    int strreplace();
    /* ------------------------------------------------------------
    Make a clean copy of s
    ------------------------------------------------------------ */
    /* --- check input --- */
    /* initialize in case of error */
    *sbuff = '\000';
    /* no input */
    if (isempty(s)) goto end_of_job;
    /* --- start with copy of s --- */
    /* leave room for replacements */
    strninit(sbuff, s, 2048);
    /* --- make some replacements -- we *must* replace \ { } first --- */
    /*change all \'s to text*/
    strreplace(sbuff, "\\", "\\backslash~\\!\\!", 0);
    /*change all {'s to \lbrace*/
    strreplace(sbuff, "{", "\\lbrace~\\!\\!", 0);
    /*change all }'s to \rbrace*/
    strreplace(sbuff, "}", "\\rbrace~\\!\\!", 0);
    /* --- now our further replacements may contain \directives{args} --- */
    /* change all _'s to \_ */
    if (mode >= 1) strreplace(sbuff, "_", "\\_", 0);
    /*change them to text*/
    else strreplace(sbuff, "_", "\\underline{\\qquad}", 0);
    /* change all <'s to text */
    if (0)strreplace(sbuff, "<", "\\textlangle ", 0);
    /* change all >'s to text */
    if (0)strreplace(sbuff, ">", "\\textrangle ", 0);
    /* change all $'s to text */
    if (0)strreplace(sbuff, "$", "\\textdollar ", 0);
    /* change all $'s to \$ */
    strreplace(sbuff, "$", "\\$", 0);
    /* change all &'s to \& */
    strreplace(sbuff, "&", "\\&", 0);
    /* change all %'s to \% */
    strreplace(sbuff, "%", "\\%", 0);
    /* change all #'s to \# */
    strreplace(sbuff, "#", "\\#", 0);
    /*strreplace(sbuff,"~","\\~",0);*/      /* change all ~'s to \~ */
    /* change all ^'s to \^ */
    strreplace(sbuff, "^", "{\\fs{+2}\\^}", 0);
end_of_job:
    /* back with clean copy of s */
    return (sbuff);
} /* --- end-of-function strdetex() --- */


/* ==========================================================================
 * Function:    strtexchr ( char *string, char *texchr )
 * Purpose: Find first texchr in string, but texchr must be followed
 *      by non-alpha
 * --------------------------------------------------------------------------
 * Arguments:   string (I)  char * to null-terminated string in which
 *              first occurrence of delim will be found
 *      texchr (I)  char * to null-terminated string that
 *              will be searched for
 * --------------------------------------------------------------------------
 * Returns: ( char * )  ptr to first char of texchr in string
 *              or NULL if not found or for any error.
 * --------------------------------------------------------------------------
 * Notes:     o texchr should contain its leading \, e.g., "\\left"
 * ======================================================================= */
/* --- entry point --- */
char    *strtexchr(char *string, char *texchr)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* ptr returned to caller*/
    char    delim, *ptexchr = (char *)NULL;
    /* start or continue up search here*/
    char    *pstring = string;
    /* #chars in texchr */
    int texchrlen = (texchr == NULL ? 0 : strlen(texchr));
    /* ------------------------------------------------------------
    locate texchr in string
    ------------------------------------------------------------ */
    if (string != (char *)NULL       /* check that we got input string */
            &&   texchrlen > 0) {           /* and a texchr to search for */
        while ((ptexchr = strstr(pstring, texchr)) /* look for texchr in string */
                != (char *)NULL)           /* found it */
            if ((delim = ptexchr[texchrlen])   /* char immediately after texchr */
                    /* texchr at very end of string */
                    ==   '\000') break;
            else
            /* if there are chars after texchr */
                if (isalpha(delim)            /*texchr is prefix of longer symbol*/
                        ||   0)              /* other tests to be determined */
                    /* continue search after texchr */
                    pstring = ptexchr + texchrlen;
                else
                /* passed all tests */
                    break;
    }                /*so return ptr to texchr to caller*/
    /* ptr to texchar back to caller */
    return (ptexchr);
} /* --- end-of-function strtexchr() --- */


/* ==========================================================================
 * Function:    findbraces ( char *expression, char *command )
 * Purpose: If expression!=NULL, finds opening left { preceding command;
 *      if expression==NULL, finds closing right } after command.
 *      For example, to parse out {a+b\over c+d} call findbraces()
 *      twice.
 * --------------------------------------------------------------------------
 * Arguments:   expression (I)  NULL to find closing right } after command,
 *              or char * to null-terminated string to find
 *              left opening { preceding command.
 *      command (I) char * to null-terminated string whose
 *              first character is usually the \ of \command
 * --------------------------------------------------------------------------
 * Returns: ( char * )  ptr to either opening { or closing },
 *              or NULL for any error.
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
char    *findbraces(char *expression, char *command)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* true to find left opening { */
    int isopen = (expression == NULL ? 0 : 1);
    char *left = "{", *right = "}",  /* delims bracketing {x\command y} */
         *delim = (isopen ? left : right),   /* delim we want,  { if isopen */
         *match = (isopen ? right : left),   /* matching delim, } if isopen */
         *brace = NULL; /* ptr to delim returned to caller */
    /* pointer increment */
    int inc = (isopen ? -1 : +1);
    /* nesting level, for {{}\command} */
    int level = 1;
    /* start search here */
    char    *ptr = command;
    /* true to set {}'s if none found */
    int setbrace = 1;
    /* ------------------------------------------------------------
    search for left opening { before command, or right closing } after command
    ------------------------------------------------------------ */
    while (1) {              /* search for brace, or until end */
        /* --- next char to check for delim --- */
        /* bump ptr left or right */
        ptr += inc;
        /* --- check for beginning or end of expression --- */
        if (isopen) {              /* going left, check for beginning */
            /* went before start of string */
            if (ptr < expression) break;
        } else {
            /* went past end of string */
            if (*ptr == '\000') break;
        }
        /* --- don't check this char if it's escaped --- */
        if (!isopen || ptr > expression)   /* very first char can't be escaped*/
            if (isthischar(*(ptr - 1), ESCAPE))  /* escape char precedes current */
                /* so don't check this char */
                continue;
        /* --- check for delim --- */
        if (isthischar(*ptr, delim))       /* found delim */
            if (--level == 0) {          /* and it's not "internally" nested*/
                /* set ptr to brace */
                brace = ptr;
                goto end_of_job;
            }      /* and return it to caller */
        /* --- check for matching delim --- */
        if (isthischar(*ptr, match))       /* found matching delim */
            /* so bump nesting level */
            level++;
    } /* --- end-of-while(1) --- */
end_of_job:
    if (brace == (char *)NULL)         /* open{ or close} not found */
        if (setbrace)            /* want to force one at start/end? */
            /* { before expressn, } after cmmnd*/
            brace = ptr;
    /*back to caller with delim or NULL*/
    return (brace);
} /* --- end-of-function findbraces() --- */


/* ==========================================================================
 * Function:    texchar ( expression, chartoken )
 * Purpose: scans expression, returning either its first character,
 *      or the next \sequence if that first char is \,
 *      and a pointer to the first expression char past that.
 * --------------------------------------------------------------------------
 * Arguments:   expression (I)  char * to first char of null-terminated
 *              string containing valid LaTeX expression
 *              to be scanned
 *      chartoken (O)   char * to null-terminated string returning
 *              either the first (non-whitespace) character
 *              of expression if that char isn't \, or else
 *              the \ and everything following it up to
 *              the next non-alphabetic character (but at
 *              least one char following the \ even if
 *              it's non-alpha)
 * --------------------------------------------------------------------------
 * Returns: ( char * )  ptr to the first char of expression
 *              past returned chartoken,
 *              or NULL for any parsing error.
 * --------------------------------------------------------------------------
 * Notes:     o Does *not* skip leading whitespace, but simply
 *      returns any whitespace character as the next character.
 * ======================================================================= */
/* --- entry point --- */
char    *texchar(mimetex_ctx *mctx, char *expression, char *chartoken)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    int esclen = 0,             /*length of escape sequence*/
                 /* max len of esc sequence */
                 maxesclen = 128;
    /* ptr into chartoken */
    char    *ptoken = chartoken;
    /* prefix index */
    int iprefix = 0;
    /*e.g., \big followed by ( */
    static  char *prefixes[] =
    {
        /* "\\left", "\\right", */
        "\\big",  "\\Big",  "\\bigg",  "\\Bigg",
        "\\bigl", "\\Bigl", "\\biggl", "\\Biggl",
        "\\bigr", "\\Bigr", "\\biggr", "\\Biggr", NULL
    };
    /* may be followed by * */
    static  char *starred[] = {
        "\\hspace",  "\\!",  NULL
    };
    /* ------------------------------------------------------------
    just return the next char if it's not \
    ------------------------------------------------------------ */
    /* --- error check for end-of-string --- */
    /* init in case of error */
    *ptoken = '\000';
    /* nothing to scan */
    if (expression == NULL) return(NULL);
    /* nothing to scan */
    if (*expression == '\000') return(NULL);
    /* --- always returning first character (either \ or some other char) --- */
    /* here's first character */
    *ptoken++ = *expression++;
    /* --- if first char isn't \, then just return it to caller --- */
    if (!isthischar(*(expression - 1), ESCAPE)) { /* not a \, so return char */
        /* add a null terminator */
        *ptoken = '\000';
        goto end_of_job;
    }              /* ptr past returned char */
    if (*expression == '\000') {         /* \ is very last char */
        /* flush bad trailing \ */
        *chartoken = '\000';
        return(NULL);
    }             /* and signal end-of-job */
    /* ------------------------------------------------------------
    we have an escape sequence, so return all alpha chars following \
    ------------------------------------------------------------ */
    /* --- accumulate chars until first non-alpha char found --- */
    for (; isalpha(*expression); esclen++) { /* till first non-alpha... */
        if (esclen < maxesclen - 3)          /* more room in chartoken */
            /*copy alpha char, bump ptr*/
            *ptoken++ = *expression;
        expression++;
    }             /* bump expression ptr */
    /* --- if we have a prefix, append next texchar, e.g., \big( --- */
    /* set null for compare */
    *ptoken = '\000';
    for (iprefix = 0; prefixes[iprefix] != NULL; iprefix++) /* run thru list */
        if (strcmp(chartoken, prefixes[iprefix]) == 0) { /* have an exact match */
            char nextchar[256];
            /* texchar after prefix */
            int nextlen = 0;
            /* skip space after prefix*/
            skipwhite(expression);
            /* get nextchar */
            expression = texchar(mctx, expression, nextchar);
            if ((nextlen = strlen(nextchar)) > 0) {  /* #chars in nextchar */
                /* append nextchar */
                strcpy(ptoken, nextchar);
                /* point to null terminator*/
                ptoken += strlen(nextchar);
                esclen += strlen(nextchar);
            }       /* and bump escape length */
            break;
        }                    /* stop checking prefixes */
    /* --- every \ must be followed by at least one char, e.g., \[ --- */
    if (esclen < 1)                  /* \ followed by non-alpha */
        /*copy non-alpha, bump ptrs*/
        *ptoken++ = *expression++;
    /* null-terminate token */
    *ptoken = '\000';
    /* --- check for \hspace* or other starred commands --- */
    for (iprefix = 0; starred[iprefix] != NULL; iprefix++) /* run thru list */
        if (strcmp(chartoken, starred[iprefix]) == 0)   /* have an exact match */
            if (*expression == '*') {          /* follows by a * */
                /* copy * and bump ptr */
                *ptoken++ = *expression++;
                /* null-terminate token */
                *ptoken = '\000';
                break;
            }                   /* stop checking */
    /* --- respect spaces in text mode, except first space after \escape --- */
    if (esclen >= 1) {               /*only for alpha \sequences*/
        if (istextmode)                /* in \rm or \it text mode */
            if (isthischar(*expression, WHITEDELIM))  /* delim follows \sequence */
                expression++;
    }             /* so flush delim */
    /* --- back to caller --- */
end_of_job:
    if (mctx->msgfp != NULL && mctx->msglevel >= 999) {
        fprintf(mctx->msgfp, "texchar> returning token = \"%s\"\n", chartoken);
        fflush(mctx->msgfp);
    }
    /*ptr to 1st non-alpha char*/
    return (expression);
} /* --- end-of-function texchar() --- */


/* ==========================================================================
 * Function:    texsubexpr (expression,subexpr,maxsubsz,
 *      left,right,isescape,isdelim)
 * Purpose: scans expression, returning everything between a balanced
 *      left{...right} subexpression if the first non-whitespace
 *      char of expression is an (escaped or unescaped) left{,
 *      or just the next texchar(mctx, ) otherwise,
 *      and a pointer to the first expression char past that.
 * --------------------------------------------------------------------------
 * Arguments:   expression (I)  char * to first char of null-terminated
 *              string containing valid LaTeX expression
 *              to be scanned
 *      subexpr (O) char * to null-terminated string returning
 *              either everything between a balanced {...}
 *              subexpression if the first char is {,
 *              or the next texchar(mctx, ) otherwise.
 *      maxsubsz (I)    int containing max #bytes returned
 *              in subexpr buffer (0 means unlimited)
 *      left (I)    char * specifying allowable left delimiters
 *              that begin subexpression, e.g., "{[(<"
 *      right (I)   char * specifying matching right delimiters
 *              in the same order as left, e.g., "}])>"
 *      isescape (I)    int controlling whether escaped and/or
 *              unescaped left,right are matched;
 *              see isbrace() comments below for details.
 *      isdelim (I) int containing true (non-zero) to return
 *              the leading left and trailing right delims
 *              (if any were found) along with subexpr,
 *              or containing false=0 to return subexpr
 *              without its delimiters
 * --------------------------------------------------------------------------
 * Returns: ( char * )  ptr to the first char of expression
 *              past returned subexpr (see Notes),
 *              or NULL for any parsing error.
 * --------------------------------------------------------------------------
 * Notes:     o If subexpr is of the form left{...right},
 *      the outer {}'s are returned as part of subexpr
 *      if isdelim is true; if isdelim is false the {}'s aren't
 *      returned.  In either case the returned pointer is
 *      *always* bumped past the closing right}, even if
 *      that closing right} isn't returned in subexpr.
 *        o If subexpr is not of the form left{...right},
 *      the returned pointer is on the character immediately
 *      following the last character returned in subexpr
 *        o \. acts as LaTeX \right. and matches any \left(
 *      And it also acts as a LaTeX \left. and matches any \right)
 * ======================================================================= */
/* --- entry point --- */
char    *texsubexpr(mimetex_ctx *mctx, char *expression, char *subexpr, int maxsubsz,
                    char *left, char *right, int isescape, int isdelim)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /*next char (or \sequence) from expression*/
    char    *leftptr, leftdelim[256] = "(\000", /* left( found in expression */
                                       /* and matching right) */
                                       rightdelim[256] = ")\000";
    /*original inputs*/
    char    *origexpression = expression, *origsubexpr = subexpr;
    /* check for \left, and get it */
    int gotescape = 0,      /* true if leading char of expression is \ */
                    /* while parsing, true if preceding char \ */
                    prevescape = 0;
    /* true matches any right with left, (...] */
    int isanyright = 1;
    /* true if left brace is a \. */
    int isleftdot = 0;
    /* current # of nested braces */
    int nestlevel = 1;
    int subsz = 0 /*,maxsubsz=MAXSUBXSZ*/; /*#chars in returned subexpr buffer*/
    /* ------------------------------------------------------------
    skip leading whitespace and just return the next char if it's not {
    ------------------------------------------------------------ */
    /* --- skip leading whitespace and error check for end-of-string --- */
    /* init in case of error */
    *subexpr = '\000';
    /*can't dereference null ptr*/
    if (expression == NULL) return(NULL);
    /* leading whitespace gone */
    skipwhite(expression);
    /* nothing left to scan */
    if (*expression == '\000') return(NULL);
    /* --- set maxsubsz --- */
    /* input 0 means unlimited */
    if (maxsubsz < 1) maxsubsz = MAXSUBXSZ - 2;
    /* --- check for escape --- */
    if (isthischar(*expression, ESCAPE))         /* expression is escaped */
        /* so set flag accordingly */
        gotescape = 1;
    /* --- check for \left...\right --- */
    if (gotescape)               /* begins with \ */
        if (memcmp(expression + 1, "left", 4))      /* and followed by left */
            if (strchr(left, 'l') != NULL)         /* caller wants \left's */
                if (strtexchr(expression, "\\left") == expression) { /*expression=\left...*/
                    char *pright = texleft(mctx, expression, subexpr, maxsubsz, /* find ...\right*/
                                           (isdelim ? NULL : leftdelim), rightdelim);
                    /* caller wants delims */
                    if (isdelim) strcat(subexpr, rightdelim);
                    /*back to caller past \right*/
                    return (pright);
                } /* --- end-of-if(expression=="\\left") --- */
    /* --- if first char isn't left{ or script, just return it to caller --- */
    if (!isbrace(mctx, expression, left, isescape)) {  /* not a left{ */
        if (!isthischar(*expression, SCRIPTS))     /* and not a script */
            /* next char to caller */
            return (texchar(mctx, expression, subexpr));
        else { /* --- kludge for super/subscripts to accommodate texscripts() --- */
            /* signal script */
            *subexpr++ = *expression;
            /* null-terminate subexpr */
            *subexpr = '\000';
            return (expression);
        }
    }     /* leave script in stream */
    /* --- extract left and find matching right delimiter --- */
    /* the left( in expression */
    *leftdelim  = *(expression + gotescape);
    if ((gotescape && *leftdelim == '.')         /* we have a left \. */
            || (gotescape && isanyright)) {         /*or are matching any right*/
        /* so just set flag */
        isleftdot = 1;
        *leftdelim = '\000';
    }          /* and reset leftdelim */
    else
    /* find matching \right */
        if ((leftptr = strchr(left, *leftdelim)) != NULL) /* ptr to that left( */
            /* get the matching right) */
            *rightdelim = right[(int)(leftptr-left)];
        else
        /* can't happen -- pgm bug */
            /*just signal eoj to caller*/
            return (NULL);
    /* ------------------------------------------------------------
    accumulate chars between balanced {}'s, i.e., till nestlevel returns to 0
    ------------------------------------------------------------ */
    /* --- first initialize by bumping past left{ or \{ --- */
    /*caller wants { in subexpr*/
    if (isdelim)   *subexpr++ = *expression++;
    /* always bump past left{ */
    else expression++;
    if (gotescape) {                 /*need to bump another char*/
        /* caller wants char, too */
        if (isdelim) *subexpr++ = *expression++;
        else expression++;
    }              /* else just bump past it */
    /* --- set maximum size for numerical arguments --- */
    if (0)                   /* check turned on or off? */
        if (!isescape && !isdelim)              /*looking for numerical arg*/
            /* set max arg size */
            maxsubsz = 96;
    /* --- search for matching right} --- */
    while (1) {                  /*until balanced right} */
        /* --- error check for end-of-string --- */
        if (*expression == '\000') {           /* premature end-of-string */
            if (0 && (!isescape && !isdelim)) {    /*looking for numerical arg,*/
                /* so end-of-string is error*/
                expression = origexpression;
                subexpr = origsubexpr;
            }      /* so reset all ptrs */
            if (isdelim) {                 /* generate fake right */
                if (gotescape) {         /* need escaped right */
                    /* set escape char */
                    *subexpr++ = '\\';
                    *subexpr++ = '.';
                }         /* and fake \right. */
                else
                /* escape not wanted */
                    *subexpr++ = *rightdelim;
            }     /* so fake actual right */
            /* null-terminate subexpr */
            *subexpr = '\000';
            return (expression);
        }           /* back with final token */
        /* --- check preceding char for escape --- */
        if (isthischar(*(expression - 1), ESCAPE)) /* previous char was \ */
            /* so flip escape flag */
            prevescape = 1 - prevescape;
        /* or turn flag off */
        else  prevescape = 0;
        /* --- check for { and } (un/escaped as per leading left) --- */
        if (gotescape == prevescape)       /* escaped iff leading is */
        {
        /* --- check for (closing) right delim and see if we're done --- */
            if (isthischar(*expression, rightdelim)    /* found a right} */
                    || (isleftdot && isthischar(*expression, right))  /*\left. matches all*/
                    || (prevescape && isthischar(*expression, ".")))   /*or found \right. */
                if (--nestlevel < 1) {           /*\right balances 1st \left*/
                    if (isdelim)             /*caller wants } in subexpr*/
                        /* so end subexpr with } */
                        *subexpr++ = *expression;
                    else
                    /*check for \ before right}*/
                        if (prevescape)            /* have unwanted \ */
                            /* so replace it with null */
                            *(subexpr - 1) = '\000';
                    /* null-terminate subexpr */
                    *subexpr = '\000';
                    return (expression + 1);
                }       /* back with char after } */
            /* --- check for (another) left{ --- */
            if (isthischar(*expression, leftdelim) /* found another left{ */
                    || (isleftdot && isthischar(*expression, left)))   /* any left{ */
                nestlevel++;
        } /* --- end-of-if(gotescape==prevescape) --- */
        /* --- not done, so copy char to subexpr and continue with next char --- */
        if (++subsz < maxsubsz - 5)            /* more room in subexpr */
            /* so copy char and bump ptr*/
            *subexpr++ = *expression;
        /* bump expression ptr */
        expression++;
    } /* --- end-of-while(1) --- */
} /* --- end-of-function texsubexpr() --- */


/* ==========================================================================
 * Function:    texleft (expression,subexpr,maxsubsz,ldelim,rdelim)
 * Purpose: scans expression, starting after opening \left,
 *      and returning ptr after matching closing \right.
 *      Everything between is returned in subexpr, if given.
 *      Likewise, if given, ldelim returns delimiter after \left
 *      and rdelim returns delimiter after \right.
 *      If ldelim is given, the returned subexpr doesn't include it.
 *      If rdelim is given, the returned pointer is after that delim.
 * --------------------------------------------------------------------------
 * Arguments:   expression (I)  char * to first char of null-terminated
 *              string immediately following opening \left
 *      subexpr (O) char * to null-terminated string returning
 *              either everything between balanced
 *              \left ... \right.  If leftdelim given,
 *              subexpr does _not_ contain that delimiter.
 *      maxsubsz (I)    int containing max #bytes returned
 *              in subexpr buffer (0 means unlimited)
 *      ldelim (O)  char * returning delimiter following
 *              opening \left
 *      rdelim (O)  char * returning delimiter following
 *              closing \right
 * --------------------------------------------------------------------------
 * Returns: ( char * )  ptr to the first char of expression
 *              past closing \right, or past closing
 *              right delimiter if rdelim!=NULL,
 *              or NULL for any error.
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
char    *texleft(mimetex_ctx *mctx, char *expression, char *subexpr, int maxsubsz,
                 char *ldelim, char *rdelim)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* locate matching \right */
    char *pright = expression;
    /* tex delimiters */
    static  char left[16] = "\\left", right[16] = "\\right";
    /* #chars between \left...\right */
    int sublen = 0;
    /* ------------------------------------------------------------
    initialization
    ------------------------------------------------------------ */
    /* --- init output --- */
    /* init subexpr, if given */
    if (subexpr != NULL) *subexpr = '\000';
    /* init ldelim,  if given */
    if (ldelim  != NULL) *ldelim  = '\000';
    /* init rdelim,  if given */
    if (rdelim  != NULL) *rdelim  = '\000';
    /* --- check args --- */
    /* no input supplied */
    if (expression == NULL) goto end_of_job;
    /* nothing after \left */
    if (*expression == '\000') goto end_of_job;
    /* --- determine left delimiter  --- */
    if (ldelim != NULL) {            /* caller wants left delim */
        /* interpret \left ( as \left( */
        skipwhite(expression);
        expression = texchar(mctx, expression, ldelim);
    } /*delim from expression*/
    /* ------------------------------------------------------------
    locate \right balancing opening \left
    ------------------------------------------------------------ */
    /* --- first \right following \left --- */
    if ((pright = strtexchr(expression, right)) /* look for \right after \left */
            !=   NULL) {                /* found it */
        /* --- find matching \right by pushing past any nested \left's --- */
        /* start after first \left( */
        char *pleft = expression;
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
    /* --- set subexpression length, push pright past \right --- */
    if (pright != (char *)NULL) {        /* found matching \right */
        /* #chars between \left...\right */
        sublen = (int)(pright - expression);
        pright += strlen(right);
    }       /* so push pright past \right */
    /* ------------------------------------------------------------
    get rightdelim and subexpr between \left...\right
    ------------------------------------------------------------ */
    /* --- get delimiter following \right --- */
    if (rdelim != NULL) {            /* caller wants right delim */
        if (pright == (char *)NULL) {       /* assume \right. at end of exprssn*/
            /* set default \right. */
            strcpy(rdelim, ".");
            /* use entire remaining expression */
            sublen = strlen(expression);
            pright = expression + sublen;
        } /* and push pright to end-of-string*/
        else {                 /* have explicit matching \right */
            /* interpret \right ) as \right) */
            skipwhite(pright);
            /* pull delim from expression */
            pright = texchar(mctx, pright, rdelim);
            if (*rdelim == '\000') strcpy(rdelim, ".");
        }
    } /* or set \right. */
    /* --- get subexpression between \left...\right --- */
    if (sublen > 0)              /* have subexpr */
        if (subexpr != NULL) {          /* and caller wants it */
            /* max buffer size */
            if (maxsubsz > 0) sublen = min2(sublen, maxsubsz - 1);
            /* stuff between \left...\right */
            memcpy(subexpr, expression, sublen);
            subexpr[sublen] = '\000';
        }       /* null-terminate subexpr */
end_of_job:
    if (mctx->msglevel >= 99 && mctx->msgfp != NULL) {
        fprintf(mctx->msgfp, "texleft> ldelim=%s, rdelim=%s, subexpr=%.128s\n",
                (ldelim == NULL ? "none" : ldelim), (rdelim == NULL ? "none" : rdelim),
                (subexpr == NULL ? "none" : subexpr));
        fflush(mctx->msgfp);
    }
    return (pright);
} /* --- end-of-function texleft --- */


/* ==========================================================================
 * Function:    texscripts ( expression, subscript, superscript, which )
 * Purpose: scans expression, returning subscript and/or superscript
 *      if expression is of the form _x^y or ^{x}_{y},
 *      or any (valid LaTeX) permutation of the above,
 *      and a pointer to the first expression char past "scripts"
 * --------------------------------------------------------------------------
 * Arguments:   expression (I)  char * to first char of null-terminated
 *              string containing valid LaTeX expression
 *              to be scanned
 *      subscript (O)   char * to null-terminated string returning
 *              subscript (without _), if found, or "\000"
 *      superscript (O) char * to null-terminated string returning
 *              superscript (without ^), if found, or "\000"
 *      which (I)   int containing 1 for subscript only,
 *              2 for superscript only, >=3 for either/both
 * --------------------------------------------------------------------------
 * Returns: ( char * )  ptr to the first char of expression
 *              past returned "scripts" (unchanged
 *              except for skipped whitespace if
 *              neither subscript nor superscript found),
 *              or NULL for any parsing error.
 * --------------------------------------------------------------------------
 * Notes:     o an input expression like ^a^b_c will return superscript="b",
 *      i.e., totally ignoring all but the last "script" encountered
 * ======================================================================= */
/* --- entry point --- */
char    *texscripts(mimetex_ctx *mctx, char *expression, char *subscript,
                    char *superscript, int which)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* next subexpression from expression */
    char    *texsubexpr();
    /* check that we don't eat, e.g., x_1_2 */
    int gotsub = 0, gotsup = 0;
    /* ------------------------------------------------------------
    init "scripts"
    ------------------------------------------------------------ */
    /*init in case no subscript*/
    if (subscript != NULL) *subscript = '\000';
    /*init in case no super*/
    if (superscript != NULL) *superscript = '\000';
    /* ------------------------------------------------------------
    get subscript and/or superscript from expression
    ------------------------------------------------------------ */
    while (expression != NULL) {
        /* leading whitespace gone */
        skipwhite(expression);
        /* nothing left to scan */
        if (*expression == '\000') return(expression);
        if (isthischar(*expression, SUBSCRIPT) /* found _ */
                && (which == 1 || which > 2)) {       /* and caller wants it */
            if (gotsub                 /* found 2nd subscript */
                    /* or no subscript buffer */
                    ||   subscript == NULL) break;
            /* set subscript flag */
            gotsub = 1;
            expression = texsubexpr(mctx, expression + 1, subscript, 0, "{", "}", 0, 0);
        } else                     /* no _, check for ^ */
            if (isthischar(*expression, SUPERSCRIPT) /* found ^ */
                    &&   which >= 2) {              /* and caller wants it */
                if (gotsup               /* found 2nd superscript */
                        /* or no superscript buffer*/
                        ||   superscript == NULL) break;
                /* set superscript flag */
                gotsup = 1;
                expression = texsubexpr(mctx, expression + 1, superscript, 0, "{", "}", 0, 0);
            } else                   /* neither _ nor ^ */
                /*return ptr past "scripts"*/
                return (expression);
    } /* --- end-of-while(expression!=NULL) --- */
    return (expression);
} /* --- end-of-function texscripts() --- */


/* ==========================================================================
 * Function:    isbrace ( expression, braces, isescape )
 * Purpose: checks leading char(s) of expression for a brace,
 *      either escaped or unescaped depending on isescape,
 *      except that { and } are always matched, if they're
 *      in braces, regardless of isescape.
 * --------------------------------------------------------------------------
 * Arguments:   expression (I)  char * to first char of null-terminated
 *              string containing a valid LaTeX expression
 *              whose leading char(s) are checked for braces
 *              that begin subexpression, e.g., "{[(<"
 *      braces (I)  char * specifying matching brace delimiters
 *              to be checked for, e.g., "{[(<" or "}])>"
 *      isescape (I)    int containing 0 to match only unescaped
 *              braces, e.g., (...) or {...}, etc,
 *              or containing 1 to match only escaped
 *              braces, e.g., \(...\) or \[...\], etc,
 *              or containing 2 to match either.
 *              But note: if {,} are in braces
 *              then they're *always* matched whether
 *              escaped or not, regardless of isescape.
 * --------------------------------------------------------------------------
 * Returns: ( int )     1 if the leading char(s) of expression
 *              is a brace, or 0 if not.
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int isbrace(mimetex_ctx *mctx, char *expression, char *braces, int isescape)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    int gotescape = 0,      /* true if leading char is an escape */
                    /*true if first non-escape char is a brace*/
                    gotbrace = 0;
    /* ------------------------------------------------------------
    check for brace
    ------------------------------------------------------------ */
    /* --- first check for end-of-string or \= ligature --- */
    if (*expression == '\000'            /* nothing to check */
            /* have a \= ligature */
            ||   mctx->isligature) goto end_of_job;
    /* --- check leading char for escape --- */
    if (isthischar(*expression, ESCAPE)) {       /* expression is escaped */
        /* so set flag accordingly */
        gotescape = 1;
        expression++;
    }             /* and bump past escape */
    /* --- check (maybe next char) for brace --- */
    if (isthischar(*expression, braces))         /* expression is braced */
        /* so set flag accordingly */
        gotbrace = 1;
    if (gotescape && *expression == '.')         /* \. matches any brace */
        /* set flag */
        gotbrace = 1;
    /* --- check for TeX brace { or } --- */
    if (gotbrace && isthischar(*expression, "{}"))   /*expression has TeX brace*/
        /* reset escape flag */
        if (isescape) isescape = 2;
    /* ------------------------------------------------------------
    back to caller
    ------------------------------------------------------------ */
end_of_job:
    if (mctx->msglevel >= 999 && mctx->msgfp != NULL) {
        fprintf(mctx->msgfp, "isbrace> expression=%.8s, gotbrace=%d (mctx->isligature=%d)\n",
                expression, gotbrace, mctx->isligature);
        fflush(mctx->msgfp);
    }
    if (gotbrace &&                 /* found a brace */
            (isescape == 2 ||               /* escape irrelevant */
             gotescape == isescape)           /* un/escaped as requested */
       ) return (1);
    /* return 1,0 accordingly */
    return (0);
} /* --- end-of-function isbrace() --- */


/* ==========================================================================
 * Function:    preamble ( expression, size, subexpr )
 * Purpose: parses $-terminated preamble, if present, at beginning
 *      of expression, re-setting size if necessary, and
 *      returning any other parameters besides size in subexpr.
 * --------------------------------------------------------------------------
 * Arguments:   expression (I)  char * to first char of null-terminated
 *              string containing LaTeX expression possibly
 *              preceded by $-terminated preamble
 *      size (I/O)  int *  containing 0-4 default font size,
 *              and returning size modified by first
 *              preamble parameter (or unchanged)
 *      subexpr(O)  char *  returning any remaining preamble
 *              parameters past size
 * --------------------------------------------------------------------------
 * Returns: ( char * )  ptr to first char past preamble in expression
 *              or NULL for any parsing error.
 * --------------------------------------------------------------------------
 * Notes:     o size can be any number >=0. If preceded by + or -, it's
 *      interpreted as an increment to input size; otherwise
 *      it's interpreted as the size.
 *        o if subexpr is passed as NULL ptr, then returned expression
 *      ptr will have "flushed" and preamble parameters after size
 * ======================================================================= */
/* --- entry point --- */
char    *preamble(mimetex_ctx *mctx, char *expression, int *size, char *subexpr)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    char    pretext[512], *prep = expression, /*pream from expression, ptr into it*/
                                  /* preamble delimiters */
                                  *dollar, *comma;
    int prelen = 0,         /* preamble length */
                 sizevalue = 0,          /* value of size parameter */
                             isfontsize = 0,         /*true if leading mctx->fontsize present*/
                                          /*true to increment passed size arg*/
                                          isdelta = 0;
    /* ------------------------------------------------------------
    initialization
    ------------------------------------------------------------ */
    if (subexpr != NULL)             /* caller passed us an address */
        /* so init assuming no preamble */
        *subexpr = '\000';
    /* no input */
    if (expression == NULL) goto end_of_job;
    /* input is an empty string */
    if (*expression == '\000') goto end_of_job;
    /* ------------------------------------------------------------
    process preamble if present
    ------------------------------------------------------------ */
    /*process_preamble:*/
    if ((dollar = strchr(expression, '$')) /* $ signals preceding preamble */
            !=   NULL) {                /* found embedded $ */
        if ((prelen = (int)(dollar - expression)) /*#chars in expression preceding $*/
                > 0) {                 /* must have preamble preceding $ */
            if (prelen < 65) {             /* too long for a prefix */
                /* local copy of preamble */
                memcpy(pretext, expression, prelen);
                /* null-terminated */
                pretext[prelen] = '\000';
                if (strchr(pretext, *(ESCAPE)) == NULL  /*shouldn't be an escape in preamble*/
                        &&   strchr(pretext, '{') == NULL) { /*shouldn't be a left{ in preamble*/
                    /* --- skip any leading whitespace  --- */
                    /* start at beginning of preamble */
                    prep = pretext;
                    /* skip any leading white space */
                    skipwhite(prep);
                    /* --- check for embedded , or leading +/- (either signalling size) --- */
                    if (isthischar(*prep, "+-"))     /* have leading + or - */
                        /* so use size value as increment */
                        isdelta = 1;
                    /* , signals leading size param */
                    comma = strchr(pretext, ',');
                    /* --- process leading size parameter if present --- */
                    if (comma != NULL            /* size param explicitly signalled */
                            ||   isdelta || isdigit(*prep)) {   /* or inferred implicitly */
                        /* --- parse size parameter and reset size accordingly --- */
                        /*, becomes null, terminating size*/
                        if (comma != NULL) *comma = '\000';
                        /* convert size string to integer */
                        sizevalue = atoi(prep);
                        if (size != NULL)          /* caller passed address for size */
                            /* so reset size */
                            *size = (isdelta ? *size + sizevalue : sizevalue);
                        /* --- finally, set flag and shift size parameter out of preamble --- */
                        /*set flag showing font size present*/
                        isfontsize = 1;
                        /*leading size param gone*/
                        if (comma != NULL) strcpy(pretext, comma + 1);
                    } /* --- end-of-if(comma!=NULL||etc) --- */
                    /* --- copy any preamble params following size to caller's subexpr --- */
                    if (comma != NULL || !isfontsize)    /*preamb contains params past size*/
                        if (subexpr != NULL)        /* caller passed us an address */
                            /*so return extra params to caller*/
                            strcpy(subexpr, pretext);
                    /* --- finally, set prep to shift preamble out of expression --- */
                    /* set prep past $ in expression */
                    prep = expression + prelen + 1;
                } /* --- end-of-if(strchr(pretext,*ESCAPE)==NULL) --- */
            } /* --- end-of-if(prelen<65) --- */
        } /* --- end-of-if(prelen>0) --- */
        else {                 /* $ is first char of expression */
            /* number of $...$ pairs removed */
            int ndollars = 0;
            /* start at beginning of expression*/
            prep = expression;
            while (*prep == '$') {         /* remove all matching $...$'s */
                /* index of last char in expression*/
                int  explen = strlen(prep) - 1;
                /* no $...$'s left to remove */
                if (explen < 2) break;
                /* unmatched $ */
                if (prep[explen] != '$') break;
                /* remove trailing $ */
                prep[explen] = '\000';
                /* and remove matching leading $ */
                prep++;
                /* count another pair removed */
                ndollars++;
            } /* --- end-of-while(*prep=='$') --- */
            /* set flag to fix \displaystyle */
            mctx->ispreambledollars = ndollars;
            if (ndollars == 1)             /* user submitted $...$ expression */
                /* so set \textstyle */
                mctx->isdisplaystyle = 0;
            if (ndollars > 1)              /* user submitted $$...$$ */
                /* so set \displaystyle */
                mctx->isdisplaystyle = 2;
            /*goto process_preamble;*/        /*check for preamble after leading $*/
        } /* --- end-of-if/else(prelen>0) --- */
    } /* --- end-of-if(dollar!=NULL) --- */
    /* ------------------------------------------------------------
    back to caller
    ------------------------------------------------------------ */
end_of_job:
    /*expression, or ptr past preamble*/
    return (prep);
} /* --- end-of-function preamble() --- */


/* ==========================================================================
 * Function:    mimeprep ( expression )
 * Purpose: preprocessor for mimeTeX input, e.g.,
 *      (a) removes comments,
 *      (b) converts \left( to \( and \right) to \),
 *      (c) xlates &html; special chars to equivalent latex
 *      Should only be called once (after unescape_url())
 * --------------------------------------------------------------------------
 * Arguments:   expression (I/O) char * to first char of null-terminated
 *              string containing mimeTeX/LaTeX expression,
 *              and returning preprocessed string
 * --------------------------------------------------------------------------
 * Returns: ( char * )  ptr to input expression,
 *              or NULL for any parsing error.
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
char *mimeprep(mimetex_ctx *mctx, char *expression)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    char    *expptr = expression,   /* ptr within expression */
                      *tokptr = NULL,         /*ptr to token found in expression*/
                                /*parse for macro args after token*/
                                *texsubexpr(), argval[8192];
    /* change leading chars of string */
    char    *strchange();
    /* replace nnn with actual num, etc*/
    int strreplace();
    /*use strwstr() instead of strstr()*/
    char    *strwstr();
    /*find left { and right } for \atop*/
    char    *findbraces();
    int idelim = 0,         /* left- or right-index */
                 /*symbols[],rightcomment[],etc index*/
                 isymbol = 0;
    /* true to xlate \left and \right */
    int xlateleft = 0;
    /* ---
     * comments
     * -------- */
    /* find leftcomment in expression */
    char    *leftptr = NULL;
    static  char *leftcomment = "%%",   /* open comment */
                 *rightcomment[] = {"\n", "%%", NULL}; /* close comments */
    /* ---
     * special long (more than 1-char) \left and \right delimiters
     * ------------------------------------------------------------ */
    /* xlate any \left suffix... */
    static  char *leftfrom[] = {
        "\\|",/* \left\| */
        "\\{", /* \left\{ */
        "\\langle", /* \left\langle */
        NULL
    }; /* --- end-of-leftfrom[] --- */

    /* ...to this instead */
    static  char *leftto[] = {
        "=", /* = */
        "{", /* { */
        "<", /* < */
        NULL
    }; /* --- end-of-leftto[] --- */
    /* xlate any \right suffix... */
    static  char *rightfrom[] = {
        "\\|", /* \right\| */
        "\\}", /* \right\} */
        "\\rangle", /* \right\rangle */
        NULL
    }; /* --- end-of-rightfrom[] --- */
    /* ...to this instead */
    static  char *rightto[] = {
        "=", /* = */
        "}", /* } */
        ">", /* > */
        NULL
    }; /* --- end-of-rightto[] --- */
    /* ---
     * { \atop }-like commands
     * ------------------------------------------------------------ */
    /* atopcommands[isymbol] */
    char    *atopsym = NULL;
    /* list of {a+b\command c+d}'s */
    static  char *atopcommands[] = {
        "\\over", /* plain tex for \frac */
        "\\choose", /* binomial coefficient */
#ifndef NOATOP
        "\\atop", /*noatop preserves old mimeTeX rule*/
#endif
        NULL
    }; /* --- end-of-atopcommands[] --- */
    /* delims for atopcommands[] */
    static  char *atopdelims[] = {
        NULL, NULL, /* \\over has no delims */
        "\\left(", "\\right)",     /* \\choose has ( ) delims*/
#ifndef NOATOP           /*noatop preserves old mimeTeX rule*/
        NULL, NULL,            /* \\atop has no delims */
#endif
        NULL, NULL
    }; /* --- end-of-atopdelims[] --- */

    /* ---
     * html special/escape chars converted to latex equivalents
     * ------------------------------------------------------------ */
    /* symbols[isymbol].html */
    char    *htmlsym = NULL;
    static  struct {
        char *html;
        char *args;
        char *latex;
    } symbols[] = {
#ifdef NEWCOMMANDS           /* -DNEWCOMMANDS=\"filename.h\" */
    /* ------------------------------------------------------------
     user-supplied newcommands
    ------------------------------------------------------------ */
#   include NEWCOMMANDS
#endif
        /* ------------------------------------------------------------
        Specials        termchar  value...
        ------------------------------------------------------------ */
        {
            "\\version",   NULL,
            "{\\small\\red\\text \\fbox{\\begin{gather}"
            "mime\\TeX version \\versionnumber \\\\"
            "last revised \\revisiondate \\\\ \\copyrighttext \\\\"
            "see \\homepagetext for details \\end{gather}}}"
        },
        {
            "\\copyright", NULL,
           "{\\small\\red\\text \\fbox{\\begin{gather}"
           "mimeTeX \\copyrighttext \\\\"
           "see \\homepagetext for details \\end{gather}}}"
        },
        { "\\versionnumber", NULL, "{\\text" VERSION "}" },
        { "\\revisiondate",  NULL, "{\\text" REVISIONDATE "}" },
        {
            "\\copyrighttext", NULL,
            "{\\text Copyright (c) 2002-2009, John Forkosh Associates, Inc.}"
        },
        {
            "\\homepagetext",  NULL,
           "{\\text http://www.forkosh.com/mimetex.html}"
        },
        /* ------------------------------------------------------------
        Cyrillic  termchar  mimeTeX equivalent...
        ------------------------------------------------------------ */
        { "\\\'G",   "embed\\", "{\\acute{G}}" },
        { "\\\'g",   "embed\\", "{\\acute{g}}" },
        { "\\\'K",   "embed\\", "{\\acute{K}}" },
        { "\\\'k",   "embed\\", "{\\acute{k}}" },
        { "\\u U",   "embed\\", "{\\breve{U}}" },
        { "\\u u",   "embed\\", "{\\breve{u}}" },
        { "\\\"I",   "embed\\", "{\\ddot{\\=I}}" },
        { "\\\"\\i", "embed\\", "{\\ddot{\\=\\i}}" },
        /* ------------------------------------------------------------
          LaTeX Macro #args,default  template...
        ------------------------------------------------------------ */
        { "\\lvec",  "2n",   "{#2_1,\\cdots,#2_{#1}}" },
        { "\\grave", "1",    "{\\stackrel{\\Huge\\gravesym}{#1}}" }, /* \grave */
        { "\\acute", "1",    "{\\stackrel{\\Huge\\acutesym}{#1}}" }, /* \acute */
        { "\\check", "1",    "{\\stackrel{\\Huge\\checksym}{#1}}" }, /* \check */
        { "\\breve", "1",    "{\\stackrel{\\Huge\\brevesym}{#1}}" }, /* \breve */
        { "\\buildrel", "3",  "{\\stackrel{#1}{#3}}" }, /* ignore #2 = \over */
        { "\\overset", NULL, "\\stackrel" },     /* just an alias */
        { "\\underset", "2", "\\relstack{#2}{#1}" }, /* reverse args */
        { "\\dfrac", "2",    "{\\frac{#1}{#2}}" },
        { "\\binom", "2",    "{\\begin{pmatrix}{#1}\\\\{#2}\\end{pmatrix}}" },
        /* ------------------------------------------------------------
          html char termchar  LaTeX equivalent...
        ------------------------------------------------------------ */
        { "&quot",   ";",    "\"" },     /* &quot; is first, &#034; */
        { "&amp",    ";",    "&" },
        { "&lt", ";",    "<" },
        { "&gt", ";",    ">" },
        { "&backslash", ";",  "\\" },
        { "&nbsp",   ";",    "~" },
        { "&iexcl",  ";",    "{\\raisebox{-2}{\\rotatebox{180}{!}}}" },
        { "&brvbar", ";",    "|" },
        { "&plusmn", ";",    "\\pm" },
        { "&sup2",   ";",    "{{}^2}" },
        { "&sup3",   ";",    "{{}^3}" },
        { "&micro",  ";",    "\\mu" },
        { "&sup1",   ";",    "{{}^1}" },
        { "&frac14", ";",    "{\\frac14}" },
        { "&frac12", ";",    "{\\frac12}" },
        { "&frac34", ";",    "{\\frac34}" },
        { "&iquest", ";",    "{\\raisebox{-2}{\\rotatebox{180}{?}}}" },
        { "&Acirc",  ";",    "{\\rm~\\hat~A}" },
        { "&Atilde", ";",    "{\\rm~\\tilde~A}" },
        { "&Auml",   ";",    "{\\rm~\\ddot~A}" },
        { "&Aring",  ";",    "{\\rm~A\\limits^{-1$o}}" },
        { "&atilde", ";",    "{\\rm~\\tilde~a}" },
        { "&yuml",   ";",    "{\\rm~\\ddot~y}" }, /* &yuml; is last, &#255; */
        { "&#",  ";",    "{[\\&\\#nnn?]}" },  /* all other explicit &#nnn's */
        /* ------------------------------------------------------------
          html tag     termchar    LaTeX equivalent...
        ------------------------------------------------------------ */
        { "< br >",    "embed\\i", "\\\\" },
        { "< br / >",  "embed\\i", "\\\\" },
        { "< dd >",    "embed\\i", " \000" },
        { "< / dd >",  "embed\\i", " \000" },
        { "< dl >",    "embed\\i", " \000" },
        { "< / dl >",  "embed\\i", " \000" },
        { "< p >",     "embed\\i", " \000" },
        { "< / p >",   "embed\\i", " \000" },
        /* ------------------------------------------------------------
          garbage      termchar  LaTeX equivalent...
        ------------------------------------------------------------ */
        { "< tex >",   "embed\\i", " \000" },
        { "< / tex >", "embed\\i", " \000" },
        /* ------------------------------------------------------------
          LaTeX   termchar   mimeTeX equivalent...
        ------------------------------------------------------------ */
        { "\\AA",    NULL,   "{\\rm~A\\limits^{-1$o}}" },
        { "\\aa",    NULL,   "{\\rm~a\\limits^{-1$o}}" },
        { "\\bmod",  NULL,   "{\\hspace2{\\rm~mod}\\hspace2}" },
        { "\\vdots", NULL,   "{\\raisebox3{\\rotatebox{90}{\\ldots}}}" },
        { "\\dots",  NULL,   "{\\cdots}" },
        { "\\cdots", NULL,   "{\\raisebox3{\\ldots}}" },
        { "\\ldots", NULL,   "{\\fs4.\\hspace1.\\hspace1.}" },
        { "\\ddots", NULL,   "{\\fs4\\raisebox8.\\hspace1\\raisebox4.\\hspace1.}"},
        { "\\notin", NULL,   "{\\not\\in}" },
        { "\\neq",   NULL,   "{\\not=}" },
        { "\\ne",    NULL,   "{\\not=}" },
        { "\\hbar",  NULL,   "{\\compose~h{{\\fs{-1}-\\atop\\vspace3}}}" },
        { "\\angle", NULL, "{\\compose{\\hspace{3}\\lt}{\\circle(10,15;-80,80)}}"},
        { "\\textcelsius", NULL, "{\\textdegree C}"},
        { "\\textdegree", NULL, "{\\Large^{^{\\tiny\\mathbf o}}}"},
        { "\\cr",    NULL,   "\\\\" },
        { "\\iiint", NULL,   "{\\int\\int\\int}\\limits" },
        { "\\iint",  NULL,   "{\\int\\int}\\limits" },
        { "\\Bigiint", NULL, "{\\Bigint\\Bigint}\\limits" },
        { "\\bigsqcap", NULL, "{\\fs{+4}\\sqcap}" },
        { "\\_", "embed", "{\\underline{\\ }}" }, /* displayed underscore */
        { "!`",  NULL,   "{\\raisebox{-2}{\\rotatebox{180}{!}}}" },
        { "?`",  NULL,   "{\\raisebox{-2}{\\rotatebox{180}{?}}}" },
        { "^\'", "embed", "\'" }, /* avoid ^^ when re-xlating \' below */
        { "\'\'\'\'", "embed", "^{\\fs{-1}\\prime\\prime\\prime\\prime}" },
        { "\'\'\'",  "embed", "^{\\fs{-1}\\prime\\prime\\prime}" },
        { "\'\'",    "embed", "^{\\fs{-1}\\prime\\prime}" },
        { "\'",  "embed", "^{\\fs{-1}\\prime}" },
        { "\\rightleftharpoons", NULL, "{\\rightharpoonup\\atop\\leftharpoondown}" },
        { "\\therefore", NULL, "{\\Huge\\raisebox{-4}{.\\atop.\\,.}}" },
        { "\\LaTeX", NULL,   "{\\rm~L\\raisebox{3}{\\fs{-1}A}\\TeX}" },
        { "\\TeX",   NULL,   "{\\rm~T\\raisebox{-3}{E}X}" },
        { "\\cyan",  NULL,   "{\\reverse\\red\\reversebg}" },
        { "\\magenta", NULL,  "{\\reverse\\green\\reversebg}" },
        { "\\yellow", NULL,   "{\\reverse\\blue\\reversebg}" },
        { "\\cancel", NULL,   "\\Not" },
        { "\\hhline", NULL,   "\\Hline" },
        { "\\Hline", NULL,   "\\hline\\,\\\\\\hline" },
        /* ------------------------------------------------------------
          As per emails with Zbigniew Fiedorowicz <fiedorow@math.ohio-state.edu>
          "Algebra Syntax"  termchar   mimeTeX/LaTeX equivalent...
        ------------------------------------------------------------ */
        { "sqrt",    "1",    "{\\sqrt{#1}}" },
        { "sin", "1",    "{\\sin{#1}}" },
        { "cos", "1",    "{\\cos{#1}}" },
        { "asin",    "1",    "{\\sin^{-1}{#1}}" },
        { "acos",    "1",    "{\\cos^{-1}{#1}}" },
        { "exp", "1",    "{{\\rm~e}^{#1}}" },
        { "det", "1",    "{\\left|{#1}\\right|}" },
        /* ------------------------------------------------------------
          LaTeX Constant    termchar   value...
        ------------------------------------------------------------ */
        { "\\thinspace", NULL,   "2" },
        { "\\thinmathspace", NULL,   "2" },
        { "\\textwidth", NULL,   "400" },
        /* --- end-of-table indicator --- */
        { NULL,  NULL,   NULL }
    }; /* --- end-of-symbols[] --- */
    /* ---
     * html &#nn chars converted to latex equivalents
     * ------------------------------------------------------------ */
    /* numbers[inum].html */
    int htmlnum = 0;
    /* ------------------------------------------------------------
    html num  LaTeX equivalent...
   ------------------------------------------------------------ */
    static  struct {
        int html;
        char *latex;
    } numbers[] = {
        { 9,     " " },          /* horizontal tab */
        { 10,    " " },          /* line feed */
        { 13,    " " },          /* carriage return */
        { 32,    " " },          /* space */
        { 33,    "!" },          /* exclamation point */
        { 34,    "\"" },         /* &quot; */
        { 35,    "#" },          /* hash mark */
        { 36,    "$" },          /* dollar */
        { 37,    "%" },          /* percent */
        { 38,    "&" },          /* &amp; */
        { 39,    "\'" },         /* apostrophe (single quote) */
        { 40,    ")" },          /* left parenthesis */
        { 41,    ")" },          /* right parenthesis */
        { 42,    "*" },          /* asterisk */
        { 43,    "+" },          /* plus */
        { 44,    "," },          /* comma */
        { 45,    "-" },          /* hyphen (minus) */
        { 46,    "." },          /* period */
        { 47,    "/" },          /* slash */
        { 58,    ":" },          /* colon */
        { 59,    ";" },          /* semicolon */
        { 60,    "<" },          /* &lt; */
        { 61,    "=" },          /* = */
        { 62,    ">" },          /* &gt; */
        { 63,    "\?" },         /* question mark */
        { 64,    "@" },          /* commercial at sign */
        { 91,    "[" },          /* left square bracket */
        { 92,    "\\" },         /* backslash */
        { 93,    "]" },          /* right square bracket */
        { 94,    "^" },          /* caret */
        { 95,    "_" },          /* underscore */
        { 96,    "`" },          /* grave accent */
        { 123,   "{" },          /* left curly brace */
        { 124,   "|" },          /* vertical bar */
        { 125,   "}" },          /* right curly brace */
        { 126,   "~" },          /* tilde */
        { 160,   "~" },          /* &nbsp; (use tilde for latex) */
        { 166,   "|" },          /* &brvbar; (broken vertical bar) */
        { 173,   "-" },          /* &shy; (soft hyphen) */
        { 177,   "{\\pm}" },     /* &plusmn; (plus or minus) */
        { 215,   "{\\times}" },      /* &times; (plus or minus) */
        { -999,  NULL }
    } ; /* --- end-of-numbers[] --- */
    /* ------------------------------------------------------------
    first remove comments
    ------------------------------------------------------------ */
    /* start search at beginning */
    expptr = expression;
    while ((leftptr = strstr(expptr, leftcomment)) != NULL) {
        /*found leftcomment*/
        /* rightcomment[isymbol] */
        char  *rightsym = NULL;
        /* start rightcomment search here */
        expptr = leftptr + strlen(leftcomment);
        /* --- check for any closing rightcomment, in given precedent order --- */
        if (*expptr != '\000')         /*have chars after this leftcomment*/
            for (isymbol = 0; (rightsym = rightcomment[isymbol]) != NULL; isymbol++)
                if ((tokptr = strstr(expptr, rightsym)) != NULL) { /*found rightcomment*/
                    /* first char after rightcomment */
                    tokptr += strlen(rightsym);
                    if (*tokptr == '\000') {      /*nothing after this rightcomment*/
                        /*so terminate expr at leftcomment*/
                        *leftptr = '\000';
                        break;
                    }          /* and stop looking for comments */
                    /* replace entire comment by ~ */
                    *leftptr = '~';
                    /* and squeeze out comment */
                    strcpy(leftptr + 1, tokptr);
                    goto next_comment;
                }     /* stop looking for rightcomment */
        /* --- no rightcomment after opening leftcomment --- */
        /* so terminate expression */
        *leftptr = '\000';
        /* --- resume search past squeezed-out comment --- */
next_comment:
        /* reached end of expression */
        if (*leftptr == '\000') break;
        /*resume search after this comment*/
        expptr = leftptr + 1;
    } /* --- end-of-while(leftptr!=NULL) --- */
    /* ------------------------------------------------------------
    run thru table, converting all occurrences of each macro to its expansion
    ------------------------------------------------------------ */
    for (isymbol = 0; (htmlsym = symbols[isymbol].html) != NULL; isymbol++) {
        /* length of escape, _without_ ; */
        int   htmllen = strlen(htmlsym);
        /* leading char alphabetic */
        int   isalgebra = isalpha((int)(*htmlsym));
        int   isembedded = 0,         /* true to xlate even if embedded */
              istag = 0, isamp = 0,   /* true for <tag>, &char; symbols */
              isstrwstr = 0,          /* true to use strwstr() */
              wstrlen = 0; /* length of strwstr() match */
        /*left,right delims for alg syntax*/
        char  *aleft = "{([<|", *aright = "})]>|";
        char  embedkeywd[99] = "embed",   /* keyword to signal embedded token*/
              embedterm = '\000'; /* char immediately after embed */
        /* #chars in embedkeywd */
        int   embedlen = strlen(embedkeywd);
        char  *args = symbols[isymbol].args,  /* number {}-args, optional []-arg */
              *htmlterm = args,       /*if *args nonumeric, then html term*/
              *latexsym = symbols[isymbol].latex, /*latex replacement for htmlsym*/
              errorsym[256]; /*or latexsym may point to error msg*/
        char  abuff[8192];
        /* macro expansion params */
        int iarg, nargs = 0;
        /* whitespace chars for strwstr() */
        char  wstrwhite[99];
        /*skip any bogus leading whitespace*/
        skipwhite(htmlsym);
        /* reset length of html token */
        htmllen = strlen(htmlsym);
        /* html <tag> starts with < */
        istag = (isthischar(*htmlsym, "<") ? 1 : 0);
        /* html &char; starts with & */
        isamp = (isthischar(*htmlsym, "&") ? 1 : 0);
        if (args != NULL)              /*we have args (or htmlterm) param*/
            if (*args != '\000') {        /* and it's not an empty string */
                if (strchr("0123456789", *args) != NULL) { /* is 1st char #args=0-9 ? */
                    /* if so, then we have no htmlterm */
                    htmlterm = NULL;
                    *abuff = *args;
                    /* #args char in ascii buffer */
                    abuff[1] = '\000';
                    nargs = atoi(abuff);
                }       /* interpret #args to numeric */
                else if (strncmp(args, embedkeywd, embedlen) == 0) { /*xlate embedded token*/
                    /* length of "embed..." string */
                    int arglen = strlen(args);
                    /* if so, then we have no htmlterm */
                    htmlterm = NULL;
                    /* turn on embedded flag */
                    isembedded = 1 ;
                    if (arglen > embedlen)        /* have embed "allow escape" flag */
                        /* char immediately after "embed" */
                        embedterm = args[embedlen];
                    if (arglen > embedlen + 1) { /* have embed,flag,white for strwstr*/
                        /* turn on strwtsr flag */
                        isstrwstr = 1;
                        strcpy(wstrwhite, args + embedlen + 1);
                    }
                } /*and set its whitespace arg*/
            } /* --- end-of-if(*args!='\000') --- */
        /* re-start search at beginning */
        expptr = expression;
        while ((tokptr = (!isstrwstr ? strstr(expptr, htmlsym) : /* just use strtsr */
                    strwstr(mctx, expptr, htmlsym, wstrwhite, &wstrlen))) /* or use our strwstr */ != NULL) {                /* found another sym */
            /* length of matched sym */
            int  toklen = (!isstrwstr ? htmllen : wstrlen);
            char termchar = *(tokptr + toklen), /* char terminating html sequence */
                 prevchar = (tokptr == expptr ? ' ' : *(tokptr - 1)); /*char preceding html*/
            /* token escaped?*/
            int  isescaped = (isthischar(prevchar, ESCAPE) ? 1 : 0);
            /* total length of escape sequence */
            int  escapelen = toklen;
            /* true to flush (don't xlate) */
            int  isflush = 0;
            /* --- check odd/even backslashes preceding tokens --- */
            if (isescaped) {       /* have one preceding backslash */
                /* ptr to that preceding backslash */
                char *p = tokptr - 1;
                while (p != expptr) {
                    /* and we may have more preceding */
                    p--;
                    /* but we don't, so quit */
                    if (!isthischar(*p, ESCAPE))break;
                    isescaped = 1 - isescaped;
                }
            }  /* or flip isescaped flag if we do */
            /* --- init with "trivial" abuff,escapelen from symbols[] table --- */
            /* default to empty string */
            *abuff = '\000';
            if (latexsym != NULL)          /* table has .latex xlation */
                if (*latexsym != '\000')      /* and it's not an empty string */
                    /* so get local copy */
                    strcpy(abuff, latexsym);
            if (!isembedded)       /*embedded sequences not terminated*/
                if (htmlterm != NULL)         /* sequence may have terminator */
                    /*add terminator*/
                    escapelen += (isthischar(termchar, htmlterm) ? 1 : 0);
            /* --- don't xlate if we just found prefix of longer symbol, etc --- */
            if (!isembedded) {         /* not embedded */
                if (isescaped)       /* escaped */
                    /* set flag to flush escaped token */
                    isflush = 1;
                if (!istag && isalpha((int)termchar))   /* followed by alpha */
                    /* so just a prefix of longer symbol*/
                    isflush = 1;
                if (isalpha((int)(*htmlsym)))    /* symbol starts with alpha */
                    if ((!isspace(prevchar) && isalpha(prevchar))) /* just a suffix*/
                        isflush = 1;
            }      /* set flag to flush token */
            if (isembedded)            /* for embedded token */
                if (isescaped)            /* and embedded \token escaped */
                    if (!isthischar(embedterm, ESCAPE))  /* don't xlate escaped \token */
                        /* set flag to flush token */
                        isflush = 1;
            if (isflush) {         /* don't xlate this token */
                /*toklen;*/
                expptr = tokptr + 1;/* just resume search after token */
                continue;
            }           /* but don't replace it */
            /* --- check for &# prefix signalling &#nnn; --- */
            if (strcmp(htmlsym, "&#") == 0) {  /* replacing special &#nnn; chars */
                /* --- accumulate chars comprising number following &# --- */
                /* chars comprising number after &# */
                char anum[32];
                /* no chars accumulated yet */
                int  inum = 0;
                while (termchar != '\000') {      /* don't go past end-of-string */
                    /* and don't go past digits */
                    if (!isdigit((int)termchar)) break;
                    /* some syntax error in expression */
                    if (inum > 10) break;
                    /* accumulate this digit */
                    anum[inum] = termchar;
                    inum++;
                    /* bump field length, token length */
                    toklen++;
                    termchar = *(tokptr + toklen);
                } /* char terminating html sequence */
                /* null-terminate anum */
                anum[inum] = '\000';
                /* length of &#nnn; sequence */
                escapelen = toklen;
                if (htmlterm != NULL)         /* sequence may have terminator */
                    /*add terminator*/
                    escapelen += (isthischar(termchar, htmlterm) ? 1 : 0);
                /* --- look up &#nnn in number[] table --- */
                /* convert anum[] to an integer */
                htmlnum = atoi(anum);
                /* init error message */
                strninit(errorsym, latexsym, 128);
                /* init latexsym as error message */
                latexsym = errorsym;
                /*place actual &#num in message*/
                strreplace(latexsym, "nnn", anum, 1);
                for (inum = 0; numbers[inum].html >= 0; inum++) /* run thru numbers[] */
                    if (htmlnum ==  numbers[inum].html) {   /* till we find a match */
                        /* latex replacement */
                        latexsym = numbers[inum].latex;
                        break;
                    }         /* no need to look any further */
                if (latexsym != NULL)         /* table has .latex xlation */
                    if (*latexsym != '\000')     /* and it's not an empty string */
                        /* so get local copy */
                        strcpy(abuff, latexsym);
            } /* --- end-of-if(strcmp(htmlsym,"&#")==0) --- */
            /* --- substitute macro arguments --- */
            if (nargs > 0) {           /*substitute #1,#2,... in latexsym*/
                /* nargs begin after macro literal */
                char *arg1ptr = tokptr + escapelen;
                /* ptr 1 char past #args digit 0-9 */
                char *optarg = args + 1;
                /* ptr to beginning of next arg */
                expptr = arg1ptr;
                for (iarg = 1; iarg <= nargs; iarg++) { /* one #`iarg` arg at a time */
                    char argsignal[32] = "#1",  /* #1...#9 signals arg replacement */
                         *argsigptr = NULL; /* ptr to argsignal in abuff[] */
                    /* --- get argument value --- */
                    /* init arg as empty string */
                    *argval = '\000';
                    /* and skip leading white space */
                    skipwhite(expptr);
                    if (iarg == 1 && *optarg != '\000'  /* check for optional [arg] */
                            &&   !isalgebra) {      /* but not in "algebra syntax" */
                        /* init with default value */
                        strcpy(argval, optarg);
                        if (*expptr == '[')   /* but user gave us [argval] */
                            /*so get it*/
                            expptr = texsubexpr(mctx, expptr, argval, 0, "[", "]", 0, 0);
                    } else { /* not optional, so get {argval} */
                        if (*expptr != '\000') {    /* check that some argval provided */
                            if (!isalgebra)       /* only { } delims for latex macro */
                                /*get {argval}*/
                                expptr = texsubexpr(mctx, expptr, argval, 0, "{", "}", 0, 0);
                            else {           /*any delim for algebra syntax macro*/
                                expptr = texsubexpr(mctx, expptr, argval, 0, aleft, aright, 0, 1);
                                if (isthischar(*argval, aleft))  /* have delim-enclosed arg */
                                    if (*argval != '{') {     /* and it's not { }-enclosed */
                                        /* insert opening \left, */
                                        strchange(0, argval, "\\left");
                                        strchange(0, argval + strlen(argval) - 1, "\\right");
                                    }
                            }/*\right*/
                        } /* --- end-of-if(*expptr!='\000') --- */
                    }
                    /* --- (recursively) call mimeprep() to prep the argument --- */
                    if (!isempty(argval))        /* have an argument */
                        /* so (recursively) prep it */
                        mimeprep(mctx, argval);
                    /* --- replace #`iarg` in macro with argval --- */
                    /* #1...#9 signals argument */
                    sprintf(argsignal, "#%d", iarg);
                    while ((argsigptr = strstr(argval, argsignal)) != NULL) /* #1...#9 */
                        /*can't be in argval*/
                        strcpy(argsigptr, argsigptr + strlen(argsignal));
                    while ((argsigptr = strstr(abuff, argsignal)) != NULL) /* #1...#9 */
                        /*replaced by argval*/
                        strchange(strlen(argsignal), argsigptr, argval);
                } /* --- end-of-for(iarg) --- */
                /* add in length of all args */
                escapelen += ((int)(expptr - arg1ptr));
            } /* --- end-of-if(nargs>0) --- */
            /*replace macro or html symbol*/
            strchange(escapelen, tokptr, abuff);
            expptr = tokptr + strlen(abuff); /*resume search after macro / html*/
        } /* --- end-of-while(tokptr!=NULL) --- */
    } /* --- end-of-for(isymbol) --- */
    /* ------------------------------------------------------------
    convert \left( to \(  and  \right) to \),  etc.
    ------------------------------------------------------------ */
    if (xlateleft) {
        /* \left...\right xlation wanted */
        for (idelim = 0; idelim < 2; idelim++) { /* 0 for \left  and  1 for \right */
            /* \left on 1st pass */
            char  *lrstr  = (idelim == 0 ? "\\left" : "\\right");
            /* strlen() of \left or \right */
            int   lrlen   = (idelim == 0 ? 5 : 6);
            char  *braces = (idelim == 0 ? LEFTBRACES "." : RIGHTBRACES "."), /*([{<or)]}>*/
                            **lrfrom = (idelim == 0 ? leftfrom : rightfrom), /* long braces like \| */
                                       **lrto  = (idelim == 0 ? leftto : rightto), /* xlated to 1-char like = */
                                                 /* lrfrom[isymbol] */
                                                 *lrsym  = NULL;
            /* start search at beginning */
            expptr = expression;
            while ((tokptr = strstr(expptr, lrstr)) != NULL) { /* found \left or \right */
                if (isthischar(*(tokptr + lrlen), braces)) { /* followed by a 1-char brace*/
                    /* so squeeze out "left" or "right"*/
                    strcpy(tokptr + 1, tokptr + lrlen);
                    expptr = tokptr + 2;
                }        /* and resume search past brace */
                else {              /* may be a "long" brace like \| */
                    /*init to resume search past\left\rt*/
                    expptr = tokptr + lrlen;
                    for (isymbol = 0; (lrsym = lrfrom[isymbol]) != NULL; isymbol++) {
                        /* #chars in delim, e.g., 2 for \| */
                        int symlen = strlen(lrsym);
                        if (memcmp(tokptr + lrlen, lrsym, symlen) == 0) { /* found long delim*/
                            /* squeeze out delim */
                            strcpy(tokptr + 1, tokptr + lrlen + symlen - 1);
                            /* last char now 1-char delim*/
                            *(tokptr + 1) = *(lrto[isymbol]);
                            /* resume search past 1-char delim*/
                            expptr = tokptr + 2 - lrlen;
                            break;
                        }          /* no need to check more lrsym's */
                    } /* --- end-of-for(isymbol) --- */
                } /* --- end-of-if/else(isthischar()) --- */
            } /* --- end-of-while(tokptr!=NULL) --- */
        } /* --- end-of-for(idelim) --- */
    }
    /* ------------------------------------------------------------
    run thru table, converting all {a+b\atop c+d} to \atop{a+b}{c+d}
    ------------------------------------------------------------ */
    for (isymbol = 0; (atopsym = atopcommands[isymbol]) != NULL; isymbol++) {
        /* #chars in \atop */
        int   atoplen = strlen(atopsym);
        /* re-start search at beginning */
        expptr = expression;
        while ((tokptr = strstr(expptr, atopsym)) != NULL) { /* found another atop */
            /*ptr to opening {, closing }*/
            char *leftbrace = NULL, *rightbrace = NULL;
            /* \atop followed by terminator */
            char termchar = *(tokptr + atoplen);
            if (mctx->msgfp != NULL && mctx->msglevel >= 999) {
                fprintf(mctx->msgfp, "mimeprep> offset=%d rhs=\"%s\"\n",
                        (int)(tokptr - expression), tokptr);
                fflush(mctx->msgfp);
            }
            if (isalpha((int)termchar)) {  /*we just have prefix of longer sym*/
                /* just resume search after prefix */
                expptr = tokptr + atoplen;
                continue;
            }           /* but don't process it */
            /* find left { */
            leftbrace  = findbraces(expression, tokptr);
            /* find right } */
            rightbrace = findbraces(NULL, tokptr + atoplen - 1);
            if (leftbrace == NULL || rightbrace == NULL) {
                /* skip command if didn't find */
                expptr += atoplen;
                continue;
            } else {           /* we have bracketed { \atop } */
                int  leftlen  = (int)(tokptr - leftbrace) - 1, /* #chars in left arg */
                     rightlen = (int)(rightbrace - tokptr) - atoplen, /* and in right*/
                     totlen   = (int)(rightbrace - leftbrace) + 1; /*tot in { \atop }*/
                char *open = atopdelims[2*isymbol], *close = atopdelims[2*isymbol+1];
                char arg[8192], command[8192];  /* left/right args, new \atop{}{} */
                /* start with null string */
                *command = '\000';
                /* add open delim if needed */
                if (open != NULL) strcat(command, open);
                /* add command with \atop */
                strcat(command, atopsym);
                /* arg starts with { */
                arg[0] = '{';
                /* extract left-hand arg */
                memcpy(arg + 1, leftbrace + 1, leftlen);
                /* and null terminate it */
                arg[leftlen+1] = '\000';
                /* concatanate {left-arg to \atop */
                strcat(command, arg);
                /* close left-arg, open right-arg */
                strcat(command, "}{");
                /* right-hand arg */
                memcpy(arg, tokptr + atoplen, rightlen);
                /* add closing } */
                arg[rightlen] = '}';
                /* and null terminate it */
                arg[rightlen+1] = '\000';
                if (isthischar(*arg, WHITEMATH))  /* 1st char was mandatory space */
                    /* so squeeze it out */
                    strcpy(arg, arg + 1);
                /* concatanate right-arg} */
                strcat(command, arg);
                /* add close delim if needed*/
                if (close != NULL) strcat(command, close);
                /* {\atop} --> {\atop{}{}} */
                strchange(totlen - 2, leftbrace + 1, command);
                /*resume search past \atop{}{}*/
                expptr = leftbrace + strlen(command);
            }
        } /* --- end-of-while(tokptr!=NULL) --- */
    } /* --- end-of-for(isymbol) --- */
    /* ------------------------------------------------------------
    back to caller with preprocessed expression
    ------------------------------------------------------------ */
    if (mctx->msgfp != NULL && mctx->msglevel >= 99) { /* display preprocessed expression */
        fprintf(mctx->msgfp, "mimeprep> expression=\"\"%s\"\"\n", expression);
        fflush(mctx->msgfp);
    }
    return (expression);
} /* --- end-of-function mimeprep() --- */



