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
#include <time.h>
#include "mimetex_priv.h"

/* ==========================================================================
 * Function:    dbltoa ( dblval, npts )
 * Purpose: Converts double to ascii, in financial format
 *      (e.g., comma-separated and negatives enclosed in ()'s).
 * -------------------------------------------------------------------------
 * Arguments:   dblval (I)  double containing value to be converted.
 *      npts (I)    int containing #places after decimal point
 *              to be displayed in returned string.
 * Returns: ( char * )  null-terminated string containing
 *              double converted to financial format.
 * -------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
char *dbltoa(double dblval, int npts)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    double  floor();

    /* buffer returned to caller */
    static  char finval[256];
    /* table of ascii decimal digits */
    static  char digittbl[32] = "0123456789*";
    /* ptr to next char being converted*/
    char    *finptr = finval;
    /* to shift out digits from dblval */
    double  dbldigit;
    /* one digit from dblval */
    int digit;
    /* reset true if dblval negative */
    int isneg = 0;
    /* npts fractional digits of dblval*/
    int ifrac = 0;
    char    digits[64];
    /* all the digits [0]=least signif */
    int ndigits = 0;
    /* ------------------------------------------------------------
    Check sign
    ------------------------------------------------------------ */
    if (dblval < 0.0) {          /* got a negative value to convert */
        /* set flag and make it positive */
        isneg = 1;
        dblval = (-dblval);
    }
    /* ------------------------------------------------------------
    Get fractional part of dblval if required
    ------------------------------------------------------------ */
    if (npts > 0) {
        /* loop index */
        int ipts = npts;
        /* fractional part as double */
        dbldigit = dblval - floor(dblval);
        /* check if rounded frac > 1 */
        digit = 1;
        while (--ipts >= 0) {      /* count down */
            /* shift left one digit at a time */
            dbldigit *= 10.0;
            digit *= 10;
        }        /* and keep max up-to-date */
        /* store fractional part as integer*/
        ifrac = (int)(dbldigit + 0.5);
        if (ifrac >= digit) {      /* round to next whole number */
            /* bump val, reset frac to zero */
            dblval++;
            ifrac = 0;
        }
    } /* --- end-of-if(npts>0) --- */
    /* no frac, round to nearest whole */
    else dblval += 0.5;
    /* ------------------------------------------------------------
    Get whole digits
    ------------------------------------------------------------ */
    /* get rid of fractional part */
    dblval = floor(dblval);
    while (dblval > 0.0) {           /* still have data digits remaining*/
        /* shift out next digit */
        dbldigit = floor(dblval / 10.0);
        /* least signif digit */
        digit = (int)(dblval - 10.0 * dbldigit + 0.01);
        /* index check */
        if (digit < 0 || digit > 9) digit = 10;
        /* store ascii digit */
        digits[ndigits++] = digittbl[digit];
        dblval = dbldigit;
    }      /* ready for next digit */
    /* store a single '0' for 0.0 */
    if (ndigits < 1) digits[ndigits++] = '0';
    /* ------------------------------------------------------------
    Format whole part from digits[] array
    ------------------------------------------------------------ */
    /* leading paren for negative value*/
    if (isneg) *finptr++ = '(';
    for (digit = ndigits - 1; digit >= 0; digit--) { /* start with most significant */
        /* store digit */
        *finptr++ = digits[digit];
        if (digit > 0 && digit % 3 == 0)   /* need a comma */
            *finptr++ = ',';
    }      /* put in separating comma */
    /* ------------------------------------------------------------
    Format fractional part using ifrac
    ------------------------------------------------------------ */
    if (npts > 0) {
        /* start with decimal point */
        *finptr++ = '.';
        /* convert to string */
        sprintf(finptr, "%0*d", npts, ifrac);
        finptr += npts;
    }         /* bump ptr past fractional digits */
    /* ------------------------------------------------------------
    End-of-Job
    ------------------------------------------------------------ */
    /*trailing paren for negative value*/
    if (isneg) *finptr++ = ')';
    /* null-terminate converted double */
    *finptr = '\000';
    /* converted double back to caller */
    return (finval);
} /* --- end-of-function dbltoa() --- */

/* ==========================================================================
 * Function:    emit_string ( fp, col1, string, comment )
 * Purpose: Emit string on fp, starting in col1,
 *      and followed by right-justified comment.
 * --------------------------------------------------------------------------
 * Arguments:   fp (I)      File ptr to output device (defaults to
 *              stdout if passed as NULL).
 *      col1 (I)    int containing 0 or #blanks preceding string
 *      string (I)  char *  containing string to be emitted.
 *              If last char of string is '\n',
 *              the emitted line ends with a newline,
 *              otherwise not.
 *      comment (I) NULL or char * containing right-justified
 *              comment (we enclose between /star and star/)
 * --------------------------------------------------------------------------
 * Returns: ( int )     1 if completed successfully,
 *              or 0 otherwise (for any error).
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int emit_string(FILE *fp, int col1, char *string, char *comment)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* construct line with caller's fields */
    char    line[256];
    /* #chars in one of caller's fields */
    int fieldlen;
    /*line length (for right-justified comment)*/
    int linelen = 72;
    /* true to emit \n at end of line */
    int isnewline = 0;
    /* ------------------------------------------------------------
    construct line containing prolog, string, epilog, and finally comment
    ------------------------------------------------------------ */
    /* --- init line --- */
    /* start line with blanks */
    memset(line, ' ', 255);
    /* --- embed string into line --- */
    if (string != NULL) {            /* if caller gave us a string... */
        /* #cols required for string */
        fieldlen = strlen(string);
        if (string[fieldlen-1] == '\n') {    /* check last char for newline */
            /* got it, so set flag */
            isnewline = 1;
            fieldlen--;
        }           /* but don't print it yet */
        /* embid string starting at col1 */
        memcpy(line + col1, string, fieldlen);
        col1 += fieldlen;
    }         /* bump col past epilog */
    /* --- embed comment into line --- */
    if (comment != NULL) {           /* if caller gave us a comment... */
        fieldlen = 6 + strlen(comment); /* plus  /star, star/, 2 spaces */
        if (linelen - fieldlen < col1)   /* comment won't fit */
            /* truncate comment to fit */
            fieldlen -= (col1 - (linelen - fieldlen));
        if (fieldlen > 6)            /* can fit all or part of comment */
            sprintf(line + linelen - fieldlen, "/%c %.*s %c/", /* so embed it in line */
                    '*', fieldlen - 6, comment, '*');
        col1 = linelen;
    }           /* indicate line filled */
    /* --- line completed --- */
    /* null-terminate completed line */
    line[col1] = '\000';
    /* ------------------------------------------------------------
    emit line, then back to caller with 1=okay, 0=failed.
    ------------------------------------------------------------ */
    /* --- first redirect null fp --- */
    /* default fp to stdout if null */
    if (fp == (FILE *)NULL) fp = stdout;
    /* --- emit line (and optional newline) --- */
    /* no more than linelen chars */
    fprintf(fp, "%.*s", linelen, line);
    /*caller wants terminating newline*/
    if (isnewline) fprintf(fp, "\n");
    return (1);
} /* --- end-of-function emit_string() --- */


