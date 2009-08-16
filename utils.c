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
char    *dbltoa(double dblval, int npts)
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
char    *timestamp(int tzdelta, int ifmt)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    int tzadjust();
    int daynumber();
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
char    *strwrap(char *s, int linelen, int tablen)
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
    /* remove \n's */
    int strreplace();
    /* add \n's and indent space */
    char    *strchange();
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
        if (0 && msgfp != NULL && msglevel >= 99) {
            fprintf(msgfp, "strwrap> rhslen=%d, sol=\"\"%s\"\"\n", rhslen, sol);
            fflush(msgfp);
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
char    *urlprune(char *url, int n)
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
int urlncmp(char *url1, char *url2, int n)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    char *urlprune();
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
char    *strwstr(char *string, char *substr, char *white, int *sublen)
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
    if (msglevel >= 999 && msgfp != NULL) {   /* debugging/diagnostic output */
        fprintf(msgfp, "strwstr> str=\"%.72s\" sub=\"%s\" found at offset %d\n",
                string, substr, (pfound == NULL ? (-1) : (int)(pfound - string)));
        fflush(msgfp);
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
int unescape_url(char *url, int isescape)
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
/* --- entry point --- */
char x2c(char *what)
{
    char digit;
    digit = (what[0] >= 'A' ? ((what[0] & 0xdf) - 'A') + 10 : (what[0] - '0'));
    digit *= 16;
    digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A') + 10 : (what[1] - '0'));
    return(digit);
} /* --- end-of-function x2c() --- */


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
    char    *strchange();
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
FILE    *rastopenfile(char *filename, char *mode)
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
    if (msglevel >= 9 && msgfp != NULL) { /* debuging */
        fprintf(msgfp, "rastopenfile> returning fopen(%s,%s) = %s\n",
                filename, amode, (fp == NULL ? "NULL" : "Okay"));
        fflush(msgfp);
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
int rastreadfile(char *filename, int islock, char *tag, char *value)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    /* pointer to opened filename */
    FILE    *fp = (FILE *)NULL, *rastopenfile();
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
        fp = rastopenfile(texfile, (islock ? "r+" : "r"));
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
int rastwritefile(char *filename, char *tag, char *value, int isstrict)
{
    /* ------------------------------------------------------------
    Allocations and Declarations
    ------------------------------------------------------------ */
    char *strchange();
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
    if (rastopenfile(texfile, NULL)      /* unchanged or .tex appended */
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
            if (rastreadfile(texfile, 1, NULL, filebuff) /* read entire existing file */
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
    if ((fp = rastopenfile(texfile, "w"))  /* open for write */
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
                    status = rastwritefile(filename, "timestamp", fbuff, 1);
                }
    /* --- return status to caller --- */
end_of_job:
    /* return status to caller */
    return (status);
} /* --- end-of-function rastwritefile() --- */

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
char    *rasteditfilename ( char *filename )
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