/* ==========================================================================
 * Function:    calendar ( year, month, day )
 * Purpose: returns null-terminated character string containing
 *      \begin{array}...\end{array} for the one-month calendar
 *      specified by year=1973...2099 and month=1...12.
 *      If either arg out-of-range, today's value is used.
 * --------------------------------------------------------------------------
 * Arguments:   year (I)    int containing 1973...2099 or 0 for current
 *              year
 *      month (I)   int containing 1...12 or 0 for current month
 *      day (I)     int containing day to emphasize or 0
 * --------------------------------------------------------------------------
 * Returns: ( char * )  char ptr to null-terminated buffer
 *              containing \begin{array}...\end{array}
 *              string that will render calendar for
 *              requested month, or NULL for any error.
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
char    *calendar(int year, int month, int day)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* calendar returned to caller */
    static char calbuff[4096];
    /* binary value returned by time() */
    time_t  time_val = (time_t)(0);
    /* interpret time_val */
    struct tm *tmstruct = (struct tm *)NULL, *localtime();
    /* today (emphasize today's dd) */
    int yy = 0, mm = 0, dd = 0;
    /* day-of-week for idd=1...31 */
    int idd = 1, iday = 0, daynumber();
    /* ascii day or 4-digit year */
    char    aval[64];
    /* --- calendar data --- */
    static  char *monthnames[] = {
        "?", "January", "February", "March", "April",
        "May", "June", "July", "August", "September", "October",
        "November", "December", "?"
    };
    static  int modays[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 0 };
    /* ------------------------------------------------------------
    initialization
    ------------------------------------------------------------ */
    /* --- get current date/time --- */
    /* get date and time */
    time((time_t *)(&time_val));
    /* interpret time_val */
    tmstruct = localtime((time_t *)(&time_val));
    /* current four-digit year */
    yy  =  1900 + (int)(tmstruct->tm_year);
    /* current month, 1-12 */
    mm  =  1 + (int)(tmstruct->tm_mon);
    /* current day, 1-31 */
    dd  = (int)(tmstruct->tm_mday);
    /* --- check args --- */
    /* current year if out-of-bounds */
    if (year < 1973 || year > 2099) year  = yy;
    /* current month if out-of-bounds */
    if (month < 1 || month > 12) month = mm;
    if (month == mm && year == yy && day == 0)   /* current month and default day */
        /* emphasize current day */
        day = dd;
    /* Feb has 29 days in leap years */
    modays[2] = (year % 4 == 0 ? 29 : 28);
    /* --- initialize calendar string --- */
    /* center `month year` above cal */
    strcpy(calbuff, "{\\begin{gather}");
    /* month set in roman */
    strcat(calbuff, "\\small\\text{");
    /* insert month name */
    strcat(calbuff, monthnames[month]);
    /* add a space */
    strcat(calbuff, " }");
    /* convert year to ascii */
    sprintf(aval, "%d", year);
    /* add year */
    strcat(calbuff, aval);
    /* end top row */
    strcat(calbuff, "\\\\");
    strcat(calbuff,
    /* now begin calendar arrayr */
           "\\begin{array}{|c|c|c|c|c|c|c|CCCCCC} \\hline"
           "\\tiny\\text{Sun} & \\tiny\\text{Mon} & \\tiny\\text{Tue} &"
           "\\tiny\\text{Wed} & \\tiny\\text{Thu} & \\tiny\\text{Fri} &"
           "\\tiny\\text{Sat} \\\\ \\hline ");
    /* ------------------------------------------------------------
    generate calendar
    ------------------------------------------------------------ */
    for (idd = 1; idd <= modays[month]; idd++) { /* run through days of month */
        /* --- get day-of-week for this day --- */
        /* 1=Monday...7=Sunday */
        iday = 1 + (daynumber(year, month, idd) % 7);
        /* now 0=Sunday...6=Saturday */
        if (iday == 7) iday = 0;
        /* --- may need empty cells at beginning of month --- */
        if (idd == 1)              /* first day of month */
            if (iday > 0) {           /* need to skip cells */
                /*cells to skip*/
                strcpy(aval, "\\ &\\ &\\ &\\ &\\ &\\ &\\ &\\ &\\ &\\");
                /*skip cells preceding 1st of month*/
                aval[3*iday] = '\000';
                strcat(calbuff, aval);
            }       /* add skip string to buffer */
        /* --- add idd to current cell --- */
        /* convert idd to ascii */
        sprintf(aval, "%d", idd);
        if (idd == day             /* emphasize today's date */
                /*&&   month==mm && year==yy*/) { /* only if this month's calendar */
            /*emphasize, 1 size smaller*/
            strcat(calbuff, "{\\fs{-1}\\left\\langle ");
            /* put in idd */
            strcat(calbuff, aval);
            strcat(calbuff, "\\right\\rangle}");
        } /* finish emphasis */
        else
        /* not today's date */
            /* so just put in idd */
            strcat(calbuff, aval);
        /* --- terminate cell --- */
        if (idd < modays[month]) {         /* not yet end-of-month */
            if (iday < 6)             /* still have days left in week */
                /* new cell in same week */
                strcat(calbuff, "&");
            else
            /* reached end-of-week */
                strcat(calbuff, "\\\\ \\hline");
        }   /* so start new week */
    } /* --- end-of-for(idd) --- */
    /* final underline at end-of-month */
    strcat(calbuff, "\\\\ \\hline");
    /* --- return calendar to caller --- */
    /* terminate array */
    strcat(calbuff, "\\end{array}\\end{gather}}");
    /* back to caller with calendar */
    return (calbuff);
} /* --- end-of-function calendar() --- */


/* ==========================================================================
 * Function:    timestamp ( tzdelta, ifmt )
 * Purpose: returns null-terminated character string containing
 *      current date:time stamp as ccyy-mm-dd:hh:mm:ss{am,pm}
 * --------------------------------------------------------------------------
 * Arguments:   tzdelta (I) integer, positive or negative, containing
 *              containing number of hours to be added or
 *              subtracted from system time (to accommodate
 *              your desired time zone).
 *      ifmt (I)    integer containing 0 for default format
 * --------------------------------------------------------------------------
 * Returns: ( char * )  ptr to null-terminated buffer
 *              containing current date:time stamp
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
char *timestamp(int tzdelta, int ifmt)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* date:time buffer back to caller */
    static  char timebuff[256];
    /*long  time_val = 0L;*/        /* binary value returned by time() */
    /* binary value returned by time() */
    time_t  time_val = (time_t)(0);
    /* interpret time_val */
    struct tm *tmstruct = (struct tm *)NULL, *localtime();
    int year = 0, hour = 0, ispm = 1,      /* adjust year, and set am/pm hour */
        month = 0, day = 0; /* adjust day and month for delta  */
    static  char *daynames[] = {
        "Monday", "Tuesday", "Wednesday",
        "Thursday", "Friday", "Saturday", "Sunday"
    };
    static  char *monthnames[] = {
        "?", "January", "February", "March", "April",
        "May", "June", "July", "August", "September", "October",
        "November", "December", "?"
    };
    /* ------------------------------------------------------------
    get current date:time, adjust values, and and format stamp
    ------------------------------------------------------------ */
    /* --- first init returned timebuff in case of any error --- */
    *timebuff = '\000';
    /* --- get current date:time --- */
    /* get date and time */
    time((time_t *)(&time_val));
    /* interpret time_val */
    tmstruct = localtime((time_t *)(&time_val));
    /* --- extract fields --- */
    /* local copy of year,  0=1900 */
    year  = (int)(tmstruct->tm_year);
    /* local copy of month, 1-12 */
    month = (int)(tmstruct->tm_mon) + 1;
    /* local copy of day,   1-31 */
    day   = (int)(tmstruct->tm_mday);
    /* local copy of hour,  0-23 */
    hour  = (int)(tmstruct->tm_hour);
    /* --- adjust year --- */
    /* set century in year */
    year += 1900;
    /* --- adjust for timezone --- */
    tzadjust(tzdelta, &year, &month, &day, &hour);
    /* --- check params --- */
    if (hour < 0  || hour > 23
            ||   day < 1   || day > 31
            ||   month < 1 || month > 12
            ||   year < 1973) goto end_of_job;
    /* --- adjust hour for am/pm --- */
    switch (ifmt) {
    default:
    case 0:
        if (hour < 12) {         /* am check */
            /* reset pm flag */
            ispm = 0;
            if (hour == 0) hour = 12;
        } /* set 00hrs = 12am */
        /* pm check sets 13hrs to 1pm, etc */
        if (hour > 12) hour -= 12;
        break;
    } /* --- end-of-switch(ifmt) --- */
    /* --- format date:time stamp --- */
    switch (ifmt) {
    default:
    case 0:  /* --- 2005-03-05:11:49:59am --- */
        sprintf(timebuff, "%04d-%02d-%02d:%02d:%02d:%02d%s", year, month, day,
                hour, (int)(tmstruct->tm_min), (int)(tmstruct->tm_sec), ((ispm) ? "pm" : "am"));
        break;
    case 1:  /* --- Saturday, March 5, 2005 --- */
        sprintf(timebuff, "%s, %s %d, %d",
                daynames[daynumber(year, month, day)%7], monthnames[month], day, year);
        break;
    case 2: /* --- Saturday, March 5, 2005, 11:49:59am --- */
        sprintf(timebuff, "%s, %s %d, %d, %d:%02d:%02d%s",
                daynames[daynumber(year, month, day)%7], monthnames[month], day, year,
                hour, (int)(tmstruct->tm_min), (int)(tmstruct->tm_sec), ((ispm) ? "pm" : "am"));
        break;
    case 3: /* --- 11:49:59am --- */
        sprintf(timebuff, "%d:%02d:%02d%s",
                hour, (int)(tmstruct->tm_min), (int)(tmstruct->tm_sec), ((ispm) ? "pm" : "am"));
        break;
    } /* --- end-of-switch(ifmt) --- */
end_of_job:
    /* return stamp to caller */
    return (timebuff);
} /* --- end-of-function timestamp() --- */


/* ==========================================================================
 * Function:    tzadjust ( tzdelta, year, month, day, hour )
 * Purpose: Adjusts hour, and day,month,year if necessary,
 *      by delta increment to accommodate your time zone.
 * --------------------------------------------------------------------------
 * Arguments:   tzdelta (I) integer, positive or negative, containing
 *              containing number of hours to be added or
 *              subtracted from given time (to accommodate
 *              your desired time zone).
 *      year (I)    addr of int containing        4-digit year
 *      month (I)   addr of int containing month  1=Jan - 12=Dec.
 *      day (I)     addr of int containing day    1-31 for Jan.
 *      hour (I)    addr of int containing hour   0-23
 * Returns: ( int )     1 for success, or 0 for error
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int tzadjust(int tzdelta, int *year, int *month, int *day, int *hour)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /*dereference args*/
    int yy = *year, mm = *month, dd = *day, hh = *hour;
    /* --- calendar data --- */
    static  int modays[] = {
        0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 0
    };
    /* ------------------------------------------------------------
    check args
    ------------------------------------------------------------ */
    /* bad month */
    if (mm < 1 || mm > 12) return(-1);
    /* bad day */
    if (dd < 1 || dd > modays[mm]) return(-1);
    /* bad hour */
    if (hh < 0 || hh > 23) return(-1);
    /* bad tzdelta */
    if (tzdelta > 23 || tzdelta < (-23)) return(-1);
    /* ------------------------------------------------------------
    make adjustments
    ------------------------------------------------------------ */
    /* --- adjust hour --- */
    /* apply caller's delta */
    hh += tzdelta;
    /* --- adjust for feb 29 --- */
    /* Feb has 29 days in leap years */
    modays[2] = (yy % 4 == 0 ? 29 : 28);
    /* --- adjust day --- */
    if (hh < 0) {                /* went to preceding day */
        dd--;
        hh += 24;
    }
    if (hh > 23) {               /* went to next day */
        dd++;
        hh -= 24;
    }
    /* --- adjust month --- */
    if (dd < 1) {                /* went to preceding month */
        mm--;
        dd = modays[mm];
    }
    if (dd > modays[mm]) {           /* went to next month */
        mm++;
        dd = 1;
    }
    /* --- adjust year --- */
    if (mm < 1) {                /* went to preceding year */
        yy--;
        mm = 12;
        dd = modays[mm];
    }
    if (mm > 12) {               /* went to next year */
        yy++;
        mm = 1;
        dd = 1;
    }
    /* --- back to caller --- */
    *year = yy;
    *month = mm;
    *day = dd;
    /* reset adjusted args */
    *hour = hh;
    return (1);
} /* --- end-of-function tzadjust() --- */


/* ==========================================================================
 * Function:    daynumber ( year, month, day )
 * Purpose: Returns number of actual calendar days from Jan 1, 1973
 *      to the given date (e.g., bvdaynumber(1974,1,1)=365).
 * --------------------------------------------------------------------------
 * Arguments:   year (I)    int containing year -- may be either 1995 or
 *              95, or may be either 2010 or 110 for those
 *              years.
 *      month (I)   int containing month, 1=Jan thru 12=Dec.
 *      day (I)     int containing day of month, 1-31 for Jan, etc.
 * Returns: ( int )     Number of days from Jan 1, 1973 to given date,
 *              or -1 for error (e.g., year<1973).
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int daynumber(int year, int month, int day)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* --- returned value (note: returned as a default "int") --- */
    /* #days since jan 1, year0 */
    int ndays;
    /* --- initial conditions --- */
    static  int year0 = 73,         /* jan 1 was a monday, 72 was a leap */
                days4yrs = 1461,        /* #days in 4 yrs = 365*4 + 1 */
                days1yr  = 365;
    /* --- table of accumulated days per month (last index not used) --- */
    static  int modays[] = {
        0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
    };
    /* --- variables for #days since day0 --- */
    /*#years, #4-yr periods since year0*/
    int nyears, nfouryrs;
    /* ------------------------------------------------------------
    Check input
    ------------------------------------------------------------ */
    if (month < 1 || month > 12)         /*month used as index, so must be ok*/
        /* otherwise, forget it */
        return (-1);
    /*use two-digit years (3 after 2000)*/
    if (year >= 1900) year -= 1900;
    /* ------------------------------------------------------------
    Find #days since jan 1, 1973
    ------------------------------------------------------------ */
    /* --- figure #complete 4-year periods and #remaining yrs till current --- */
    /* #years since year0 */
    nyears = year - year0;
    /* we're not working backwards */
    if (nyears < 0) return (-1);
    /* #complete four-year periods */
    nfouryrs = nyears / 4;
    /* remainder excluding current year*/
    nyears -= (4 * nfouryrs);
    /* --- #days from jan 1, year0 till jan 1, this year --- */
    ndays = (days4yrs * nfouryrs)   /* #days in 4-yr periods */
            /* +remaining days */
            + (days1yr * nyears);
    /*if ( year > 100 ) ndays--;*/      /* subtract leap year for 2000AD */
    /* --- add #days within current year --- */
    ndays += (modays[month-1] + (day - 1));
    /* --- may need an extra day if current year is a leap year --- */
    if (nyears == 3) {           /*three preceding yrs so this is 4th*/
        if (month > 2)             /* past feb so need an extra day */
            /*if ( year != 100 )*/      /* unless it's 2000AD */
            ndays++;
    }            /* so add it in */
    /* #days back to caller */
    return ((int)(ndays));
} /* --- end-of-function daynumber() --- */


/* ==========================================================================
 * Function:    strwrap ( s, linelen, tablen )
 * Purpose: Inserts \n's and spaces in (a copy of) s to wrap lines
 *      at linelen and indent them by tablen.
 * --------------------------------------------------------------------------
 * Arguments:   s (I)       char * to null-terminated string
 *              to be wrapped.
 *      linelen (I) int containing maximum linelen
 *              between \\'s.
 *      tablen (I)  int containing number of spaces to indent
 *              lines.  0=no indent.  Positive means
 *              only indent first line and not others.
 *              Negative means indent all lines except first.
 * --------------------------------------------------------------------------
 * Returns: ( char * )  ptr to "line-wrapped" copy of s
 *              or "" (empty string) for any error.
 * --------------------------------------------------------------------------
 * Notes:     o The returned copy of s has embedded \\'s as necessary
 *      to wrap lines at linelen.  Any \\'s in the input copy
 *      are removed first.  If (and only if) the input s contains
 *      a terminating \\ then so does the returned copy.
 *        o The returned pointer addresses a static buffer,
 *      so don't call strwrap() again until you're finished
 *      with output from the preceding call.
 *        o Modified for mimetex from original version written
 *      for mathtex (where \n in verbatim mode instead of \\
 *      produced linebreaks).
 * ======================================================================= */
/* --- entry point --- */
char    *strwrap(mimetex_ctx *mctx, char *s, int linelen, int tablen)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* line-wrapped copy of s */
    static  char sbuff[4096];
    /* ptr to start of current line*/
    char    *sol = sbuff;
    /* tab string */
    char    tab[32] = "                 ";
    /*newline at end of string?*/
    int finalnewline = (lastchar(s) == '\n' ? 1 : 0);
    int istab = (tablen > 0 ? 1 : 0),     /* init true to indent first line */
        iswhite = 0; /* true if line break on whitespace*/
    int rhslen  = 0,            /* remaining right hand side length*/
        thislen = 0,            /* length of current line segment */
        thistab = 0,            /* length of tab on current line */
        wordlen = 0;            /* length to next whitespace char */
    /* ------------------------------------------------------------
    Make a clean copy of s
    ------------------------------------------------------------ */
    /* --- check input --- */
    /* initialize in case of error */
    *sbuff = '\000';
    /* no input */
    if (isempty(s)) goto end_of_job;
    /* set positive tablen */
    if (tablen < 0) tablen = (-tablen);
    /* tab was longer than line */
    if (tablen >= linelen) tablen = linelen - 1;
    /* null-terminate tab string */
    tab[min2(tablen, 16)] = '\000';
    /* reset to actual tab length */
    tablen = strlen(tab);
    /* turned off for mimetex version */
    finalnewline = 0;
    /* --- start with copy of s --- */
    /* leave room for \n's and tabs */
    strninit(sbuff, s, 3000);
    /* can't do anything */
    if (linelen < 1) goto end_of_job;
    trimwhite(sbuff);
    /*remove leading/trailing whitespace*/
    /* remove any original \n's */
    strreplace(sbuff, "\n", " ", 0);
    /* remove any original \r's */
    strreplace(sbuff, "\r", " ", 0);
    /* remove any original \t's */
    strreplace(sbuff, "\t", " ", 0);
    /* remove any original \f's */
    strreplace(sbuff, "\f", " ", 0);
    /* remove any original \v's */
    strreplace(sbuff, "\v", " ", 0);
    /* remove any original \\'s */
    strreplace(sbuff, "\\\\", " ", 0);
    /* ------------------------------------------------------------
    Insert \\'s and spaces as needed
    ------------------------------------------------------------ */
    while (1) {                  /* till end-of-line */
        /* --- init --- */
        trimwhite(sol);
        /*remove leading/trailing whitespace*/
        /* no chars in current line yet */
        thislen = thistab = 0;
        if (istab && tablen > 0) {     /* need to indent this line */
            /* insert indent at start of line */
            strchange(0, sol, tab);
            thistab = tablen;
        }         /* line starts with whitespace tab */
        /* flip tab flag after first line */
        if (sol == sbuff) istab = 1 - istab;
        /* skip tab */
        sol += thistab;
        /* remaining right hand side chars */
        rhslen = strlen(sol);
        /* no more \\'s needed */
        if (rhslen + thistab <= linelen) break;
        if (0 && mctx->msgfp != NULL && mctx->msglevel >= 99) {
            fprintf(mctx->msgfp, "strwrap> rhslen=%d, sol=\"\"%s\"\"\n", rhslen, sol);
            fflush(mctx->msgfp);
        }
        /* --- look for last whitespace preceding linelen --- */
        while (1) {                /* till we exceed linelen */
            wordlen = strcspn(sol + thislen, " \t\n\r\f\v :;.,"); /*ptr to next white/break*/
            if (sol[thislen+wordlen] == '\000')   /* no more whitespace in string */
                /* so nothing more we can do */
                goto end_of_job;
            if (thislen + thistab + wordlen >= linelen) /* next word won't fit */
                /* but make sure line has one word */
                if (thislen > 0) break;
            thislen += (wordlen + 1);
        }       /* ptr past next whitespace char */
        /* line will have one too-long word*/
        if (thislen < 1) break;
        /*sol[thislen-1] = '\n';*/        /* replace last space with newline */
        /*sol += thislen;*/           /* next line starts after newline */
        /*linebreak on space?*/
        iswhite = (isthischar(sol[thislen-1], ":;.,") ? 0 : 1);
        /* put \\ at end of line */
        strchange(iswhite, sol + thislen - iswhite, "\\\\");
        /* next line starts after \\ */
        sol += (thislen + 2 - iswhite);
    } /* --- end-of-while(1) --- */
end_of_job:
    /* replace final newline */
    if (finalnewline) strcat(sbuff, "\\\\");
    /* back with clean copy of s */
    return (sbuff);
} /* --- end-of-function strwrap() --- */


/* ==========================================================================
 * Function:    strnlower ( s, n )
 * Purpose: lowercase the first n chars of string s
 * --------------------------------------------------------------------------
 * Arguments:   s (I/O)     (char *)pointer to null-terminated string
 *              whose chars are to be lowercased
 *      n (I)       int containing max number of chars to be
 *              lowercased (less than n will be lowercased
 *              if terminating '\000' found first)
 *              If n<=0 (or n>=strlen(s)) then the entire
 *              string s will be lowercased
 * --------------------------------------------------------------------------
 * Returns: ( char * )  s (always same as input)
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
char    *strnlower(char *s, int n)
{
    /* ------------------------------------------------------------
    lowercase s
    ------------------------------------------------------------ */
    /* save s for return to caller */
    char    *p = s;
    if (!isempty(s))             /* check for valid input */
        while (*p != '\000') {         /* lowercase each char till end */
            /* lowercase this char */
            *p = tolower(*p);
            if (n > 0)           /* only lowercase first n chars */
                /* quit when we're done */
                if (--n < 1) break;
            p++;
        }              /* proceed to next char */
    /* back to caller with s */
    return (s);
} /* --- end-of-function strnlower() --- */

/* ==========================================================================
 * Function:    strchange ( int nfirst, char *from, char *to )
 * Purpose: Changes the nfirst leading chars of `from` to `to`.
 *      For example, to change char x[99]="12345678" to "123ABC5678"
 *      call strchange(1,x+3,"ABC")
 * --------------------------------------------------------------------------
 * Arguments:   nfirst (I)  int containing #leading chars of `from`
 *              that will be replace by `to`
 *      from (I/O)  char * to null-terminated string whose nfirst
 *              leading chars will be replaced by `to`
 *      to (I)      char * to null-terminated string that will
 *              replace the nfirst leading chars of `from`
 * --------------------------------------------------------------------------
 * Returns: ( char * )  ptr to first char of input `from`
 *              or NULL for any error.
 * --------------------------------------------------------------------------
 * Notes:     o If strlen(to)>nfirst, from must have memory past its null
 *      (i.e., we don't do a realloc)
 * ======================================================================= */
/* --- entry point --- */
char    *strchange(int nfirst, char *from, char *to)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    int tolen = (to == NULL ? 0 : strlen(to)), /* #chars in replacement string */
                /*need to shift from left or right*/
                nshift = abs(tolen - nfirst);
    /* ------------------------------------------------------------
    shift from left or right to accommodate replacement of its nfirst chars by to
    ------------------------------------------------------------ */
    if (tolen < nfirst)              /* shift left is easy */
        /* because memory doesn't overlap */
        strcpy(from, from + nshift);
    if (tolen > nfirst) {            /* need more room at start of from */
        /* ptr to null terminating from */
        char *pfrom = from + strlen(from);
        for (; pfrom >= from; pfrom--)   /* shift all chars including null */
            *(pfrom + nshift) = *pfrom;
    }   /* shift chars nshift places right */
    /* ------------------------------------------------------------
    from has exactly the right number of free leading chars, so just put to there
    ------------------------------------------------------------ */
    if (tolen != 0)              /* make sure to not empty or null */
        /* chars moved into place */
        memcpy(from, to, tolen);
    /* changed string back to caller */
    return (from);
} /* --- end-of-function strchange() --- */


/* ==========================================================================
 * Function:    strreplace (char *string, char *from, char *to, int nreplace)
 * Purpose: Changes the first nreplace occurrences of 'from' to 'to'
 *      in string, or all occurrences if nreplace=0.
 * --------------------------------------------------------------------------
 * Arguments:   string (I/0)    char * to null-terminated string in which
 *              occurrence of 'from' will be replaced by 'to'
 *      from (I)    char * to null-terminated string
 *              to be replaced by 'to'
 *      to (I)      char * to null-terminated string that will
 *              replace 'from'
 *      nreplace (I)    int containing (maximum) number of
 *              replacements, or 0 to replace all.
 * --------------------------------------------------------------------------
 * Returns: ( int )     number of replacements performed,
 *              or 0 for no replacements or -1 for any error.
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int strreplace(char *string, char *from, char *to, int nreplace)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    int fromlen = (from == NULL ? 0 : strlen(from)), /* #chars to be replaced */
        tolen = (to == NULL ? 0 : strlen(to)); /* #chars in replacement string */
    char *pfrom = (char *)NULL,      /*ptr to 1st char of from in string*/
         *pstring = string,      /*ptr past previously replaced from*/
         *strchange(); /* change 'from' to 'to' */
    /* #replacements returned to caller*/
    int nreps = 0;
    /* ------------------------------------------------------------
    repace occurrences of 'from' in string to 'to'
    ------------------------------------------------------------ */
    if (string == (char *)NULL       /* no input string */
            || (fromlen < 1 && nreplace <= 0))  /* replacing empty string forever */ {
        /* so signal error */
        nreps = (-1);
    } else {
        /* args okay */
        while (nreplace < 1 || nreps < nreplace) { /* up to #replacements requested */
            if (fromlen > 0)             /* have 'from' string */
                /*ptr to 1st char of from in string*/
                pfrom = strstr(pstring, from);
            /*or empty from at start of string*/
            else  pfrom = pstring;
            /*no more from's, so back to caller*/
            if (pfrom == (char *)NULL) break;
            if (strchange(fromlen, pfrom, to) /* leading 'from' changed to 'to' */
                    == (char *)NULL) {
                /* signal error to caller */
                nreps = (-1);
                break;
            }
            /* count another replacement */
            nreps++;
            /* pick up search after 'to' */
            pstring = pfrom + tolen;
            /* but quit at end of string */
            if (*pstring == '\000') break;
        } /* --- end-of-while() --- */
    }
    /* #replacements back to caller */
    return (nreps);
} /* --- end-of-function strreplace() --- */


/* ==========================================================================
 * Function:    strwstr (char *string, char *substr, char *white, int *sublen)
 * Purpose: Find first substr in string, but wherever substr contains
 *      a whitespace char (in white), string may contain any number
 *      (including 0) of whitespace chars. If white contains I or i,
 *      then match is case-insensitive (and I,i _not_ whitespace).
 * --------------------------------------------------------------------------
 * Arguments:   string (I)  char * to null-terminated string in which
 *              first occurrence of substr will be found
 *      substr (I)  char * to null-terminated string containing
 *              "template" that will be searched for
 *      white (I)   char * to null-terminated string containing
 *              whitespace chars.  If NULL or empty, then
 *              "~ \t\n\r\f\v" (WHITEMATH in mimetex.h) used.
 *              If white contains I or i, then match is
 *              case-insensitive (and I,i _not_ considered
 *              whitespace).
 *      sublen (O)  address of int returning "length" of substr
 *              found in string (which may be longer or
 *              shorter than substr itself).
 * --------------------------------------------------------------------------
 * Returns: ( char * )  ptr to first char of substr in string
 *              or NULL if not found or for any error.
 * --------------------------------------------------------------------------
 * Notes:     o Wherever a single whitespace char appears in substr,
 *      the corresponding position in string may contain any
 *      number (including 0) of whitespace chars, e.g.,
 *      string="abc   def" and string="abcdef" both match
 *      substr="c d" at offset 2 of string.
 *        o If substr="c  d" (two spaces between c and d),
 *      then string must have at least one space, so now "abcdef"
 *      doesn't match.  In general, the minimum number of spaces
 *      in string is the number of spaces in substr minus 1
 *      (so 1 space in substr permits 0 spaces in string).
 *        o Embedded spaces are counted in sublen, e.g.,
 *      string="c   d" (three spaces) matches substr="c d"
 *      with sublen=5 returned.  But string="ab   c   d" will
 *      also match substr="  c d" returning sublen=5 and
 *      a ptr to the "c".  That is, the mandatory preceding
 *      space is _not_ counted as part of the match.
 *      But all the embedded space is counted.
 *      (An inconsistent bug/feature is that mandatory
 *      terminating space is counted.)
 *        o Moreover, string="c   d" matches substr="  c d", i.e.,
 *      the very beginning of a string is assumed to be preceded
 *      by "virtual blanks".
 * ======================================================================= */
/* --- entry point --- */
char    *strwstr(mimetex_ctx *mctx, char *string, char *substr, char *white, int *sublen)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    char    *psubstr = substr, *pstring = string,/*ptr to current char in substr,str*/
            *pfound = (char *)NULL; /*ptr to found substr back to caller*/
    /* callers white whithout i,I */
    char    *pwhite = NULL, whitespace[256];
    int iscase = (white == NULL ? 1 :    /* case-sensitive if i,I in white */
                  strchr(white, 'i') == NULL && strchr(white, 'I') == NULL);
    /* length of substr found in string*/
    int foundlen = 0;
    int nstrwhite = 0, nsubwhite = 0,   /* #leading white chars in str,sub */
        nminwhite = 0; /* #mandatory leading white in str */
    int nstrchars = 0, nsubchars = 0,   /* #non-white chars to be matched */
        isncmp = 0; /*strncmp() or strncasecmp() result*/
    /* ------------------------------------------------------------
    Initialization
    ------------------------------------------------------------ */
    /* --- set up whitespace --- */
    /*default if no user input for white*/
    strcpy(whitespace, WHITEMATH);
    if (white != NULL) {
        /*user provided ptr to white string*/
        if (*white != '\000') {         /*and it's not just an empty string*/
            /* so use caller's white spaces */
            strcpy(whitespace, white);
            while ((pwhite = strchr(whitespace, 'i')) != NULL) /* have an embedded i */
                /* so squeeze it out */
                strcpy(pwhite, pwhite + 1);
            while ((pwhite = strchr(whitespace, 'I')) != NULL) /* have an embedded I */
                /* so squeeze it out */
                strcpy(pwhite, pwhite + 1);
            if (*whitespace == '\000')        /* caller's white just had i,I */
                strcpy(whitespace, WHITEMATH);
        }    /* so revert back to default */
    }
    /* ------------------------------------------------------------
    Find first occurrence of substr in string
    ------------------------------------------------------------ */
    if (string != NULL) {
        /* caller passed us a string ptr */
        while (*pstring != '\000') {        /* break when string exhausted */
            /* (re)start at next char in string*/
            char  *pstrptr = pstring;
            /* leading whitespace */
            int   leadingwhite = 0;
            /* start at beginning of substr */
            psubstr = substr;
            /* reset length of found substr */
            foundlen = 0;
            if (substr != NULL) {
                /* caller passed us a substr ptr */
                while (*psubstr != '\000') {      /*see if pstring begins with substr*/
                    /* --- check for end-of-string before finding match --- */
                    if (*pstrptr == '\000')          /* end-of-string without a match */
                        /* keep trying with next char */
                        goto nextstrchar;
                    /* --- actual amount of whitespace in string and substr --- */
                    /* #leading white chars in sub */
                    nsubwhite = strspn(psubstr, whitespace);
                    /* #leading white chars in str */
                    nstrwhite = strspn(pstrptr, whitespace);
                    /* #mandatory leading white in str */
                    nminwhite = max2(0, nsubwhite - 1);
                    /* --- check for mandatory leading whitespace in string --- */
                    if (pstrptr != string)       /*not mandatory at start of string*/
                        if (nstrwhite < nminwhite)     /* too little leading white space */
                            /* keep trying with next char */
                            goto nextstrchar;
                    /* ---hold on to #whitespace chars in string preceding substr match--- */
                    if (pstrptr == pstring)          /* whitespace at start of substr */
                        /* save it as leadingwhite */
                        leadingwhite = nstrwhite;
                    /* --- check for optional whitespace --- */
                    if (psubstr != substr)       /* always okay at start of substr */
                        if (nstrwhite > 0 && nsubwhite < 1)  /* too much leading white space */
                            /* keep trying with next char */
                            goto nextstrchar;
                    /* --- skip any leading whitespace in substr and string --- */
                    /* push past leading sub whitespace*/
                    psubstr += nsubwhite;
                    /* push past leading str whitespace*/
                    pstrptr += nstrwhite;
                    /* --- now get non-whitespace chars that we have to match --- */
                    /* #non-white chars in sub */
                    nsubchars = strcspn(psubstr, whitespace);
                    /* #non-white chars in str */
                    nstrchars = strcspn(pstrptr, whitespace);
                    if (nstrchars < nsubchars)   /* too few chars for match */
                        /* keep trying with next char */
                        goto nextstrchar;
                    /* --- see if next nsubchars are a match --- */
                    isncmp = (iscase ? strncmp(pstrptr, psubstr, nsubchars) : /*case sensitive*/
                              /*case insensitive*/
                              strncasecmp(pstrptr, psubstr, nsubchars));
                    if (isncmp != 0)             /* no match */
                        /* keep trying with next char */
                        goto nextstrchar;
                    /* --- push past matched chars --- */
                    psubstr += nsubchars;
                    /*nsubchars were matched*/
                    pstrptr += nsubchars;
                } /* --- end-of-while(*psubstr!='\000') --- */
            }
            /* found match starting at pstring */
            pfound = pstring + leadingwhite;
            /* consisting of this many chars */
            foundlen = (int)(pstrptr - pfound);
            /* back to caller */
            goto end_of_job;
            /* ---failed to find substr, continue trying with next char in string--- */
        nextstrchar:
            /* continue outer loop */
            /* bump to next char in string */
            pstring++;
        } /* --- end-of-while(*pstring!='\000') --- */
    }
    /* ------------------------------------------------------------
    Back to caller with ptr to first occurrence of substr in string
    ------------------------------------------------------------ */
end_of_job:
    if (mctx->msglevel >= 999 && mctx->msgfp != NULL) {   /* debugging/diagnostic output */
        fprintf(mctx->msgfp, "strwstr> str=\"%.72s\" sub=\"%s\" found at offset %d\n",
                string, substr, (pfound == NULL ? (-1) : (int)(pfound - string)));
        fflush(mctx->msgfp);
    }
    if (sublen != NULL)            /*caller wants length of found substr*/
        /* give it to him along with ptr */
        *sublen = foundlen;
    /*ptr to first found substr, or NULL*/
    return (pfound);
} /* --- end-of-function strwstr() --- */

/* ==========================================================================
 * Function:    isstrstr ( char *string, char *snippets, int iscase )
 * Purpose: determine whether any substring of 'string'
 *      matches any of the comma-separated list of 'snippets',
 *      ignoring case if iscase=0.
 * --------------------------------------------------------------------------
 * Arguments:   string (I)  char * containing null-terminated
 *              string that will be searched for
 *              any one of the specified snippets
 *      snippets (I)    char * containing null-terminated,
 *              comma-separated list of snippets
 *              to be searched for in string
 *      iscase (I)  int containing 0 for case-insensitive
 *              comparisons, or 1 for case-sensitive
 * --------------------------------------------------------------------------
 * Returns: ( int )     1 if any snippet is a substring of
 *              string, 0 if not
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int isstrstr(char *string, char *snippets, int iscase)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /*1 if any snippet found in string*/
    int status = 0;
    char snip[99], *snipptr = snippets,  /* munge through each snippet */
         delim = ',', *delimptr = NULL; /* separated by delim's */
    char stringcp[999], *cp = stringcp; /*maybe lowercased copy of string*/
    /* ------------------------------------------------------------
    initialization
    ------------------------------------------------------------ */
    /* --- arg check --- */
    /* missing arg */
    if (string == NULL || snippets == NULL) goto end_of_job;
    /* empty arg */
    if (*string == '\000' || *snippets == '\000') goto end_of_job;
    /* --- copy string and lowercase it if case-insensitive --- */
    /* local copy of string */
    strcpy(stringcp, string);
    if (!iscase)                 /* want case-insensitive compares */
        for (cp = stringcp; *cp != '\000'; cp++) /* so for each string char */
            /*lowercase any uppercase chars*/
            if (isupper(*cp)) *cp = tolower(*cp);
    /* ------------------------------------------------------------
    extract each snippet and see if it's a substring of string
    ------------------------------------------------------------ */
    while (snipptr != NULL) {        /* while we still have snippets */
        /* --- extract next snippet --- */
        if ((delimptr = strchr(snipptr, delim)) /* locate next comma delim */
                ==   NULL) {              /*not found following last snippet*/
            /* local copy of last snippet */
            strcpy(snip, snipptr);
            snipptr = NULL;
        }         /* signal end-of-string */
        else {                /* snippet ends just before delim */
            /* #chars in snippet */
            int sniplen = (int)(delimptr - snipptr) - 1;
            /* local copy of snippet chars */
            memcpy(snip, snipptr, sniplen);
            /* null-terminated snippet */
            snip[sniplen] = '\000';
            snipptr = delimptr + 1;
        }     /* next snippet starts after delim */
        /* --- lowercase snippet if case-insensitive --- */
        if (!iscase)           /* want case-insensitive compares */
            for (cp = snip; *cp != '\000'; cp++) /* so for each snippet char */
                /*lowercase any uppercase chars*/
                if (isupper(*cp)) *cp = tolower(*cp);
        /* --- check if snippet in string --- */
        if (strstr(stringcp, snip) != NULL) {  /* found snippet in string */
            /* so reset return status */
            status = 1;
            break;
        }              /* no need to check any further */
    } /* --- end-of-while(*snipptr!=0) --- */
end_of_job:
    /*1 if snippet found in list, else 0*/
    return (status);
} /* --- end-of-function isstrstr() --- */

/* --- entry point --- */
char x2c(char *what)
{
    char digit;
    digit = (what[0] >= 'A' ? ((what[0] & 0xdf) - 'A') + 10 : (what[0] - '0'));
    digit *= 16;
    digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A') + 10 : (what[1] - '0'));
    return(digit);
} /* --- end-of-function x2c() --- */

/* ==========================================================================
 * Function:    hex_bitmap ( rp, fp, col1, isstr )
 * Purpose: Emit a hex dump of the bitmap of rp on fp, starting in col1.
 *      If isstr (is string) is true, the dump is of the form
 *          "\x01\x02\x03\x04\x05..."
 *      Otherwise, if isstr is false, the dump is of the form
 *          0x01,0x02,0x03,0x04,0x05...
 * --------------------------------------------------------------------------
 * Arguments:   rp (I)      ptr to raster struct for which
 *              a hex dump is to be constructed.
 *      fp (I)      File ptr to output device (defaults to
 *              stdout if passed as NULL).
 *      col1 (I)    int containing 0...65; output lines
 *              are preceded by col1 blanks.
 *      isstr (I)   int specifying dump format as described above
 * --------------------------------------------------------------------------
 * Returns: ( int )     1 if completed successfully,
 *              or 0 otherwise (for any error).
 * --------------------------------------------------------------------------
 * Notes:
 * ======================================================================= */
/* --- entry point --- */
int hex_bitmap(raster *rp, FILE *fp, int col1, int isstr)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    int ibyte,              /* pixmap[ibyte] index */
    /*#bytes in bitmap or .gf-formatted*/
    nbytes = pixbytes(rp);
    /* col1 leading blanks */
    char    stub[64] = "                                ";
    int linewidth = 64,         /* (roughly) rightmost column */
                    /* #cols required for each byte */
                    colwidth = (isstr ? 4 : 5);
    /* new line after ncols bytes */
    int ncols = (linewidth - col1) / colwidth;
    /* ------------------------------------------------------------
    initialization
    ------------------------------------------------------------ */
    /* --- redirect null fp --- */
    /* default fp to stdout if null */
    if (fp == (FILE *)NULL) fp = stdout;
    /* --- emit initial stub if wanted --- */
    /* stub preceding 1st line */
    if (col1 > 0) fprintf(fp, "%.*s", col1, stub);
    /* ------------------------------------------------------------
    emit hex dump of rp->bitmap image
    ------------------------------------------------------------ */
    /* opening " before first line */
    if (isstr) fprintf(fp, "\"");
    for (ibyte = 0; ibyte < nbytes; ibyte++) { /* one byte at a time */
        /* --- display a byte as hex char or number, depending on isstr --- */
        if (isstr)                 /* string format wanted */
            /*print byte as hex char*/
            fprintf(fp, "\\x%02x", (rp->pixmap)[ibyte]);
        else
        /* comma-separated format wanted */
            /*print byte as hex number*/
            fprintf(fp, "0x%02x", (rp->pixmap)[ibyte]);
        /* --- add a separator and newline, etc, as necessary --- */
        if (ibyte < nbytes - 1) {  /* not the last byte yet */
            /* follow hex number with comma */
            if (!isstr) fprintf(fp, ",");
            if ((ibyte + 1) % ncols == 0) {  /* need new line after every ncols */
                if (!isstr)            /* for hex numbers format ... */
                    /* ...just need newline and stub */
                    fprintf(fp, "\n%.*s", col1, stub);
                else
                /* for string format... */
                    fprintf(fp, "\"\n%.*s\"", col1, stub);
            } /*...need closing, opening "s*/
        } /* --- end-of-if(ibyte<nbytes-1) --- */
    } /* --- end-of-for(ibyte) --- */
    /* closing " after last line */
    if (isstr) fprintf(fp, "\"");
    /* back with 1=okay, 0=failed */
    return (1);
} /* --- end-of-function hex_bitmap() --- */

