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
 * --------------------------------------------------------------------------
 *
 * Purpose:   o MimeTeX, licensed under the gpl, lets you easily embed
 *      LaTeX math in your html pages.  It parses a LaTeX math
 *      expression and immediately emits the corresponding gif
 *      image, rather than the usual TeX dvi.  And mimeTeX is an
 *      entirely separate little program that doesn't use TeX or
 *      its fonts in any way.  It's just one cgi that you put in
 *      your site's cgi-bin/ directory, with no other dependencies.
 *           So mimeTeX is very easy to install.  And it's equally
 *      easy to use.  Just place an html <img> tag in your document
 *      wherever you want to see the corresponding LaTeX expression.
 *      For example,
 *       <img src="../cgi-bin/mimetex.cgi?\int_{-\infty}^xe^{-t^2}dt"
 *        alt="" border=0 align=middle>
 *      immediately generates the corresponding gif image on-the-fly,
 *      displaying the rendered expression wherever you put that
 *      <img> tag.
 *           MimeTeX doesn't need intermediate dvi-to-gif conversion,
 *      and it doesn't clutter up your filesystem with separate
 *      little gif files for each converted expression.
 *      But image caching is available by using mimeTeX's
 *      -DCACHEPATH=\"path/\" compile option (see below).
 *      There's also no inherent need to repeatedly write the
 *      cumbersome <img> tag illustrated above.  You can write
 *      your own custom tags, or write a wrapper script around
 *      mimeTeX to simplify the notation.
 *           Further discussion about mimeTeX's features and
 *      usage is available on its homepage,
 *        http://www.forkosh.com/mimetex.html
 *      and similarly in mimetex.html included with your mimetex.zip
 *      distribution file. (Note: http://www.forkosh.com/mimetex.html
 *      is a "quickstart" version of the the full mimetex.html manual
 *      included in your mimetex.zip distribution file.)
 *
 * Functions:   The following "table of contents" lists each function
 *      comprising mimeTeX in the order it appears in this file.
 *      See individual function entry points for specific comments
 *      about its purpose, calling sequence, side effects, etc.
 *      (All these functions eventually belong in several
 *      different modules, possibly along the lines suggested
 *      by the divisions below.  But until the best decomposition
 *      becomes clear, it seems better to keep mimetex.c
 *      neatly together, avoiding a bad decomposition that
 *      becomes permanent by default.)
 *      ===================== Raster Functions ======================
 *  PART2   --- raster constructor functions ---
 *      new_raster(width,height,pixsz)   allocation (and constructor)
 *      new_subraster(width,height,pixsz)allocation (and constructor)
 *      new_chardef()                         allocate chardef struct
 *      delete_raster(rp)        deallocate raster (rp =  raster ptr)
 *      delete_subraster(sp)  deallocate subraster (sp=subraster ptr)
 *      delete_chardef(cp)      deallocate chardef (cp = chardef ptr)
 *      --- primitive (sub)raster functions ---
 *      rastcpy(rp)                           allocate new copy of rp
 *      subrastcpy(sp)                        allocate new copy of sp
 *      rastrot(rp)         new raster rotated right 90 degrees to rp
 *      rastref(rp,axis)    new raster reflected (axis 1=horz,2=vert)
 *      rastput(target,source,top,left,isopaque)  overlay src on trgt
 *      rastcompose(sp1,sp2,offset2,isalign,isfree) sp2 on top of sp1
 *      rastcat(sp1,sp2,isfree)                  concatanate sp1||sp2
 *      rastack(sp1,sp2,base,space,iscenter,isfree)stack sp2 atop sp1
 *      rastile(tiles,ntiles)      create composite raster from tiles
 *      rastsmash(sp1,sp2,xmin,ymin)      calc #smash pixels sp1||sp2
 *      rastsmashcheck(term)         check if term is "safe" to smash
 *      --- raster "drawing" functions ---
 *      accent_subraster(accent,width,height)       draw \hat\vec\etc
 *      arrow_subraster(width,height,drctn,isBig)    left/right arrow
 *      uparrow_subraster(width,height,drctn,isBig)     up/down arrow
 *      rule_raster(rp,top,left,width,height,type)    draw rule in rp
 *      line_raster(rp,row0,col0,row1,col1,thickness) draw line in rp
 *      line_recurse(rp,row0,col0,row1,col1,thickness)   recurse line
 *      circle_raster(rp,row0,col0,row1,col1,thickness,quads) ellipse
 *      circle_recurse(rp,row0,col0,row1,col1,thickness,theta0,theta1)
 *      bezier_raster(rp,r0,c0,r1,c1,rt,ct)   draw bezier recursively
 *      border_raster(rp,ntop,nbot,isline,isfree)put border around rp
 *      backspace_raster(rp,nback,pback,minspace,isfree)    neg space
 *      --- raster (and chardef) output functions ---
 *      type_raster(rp,fp)       emit ascii dump of rp on file ptr fp
 *      type_bytemap(bp,grayscale,width,height,fp) dump bytemap on fp
 *      xbitmap_raster(rp,fp)           emit mime xbitmap of rp on fp
 *      type_pbmpgm(rp,ptype,file)     pbm or pgm image of rp to file
 *      cstruct_chardef(cp,fp,col1)         emit C struct of cp on fp
 *      cstruct_raster(rp,fp,col1)          emit C struct of rp on fp
 *      hex_bitmap(rp,fp,col1,isstr)emit hex dump of rp->pixmap on fp
 *      --- ancillary output functions ---
 *      emit_string(fp,col1,string,comment) emit string and C comment
 *      gftobitmap(rp)        convert .gf-like pixmap to bitmap image
 *      ====================== Font Functions =======================
 *      --- font lookup functions ---
 *      get_symdef(symbol)              return mathchardef for symbol
 *      get_ligature(expr,family)  return symtable index for ligature
 *      get_chardef(symdef,size)       return chardef for symdef,size
 *      get_charsubraster(symdef,size)  wrap subraster around chardef
 *      get_symsubraster(symbol,size)    returns subraster for symbol
 *      --- ancillary font functions ---
 *      get_baseline(gfdata)       determine baseline (in our coords)
 *      get_delim(symbol,height,family) delim just larger than height
 *      make_delim(symbol,height) construct delim exactly height size
 *      ================= Tokenize/Parse Functions ==================
 *      texchar(expression,chartoken)  retruns next char or \sequence
 *      texsubexpr(expr,subexpr,maxsubsz,left,right,isescape,isdelim)
 *      texleft(expr,subexpr,maxsubsz,ldelim,rdelim)   \left...\right
 *      texscripts(expression,subscript,superscript,which)get scripts
 *      --- ancillary parse functions ---
 *      isbrace(expression,braces,isescape)   check for leading brace
 *      preamble(expression,size,subexpr)              parse preamble
 *      mimeprep(expression) preprocessor converts \left( to \(, etc.
 *      strchange(nfirst,from,to)   change nfirst chars of from to to
 *      strreplace(string,from,to,nreplace)  change from to to in str
 *      strwstr(string,substr,white,sublen)     find substr in string
 *      strdetex(s,mode)    replace math chars like \^_{} for display
 *      strtexchr(string,texchr)                find texchr in string
 *      findbraces(expression,command)    find opening { or closing }
 *      isstrstr(string,snippets,iscase)  are any snippets in string?
 *      unescape_url(url,isescape), x2c(what)   xlate %xx url-encoded
 *  PART3   =========== Rasterize an Expression (recursively) ===========
 *      --- here's the primary entry point for all of mimeTeX ---
 *      rasterize(expression,size)     parse and rasterize expression
 *      --- explicitly called handlers that rasterize... ---
 *      rastparen(subexpr,size,basesp)          parenthesized subexpr
 *      rastlimits(expression,size,basesp)    dispatch super/sub call
 *      rastscripts(expression,size,basesp) super/subscripted exprssn
 *      rastdispmath(expression,size,sp)      scripts for displaymath
 *      --- table-driven handlers that rasterize... ---
 *      rastleft(expression,size,basesp,ildelim,arg2,arg3)\left\right
 *      rastright(expression,size,basesp,ildelim,arg2,arg3) ...\right
 *      rastmiddle(expression,size,basesp,arg1,arg2,arg3)     \middle
 *      rastflags(expression,size,basesp,flag,value,arg3)    set flag
 *      rastspace(expression,size,basesp,width,isfill,isheight)\,\:\;
 *      rastnewline(expression,size,basesp,arg1,arg2,arg3)         \\
 *      rastarrow(expression,size,basesp,width,height,drctn) \longarr
 *      rastuparrow(expression,size,basesp,width,height,drctn)up/down
 *      rastoverlay(expression,size,basesp,overlay,arg2,arg3)    \not
 *      rastfrac(expression,size,basesp,isfrac,arg2,arg3) \frac \atop
 *      rastackrel(expression,size,basesp,base,arg2,arg3)   \stackrel
 *      rastmathfunc(expression,size,basesp,base,arg2,arg3) \lim,\etc
 *      rastsqrt(expression,size,basesp,arg1,arg2,arg3)         \sqrt
 *      rastaccent(expression,size,basesp,accent,isabove,isscript)
 *      rastfont(expression,size,basesp,font,arg2,arg3) \cal{},\scr{}
 *      rastbegin(expression,size,basesp,arg1,arg2,arg3)     \begin{}
 *      rastarray(expression,size,basesp,arg1,arg2,arg3)       \array
 *      rastpicture(expression,size,basesp,arg1,arg2,arg3)   \picture
 *      rastline(expression,size,basesp,arg1,arg2,arg3)         \line
 *      rastrule(expression,size,basesp,arg1,arg2,arg3)         \rule
 *      rastcircle(expression,size,basesp,arg1,arg2,arg3)     \circle
 *      rastbezier(expression,size,basesp,arg1,arg2,arg3)     \bezier
 *      rastraise(expression,size,basesp,arg1,arg2,arg3)    \raisebox
 *      rastrotate(expression,size,basesp,arg1,arg2,arg3)  \rotatebox
 *      rastreflect(expression,size,basesp,arg1,arg2,arg3)\reflectbox
 *      rastfbox(expression,size,basesp,arg1,arg2,arg3)         \fbox
 *      rastinput(expression,size,basesp,arg1,arg2,arg3)       \input
 *      rastcounter(expression,size,basesp,arg1,arg2,arg3)   \counter
 *      rasttoday(expression,size,basesp,arg1,arg2,arg3)       \today
 *      rastcalendar(expression,size,basesp,arg1,arg2,arg3) \calendar
 *      rastenviron(expression,size,basesp,arg1,arg2,arg3)   \environ
 *      rastmessage(expression,size,basesp,arg1,arg2,arg3)   \message
 *      rastnoop(expression,size,basesp,arg1,arg2,arg3) flush \escape
 *      --- helper functions for handlers ---
 *      rastopenfile(filename,mode)      opens filename[.tex] in mode
 *      sanitize_pathname(filename)       edit filename (for security)
 *      rastreadfile(filename,islock,tag,value)   read <tag>...</tag>
 *      rastwritefile(filename,tag,value,isstrict)write<tag>...</tag>
 *      calendar(year,month,day)    formats one-month calendar string
 *      timestamp(tzdelta,ifmt)              formats timestamp string
 *      tzadjust(tzdelta,year,month,day,hour)        adjust date/time
 *      daynumber(year,month,day)     #days since Monday, Jan 1, 1973
 *      strwrap(s,linelen,tablen)insert \n's and spaces to wrap lines
 *      strnlower(s,n)        lowercase the first n chars of string s
 *      urlprune(url,n)  http://abc.def.ghi.com/etc-->abc.def.ghi.com
 *      urlncmp(url1,url2,n)   compares topmost n levels of two url's
 *      dbltoa(d,npts)                double to comma-separated ascii
 *      === Anti-alias completed raster (lowpass) or symbols (ss) ===
 *      aalowpass(rp,bytemap,grayscale)     lowpass grayscale bytemap
 *      aapnm(rp,bytemap,grayscale)       lowpass based on pnmalias.c
 *      aapnmlookup(rp,bytemap,grayscale)  aapnm based on aagridnum()
 *      aapatterns(rp,irow,icol,gridnum,patternum,grayscale) call 19,
 *      aapattern1124(rp,irow,icol,gridnum,grayscale)antialias pattrn
 *      aapattern19(rp,irow,icol,gridnum,grayscale) antialias pattern
 *      aapattern20(rp,irow,icol,gridnum,grayscale) antialias pattern
 *      aapattern39(rp,irow,icol,gridnum,grayscale) antialias pattern
 *      aafollowline(rp,irow,icol,direction)       looks for a "turn"
 *      aagridnum(rp,irow,icol)             calculates gridnum, 0-511
 *      aapatternnum(gridnum)    looks up pattern#, 1-51, for gridnum
 *      aalookup(gridnum)     table lookup for all possible 3x3 grids
 *      aalowpasslookup(rp,bytemap,grayscale)   driver for aalookup()
 *      aasupsamp(rp,aa,sf,grayscale)             or by supersampling
 *      aacolormap(bytemap,nbytes,colors,colormap)make colors,colormap
 *      aaweights(width,height)      builds "canonical" weight matrix
 *      aawtpixel(image,ipixel,weights,rotate) weight image at ipixel
 *      === miscellaneous ===
 *      mimetexsetmsg(newmsglevel,newmsgfp)    set msglevel and msgfp
 *  PART1   ========================== Driver ===========================
 *      main(argc,argv) parses math expression and emits mime xbitmap
 *      CreateGifFromEq(expression,gifFileName)  entry pt for win dll
 *      ismonth(month)          is month current month ("jan"-"dec")?
 *      logger(fp,msglevel,logvars)        logs environment variables
 *      emitcache(cachefile,maxage,valign,isbuffer)    emit cachefile
 *      readcachefile(cachefile,buffer)    read cachefile into buffer
 *      md5str(instr)                      md5 hash library functions
 *      GetPixel(x,y)           callback function for gifsave library
 *
 * Source:  mimetex.c  (needs mimetex.h and texfonts.h to compile,
 *      and also needs gifsave.c when compiled with -DAA or -DGIF)
 *
 * --------------------------------------------------------------------------
 * Notes      o See individual function entry points for specific comments
 *      about the purpose, calling sequence, side effects, etc
 *      of each mimeTeX function listed above.
 *        o See bottom of file for main() driver (and "friends"),
 *      and compile as
 *         cc -DAA mimetex.c gifsave.c -lm -o mimetex.cgi
 *      to produce an executable that emits gif images with
 *      anti-aliasing (see Notes below).  You may also compile
 *         cc -DGIF mimetex.c gifsave.c -lm -o mimetex.cgi
 *      to produce an executable that emits gif images without
 *      anti-aliasing.  Alternatively, compile mimeTeX as
 *         cc -DXBITMAP mimetex.c -lm -o mimetex.cgi
 *      to produce an executable that just emits mime xbitmaps.
 *      In either case you'll need mimetex.h and texfonts.h,
 *      and with -DAA or -DGIF you'll also need gifsave.c
 *        o The font information in texfonts.h was produced by multiple
 *      runs of gfuntype, one run per struct (i.e., one run per font
 *      family at a particular size).  Compile gfuntype as
 *         cc gfuntype.c mimetex.c -lm -o gfuntype
 *      See gfuntype.c, and also mimetex.html#fonts, for details.
 *        o For gif images, the gifsave.c library by Sverre H. Huseby
 *      <http://shh.thathost.com> slightly modified by me to allow
 *      (a)sending output to stdout or returning it in memory,
 *      and (b)specifying a transparent background color index,
 *      is included with mimeTeX, and it's documented in
 *      mimetex.html#gifsave
 *        o MimeTeX's principal reusable function is rasterize(),
 *      which takes a string like "f(x)=\int_{-\infty}^xe^{-t^2}dt"
 *      and returns a (sub)raster representing it as a bit or bytemap.
 *      Your application can do anything it likes with this pixel map.
 *      MimeTeX just outputs it, either as a mime xbitmap or as a gif.
 *      See  mimetex.html#makeraster  for further discussion
 *      and examples.
 *        o File mimetex.c also contains library functions implementing
 *      a raster datatype, functions to manipulate rasterized .mf
 *      fonts (see gfuntype.c which rasterizes .mf fonts), functions
 *      to parse LaTeX expressions, etc.  As already mentioned,
 *      a complete list of mimetex.c functions is above.  See their
 *      individual entry points below for further comments.
 *         As also mentioned, these functions eventually belong in
 *      several different modules, possibly along the lines suggested
 *      by the divisions above.  But until the best decomposition
 *      becomes clear, it seems better to keep mimetex.c
 *      neatly together, avoiding a bad decomposition that
 *      becomes permanent by default.
 *        o Optional compile-line -D defined symbols are documented
 *      in mimetex.html#options .  They include (additional -D
 *      switches are discussed at mimetex.html#options)...
 *      -DAA
 *          Turns on gif anti-aliasing with default values
 *          (CENTERWT=32, ADJACENTWT=3, CORNERWT=1)
 *          for the following anti-aliasing parameters...
 *      -DCENTERWT=n
 *      -DADJACENTWT=j
 *      -DCORNERWT=k
 *          *** Note: Ignore these three switches because
 *          *** mimeTeX's current anti-aliasing algorithm
 *          *** no longer uses them (as of version 1.60).
 *          MimeTeX currently provides a lowpass filtering
 *          algorithm for anti-aliasing, which is applied to the
 *          existing set of bitmap fonts.  This lowpass filter
 *          applies default weights
 *              1   2   1
 *              2   8   2
 *              1   2   1
 *          to neighboring pixels. The defaults weights are
 *          CENTERWT=8, ADJACENTWT=2 and CORNERWT=1,
 *          which you can adjust to control anti-aliasing.
 *          Lower CENTERWT values will blur/spread out lines
 *          while higher values will tend to sharpen lines.
 *          Experimentation is recommended to determine
 *          what value works best for you.
 *      -DCACHEPATH=\"path/\"
 *          This option saves each rendered image to a file
 *          in directory  path/  which mimeTeX reads rather than
 *          re-rendering the same image every time it's given
 *          the same LaTeX expression.  Sometimes mimeTeX disables
 *          caching, e.g., expressions containing \input{ } are
 *          re-rendered since the contents of the inputted file
 *          may have changed.  If compiled without -DCACHEPATH
 *          mimeTeX always re-renders expressions.  This usually
 *          isn't too cpu intensive, but if you have unusually
 *          high hit rates then image caching may be helpful.
 *          The  path/  is relative to mimetex.cgi, and must
 *          be writable by it.  Files created under  path/  are
 *          named filename.gif, where filename is the 32-character
 *          MD5 hash of the LaTeX expression.
 *      -DDEFAULTSIZE=n
 *          MimeTeX currently has eight font sizes numbered 0-7,
 *          and always starts in DEFAULTSIZE whose default value
 *          is 3 (corresponding to \large). Specify -DDEFAULTSIZE=4
 *          on the compile line if you prefer mimeTeX to start in
 *          larger default size 4 (corresponding to \Large), etc.
 *      -DDISPLAYSIZE=n
 *          By default, operator limits like \int_a^b are rendered
 *          \textstyle at font sizes \normalsize and smaller,
 *          and rendered \displaystyle at font sizes \large and
 *          larger.  This default corresponds to -DDISPLAYSIZE=3,
 *          which you can adjust; e.g., -DDISPLAYSIZE=0 always
 *          defaults to \displaystyle, and 99 (or any large number)
 *          always defaults to \textstyle.  Note that explicit
 *          \textstyle, \displaystyle, \limits or \nolimits
 *          directives in an expression always override
 *          the DISPLAYSIZE default.
 *      -DERRORSTATUS=n
 *          The default, 0, means mimeTeX always exits with status 0,
 *          regardless of whether or not it detects error(s) while
 *          trying to render your expression.  Specify any non-zero
 *          value (typically -1) if you write a script/plugin for
 *          mimeTeX that traps non-zero exit statuses.  MimeTeX then
 *          exits with its own non-zero status when it detects an
 *          error it can identify, or with your ERRORSTATUS value
 *          for errors it can't specifically identify.
 *      -DREFERER=\"domain\"   -or-
 *      -DREFERER=\"domain1,domain2,etc\"
 *          Blocks mimeTeX requests from unauthorized domains that
 *          may be using your server's mimetex.cgi without permission.
 *          If REFERER is defined, mimeTeX checks for the environment
 *          variable HTTP_REFERER and, if it exists, performs a
 *          case-insensitive test to make sure it contains 'domain'
 *          as a substring.  If given several 'domain's (second form)
 *          then HTTP_REFERER must contain either 'domain1' or
 *          'domain2', etc, as a (case-insensitive) substring.
 *          If HTTP_REFERER fails to contain a substring matching
 *          any of these domain(s), mimeTeX emits an error message
 *          image corresponding to the expression specified by
 *          the  invalid_referer_msg  string defined in main().
 *          Note: if HTTP_REFERER is not an environment variable,
 *          mimeTeX correctly generates the requested expression
 *          (i.e., no referer error).
 *      -DWARNINGS=n  -or-
 *      -DNOWARNINGS
 *          If an expression submitted to mimeTeX contains an
 *          unrecognzied escape sequence, e.g., "y=x+\abc+1", then
 *          mimeTeX generates a gif image containing an embedded
 *          warning in the form "y=x+[\abc?]+1".  If you want these
 *          warnings suppressed, -DWARNINGS=0 or -DNOWARNINGS tells
 *          mimeTeX to ignore unrecognized symbols, and the rendered
 *          image is "y=x++1" instead.
 *      -DWHITE
 *          MimeTeX usually renders black symbols on a white
 *          background.  This option renders white symbols on
 *          a black background instead.
 * --------------------------------------------------------------------------
 * Revision History:
 * 09/18/02 J.Forkosh   Installation.
 * 12/11/02 J.Forkosh   Version 1.00 released.
 * 07/04/03 J.Forkosh   Version 1.01 released.
 * 10/17/03 J.Forkosh   Version 1.20 released.
 * 12/21/03 J.Forkosh   Version 1.30 released.
 * 02/01/04 J.Forkosh   Version 1.40 released.
 * 10/02/04 J.Forkosh   Version 1.50 released.
 * 11/30/04 J.Forkosh   Version 1.60 released.
 * 10/11/05 J.Forkosh   Version 1.64 released.
 * 11/30/06 J.Forkosh   Version 1.65 released.
 * 09/06/08 J.Forkosh   Version 1.70 released.
 * 03/23/09 J.Forkosh   Version 1.71 released.
 * 06/17/09 J.Forkosh   Most recent revision (also see REVISIONDATE).
 *
 ****************************************************************************/

/* ------------------------------------------------------------
header files and macros
------------------------------------------------------------ */
/* --- standard headers --- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "mimetex_priv.h"

#include "mimetex.h"

/* --- anti-aliasing parameter values by font weight --- */
aaparameters aaparams[] = {
 /* ------------------------------------------------------------
    centerwt adj corner minadj max  fgalias,only,bgalias,only
    ------------------------------------------------------------ */
      { 64,  1,  1,    6,  8,     1, 0, 0, 0 },  /* 0 = light */
      /* below is { CENTERWT, ADJACENTWT, CORNERWT, MINADJACENT, MAXADJACENT, 1, 0, 0, 0 }, */
      { 8,   2,  1,    6,  8,     1, 0, 0, 0 },
      { 8,   1,  1,    5,  8,     1, 0, 0, 0 },  /* 2 = semibold */
      { 8,   2,  1,    4,  9,     1, 0, 0, 0 }   /* 3 = bold */
}; /* --- end-of-aaparams[] --- */

/* ------------------------------------------------------------
other variables
------------------------------------------------------------ */
/* --- "smash" margin (0 means no smashing) --- */
#ifndef SMASHMARGIN
#ifdef NOSMASH
#define SMASHMARGIN 0
#else
#define SMASHMARGIN 3
#endif
#endif
#ifndef SMASHCHECK
#define SMASHCHECK 0
#endif
/* --- textwidth --- */
#ifndef TEXTWIDTH
#define TEXTWIDTH (400)
#endif
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
/* ------------------------------------------------------------
debugging and logging / error reporting
------------------------------------------------------------ */
/* --- debugging and error reporting --- */
#ifndef MSGLEVEL
#define MSGLEVEL 1
#endif
#define DBGLEVEL 9          /* debugging if mctx->msglevel>=DBGLEVEL */
#define LOGLEVEL 3          /* logging if mctx->msglevel>=LOGLEVEL */
#ifndef ERRORSTATUS         /* exit(ERRORSTATUS) for any error */
#define ERRORSTATUS 0         /* default doesn't signal errors */
#endif

/* --- embed warnings in rendered expressions, [\xxx?] if \xxx unknown --- */
#ifdef WARNINGS
#define WARNINGLEVEL WARNINGS
#else
#ifdef NOWARNINGS
#define WARNINGLEVEL 0
#else
#define WARNINGLEVEL 1
#endif
#endif

int nfontinfo =  8;

struct fontinfo_struct fontinfo[] = {/* --- name family istext class --- */
    { "\\math",    0,       0,  0 }, /*(0) default math mode */
    { "\\mathcal", CMSY10,  0,  1 }, /*(1) calligraphic, uppercase */
    { "\\mathscr", RSFS10,  0,  1 }, /*(2) rsfs/script, uppercase */
    { "\\textrm",  CMR10,   1, -1 }, /*(3) \rm,\text{abc} --> {\textrm~abc}*/
    { "\\textit",  CMMI10,  1, -1 }, /*(4) \it,\textit{abc}-->{\textit~abc}*/
    { "\\mathbb",  BBOLD10, 0, -1 }, /*(5) \bb,\mathbb{abc}-->{\mathbb~abc}*/
    { "\\mathbf",  CMMIB10, 0, -1 }, /*(6) \bf,\mathbf{abc}-->{\mathbf~abc}*/
    { "\\mathrm",  CMR10,   0, -1 }, /*(7) \mathrm */
    { "\\cyr",     CYR10,   1, -1 }, /*(8) \cyr (defaults as text mode) */
    {  NULL,       0,       0,  0 }
}; /* --- end-of-fonts[] --- */

/* ---
 * space between adjacent symbols, e.g., symspace[RELATION][VARIABLE]
 * ------------------------------------------------------------------ */
int symspace[11][11] = {
 /* -----------------------------------------------------------------------
         Right... ORD OPER  BIN  REL OPEN CLOS PUNC  VAR DISP SPACE unused
    Left... -------------------------------------------------------------- */
      /*ORDINARY*/  {  2,   3,   3,   5,   3,   2,   2,   2,   3,   0,    0 },
      /*OPERATOR*/  {  3,   1,   1,   5,   3,   2,   2,   2,   3,   0,    0 },
      /*BINARYOP*/  {  2,   1,   1,   5,   3,   2,   2,   2,   3,   0,    0 },
      /*RELATION*/  {  5,   5,   5,   2,   5,   5,   2,   5,   5,   0,    0 },
      /*OPENING*/  {  2,   2,   2,   5,   2,   4,   2,   2,   3,   0,    0 },
      /*CLOSING*/  {  2,   3,   3,   5,   4,   2,   1,   2,   3,   0,    0 },
      /*PUNCTION*/  {  2,   2,   2,   5,   2,   2,   1,   2,   2,   0,    0 },
      /*VARIABLE*/  {  2,   2,   2,   5,   2,   2,   1,   2,   2,   0,    0 },
      /*DISPOPER*/  {  2,   3,   3,   5,   2,   3,   2,   2,   2,   0,    0 },
      /*SPACEOPER*/  {  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0 },
      /*unused*/  {  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0 }
}; /* --- end-of-symspace[][] --- */

#include "texfonts.h"

/* ---
 * font families (by size), just a table of preceding font info
 * ------------------------------------------------------------ */
/* --- for low-pass anti-aliasing --- */
fontfamily aafonttable[] = {
/* -----------------------------------------------------------------------------------------
    family     size=0,        1,        2,        3,        4,        5,        6,        7
  ----------------------------------------------------------------------------------------- */
      {   CMR10, {   cmr83,   cmr100,   cmr118,   cmr131,   cmr160,   cmr180,   cmr210,   cmr250}},
      {  CMMI10, {  cmmi83,  cmmi100,  cmmi118,  cmmi131,  cmmi160,  cmmi180,  cmmi210,  cmmi250}},
      { CMMIB10, { cmmib83, cmmib100, cmmib118, cmmib131, cmmib160, cmmib180, cmmib210, cmmib250}},
      {  CMSY10, {  cmsy83,  cmsy100,  cmsy118,  cmsy131,  cmsy160,  cmsy180,  cmsy210,  cmsy250}},
      {  CMEX10, {  cmex83,  cmex100,  cmex118,  cmex131,  cmex160,  cmex180,  cmex210,  cmex250}},
      {  RSFS10, {  rsfs83,  rsfs100,  rsfs118,  rsfs131,  rsfs160,  rsfs180,  rsfs210,  rsfs250}},
      { BBOLD10, { bbold83, bbold100, bbold118, bbold131, bbold160, bbold180, bbold210, bbold250}},
      {STMARY10, {stmary83, stmary100, stmary118, stmary131, stmary160, stmary180, stmary210, stmary250}},
      {   CYR10, { wncyr83, wncyr100, wncyr118, wncyr131, wncyr160, wncyr180, wncyr210, wncyr250}},
      {    -999, {    NULL,     NULL,     NULL,     NULL,     NULL,     NULL,     NULL,     NULL}}
}; /* --- end-of-aafonttable[] --- */


/*supersampling mctx->shrinkfactor by size*/ 
int shrinkfactors[]= {
    3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

static mathchardef handlers[] = {
    /* ---------- c o m m a n d  h a n d l e r s --------------
          symbol    arg1     arg2     arg3       function
    -------------------------------------------------------- */
    /* --- commands --- */
    { "\\left", NOVALUE, NOVALUE, NOVALUE, rastleft },
    { "\\middle", NOVALUE, NOVALUE, NOVALUE, rastmiddle },
    { "\\frac",   1,    NOVALUE, NOVALUE, rastfrac },
    { "\\over",   1,    NOVALUE, NOVALUE, rastfrac },
    { "\\atop",   0,    NOVALUE, NOVALUE, rastfrac },
    { "\\choose", 0,    NOVALUE, NOVALUE, rastfrac },
    { "\\not",    1,          0, NOVALUE, rastoverlay },
    { "\\Not",    2,          0, NOVALUE, rastoverlay },
    { "\\widenot", 2,          0, NOVALUE, rastoverlay },
    { "\\sout",   3,    NOVALUE, NOVALUE, rastoverlay },
    { "\\strikeout", 3,  NOVALUE, NOVALUE, rastoverlay },
    { "\\compose", NOVALUE, NOVALUE, NOVALUE, rastoverlay },
    { "\\stackrel", 2,  NOVALUE, NOVALUE, rastackrel },
    { "\\relstack", 1,  NOVALUE, NOVALUE, rastackrel },
    { "\\sqrt", NOVALUE, NOVALUE, NOVALUE, rastsqrt },
    { "\\overbrace",  OVERBRACE, 1,    1, rastaccent },
    { "\\underbrace", UNDERBRACE, 0,    1, rastaccent },
    { "\\overline",   BARACCENT, 1,    0, rastaccent },
    { "\\underline", UNDERBARACCENT, 0, 0, rastaccent },
    { "\\begin", NOVALUE, NOVALUE, NOVALUE, rastbegin },
    { "\\array", NOVALUE, NOVALUE, NOVALUE, rastarray },
    { "\\matrix", NOVALUE, NOVALUE, NOVALUE, rastarray },
    { "\\tabular", NOVALUE, NOVALUE, NOVALUE, rastarray },
    { "\\picture", NOVALUE, NOVALUE, NOVALUE, rastpicture },
    { "\\line", NOVALUE, NOVALUE, NOVALUE, rastline },
    { "\\rule", NOVALUE, NOVALUE, NOVALUE, rastrule },
    { "\\circle", NOVALUE, NOVALUE, NOVALUE, rastcircle },
    { "\\bezier", NOVALUE, NOVALUE, NOVALUE, rastbezier },
    { "\\qbezier", NOVALUE, NOVALUE, NOVALUE, rastbezier },
    { "\\raisebox", NOVALUE, NOVALUE, NOVALUE, rastraise },
    { "\\rotatebox", NOVALUE, NOVALUE, NOVALUE, rastrotate },
    { "\\reflectbox", NOVALUE, NOVALUE, NOVALUE, rastreflect },
    { "\\fbox", NOVALUE, NOVALUE, NOVALUE, rastfbox },
    { "\\today", NOVALUE, NOVALUE, NOVALUE, rasttoday },
    { "\\calendar", NOVALUE, NOVALUE, NOVALUE, rastcalendar },
    /* --- spaces --- */
    { "\\/",    1,  NOVALUE, NOVALUE, rastspace },
    { "\\,",    2,  NOVALUE, NOVALUE, rastspace },
    { "\\:",    4,  NOVALUE, NOVALUE, rastspace },
    { "\\;",    6,  NOVALUE, NOVALUE, rastspace },
    { "\\\n",   3,  NOVALUE, NOVALUE, rastspace },
    { "\\\r",   3,  NOVALUE, NOVALUE, rastspace },
    { "\\\t",   3,  NOVALUE, NOVALUE, rastspace },
    /*{ "\\~",5,NOVALUE,NOVALUE,rastspace },*/
    { "~",  5,  NOVALUE, NOVALUE, rastspace },
    { "\\ ",    5,  NOVALUE, NOVALUE, rastspace },
    { " ",  5,  NOVALUE, NOVALUE, rastspace },
    { "\\!",    -2, NOVALUE, NOVALUE, rastspace },
    /*{ "\\!*", -2,      99,NOVALUE,  rastspace },*/
    { "\\quad", 6,  NOVALUE, NOVALUE, rastspace },
    { "\\qquad", 10, NOVALUE, NOVALUE, rastspace },
    { "\\hspace", 0, NOVALUE, NOVALUE, rastspace },
    { "\\hspace*", 0,         99, NOVALUE, rastspace },
    { "\\vspace", 0, NOVALUE,      1, rastspace },
    { "\\hfill", 0,        1, NOVALUE, rastspace },
    /* --- newline --- */
    { "\\\\",   NOVALUE, NOVALUE, NOVALUE, rastnewline },
    /* --- arrows --- */
    { "\\longrightarrow",   1, 0, NOVALUE, rastarrow },
    { "\\Longrightarrow",   1, 1, NOVALUE, rastarrow },
    { "\\longleftarrow",   -1, 0, NOVALUE, rastarrow },
    { "\\Longleftarrow",   -1, 1, NOVALUE, rastarrow },
    { "\\longleftrightarrow", 0, 0, NOVALUE, rastarrow },
    { "\\Longleftrightarrow", 0, 1, NOVALUE, rastarrow },
    { "\\longuparrow",      1, 0, NOVALUE, rastuparrow },
    { "\\Longuparrow",      1, 1, NOVALUE, rastuparrow },
    { "\\longdownarrow",   -1, 0, NOVALUE, rastuparrow },
    { "\\Longdownarrow",   -1, 1, NOVALUE, rastuparrow },
    { "\\longupdownarrow",  0, 0, NOVALUE, rastuparrow },
    { "\\Longupdownarrow",  0, 1, NOVALUE, rastuparrow },
    /* --- modes and values --- */
    { "\\cal",        1,     NOVALUE, NOVALUE, rastfont },
    { "\\mathcal",    1,     NOVALUE, NOVALUE, rastfont },
    { "\\scr",        2,     NOVALUE, NOVALUE, rastfont },
    { "\\mathscr",    2,     NOVALUE, NOVALUE, rastfont },
    { "\\mathfrak",   2,     NOVALUE, NOVALUE, rastfont },
    { "\\mathbb",     5,     NOVALUE, NOVALUE, rastfont },
    { "\\rm",         3,     NOVALUE, NOVALUE, rastfont },
    { "\\text",       3,     NOVALUE, NOVALUE, rastfont },
    { "\\textrm",     3,     NOVALUE, NOVALUE, rastfont },
    { "\\mathrm",     7,     NOVALUE, NOVALUE, rastfont },
    { "\\cyr",        8,     NOVALUE, NOVALUE, rastfont },
    { "\\mathbf",     6,     NOVALUE, NOVALUE, rastfont },
    { "\\bf",         6,     NOVALUE, NOVALUE, rastfont },
    { "\\mathtt",     3,     NOVALUE, NOVALUE, rastfont },
    { "\\mathsf",     3,     NOVALUE, NOVALUE, rastfont },
    { "\\mbox",       3,     NOVALUE, NOVALUE, rastfont },
    { "\\operatorname",   3,     NOVALUE, NOVALUE, rastfont },
    { "\\it",         4,     NOVALUE, NOVALUE, rastfont },
    { "\\textit",     4,     NOVALUE, NOVALUE, rastfont },
    { "\\mathit",     4,     NOVALUE, NOVALUE, rastfont },
    { "\\rm",     ISFONTFAM,           3, NOVALUE, rastflags },
    { "\\it",     ISFONTFAM,           4, NOVALUE, rastflags },
    { "\\sl",     ISFONTFAM,           4, NOVALUE, rastflags },
    { "\\bb",     ISFONTFAM,           5, NOVALUE, rastflags },
    { "\\bf",     ISFONTFAM,           6, NOVALUE, rastflags },
    { "\\text",   ISFONTFAM,           3, NOVALUE, rastflags },
    { "\\math",   ISFONTFAM,           0, NOVALUE, rastflags },
    { "\\ascii",     ISSTRING,         1, NOVALUE, rastflags },
    { "\\image",     ISSTRING,         0, NOVALUE, rastflags },
    { "\\limits",    ISDISPLAYSTYLE,   2, NOVALUE, rastflags },
    { "\\nolimits",  ISDISPLAYSTYLE,   0, NOVALUE, rastflags },
    { "\\displaystyle", ISDISPLAYSTYLE, 2, NOVALUE, rastflags },
    { "\\textstyle", ISDISPLAYSTYLE,   0, NOVALUE, rastflags },
    { "\\displaysize", ISDISPLAYSIZE, NOVALUE, NOVALUE, rastflags},
    { "\\tiny",      ISFONTSIZE,       0, NOVALUE, rastflags },
    { "\\scriptsize", ISFONTSIZE,       0, NOVALUE, rastflags },
    { "\\footnotesize", ISFONTSIZE,     1, NOVALUE, rastflags },
    { "\\small",     ISFONTSIZE,       1, NOVALUE, rastflags },
    { "\\normalsize", ISFONTSIZE,       2, NOVALUE, rastflags },
    { "\\large",     ISFONTSIZE,       3, NOVALUE, rastflags },
    { "\\Large",     ISFONTSIZE,       4, NOVALUE, rastflags },
    { "\\LARGE",     ISFONTSIZE,       5, NOVALUE, rastflags },
    { "\\huge",      ISFONTSIZE,       6, NOVALUE, rastflags },
    { "\\Huge",      ISFONTSIZE,       7, NOVALUE, rastflags },
    { "\\HUGE",      ISFONTSIZE,       7, NOVALUE, rastflags },
    { "\\fontsize",  ISFONTSIZE, NOVALUE, NOVALUE, rastflags },
    { "\\fs",        ISFONTSIZE, NOVALUE, NOVALUE, rastflags },
    { "\\light",     ISWEIGHT,         0, NOVALUE, rastflags },
    { "\\regular",   ISWEIGHT,         1, NOVALUE, rastflags },
    { "\\semibold",  ISWEIGHT,         2, NOVALUE, rastflags },
    { "\\bold",      ISWEIGHT,         3, NOVALUE, rastflags },
    { "\\fontweight", ISWEIGHT,   NOVALUE, NOVALUE, rastflags },
    { "\\fw",        ISWEIGHT,   NOVALUE, NOVALUE, rastflags },
    { "\\centerwt",  ISCENTERWT, NOVALUE, NOVALUE, rastflags },
    { "\\adjacentwt", ISADJACENTWT, NOVALUE, NOVALUE, rastflags },
    { "\\cornerwt",  ISCORNERWT, NOVALUE, NOVALUE, rastflags },
    { "\\aaalg", ISAAALGORITHM,   NOVALUE, NOVALUE, rastflags },
    { "\\pnmparams", PNMPARAMS,   NOVALUE, NOVALUE, rastflags },
    { "\\gammacorrection", ISGAMMA, NOVALUE, NOVALUE, rastflags },
    { "\\nocontenttype", ISCONTENTTYPE, 0, NOVALUE, rastflags },
    { "\\opaque",    ISOPAQUE,         0, NOVALUE, rastflags },
    { "\\transparent", ISOPAQUE,        1, NOVALUE, rastflags },
    { "\\squash",    ISSMASH,          3, 1, rastflags },
    { "\\smash",     ISSMASH,          3, 1, rastflags },
    { "\\nosquash",  ISSMASH,          0, NOVALUE, rastflags },
    { "\\nosmash",   ISSMASH,          0, NOVALUE, rastflags },
    { "\\squashmargin", ISSMASH,  NOVALUE, NOVALUE, rastflags },
    { "\\smashmargin", ISSMASH,  NOVALUE, NOVALUE, rastflags },
    { "\\unitlength", UNITLENGTH, NOVALUE, NOVALUE, rastflags },
    { "\\reverse",   ISREVERSE,  NOVALUE, NOVALUE, rastflags },
    { "\\reversefg", ISREVERSE,        1, NOVALUE, rastflags },
    { "\\reversebg", ISREVERSE,        2, NOVALUE, rastflags },
    { "\\color",     ISCOLOR,    NOVALUE, NOVALUE, rastflags },
    { "\\red",       ISCOLOR,          1, NOVALUE, rastflags },
    { "\\green",     ISCOLOR,          2, NOVALUE, rastflags },
    { "\\blue",      ISCOLOR,          3, NOVALUE, rastflags },
    { "\\black",     ISCOLOR,          0, NOVALUE, rastflags },
    { "\\white",     ISCOLOR,          7, NOVALUE, rastflags },
    /* --- accents --- */
    { "\\vec",  VECACCENT,    1,      0, rastaccent },
    { "\\widevec", VECACCENT, 1,      0, rastaccent },
    { "\\bar",  BARACCENT,    1,      0, rastaccent },
    { "\\widebar", BARACCENT, 1,      0, rastaccent },
    { "\\hat",  HATACCENT,    1,      0, rastaccent },
    { "\\widehat", HATACCENT, 1,      0, rastaccent },
    { "\\tilde", TILDEACCENT, 1,      0, rastaccent },
    { "\\widetilde", TILDEACCENT, 1,    0, rastaccent },
    { "\\dot",  DOTACCENT,    1,      0, rastaccent },
    { "\\widedot", DOTACCENT, 1,      0, rastaccent },
    { "\\ddot", DDOTACCENT,   1,      0, rastaccent },
    { "\\wideddot", DDOTACCENT, 1,      0, rastaccent },
    /* --- math functions --- */
    { "\\arccos",   1,   0, NOVALUE, rastmathfunc },
    { "\\arcsin",   2,   0, NOVALUE, rastmathfunc },
    { "\\arctan",   3,   0, NOVALUE, rastmathfunc },
    { "\\arg",      4,   0, NOVALUE, rastmathfunc },
    { "\\cos",      5,   0, NOVALUE, rastmathfunc },
    { "\\cosh",     6,   0, NOVALUE, rastmathfunc },
    { "\\cot",      7,   0, NOVALUE, rastmathfunc },
    { "\\coth",     8,   0, NOVALUE, rastmathfunc },
    { "\\csc",      9,   0, NOVALUE, rastmathfunc },
    { "\\deg",      10,  0, NOVALUE, rastmathfunc },
    { "\\det",      11,  1, NOVALUE, rastmathfunc },
    { "\\dim",      12,  0, NOVALUE, rastmathfunc },
    { "\\exp",      13,  0, NOVALUE, rastmathfunc },
    { "\\gcd",      14,  1, NOVALUE, rastmathfunc },
    { "\\hom",      15,  0, NOVALUE, rastmathfunc },
    { "\\inf",      16,  1, NOVALUE, rastmathfunc },
    { "\\ker",      17,  0, NOVALUE, rastmathfunc },
    { "\\lg",       18,  0, NOVALUE, rastmathfunc },
    { "\\lim",      19,  1, NOVALUE, rastmathfunc },
    { "\\liminf",   20,  1, NOVALUE, rastmathfunc },
    { "\\limsup",   21,  1, NOVALUE, rastmathfunc },
    { "\\ln",       22,  0, NOVALUE, rastmathfunc },
    { "\\log",      23,  0, NOVALUE, rastmathfunc },
    { "\\max",      24,  1, NOVALUE, rastmathfunc },
    { "\\min",      25,  1, NOVALUE, rastmathfunc },
    { "\\Pr",       26,  1, NOVALUE, rastmathfunc },
    { "\\sec",      27,  0, NOVALUE, rastmathfunc },
    { "\\sin",      28,  0, NOVALUE, rastmathfunc },
    { "\\sinh",     29,  0, NOVALUE, rastmathfunc },
    { "\\sup",      30,  1, NOVALUE, rastmathfunc },
    { "\\tan",      31,  0, NOVALUE, rastmathfunc },
    { "\\tanh",     32,  0, NOVALUE, rastmathfunc },
    { "\\tr",       33,  0, NOVALUE, rastmathfunc },
    { "\\pmod",     34,  0, NOVALUE, rastmathfunc },
    /* --- flush -- recognized but not yet handled by mimeTeX --- */
    { "\\nooperation", 0, NOVALUE, NOVALUE, rastnoop },
    { "\\bigskip",   0, NOVALUE, NOVALUE, rastnoop },
    { "\\phantom",   1, NOVALUE, NOVALUE, rastnoop },
    { "\\nocaching", 0, NOVALUE, NOVALUE, rastnoop },
    { "\\noconten",  0, NOVALUE, NOVALUE, rastnoop },
    { "\\nonumber",  0, NOVALUE, NOVALUE, rastnoop },
    /* { "\\!",      0, NOVALUE,NOVALUE,  rastnoop }, */
    { "\\cydot",     0, NOVALUE, NOVALUE, rastnoop },
    { NULL,     -999,   -999,   -999,       NULL }
};

static mathchardef symbols_cmmi10[] = {
    /* --------------------- C M M I --------------------------
          symbol     charnum    family    class     function
    -------------------------------------------------------- */
    /* --- uppercase greek letters --- */
    { "\\Gamma",    0,  CMMI10,   VARIABLE, NULL },
    { "\\Delta",    1,  CMMI10,   VARIABLE, NULL },
    { "\\Theta",    2,  CMMI10,   VARIABLE, NULL },
    { "\\Lambda",   3,  CMMI10,   VARIABLE, NULL },
    { "\\Xi",       4,  CMMI10,   VARIABLE, NULL },
    { "\\Pi",       5,  CMMI10,   VARIABLE, NULL },
    { "\\Sigma",    6,  CMMI10,   VARIABLE, NULL },
    { "\\smallsum", 6,  CMMI10,   OPERATOR, NULL },
    { "\\Upsilon",  7,  CMMI10,   VARIABLE, NULL },
    { "\\Phi",      8,  CMMI10,   VARIABLE, NULL },
    { "\\Psi",      9,  CMMI10,   VARIABLE, NULL },
    { "\\Omega",    10, CMMI10,   VARIABLE, NULL },
    /* --- lowercase greek letters --- */
    { "\\alpha",    11, CMMI10,   VARIABLE, NULL },
    { "\\beta",     12, CMMI10,   VARIABLE, NULL },
    { "\\gamma",    13, CMMI10,   VARIABLE, NULL },
    { "\\delta",    14, CMMI10,   VARIABLE, NULL },
    { "\\epsilon",  15, CMMI10,   VARIABLE, NULL },
    { "\\zeta",     16, CMMI10,   VARIABLE, NULL },
    { "\\eta",      17, CMMI10,   VARIABLE, NULL },
    { "\\theta",    18, CMMI10,   VARIABLE, NULL },
    { "\\iota",     19, CMMI10,   VARIABLE, NULL },
    { "\\kappa",    20, CMMI10,   VARIABLE, NULL },
    { "\\lambda",   21, CMMI10,   VARIABLE, NULL },
    { "\\mu",       22, CMMI10,   VARIABLE, NULL },
    { "\\nu",       23, CMMI10,   VARIABLE, NULL },
    { "\\xi",       24, CMMI10,   VARIABLE, NULL },
    { "\\pi",       25, CMMI10,   VARIABLE, NULL },
    { "\\rho",      26, CMMI10,   VARIABLE, NULL },
    { "\\sigma",    27, CMMI10,   VARIABLE, NULL },
    { "\\tau",      28, CMMI10,   VARIABLE, NULL },
    { "\\upsilon",  29, CMMI10,   VARIABLE, NULL },
    { "\\phi",      30, CMMI10,   VARIABLE, NULL },
    { "\\chi",      31, CMMI10,   VARIABLE, NULL },
    { "\\psi",      32, CMMI10,   VARIABLE, NULL },
    { "\\omega",    33, CMMI10,   VARIABLE, NULL },
    { "\\varepsilon",   34, CMMI10,   VARIABLE, NULL },
    { "\\vartheta", 35, CMMI10,   VARIABLE, NULL },
    { "\\varpi",    36, CMMI10,   VARIABLE, NULL },
    { "\\varrho",   37, CMMI10,   VARIABLE, NULL },
    { "\\varsigma", 38, CMMI10,   VARIABLE, NULL },
    { "\\varphi",   39, CMMI10,   VARIABLE, NULL },
    /* --- arrow relations --- */
    { "\\leftharpoonup",    40, CMMI10,   ARROW,    NULL },
    { "\\leftharpoondown",  41, CMMI10,   ARROW,    NULL },
    { "\\rightharpoonup",   42, CMMI10,   ARROW,    NULL },
    { "\\rightharpoondown", 43, CMMI10,   ARROW,    NULL },
    /* --- punctuation --- */
    { "`",      44, CMMI10,   PUNCTION, NULL },
    { "\'",     45, CMMI10,   PUNCTION, NULL },
    /* --- triangle binary relations --- */
    { "\\triangleright",    46, CMMI10,   RELATION, NULL },
    { "\\triangleleft",     47, CMMI10,   RELATION, NULL },
    /* --- digits 0-9 --- */
    { "\\0",        48, CMMI10,   ORDINARY, NULL },
    { "\\1",        49, CMMI10,   ORDINARY, NULL },
    { "\\2",        50, CMMI10,   ORDINARY, NULL },
    { "\\3",        51, CMMI10,   ORDINARY, NULL },
    { "\\4",        52, CMMI10,   ORDINARY, NULL },
    { "\\5",        53, CMMI10,   ORDINARY, NULL },
    { "\\6",        54, CMMI10,   ORDINARY, NULL },
    { "\\7",        55, CMMI10,   ORDINARY, NULL },
    { "\\8",        56, CMMI10,   ORDINARY, NULL },
    { "\\9",        57, CMMI10,   ORDINARY, NULL },
    /* --- punctuation --- */
    { ".",      58, CMMI10,   PUNCTION, NULL },
    { ",",      59, CMMI10,   PUNCTION, NULL },
    /* --- operations (some ordinary) --- */
    { "<",      60, CMMI10,   OPENING,  NULL },
    { "\\<",        60, CMMI10,   OPENING,  NULL },
    { "\\lt",       60, CMMI10,   OPENING,  NULL },
    { "/",      61, CMMI10,   BINARYOP, NULL },
    { ">",      62, CMMI10,   CLOSING,  NULL },
    { "\\>",        62, CMMI10,   CLOSING,  NULL },
    { "\\gt",       62, CMMI10,   CLOSING,  NULL },
    { "\\star",     63, CMMI10,   BINARYOP, NULL },
    { "\\partial",  64, CMMI10,   VARIABLE, NULL },
    /* --- uppercase letters --- */
    { "A",      65, CMMI10,   VARIABLE, NULL },
    { "B",      66, CMMI10,   VARIABLE, NULL },
    { "C",      67, CMMI10,   VARIABLE, NULL },
    { "D",      68, CMMI10,   VARIABLE, NULL },
    { "E",      69, CMMI10,   VARIABLE, NULL },
    { "F",      70, CMMI10,   VARIABLE, NULL },
    { "G",      71, CMMI10,   VARIABLE, NULL },
    { "H",      72, CMMI10,   VARIABLE, NULL },
    { "I",      73, CMMI10,   VARIABLE, NULL },
    { "J",      74, CMMI10,   VARIABLE, NULL },
    { "K",      75, CMMI10,   VARIABLE, NULL },
    { "L",      76, CMMI10,   VARIABLE, NULL },
    { "M",      77, CMMI10,   VARIABLE, NULL },
    { "N",      78, CMMI10,   VARIABLE, NULL },
    { "O",      79, CMMI10,   VARIABLE, NULL },
    { "P",      80, CMMI10,   VARIABLE, NULL },
    { "Q",      81, CMMI10,   VARIABLE, NULL },
    { "R",      82, CMMI10,   VARIABLE, NULL },
    { "S",      83, CMMI10,   VARIABLE, NULL },
    { "T",      84, CMMI10,   VARIABLE, NULL },
    { "U",      85, CMMI10,   VARIABLE, NULL },
    { "V",      86, CMMI10,   VARIABLE, NULL },
    { "W",      87, CMMI10,   VARIABLE, NULL },
    { "X",      88, CMMI10,   VARIABLE, NULL },
    { "Y",      89, CMMI10,   VARIABLE, NULL },
    { "Z",      90, CMMI10,   VARIABLE, NULL },
    /* --- miscellaneous symbols and relations --- */
    { "\\flat",     91, CMMI10,   ORDINARY, NULL },
    { "\\natural",  92, CMMI10,   ORDINARY, NULL },
    { "\\sharp",    93, CMMI10,   ORDINARY, NULL },
    { "\\smile",    94, CMMI10,   RELATION, NULL },
    { "\\frown",    95, CMMI10,   RELATION, NULL },
    { "\\ell",      96, CMMI10,   ORDINARY, NULL },
    /* --- lowercase letters --- */
    { "a",      97, CMMI10,   VARIABLE, NULL },
    { "b",      98, CMMI10,   VARIABLE, NULL },
    { "c",      99, CMMI10,   VARIABLE, NULL },
    { "d",      100,    CMMI10,   VARIABLE, NULL },
    { "e",      101,    CMMI10,   VARIABLE, NULL },
    { "f",      102,    CMMI10,   VARIABLE, NULL },
    { "g",      103,    CMMI10,   VARIABLE, NULL },
    { "h",      104,    CMMI10,   VARIABLE, NULL },
    { "i",      105,    CMMI10,   VARIABLE, NULL },
    { "j",      106,    CMMI10,   VARIABLE, NULL },
    { "k",      107,    CMMI10,   VARIABLE, NULL },
    { "l",      108,    CMMI10,   VARIABLE, NULL },
    { "m",      109,    CMMI10,   VARIABLE, NULL },
    { "n",      110,    CMMI10,   VARIABLE, NULL },
    { "o",      111,    CMMI10,   VARIABLE, NULL },
    { "p",      112,    CMMI10,   VARIABLE, NULL },
    { "q",      113,    CMMI10,   VARIABLE, NULL },
    { "r",      114,    CMMI10,   VARIABLE, NULL },
    { "s",      115,    CMMI10,   VARIABLE, NULL },
    { "t",      116,    CMMI10,   VARIABLE, NULL },
    { "u",      117,    CMMI10,   VARIABLE, NULL },
    { "v",      118,    CMMI10,   VARIABLE, NULL },
    { "w",      119,    CMMI10,   VARIABLE, NULL },
    { "x",      120,    CMMI10,   VARIABLE, NULL },
    { "y",      121,    CMMI10,   VARIABLE, NULL },
    { "z",      122,    CMMI10,   VARIABLE, NULL },
    /* --- miscellaneous symbols and relations --- */
    { "\\imath",    123,    CMMI10,   VARIABLE, NULL },
    { "\\jmath",    124,    CMMI10,   VARIABLE, NULL },
    { "\\wp",       125,    CMMI10,   ORDINARY, NULL },
    { "\\vec",      126,    CMMI10,   ORDINARY, NULL },
    { NULL,     -999,   -999,   -999,       NULL }
};

static mathchardef symbols_cmmib10[] = {
    /* --------------------- C M M I B ------------------------
          symbol     charnum    family    class     function
    -------------------------------------------------------- */
    /* --- uppercase greek letters --- */
    { "\\Gamma",    0,  CMMIB10,  VARIABLE, NULL },
    { "\\Delta",    1,  CMMIB10,  VARIABLE, NULL },
    { "\\Theta",    2,  CMMIB10,  VARIABLE, NULL },
    { "\\Lambda",   3,  CMMIB10,  VARIABLE, NULL },
    { "\\Xi",       4,  CMMIB10,  VARIABLE, NULL },
    { "\\Pi",       5,  CMMIB10,  VARIABLE, NULL },
    { "\\Sigma",    6,  CMMIB10,  VARIABLE, NULL },
    { "\\smallsum", 6,  CMMIB10,  OPERATOR, NULL },
    { "\\Upsilon",  7,  CMMIB10,  VARIABLE, NULL },
    { "\\Phi",      8,  CMMIB10,  VARIABLE, NULL },
    { "\\Psi",      9,  CMMIB10,  VARIABLE, NULL },
    { "\\Omega",    10, CMMIB10,  VARIABLE, NULL },
    /* --- lowercase greek letters --- */
    { "\\alpha",    11, CMMIB10,  VARIABLE, NULL },
    { "\\beta",     12, CMMIB10,  VARIABLE, NULL },
    { "\\gamma",    13, CMMIB10,  VARIABLE, NULL },
    { "\\delta",    14, CMMIB10,  VARIABLE, NULL },
    { "\\epsilon",  15, CMMIB10,  VARIABLE, NULL },
    { "\\zeta",     16, CMMIB10,  VARIABLE, NULL },
    { "\\eta",      17, CMMIB10,  VARIABLE, NULL },
    { "\\theta",    18, CMMIB10,  VARIABLE, NULL },
    { "\\iota",     19, CMMIB10,  VARIABLE, NULL },
    { "\\kappa",    20, CMMIB10,  VARIABLE, NULL },
    { "\\lambda",   21, CMMIB10,  VARIABLE, NULL },
    { "\\mu",       22, CMMIB10,  VARIABLE, NULL },
    { "\\nu",       23, CMMIB10,  VARIABLE, NULL },
    { "\\xi",       24, CMMIB10,  VARIABLE, NULL },
    { "\\pi",       25, CMMIB10,  VARIABLE, NULL },
    { "\\rho",      26, CMMIB10,  VARIABLE, NULL },
    { "\\sigma",    27, CMMIB10,  VARIABLE, NULL },
    { "\\tau",      28, CMMIB10,  VARIABLE, NULL },
    { "\\upsilon",  29, CMMIB10,  VARIABLE, NULL },
    { "\\phi",      30, CMMIB10,  VARIABLE, NULL },
    { "\\chi",      31, CMMIB10,  VARIABLE, NULL },
    { "\\psi",      32, CMMIB10,  VARIABLE, NULL },
    { "\\omega",    33, CMMIB10,  VARIABLE, NULL },
    { "\\varepsilon",   34, CMMIB10,  VARIABLE, NULL },
    { "\\vartheta", 35, CMMIB10,  VARIABLE, NULL },
    { "\\varpi",    36, CMMIB10,  VARIABLE, NULL },
    { "\\varrho",   37, CMMIB10,  VARIABLE, NULL },
    { "\\varsigma", 38, CMMIB10,  VARIABLE, NULL },
    { "\\varphi",   39, CMMIB10,  VARIABLE, NULL },
    /* --- arrow relations --- */
    { "\\bfleftharpoonup",  40, CMMIB10,  ARROW,    NULL },
    { "\\bfleftharpoondown", 41, CMMIB10,  ARROW,    NULL },
    { "\\bfrightharpoonup", 42, CMMIB10,  ARROW,    NULL },
    { "\\bfrightharpoondown", 43, CMMIB10,  ARROW,    NULL },
    /* --- punctuation --- */
    { "`",      44, CMMIB10,  PUNCTION, NULL },
    { "\'",     45, CMMIB10,  PUNCTION, NULL },
    /* --- triangle binary relations --- */
    { "\\triangleright",    46, CMMIB10,  RELATION, NULL },
    { "\\triangleleft",     47, CMMIB10,  RELATION, NULL },
    /* --- digits 0-9 --- */
    { "\\0",        48, CMMIB10,  ORDINARY, NULL },
    { "\\1",        49, CMMIB10,  ORDINARY, NULL },
    { "\\2",        50, CMMIB10,  ORDINARY, NULL },
    { "\\3",        51, CMMIB10,  ORDINARY, NULL },
    { "\\4",        52, CMMIB10,  ORDINARY, NULL },
    { "\\5",        53, CMMIB10,  ORDINARY, NULL },
    { "\\6",        54, CMMIB10,  ORDINARY, NULL },
    { "\\7",        55, CMMIB10,  ORDINARY, NULL },
    { "\\8",        56, CMMIB10,  ORDINARY, NULL },
    { "\\9",        57, CMMIB10,  ORDINARY, NULL },
    /* --- punctuation --- */
    { ".",      58, CMMIB10,  PUNCTION, NULL },
    { ",",      59, CMMIB10,  PUNCTION, NULL },
    /* --- operations (some ordinary) --- */
    { "<",      60, CMMIB10,  OPENING,  NULL },
    { "\\lt",       60, CMMIB10,  OPENING,  NULL },
    { "/",      61, CMMIB10,  BINARYOP, NULL },
    { ">",      62, CMMIB10,  CLOSING,  NULL },
    { "\\gt",       62, CMMIB10,  CLOSING,  NULL },
    { "\\star",     63, CMMIB10,  BINARYOP, NULL },
    { "\\partial",  64, CMMIB10,  VARIABLE, NULL },
    /* --- uppercase letters --- */
    { "A",      65, CMMIB10,  VARIABLE, NULL },
    { "B",      66, CMMIB10,  VARIABLE, NULL },
    { "C",      67, CMMIB10,  VARIABLE, NULL },
    { "D",      68, CMMIB10,  VARIABLE, NULL },
    { "E",      69, CMMIB10,  VARIABLE, NULL },
    { "F",      70, CMMIB10,  VARIABLE, NULL },
    { "G",      71, CMMIB10,  VARIABLE, NULL },
    { "H",      72, CMMIB10,  VARIABLE, NULL },
    { "I",      73, CMMIB10,  VARIABLE, NULL },
    { "J",      74, CMMIB10,  VARIABLE, NULL },
    { "K",      75, CMMIB10,  VARIABLE, NULL },
    { "L",      76, CMMIB10,  VARIABLE, NULL },
    { "M",      77, CMMIB10,  VARIABLE, NULL },
    { "N",      78, CMMIB10,  VARIABLE, NULL },
    { "O",      79, CMMIB10,  VARIABLE, NULL },
    { "P",      80, CMMIB10,  VARIABLE, NULL },
    { "Q",      81, CMMIB10,  VARIABLE, NULL },
    { "R",      82, CMMIB10,  VARIABLE, NULL },
    { "S",      83, CMMIB10,  VARIABLE, NULL },
    { "T",      84, CMMIB10,  VARIABLE, NULL },
    { "U",      85, CMMIB10,  VARIABLE, NULL },
    { "V",      86, CMMIB10,  VARIABLE, NULL },
    { "W",      87, CMMIB10,  VARIABLE, NULL },
    { "X",      88, CMMIB10,  VARIABLE, NULL },
    { "Y",      89, CMMIB10,  VARIABLE, NULL },
    { "Z",      90, CMMIB10,  VARIABLE, NULL },
    /* --- miscellaneous symbols and relations --- */
    { "\\flat",     91, CMMIB10,  ORDINARY, NULL },
    { "\\natural",  92, CMMIB10,  ORDINARY, NULL },
    { "\\sharp",    93, CMMIB10,  ORDINARY, NULL },
    { "\\smile",    94, CMMIB10,  RELATION, NULL },
    { "\\frown",    95, CMMIB10,  RELATION, NULL },
    { "\\ell",      96, CMMIB10,  ORDINARY, NULL },
    /* --- lowercase letters --- */
    { "a",      97, CMMIB10,  VARIABLE, NULL },
    { "b",      98, CMMIB10,  VARIABLE, NULL },
    { "c",      99, CMMIB10,  VARIABLE, NULL },
    { "d",      100,    CMMIB10,  VARIABLE, NULL },
    { "e",      101,    CMMIB10,  VARIABLE, NULL },
    { "f",      102,    CMMIB10,  VARIABLE, NULL },
    { "g",      103,    CMMIB10,  VARIABLE, NULL },
    { "h",      104,    CMMIB10,  VARIABLE, NULL },
    { "i",      105,    CMMIB10,  VARIABLE, NULL },
    { "j",      106,    CMMIB10,  VARIABLE, NULL },
    { "k",      107,    CMMIB10,  VARIABLE, NULL },
    { "l",      108,    CMMIB10,  VARIABLE, NULL },
    { "m",      109,    CMMIB10,  VARIABLE, NULL },
    { "n",      110,    CMMIB10,  VARIABLE, NULL },
    { "o",      111,    CMMIB10,  VARIABLE, NULL },
    { "p",      112,    CMMIB10,  VARIABLE, NULL },
    { "q",      113,    CMMIB10,  VARIABLE, NULL },
    { "r",      114,    CMMIB10,  VARIABLE, NULL },
    { "s",      115,    CMMIB10,  VARIABLE, NULL },
    { "t",      116,    CMMIB10,  VARIABLE, NULL },
    { "u",      117,    CMMIB10,  VARIABLE, NULL },
    { "v",      118,    CMMIB10,  VARIABLE, NULL },
    { "w",      119,    CMMIB10,  VARIABLE, NULL },
    { "x",      120,    CMMIB10,  VARIABLE, NULL },
    { "y",      121,    CMMIB10,  VARIABLE, NULL },
    { "z",      122,    CMMIB10,  VARIABLE, NULL },
    /* --- miscellaneous symbols and relations --- */
    { "\\imath",    123,    CMMIB10,  VARIABLE, NULL },
    { "\\jmath",    124,    CMMIB10,  VARIABLE, NULL },
    { "\\wp",       125,    CMMIB10,  ORDINARY, NULL },
    { "\\bfvec",    126,    CMMIB10,  ORDINARY, NULL },
    { NULL,     -999,   -999,   -999,       NULL }
};

static mathchardef symbols_cmsy10[] = {
    /* --------------------- C M S Y --------------------------
          symbol     charnum    family    class     function
    -------------------------------------------------------- */
    /* --- operations --- */
    { "-",      0,  CMSY10,   BINARYOP, NULL },
    { "\\cdot",     1,  CMSY10,   BINARYOP, NULL },
    { "\\times",    2,  CMSY10,   BINARYOP, NULL },
    { "\\ast",      3,  CMSY10,   BINARYOP, NULL },
    { "\\div",      4,  CMSY10,   BINARYOP, NULL },
    { "\\diamond",  5,  CMSY10,   BINARYOP, NULL },
    { "\\pm",       6,  CMSY10,   BINARYOP, NULL },
    { "\\mp",       7,  CMSY10,   BINARYOP, NULL },
    { "\\oplus",    8,  CMSY10,   BINARYOP, NULL },
    { "\\ominus",   9,  CMSY10,   BINARYOP, NULL },
    { "\\otimes",   10, CMSY10,   BINARYOP, NULL },
    { "\\oslash",   11, CMSY10,   BINARYOP, NULL },
    { "\\odot",     12, CMSY10,   BINARYOP, NULL },
    { "\\bigcirc",  13, CMSY10,   BINARYOP, NULL },
    { "\\circ",     14, CMSY10,   BINARYOP, NULL },
    { "\\bullet",   15, CMSY10,   BINARYOP, NULL },
    /* --- relations --- */
    { "\\asymp",    16, CMSY10,   RELATION, NULL },
    { "\\equiv",    17, CMSY10,   RELATION, NULL },
    { "\\subseteq", 18, CMSY10,   RELATION, NULL },
    { "\\supseteq", 19, CMSY10,   RELATION, NULL },
    { "\\leq",      20, CMSY10,   RELATION, NULL },
    { "\\geq",      21, CMSY10,   RELATION, NULL },
    { "\\preceq",   22, CMSY10,   RELATION, NULL },
    { "\\succeq",   23, CMSY10,   RELATION, NULL },
    { "\\sim",      24, CMSY10,   RELATION, NULL },
    { "\\approx",   25, CMSY10,   RELATION, NULL },
    { "\\subset",   26, CMSY10,   RELATION, NULL },
    { "\\supset",   27, CMSY10,   RELATION, NULL },
    { "\\ll",       28, CMSY10,   RELATION, NULL },
    { "\\gg",       29, CMSY10,   RELATION, NULL },
    { "\\prec",     30, CMSY10,   RELATION, NULL },
    { "\\succ",     31, CMSY10,   RELATION, NULL },
    /* --- (mostly) arrows --- */
    { "\\leftarrow",    32, CMSY10,   ARROW,    NULL },
    { "\\rightarrow",   33, CMSY10,   ARROW,    NULL },
    { "\\to",       33, CMSY10,   ARROW,    NULL },
    { "\\mapsto",   33, CMSY10,   ARROW,    NULL },
    { "\\uparrow",  34, CMSY10,   ARROW,    NULL },
    { "\\downarrow",    35, CMSY10,   ARROW,    NULL },
    { "\\leftrightarrow",   36, CMSY10,   ARROW,    NULL },
    { "\\nearrow",  37, CMSY10,   ARROW,    NULL },
    { "\\searrow",  38, CMSY10,   ARROW,    NULL },
    { "\\simeq",    39, CMSY10,   RELATION, NULL },
    { "\\Leftarrow",    40, CMSY10,   ARROW,    NULL },
    { "\\Rightarrow",   41, CMSY10,   ARROW,    NULL },
    { "\\Uparrow",  42, CMSY10,   ARROW,    NULL },
    { "\\Downarrow",    43, CMSY10,   ARROW,    NULL },
    { "\\Leftrightarrow",   44, CMSY10,   ARROW,    NULL },
    { "\\nwarrow",  45, CMSY10,   ARROW,    NULL },
    { "\\swarrow",  46, CMSY10,   ARROW,    NULL },
    { "\\propto",   47, CMSY10,   RELATION, NULL },
    /* --- symbols --- */
    { "\\prime",    48, CMSY10,   ORDINARY, NULL },
    { "\\infty",    49, CMSY10,   ORDINARY, NULL },
    /* --- relations --- */
    { "\\in",       50, CMSY10,   RELATION, NULL },
    { "\\ni",       51, CMSY10,   RELATION, NULL },
    /* --- symbols --- */
    { "\\triangle",     52, CMSY10,   ORDINARY, NULL },
    { "\\bigtriangleup",    52, CMSY10,   ORDINARY, NULL },
    { "\\bigtriangledown",  53, CMSY10,   ORDINARY, NULL },
    { "\\boldslash",    54, CMSY10,   BINARYOP, NULL },
    { "\\'",        55, CMSY10,   ORDINARY, NULL },
    { "\\forall",   56, CMSY10,   OPERATOR, NULL },
    { "\\exists",   57, CMSY10,   OPERATOR, NULL },
    { "\\neg",      58, CMSY10,   OPERATOR, NULL },
    { "\\emptyset", 59, CMSY10,   ORDINARY, NULL },
    { "\\Re",       60, CMSY10,   ORDINARY, NULL },
    { "\\Im",       61, CMSY10,   ORDINARY, NULL },
    { "\\top",      62, CMSY10,   ORDINARY, NULL },
    { "\\bot",      63, CMSY10,   ORDINARY, NULL },
    { "\\perp",     63, CMSY10,   BINARYOP, NULL },
    { "\\aleph",    64, CMSY10,   ORDINARY, NULL },
    /* --- calligraphic letters (we use \\calA...\\calZ --- */
    { "\\calA",     65, CMSY10,   VARIABLE, NULL },
    { "\\calB",     66, CMSY10,   VARIABLE, NULL },
    { "\\calC",     67, CMSY10,   VARIABLE, NULL },
    { "\\calD",     68, CMSY10,   VARIABLE, NULL },
    { "\\calE",     69, CMSY10,   VARIABLE, NULL },
    { "\\calF",     70, CMSY10,   VARIABLE, NULL },
    { "\\calG",     71, CMSY10,   VARIABLE, NULL },
    { "\\calH",     72, CMSY10,   VARIABLE, NULL },
    { "\\calI",     73, CMSY10,   VARIABLE, NULL },
    { "\\calJ",     74, CMSY10,   VARIABLE, NULL },
    { "\\calK",     75, CMSY10,   VARIABLE, NULL },
    { "\\calL",     76, CMSY10,   VARIABLE, NULL },
    { "\\calM",     77, CMSY10,   VARIABLE, NULL },
    { "\\calN",     78, CMSY10,   VARIABLE, NULL },
    { "\\calO",     79, CMSY10,   VARIABLE, NULL },
    { "\\calP",     80, CMSY10,   VARIABLE, NULL },
    { "\\calQ",     81, CMSY10,   VARIABLE, NULL },
    { "\\calR",     82, CMSY10,   VARIABLE, NULL },
    { "\\calS",     83, CMSY10,   VARIABLE, NULL },
    { "\\calT",     84, CMSY10,   VARIABLE, NULL },
    { "\\calU",     85, CMSY10,   VARIABLE, NULL },
    { "\\calV",     86, CMSY10,   VARIABLE, NULL },
    { "\\calW",     87, CMSY10,   VARIABLE, NULL },
    { "\\calX",     88, CMSY10,   VARIABLE, NULL },
    { "\\calY",     89, CMSY10,   VARIABLE, NULL },
    { "\\calZ",     90, CMSY10,   VARIABLE, NULL },
    { "A",      65, CMSY10,   VARIABLE, NULL },
    { "B",      66, CMSY10,   VARIABLE, NULL },
    { "C",      67, CMSY10,   VARIABLE, NULL },
    { "D",      68, CMSY10,   VARIABLE, NULL },
    { "E",      69, CMSY10,   VARIABLE, NULL },
    { "F",      70, CMSY10,   VARIABLE, NULL },
    { "G",      71, CMSY10,   VARIABLE, NULL },
    { "H",      72, CMSY10,   VARIABLE, NULL },
    { "I",      73, CMSY10,   VARIABLE, NULL },
    { "J",      74, CMSY10,   VARIABLE, NULL },
    { "K",      75, CMSY10,   VARIABLE, NULL },
    { "L",      76, CMSY10,   VARIABLE, NULL },
    { "M",      77, CMSY10,   VARIABLE, NULL },
    { "N",      78, CMSY10,   VARIABLE, NULL },
    { "O",      79, CMSY10,   VARIABLE, NULL },
    { "P",      80, CMSY10,   VARIABLE, NULL },
    { "Q",      81, CMSY10,   VARIABLE, NULL },
    { "R",      82, CMSY10,   VARIABLE, NULL },
    { "S",      83, CMSY10,   VARIABLE, NULL },
    { "T",      84, CMSY10,   VARIABLE, NULL },
    { "U",      85, CMSY10,   VARIABLE, NULL },
    { "V",      86, CMSY10,   VARIABLE, NULL },
    { "W",      87, CMSY10,   VARIABLE, NULL },
    { "X",      88, CMSY10,   VARIABLE, NULL },
    { "Y",      89, CMSY10,   VARIABLE, NULL },
    { "Z",      90, CMSY10,   VARIABLE, NULL },
    /* --- operations and relations --- */
    { "\\cup",      91, CMSY10,   OPERATOR, NULL },
    { "\\cap",      92, CMSY10,   OPERATOR, NULL },
    { "\\uplus",    93, CMSY10,   OPERATOR, NULL },
    { "\\wedge",    94, CMSY10,   OPERATOR, NULL },
    { "\\vee",      95, CMSY10,   OPERATOR, NULL },
    { "\\vdash",    96, CMSY10,   RELATION, NULL },
    { "\\dashv",    97, CMSY10,   RELATION, NULL },
    /* --- brackets --- */
    { "\\lfloor",   98, CMSY10,   OPENING,  NULL },
    { "\\rfloor",   99, CMSY10,   CLOSING,  NULL },
    { "\\lceil",    100,    CMSY10,   OPENING,  NULL },
    { "\\rceil",    101,    CMSY10,   CLOSING,  NULL },
    { "\\lbrace",   102,    CMSY10,   OPENING,  NULL },
    { "{",      102,    CMSY10,   OPENING,  NULL },
    { "\\{",        102,    CMSY10,   OPENING,  NULL },
    { "\\rbrace",   103,    CMSY10,   CLOSING,  NULL },
    { "}",      103,    CMSY10,   CLOSING,  NULL },
    { "\\}",        103,    CMSY10,   CLOSING,  NULL },
    { "\\langle",   104,    CMSY10,   OPENING,  NULL },
    { "\\rangle",   105,    CMSY10,   CLOSING,  NULL },
    { "\\mid",      106,    CMSY10,   ORDINARY, NULL },
    { "|",      106,    CMSY10,   BINARYOP, NULL },
    { "\\parallel", 107,    CMSY10,   BINARYOP, NULL },
    { "\\|",        107,    CMSY10,   BINARYOP, NULL },
    /* --- arrows --- */
    { "\\updownarrow",  108,    CMSY10,   ARROW,    NULL },
    { "\\Updownarrow",  109,    CMSY10,   ARROW,    NULL },
    /* --- symbols and operations and relations --- */
    { "\\setminus", 110,    CMSY10,   BINARYOP, NULL },
    { "\\backslash",    110,    CMSY10,   BINARYOP, NULL },
    { "\\wr",       111,    CMSY10,   BINARYOP, NULL },
    { "\\surd",     112,    CMSY10,   OPERATOR, NULL },
    { "\\amalg",    113,    CMSY10,   BINARYOP, NULL },
    { "\\nabla",    114,    CMSY10,   VARIABLE, NULL },
    { "\\smallint", 115,    CMSY10,   OPERATOR, NULL },
    { "\\sqcup",    116,    CMSY10,   OPERATOR, NULL },
    { "\\sqcap",    117,    CMSY10,   OPERATOR, NULL },
    { "\\sqsubseteq",   118,    CMSY10,   RELATION, NULL },
    { "\\sqsupseteq",   119,    CMSY10,   RELATION, NULL },
    /* --- special characters --- */
    { "\\S",        120,    CMSY10,   ORDINARY, NULL },
    { "\\dag",      121,    CMSY10,   ORDINARY, NULL },
    { "\\dagger",   121,    CMSY10,   ORDINARY, NULL },
    { "\\ddag",     122,    CMSY10,   ORDINARY, NULL },
    { "\\ddagger",  122,    CMSY10,   ORDINARY, NULL },
    { "\\P",        123,    CMSY10,   ORDINARY, NULL },
    { "\\clubsuit", 124,    CMSY10,   ORDINARY, NULL },
    { "\\Diamond",  125,    CMSY10,   ORDINARY, NULL },
    { "\\Heart",    126,    CMSY10,   ORDINARY, NULL },
    { "\\spadesuit",    127,    CMSY10,   ORDINARY, NULL },
    { NULL,     -999,   -999,   -999,       NULL }
};

static mathchardef symbols_cmr10[] = {
    /* ---------------------- C M R ---------------------------
          symbol     charnum    family    class     function
    -------------------------------------------------------- */
    /* --- uppercase greek letters --- */
    { "\\Gamma",    0,  CMR10,   VARIABLE,  NULL },
    { "\\Delta",    1,  CMR10,   VARIABLE,  NULL },
    { "\\Theta",    2,  CMR10,   VARIABLE,  NULL },
    { "\\Lambda",   3,  CMR10,   VARIABLE,  NULL },
    { "\\Xi",       4,  CMR10,   VARIABLE,  NULL },
    { "\\Pi",       5,  CMR10,   VARIABLE,  NULL },
    { "\\Sigma",    6,  CMR10,   VARIABLE,  NULL },
    { "\\smallsum", 6,  CMR10,   OPERATOR,  NULL },
    { "\\Upsilon",  7,  CMR10,   VARIABLE,  NULL },
    { "\\Phi",      8,  CMR10,   VARIABLE,  NULL },
    { "\\Psi",      9,  CMR10,   VARIABLE,  NULL },
    { "\\Omega",    10, CMR10,   VARIABLE,  NULL },
    /* ---  --- */
    { "\\ff",       11, CMR10,   ORDINARY,  NULL },
    { "\\fi",       12, CMR10,   ORDINARY,  NULL },
    { "\\fl",       13, CMR10,   ORDINARY,  NULL },
    { "\\ffi",      14, CMR10,   ORDINARY,  NULL },
    { "\\ffl",      15, CMR10,   ORDINARY,  NULL },
    { "\\imath",    16, CMR10,   ORDINARY,  NULL },
    { "\\jmath",    17, CMR10,   ORDINARY,  NULL },
    /* --- foreign letters --- */
    { "\\ss",       25, CMR10,   ORDINARY,  NULL },
    { "\\ae",       26, CMR10,   ORDINARY,  NULL },
    { "\\oe",       27, CMR10,   ORDINARY,  NULL },
    { "\\AE",       29, CMR10,   ORDINARY,  NULL },
    { "\\OE",       30, CMR10,   ORDINARY,  NULL },
    /* --- digits 0-9 --- */
    { "0",      48, CMR10,   ORDINARY,  NULL },
    { "1",      49, CMR10,   ORDINARY,  NULL },
    { "2",      50, CMR10,   ORDINARY,  NULL },
    { "3",      51, CMR10,   ORDINARY,  NULL },
    { "4",      52, CMR10,   ORDINARY,  NULL },
    { "5",      53, CMR10,   ORDINARY,  NULL },
    { "6",      54, CMR10,   ORDINARY,  NULL },
    { "7",      55, CMR10,   ORDINARY,  NULL },
    { "8",      56, CMR10,   ORDINARY,  NULL },
    { "9",      57, CMR10,   ORDINARY,  NULL },
    /* --- symbols, relations, etc --- */
    { "\\gravesym", 18, CMR10,   ORDINARY,  NULL },
    { "\\acutesym", 19, CMR10,   ORDINARY,  NULL },
    { "\\checksym", 20, CMR10,   ORDINARY,  NULL },
    { "\\brevesym", 21, CMR10,   ORDINARY,  NULL },
    { "!",      33, CMR10,   BINARYOP,  NULL },
    { "\"",     34, CMR10,   ORDINARY,  NULL },
    { "\\quote",    34, CMR10,   ORDINARY,  NULL },
    { "#",      35, CMR10,   BINARYOP,  NULL },
    { "\\#",        35, CMR10,   BINARYOP,  NULL },
    { "$",      36, CMR10,   BINARYOP,  NULL },
    { "\\$",        36, CMR10,   BINARYOP,  NULL },
    { "%",      37, CMR10,   BINARYOP,  NULL },
    { "\\%",        37, CMR10,   BINARYOP,  NULL },
    { "\\percent",  37, CMR10,   BINARYOP,  NULL },
    { "&",      38, CMR10,   BINARYOP,  NULL },
    { "\\&",        38, CMR10,   BINARYOP,  NULL },
    { "\'",     39, CMR10,   BINARYOP,  NULL },
    { "\\\'",       39, CMR10,   BINARYOP,  NULL },
    { "\\apostrophe",   39, CMR10,   ORDINARY,  NULL },
    { "(",      40, CMR10,   OPENING,   NULL },
    { "\\(",        40, CMR10,   OPENING,   NULL },
    { ")",      41, CMR10,   CLOSING,   NULL },
    { "\\)",        41, CMR10,   CLOSING,   NULL },
    { "*",      42, CMR10,   BINARYOP,  NULL },
    { "+",      43, CMR10,   BINARYOP,  NULL },
    { "/",      47, CMR10,   BINARYOP,  NULL },
    { ":",      58, CMR10,   ORDINARY,  NULL },
    { ";",      59, CMR10,   ORDINARY,  NULL },
    { "=",      61, CMR10,   RELATION,  NULL },
    { "?",      63, CMR10,   BINARYOP,  NULL },
    { "@",      64, CMR10,   BINARYOP,  NULL },
    { "[",      91, CMR10,   OPENING,   NULL },
    { "\\[",        91, CMR10,   OPENING,   NULL },
    { "]",      93, CMR10,   CLOSING,   NULL },
    { "\\]",        93, CMR10,   CLOSING,   NULL },
    { "\\^",        94, CMR10,   BINARYOP,  NULL },
    { "\\~",        126,    CMR10,   OPERATOR,  NULL },
    /* --- uppercase letters --- */
    { "A",      65, CMR10,   VARIABLE,  NULL },
    { "B",      66, CMR10,   VARIABLE,  NULL },
    { "C",      67, CMR10,   VARIABLE,  NULL },
    { "D",      68, CMR10,   VARIABLE,  NULL },
    { "E",      69, CMR10,   VARIABLE,  NULL },
    { "F",      70, CMR10,   VARIABLE,  NULL },
    { "G",      71, CMR10,   VARIABLE,  NULL },
    { "H",      72, CMR10,   VARIABLE,  NULL },
    { "I",      73, CMR10,   VARIABLE,  NULL },
    { "J",      74, CMR10,   VARIABLE,  NULL },
    { "K",      75, CMR10,   VARIABLE,  NULL },
    { "L",      76, CMR10,   VARIABLE,  NULL },
    { "M",      77, CMR10,   VARIABLE,  NULL },
    { "N",      78, CMR10,   VARIABLE,  NULL },
    { "O",      79, CMR10,   VARIABLE,  NULL },
    { "P",      80, CMR10,   VARIABLE,  NULL },
    { "Q",      81, CMR10,   VARIABLE,  NULL },
    { "R",      82, CMR10,   VARIABLE,  NULL },
    { "S",      83, CMR10,   VARIABLE,  NULL },
    { "T",      84, CMR10,   VARIABLE,  NULL },
    { "U",      85, CMR10,   VARIABLE,  NULL },
    { "V",      86, CMR10,   VARIABLE,  NULL },
    { "W",      87, CMR10,   VARIABLE,  NULL },
    { "X",      88, CMR10,   VARIABLE,  NULL },
    { "Y",      89, CMR10,   VARIABLE,  NULL },
    { "Z",      90, CMR10,   VARIABLE,  NULL },
    /* --- lowercase letters --- */
    { "a",      97, CMR10,   VARIABLE,  NULL },
    { "b",      98, CMR10,   VARIABLE,  NULL },
    { "c",      99, CMR10,   VARIABLE,  NULL },
    { "d",      100,    CMR10,   VARIABLE,  NULL },
    { "e",      101,    CMR10,   VARIABLE,  NULL },
    { "f",      102,    CMR10,   VARIABLE,  NULL },
    { "g",      103,    CMR10,   VARIABLE,  NULL },
    { "h",      104,    CMR10,   VARIABLE,  NULL },
    { "i",      105,    CMR10,   VARIABLE,  NULL },
    { "j",      106,    CMR10,   VARIABLE,  NULL },
    { "k",      107,    CMR10,   VARIABLE,  NULL },
    { "l",      108,    CMR10,   VARIABLE,  NULL },
    { "m",      109,    CMR10,   VARIABLE,  NULL },
    { "n",      110,    CMR10,   VARIABLE,  NULL },
    { "o",      111,    CMR10,   VARIABLE,  NULL },
    { "p",      112,    CMR10,   VARIABLE,  NULL },
    { "q",      113,    CMR10,   VARIABLE,  NULL },
    { "r",      114,    CMR10,   VARIABLE,  NULL },
    { "s",      115,    CMR10,   VARIABLE,  NULL },
    { "t",      116,    CMR10,   VARIABLE,  NULL },
    { "u",      117,    CMR10,   VARIABLE,  NULL },
    { "v",      118,    CMR10,   VARIABLE,  NULL },
    { "w",      119,    CMR10,   VARIABLE,  NULL },
    { "x",      120,    CMR10,   VARIABLE,  NULL },
    { "y",      121,    CMR10,   VARIABLE,  NULL },
    { "z",      122,    CMR10,   VARIABLE,  NULL },
    { NULL,     -999,   -999,   -999,       NULL }
};

static mathchardef symbols_cmex10[] = {
    /* --------------------- C M E X --------------------------
          symbol     charnum    family    class     function
    -------------------------------------------------------- */
    /* --- parens ()'s --- */
    { "\\big(",     0,  CMEX10,   OPENING,  NULL },
    { "\\big)",     1,  CMEX10,   CLOSING,  NULL },
    { "\\Big(",     16, CMEX10,   OPENING,  NULL },
    { "\\Big)",     17, CMEX10,   CLOSING,  NULL },
    { "\\bigg(",    18, CMEX10,   OPENING,  NULL },
    { "\\bigg)",    19, CMEX10,   CLOSING,  NULL },
    { "\\Bigg(",    32, CMEX10,   OPENING,  NULL },
    { "\\Bigg)",    33, CMEX10,   CLOSING,  NULL },
    { "\\bigl(",    0,  CMEX10,   OPENING,  NULL },
    { "\\bigr)",    1,  CMEX10,   CLOSING,  NULL },
    { "\\Bigl(",    16, CMEX10,   OPENING,  NULL },
    { "\\Bigr)",    17, CMEX10,   CLOSING,  NULL },
    { "\\biggl(",   18, CMEX10,   OPENING,  NULL },
    { "\\biggr)",   19, CMEX10,   CLOSING,  NULL },
    { "\\Biggl(",   32, CMEX10,   OPENING,  NULL },
    { "\\Biggr)",   33, CMEX10,   CLOSING,  NULL },
    /* --- brackets []'s --- */
    { "\\big[",     2,  CMEX10,   OPENING,  NULL },
    { "\\big]",     3,  CMEX10,   CLOSING,  NULL },
    { "\\bigg[",    20, CMEX10,   OPENING,  NULL },
    { "\\bigg]",    21, CMEX10,   CLOSING,  NULL },
    { "\\Bigg[",    34, CMEX10,   OPENING,  NULL },
    { "\\Bigg]",    35, CMEX10,   CLOSING,  NULL },
    { "\\Big[",     104,    CMEX10,   OPENING,  NULL },
    { "\\Big]",     105,    CMEX10,   CLOSING,  NULL },
    { "\\bigl[",    2,  CMEX10,   OPENING,  NULL },
    { "\\bigr]",    3,  CMEX10,   CLOSING,  NULL },
    { "\\biggl[",   20, CMEX10,   OPENING,  NULL },
    { "\\biggr]",   21, CMEX10,   CLOSING,  NULL },
    { "\\Biggl[",   34, CMEX10,   OPENING,  NULL },
    { "\\Biggr]",   35, CMEX10,   CLOSING,  NULL },
    { "\\Bigl[",    104,    CMEX10,   OPENING,  NULL },
    { "\\Bigr]",    105,    CMEX10,   CLOSING,  NULL },
    /* --- braces {}'s --- */
    { "\\big{",     8,  CMEX10,   OPENING,  NULL },
    { "\\big}",     9,  CMEX10,   CLOSING,  NULL },
    { "\\bigg{",    26, CMEX10,   OPENING,  NULL },
    { "\\bigg}",    27, CMEX10,   CLOSING,  NULL },
    { "\\Bigg{",    40, CMEX10,   OPENING,  NULL },
    { "\\Bigg}",    41, CMEX10,   CLOSING,  NULL },
    { "\\Big{",     110,    CMEX10,   OPENING,  NULL },
    { "\\Big}",     111,    CMEX10,   CLOSING,  NULL },
    { "\\bigl{",    8,  CMEX10,   OPENING,  NULL },
    { "\\bigr}",    9,  CMEX10,   CLOSING,  NULL },
    { "\\biggl{",   26, CMEX10,   OPENING,  NULL },
    { "\\biggr}",   27, CMEX10,   CLOSING,  NULL },
    { "\\Biggl{",   40, CMEX10,   OPENING,  NULL },
    { "\\Biggr}",   41, CMEX10,   CLOSING,  NULL },
    { "\\Bigl{",    110,    CMEX10,   OPENING,  NULL },
    { "\\Bigr}",    111,    CMEX10,   CLOSING,  NULL },
    { "\\big\\{",   8,  CMEX10,   OPENING,  NULL },
    { "\\big\\}",   9,  CMEX10,   CLOSING,  NULL },
    { "\\bigg\\{",  26, CMEX10,   OPENING,  NULL },
    { "\\bigg\\}",  27, CMEX10,   CLOSING,  NULL },
    { "\\Bigg\\{",  40, CMEX10,   OPENING,  NULL },
    { "\\Bigg\\}",  41, CMEX10,   CLOSING,  NULL },
    { "\\Big\\{",   110,    CMEX10,   OPENING,  NULL },
    { "\\Big\\}",   111,    CMEX10,   CLOSING,  NULL },
    { "\\bigl\\{",  8,  CMEX10,   OPENING,  NULL },
    { "\\bigr\\}",  9,  CMEX10,   CLOSING,  NULL },
    { "\\biggl\\{", 26, CMEX10,   OPENING,  NULL },
    { "\\biggr\\}", 27, CMEX10,   CLOSING,  NULL },
    { "\\Biggl\\{", 40, CMEX10,   OPENING,  NULL },
    { "\\Biggr\\}", 41, CMEX10,   CLOSING,  NULL },
    { "\\Bigl\\{",  110,    CMEX10,   OPENING,  NULL },
    { "\\Bigr\\}",  111,    CMEX10,   CLOSING,  NULL },
    { "\\big\\lbrace",  8,  CMEX10,   OPENING,  NULL },
    { "\\big\\rbrace",  9,  CMEX10,   CLOSING,  NULL },
    { "\\bigg\\lbrace", 26, CMEX10,   OPENING,  NULL },
    { "\\bigg\\rbrace", 27, CMEX10,   CLOSING,  NULL },
    { "\\Bigg\\lbrace", 40, CMEX10,   OPENING,  NULL },
    { "\\Bigg\\rbrace", 41, CMEX10,   CLOSING,  NULL },
    { "\\Big\\lbrace",  110,    CMEX10,   OPENING,  NULL },
    { "\\Big\\rbrace",  111,    CMEX10,   CLOSING,  NULL },
    /* --- angles <>'s --- */
    { "\\big<",     10, CMEX10,   OPENING,  NULL },
    { "\\big>",     11, CMEX10,   CLOSING,  NULL },
    { "\\bigg<",    28, CMEX10,   OPENING,  NULL },
    { "\\bigg>",    29, CMEX10,   CLOSING,  NULL },
    { "\\Bigg<",    42, CMEX10,   OPENING,  NULL },
    { "\\Bigg>",    43, CMEX10,   CLOSING,  NULL },
    { "\\Big<",     68, CMEX10,   OPENING,  NULL },
    { "\\Big>",     69, CMEX10,   CLOSING,  NULL },
    { "\\bigl<",    10, CMEX10,   OPENING,  NULL },
    { "\\bigr>",    11, CMEX10,   CLOSING,  NULL },
    { "\\biggl<",   28, CMEX10,   OPENING,  NULL },
    { "\\biggr>",   29, CMEX10,   CLOSING,  NULL },
    { "\\Biggl<",   42, CMEX10,   OPENING,  NULL },
    { "\\Biggr>",   43, CMEX10,   CLOSING,  NULL },
    { "\\Bigl<",    68, CMEX10,   OPENING,  NULL },
    { "\\Bigr>",    69, CMEX10,   CLOSING,  NULL },
    { "\\big\\langle",  10, CMEX10,   OPENING,  NULL },
    { "\\big\\rangle",  11, CMEX10,   CLOSING,  NULL },
    { "\\bigg\\langle", 28, CMEX10,   OPENING,  NULL },
    { "\\bigg\\rangle", 29, CMEX10,   CLOSING,  NULL },
    { "\\Bigg\\langle", 42, CMEX10,   OPENING,  NULL },
    { "\\Bigg\\rangle", 43, CMEX10,   CLOSING,  NULL },
    { "\\Big\\langle",  68, CMEX10,   OPENING,  NULL },
    { "\\Big\\rangle",  69, CMEX10,   CLOSING,  NULL },
    /* --- hats ^ --- */
    { "^",      98, CMEX10,   OPERATOR, NULL },
    { "^",      99, CMEX10,   OPERATOR, NULL },
    { "^",      100,    CMEX10,   OPERATOR, NULL },
    /* --- tildes --- */
    { "~",      101,    CMEX10,   OPERATOR, NULL },
    { "~",      102,    CMEX10,   OPERATOR, NULL },
    { "~",      103,    CMEX10,   OPERATOR, NULL },
    /* --- /'s --- */
    { "/",      44, CMEX10,   OPENING,  NULL },
    { "/",      46, CMEX10,   OPENING,  NULL },
    { "\\",     45, CMEX10,   OPENING,  NULL },
    { "\\",     47, CMEX10,   OPENING,  NULL },
    /* --- \sum, \int and other (displaymath) symbols --- */
    { "\\bigsqcup", 70, CMEX10,   LOWERBIG, NULL },
    { "\\Bigsqcup", 71, CMEX10,   UPPERBIG, NULL },
    { "\\oint",     72, CMEX10,   OPERATOR, NULL },
    { "\\bigoint",  72, CMEX10,   LOWERBIG, NULL },
    { "\\Bigoint",  73, CMEX10,   UPPERBIG, NULL },
    { "\\bigodot",  74, CMEX10,   LOWERBIG, NULL },
    { "\\Bigodot",  75, CMEX10,   UPPERBIG, NULL },
    { "\\bigoplus", 76, CMEX10,   LOWERBIG, NULL },
    { "\\Bigoplus", 77, CMEX10,   UPPERBIG, NULL },
    { "\\bigotimes",    78, CMEX10,   LOWERBIG, NULL },
    { "\\Bigotimes",    79, CMEX10,   UPPERBIG, NULL },
    { "\\sum",      80, CMEX10,   OPERATOR, NULL },
    { "\\bigsum",   80, CMEX10,   LOWERBIG, NULL },
    { "\\prod",     81, CMEX10,   OPERATOR, NULL },
    { "\\bigprod",  81, CMEX10,   LOWERBIG, NULL },
    { "\\int",      82, CMEX10,   OPERATOR, NULL },
    { "\\bigint",   82, CMEX10,   LOWERBIG, NULL },
    { "\\bigcup",   83, CMEX10,   LOWERBIG, NULL },
    { "\\bigcap",   84, CMEX10,   LOWERBIG, NULL },
    { "\\biguplus", 85, CMEX10,   LOWERBIG, NULL },
    { "\\bigwedge", 86, CMEX10,   LOWERBIG, NULL },
    { "\\bigvee",   87, CMEX10,   LOWERBIG, NULL },
    { "\\Bigsum",   88, CMEX10,   UPPERBIG, NULL },
    { "\\big\\sum", 88, CMEX10,   UPPERBIG, NULL },
    { "\\Big\\sum", 88, CMEX10,   UPPERBIG, NULL },
    { "\\bigg\\sum",    88, CMEX10,   UPPERBIG, NULL },
    { "\\Bigg\\sum",    88, CMEX10,   UPPERBIG, NULL },
    { "\\Bigprod",  89, CMEX10,   UPPERBIG, NULL },
    { "\\Bigint",   90, CMEX10,   UPPERBIG, NULL },
    { "\\big\\int", 90, CMEX10,   UPPERBIG, NULL },
    { "\\Big\\int", 90, CMEX10,   UPPERBIG, NULL },
    { "\\bigg\\int",    90, CMEX10,   UPPERBIG, NULL },
    { "\\Bigg\\int",    90, CMEX10,   UPPERBIG, NULL },
    { "\\Bigcup",   91, CMEX10,   UPPERBIG, NULL },
    { "\\Bigcap",   92, CMEX10,   UPPERBIG, NULL },
    { "\\Biguplus", 93, CMEX10,   UPPERBIG, NULL },
    { "\\Bigwedge", 94, CMEX10,   UPPERBIG, NULL },
    { "\\Bigvee",   95, CMEX10,   UPPERBIG, NULL },
    { "\\coprod",   96, CMEX10,   LOWERBIG, NULL },
    { "\\bigcoprod",    96, CMEX10,   LOWERBIG, NULL },
    { "\\Bigcoprod",    97, CMEX10,   UPPERBIG, NULL },
    /* --- symbol pieces (see TeXbook page 432) --- */
    { "\\leftbracetop", 56, CMEX10,   OPENING,  NULL },
    { "\\rightbracetop", 57, CMEX10,   CLOSING,  NULL },
    { "\\leftbracebot", 58, CMEX10,   OPENING,  NULL },
    { "\\rightbracebot", 59, CMEX10,   CLOSING,  NULL },
    { "\\leftbracemid", 60, CMEX10,   OPENING,  NULL },
    { "\\rightbracemid", 61, CMEX10,   CLOSING,  NULL },
    { "\\leftbracebar", 62, CMEX10,   OPENING,  NULL },
    { "\\rightbracebar", 62, CMEX10,   CLOSING,  NULL },
    { "\\leftparentop", 48, CMEX10,   OPENING,  NULL },
    { "\\rightparentop", 49, CMEX10,   CLOSING,  NULL },
    { "\\leftparenbot", 64, CMEX10,   OPENING,  NULL },
    { "\\rightparenbot", 65, CMEX10,   CLOSING,  NULL },
    { "\\leftparenbar", 66, CMEX10,   OPENING,  NULL },
    { "\\rightparenbar", 67, CMEX10,   CLOSING,  NULL },
    { NULL,     -999,   -999,   -999,       NULL }
};

static mathchardef symbols_rsfs10[] = {
    /* --------------------- R S F S --------------------------
          symbol     charnum    family    class     function
    -------------------------------------------------------- */
    /* --- rsfs script letters (written as \scr{A...Z}) --- */
    { "A",       0, RSFS10,   VARIABLE, NULL },
    { "B",       1, RSFS10,   VARIABLE, NULL },
    { "C",       2, RSFS10,   VARIABLE, NULL },
    { "D",       3, RSFS10,   VARIABLE, NULL },
    { "E",       4, RSFS10,   VARIABLE, NULL },
    { "F",       5, RSFS10,   VARIABLE, NULL },
    { "G",       6, RSFS10,   VARIABLE, NULL },
    { "H",       7, RSFS10,   VARIABLE, NULL },
    { "I",       8, RSFS10,   VARIABLE, NULL },
    { "J",       9, RSFS10,   VARIABLE, NULL },
    { "K",      10, RSFS10,   VARIABLE, NULL },
    { "L",      11, RSFS10,   VARIABLE, NULL },
    { "M",      12, RSFS10,   VARIABLE, NULL },
    { "N",      13, RSFS10,   VARIABLE, NULL },
    { "O",      14, RSFS10,   VARIABLE, NULL },
    { "P",      15, RSFS10,   VARIABLE, NULL },
    { "Q",      16, RSFS10,   VARIABLE, NULL },
    { "R",      17, RSFS10,   VARIABLE, NULL },
    { "S",      18, RSFS10,   VARIABLE, NULL },
    { "T",      19, RSFS10,   VARIABLE, NULL },
    { "U",      20, RSFS10,   VARIABLE, NULL },
    { "V",      21, RSFS10,   VARIABLE, NULL },
    { "W",      22, RSFS10,   VARIABLE, NULL },
    { "X",      23, RSFS10,   VARIABLE, NULL },
    { "Y",      24, RSFS10,   VARIABLE, NULL },
    { "Z",      25, RSFS10,   VARIABLE, NULL },
    /* --- rsfs script letters (written as \scrA...\scrZ) --- */
    { "\\scrA",      0, RSFS10,   VARIABLE, NULL },
    { "\\scrB",      1, RSFS10,   VARIABLE, NULL },
    { "\\scrC",      2, RSFS10,   VARIABLE, NULL },
    { "\\scrD",      3, RSFS10,   VARIABLE, NULL },
    { "\\scrE",      4, RSFS10,   VARIABLE, NULL },
    { "\\scrF",      5, RSFS10,   VARIABLE, NULL },
    { "\\scrG",      6, RSFS10,   VARIABLE, NULL },
    { "\\scrH",      7, RSFS10,   VARIABLE, NULL },
    { "\\scrI",      8, RSFS10,   VARIABLE, NULL },
    { "\\scrJ",      9, RSFS10,   VARIABLE, NULL },
    { "\\scrK",     10, RSFS10,   VARIABLE, NULL },
    { "\\scrL",     11, RSFS10,   VARIABLE, NULL },
    { "\\scrM",     12, RSFS10,   VARIABLE, NULL },
    { "\\scrN",     13, RSFS10,   VARIABLE, NULL },
    { "\\scrO",     14, RSFS10,   VARIABLE, NULL },
    { "\\scrP",     15, RSFS10,   VARIABLE, NULL },
    { "\\scrQ",     16, RSFS10,   VARIABLE, NULL },
    { "\\scrR",     17, RSFS10,   VARIABLE, NULL },
    { "\\scrS",     18, RSFS10,   VARIABLE, NULL },
    { "\\scrT",     19, RSFS10,   VARIABLE, NULL },
    { "\\scrU",     20, RSFS10,   VARIABLE, NULL },
    { "\\scrV",     21, RSFS10,   VARIABLE, NULL },
    { "\\scrW",     22, RSFS10,   VARIABLE, NULL },
    { "\\scrX",     23, RSFS10,   VARIABLE, NULL },
    { "\\scrY",     24, RSFS10,   VARIABLE, NULL },
    { "\\scrZ",     25, RSFS10,   VARIABLE, NULL },
    { NULL,     -999,   -999,   -999,       NULL }
};

static mathchardef symbols_bbold10[] = {
    /* -------------------- B B O L D -------------------------
          symbol     charnum    family    class     function
    -------------------------------------------------------- */
    /* --- uppercase greek letters --- */
    { "\\Gamma",    0,     BBOLD10,   VARIABLE, NULL },
    { "\\Delta",    1,     BBOLD10,   VARIABLE, NULL },
    { "\\Theta",    2,     BBOLD10,   VARIABLE, NULL },
    { "\\Lambda",   3,     BBOLD10,   VARIABLE, NULL },
    { "\\Xi",       4,     BBOLD10,   VARIABLE, NULL },
    { "\\Pi",       5,     BBOLD10,   VARIABLE, NULL },
    { "\\Sigma",    6,     BBOLD10,   VARIABLE, NULL },
    { "\\smallsum", 6,     BBOLD10,   OPERATOR, NULL },
    { "\\Upsilon",  7,     BBOLD10,   VARIABLE, NULL },
    { "\\Phi",      8,     BBOLD10,   VARIABLE, NULL },
    { "\\Psi",      9,     BBOLD10,   VARIABLE, NULL },
    { "\\Omega",    10,    BBOLD10,   VARIABLE, NULL },
    /* --- lowercase greek letters --- */
    { "\\alpha",    11,    BBOLD10,   VARIABLE, NULL },
    { "\\beta",     12,    BBOLD10,   VARIABLE, NULL },
    { "\\gamma",    13,    BBOLD10,   VARIABLE, NULL },
    { "\\delta",    14,    BBOLD10,   VARIABLE, NULL },
    { "\\epsilon",  15,    BBOLD10,   VARIABLE, NULL },
    { "\\zeta",     16,    BBOLD10,   VARIABLE, NULL },
    { "\\eta",      17,    BBOLD10,   VARIABLE, NULL },
    { "\\theta",    18,    BBOLD10,   VARIABLE, NULL },
    { "\\iota",     19,    BBOLD10,   VARIABLE, NULL },
    { "\\kappa",    20,    BBOLD10,   VARIABLE, NULL },
    { "\\lambda",   21,    BBOLD10,   VARIABLE, NULL },
    { "\\mu",       22,    BBOLD10,   VARIABLE, NULL },
    { "\\nu",       23,    BBOLD10,   VARIABLE, NULL },
    { "\\xi",       24,    BBOLD10,   VARIABLE, NULL },
    { "\\pi",       25,    BBOLD10,   VARIABLE, NULL },
    { "\\rho",      26,    BBOLD10,   VARIABLE, NULL },
    { "\\sigma",    27,    BBOLD10,   VARIABLE, NULL },
    { "\\tau",      28,    BBOLD10,   VARIABLE, NULL },
    { "\\upsilon",  29,    BBOLD10,   VARIABLE, NULL },
    { "\\phi",      30,    BBOLD10,   VARIABLE, NULL },
    { "\\chi",      31,    BBOLD10,   VARIABLE, NULL },
    { "\\psi",      32,    BBOLD10,   VARIABLE, NULL },
    { "\\omega",    127,   BBOLD10,   VARIABLE, NULL },
    /* --- digits 0-9 --- */
    { "0",      48,    BBOLD10,   ORDINARY, NULL },
    { "1",      49,    BBOLD10,   ORDINARY, NULL },
    { "2",      50,    BBOLD10,   ORDINARY, NULL },
    { "3",      51,    BBOLD10,   ORDINARY, NULL },
    { "4",      52,    BBOLD10,   ORDINARY, NULL },
    { "5",      53,    BBOLD10,   ORDINARY, NULL },
    { "6",      54,    BBOLD10,   ORDINARY, NULL },
    { "7",      55,    BBOLD10,   ORDINARY, NULL },
    { "8",      56,    BBOLD10,   ORDINARY, NULL },
    { "9",      57,    BBOLD10,   ORDINARY, NULL },
    { "\\0",        48,    BBOLD10,   ORDINARY, NULL },
    { "\\1",        49,    BBOLD10,   ORDINARY, NULL },
    { "\\2",        50,    BBOLD10,   ORDINARY, NULL },
    { "\\3",        51,    BBOLD10,   ORDINARY, NULL },
    { "\\4",        52,    BBOLD10,   ORDINARY, NULL },
    { "\\5",        53,    BBOLD10,   ORDINARY, NULL },
    { "\\6",        54,    BBOLD10,   ORDINARY, NULL },
    { "\\7",        55,    BBOLD10,   ORDINARY, NULL },
    { "\\8",        56,    BBOLD10,   ORDINARY, NULL },
    { "\\9",        57,    BBOLD10,   ORDINARY, NULL },
    /* --- uppercase letters --- */
    { "A",      65,    BBOLD10,   VARIABLE, NULL },
    { "B",      66,    BBOLD10,   VARIABLE, NULL },
    { "C",      67,    BBOLD10,   VARIABLE, NULL },
    { "D",      68,    BBOLD10,   VARIABLE, NULL },
    { "E",      69,    BBOLD10,   VARIABLE, NULL },
    { "F",      70,    BBOLD10,   VARIABLE, NULL },
    { "G",      71,    BBOLD10,   VARIABLE, NULL },
    { "H",      72,    BBOLD10,   VARIABLE, NULL },
    { "I",      73,    BBOLD10,   VARIABLE, NULL },
    { "J",      74,    BBOLD10,   VARIABLE, NULL },
    { "K",      75,    BBOLD10,   VARIABLE, NULL },
    { "L",      76,    BBOLD10,   VARIABLE, NULL },
    { "M",      77,    BBOLD10,   VARIABLE, NULL },
    { "N",      78,    BBOLD10,   VARIABLE, NULL },
    { "O",      79,    BBOLD10,   VARIABLE, NULL },
    { "P",      80,    BBOLD10,   VARIABLE, NULL },
    { "Q",      81,    BBOLD10,   VARIABLE, NULL },
    { "R",      82,    BBOLD10,   VARIABLE, NULL },
    { "S",      83,    BBOLD10,   VARIABLE, NULL },
    { "T",      84,    BBOLD10,   VARIABLE, NULL },
    { "U",      85,    BBOLD10,   VARIABLE, NULL },
    { "V",      86,    BBOLD10,   VARIABLE, NULL },
    { "W",      87,    BBOLD10,   VARIABLE, NULL },
    { "X",      88,    BBOLD10,   VARIABLE, NULL },
    { "Y",      89,    BBOLD10,   VARIABLE, NULL },
    { "Z",      90,    BBOLD10,   VARIABLE, NULL },
    /* --- lowercase letters --- */
    { "a",      97,    BBOLD10,   VARIABLE, NULL },
    { "b",      98,    BBOLD10,   VARIABLE, NULL },
    { "c",      99,    BBOLD10,   VARIABLE, NULL },
    { "d",      100,   BBOLD10,   VARIABLE, NULL },
    { "e",      101,   BBOLD10,   VARIABLE, NULL },
    { "f",      102,   BBOLD10,   VARIABLE, NULL },
    { "g",      103,   BBOLD10,   VARIABLE, NULL },
    { "h",      104,   BBOLD10,   VARIABLE, NULL },
    { "i",      105,   BBOLD10,   VARIABLE, NULL },
    { "j",      106,   BBOLD10,   VARIABLE, NULL },
    { "k",      107,   BBOLD10,   VARIABLE, NULL },
    { "l",      108,   BBOLD10,   VARIABLE, NULL },
    { "m",      109,   BBOLD10,   VARIABLE, NULL },
    { "n",      110,   BBOLD10,   VARIABLE, NULL },
    { "o",      111,   BBOLD10,   VARIABLE, NULL },
    { "p",      112,   BBOLD10,   VARIABLE, NULL },
    { "q",      113,   BBOLD10,   VARIABLE, NULL },
    { "r",      114,   BBOLD10,   VARIABLE, NULL },
    { "s",      115,   BBOLD10,   VARIABLE, NULL },
    { "t",      116,   BBOLD10,   VARIABLE, NULL },
    { "u",      117,   BBOLD10,   VARIABLE, NULL },
    { "v",      118,   BBOLD10,   VARIABLE, NULL },
    { "w",      119,   BBOLD10,   VARIABLE, NULL },
    { "x",      120,   BBOLD10,   VARIABLE, NULL },
    { "y",      121,   BBOLD10,   VARIABLE, NULL },
    { "z",      122,   BBOLD10,   VARIABLE, NULL },
    /* --- symbols, relations, etc --- */
    { "!",      33,    BBOLD10,   BINARYOP, NULL },
    { "#",      35,    BBOLD10,   BINARYOP, NULL },
    { "\\#",        35,    BBOLD10,   BINARYOP, NULL },
    { "$",      36,    BBOLD10,   BINARYOP, NULL },
    { "\\$",        36,    BBOLD10,   BINARYOP, NULL },
    { "%",      37,    BBOLD10,   BINARYOP, NULL },
    { "\\%",        37,    BBOLD10,   BINARYOP, NULL },
    { "\\percent",  37,    BBOLD10,   BINARYOP, NULL },
    { "&",      38,    BBOLD10,   BINARYOP, NULL },
    { "\\&",        38,    BBOLD10,   BINARYOP, NULL },
    { "\'",     39,    BBOLD10,   BINARYOP, NULL },
    { "\\apostrophe",   39,    BBOLD10,   ORDINARY, NULL },
    { "(",      40,    BBOLD10,   OPENING,  NULL },
    { "\\(",        40,    BBOLD10,   OPENING,  NULL },
    { ")",      41,    BBOLD10,   CLOSING,  NULL },
    { "\\)",        41,    BBOLD10,   CLOSING,  NULL },
    { "*",      42,    BBOLD10,   BINARYOP, NULL },
    { "+",      43,    BBOLD10,   BINARYOP, NULL },
    { ",",      44,    BBOLD10,   PUNCTION, NULL },
    { "-",      45,    BBOLD10,   BINARYOP, NULL },
    { ".",      46,    BBOLD10,   PUNCTION, NULL },
    { "/",      47,    BBOLD10,   BINARYOP, NULL },
    { ":",      58,    BBOLD10,   ORDINARY, NULL },
    { ";",      59,    BBOLD10,   ORDINARY, NULL },
    { "<",      60,    BBOLD10,   RELATION, NULL },
    { "\\<",        60,    BBOLD10,   RELATION, NULL },
    { "\\cdot",     61,    BBOLD10,   BINARYOP, NULL },
    { ">",      62,    BBOLD10,   RELATION, NULL },
    { "\\>",        62,    BBOLD10,   RELATION, NULL },
    { "?",      63,    BBOLD10,   BINARYOP, NULL },
    { "@",      64,    BBOLD10,   BINARYOP, NULL },
    { "[",      91,    BBOLD10,   OPENING,  NULL },
    { "\\[",        91,    BBOLD10,   OPENING,  NULL },
    { "\\\\",       92,    BBOLD10,   OPENING,  NULL },
    { "\\backslash",    92,    BBOLD10,   OPENING,  NULL },
    { "]",      93,    BBOLD10,   CLOSING,  NULL },
    { "\\]",        93,    BBOLD10,   CLOSING,  NULL },
    { "|",      124,   BBOLD10,   BINARYOP, NULL },
    { "\\-",        123,   BBOLD10,   BINARYOP, NULL },
    { NULL,     -999,   -999,   -999,       NULL }
};

static mathchardef symbols_stmary10[] = {
    /* ------------------- S T M A R Y ------------------------
          symbol     charnum    family    class     function
    -------------------------------------------------------- */
    /* --- stmaryrd symbols (see stmaryrd.sty for defs) --- */
    { "\\shortleftarrow",   0, STMARY10,  ARROW,    NULL },
    { "\\shortrightarrow",  1, STMARY10,  ARROW,    NULL },
    { "\\shortuparrow",     2, STMARY10,  ARROW,    NULL },
    { "\\shortdownarrow",   3, STMARY10,  ARROW,    NULL },
    { "\\Yup",          4, STMARY10,  BINARYOP, NULL },
    { "\\Ydown",        5, STMARY10,  BINARYOP, NULL },
    { "\\Yleft",        6, STMARY10,  BINARYOP, NULL },
    { "\\Yright",       7, STMARY10,  BINARYOP, NULL },
    { "\\varcurlyvee",      8, STMARY10,  BINARYOP, NULL },
    { "\\varcurlywedge",    9, STMARY10,  BINARYOP, NULL },
    { "\\minuso",      10, STMARY10,  BINARYOP, NULL },
    { "\\baro",        11, STMARY10,  BINARYOP, NULL },
    { "\\sslash",      12, STMARY10,  BINARYOP, NULL },
    { "\\bblash",      13, STMARY10,  BINARYOP, NULL },
    { "\\moo",         14, STMARY10,  BINARYOP, NULL },
    { "\\varotimes",       15, STMARY10,  BINARYOP, NULL },
    { "\\varoast",     16, STMARY10,  BINARYOP, NULL },
    { "\\varobar",     17, STMARY10,  BINARYOP, NULL },
    { "\\varodot",     18, STMARY10,  BINARYOP, NULL },
    { "\\varoslash",       19, STMARY10,  BINARYOP, NULL },
    { "\\varobslash",      20, STMARY10,  BINARYOP, NULL },
    { "\\varocircle",      21, STMARY10,  BINARYOP, NULL },
    { "\\varoplus",    22, STMARY10,  BINARYOP, NULL },
    { "\\varominus",       23, STMARY10,  BINARYOP, NULL },
    { "\\boxast",      24, STMARY10,  BINARYOP, NULL },
    { "\\boxbar",      25, STMARY10,  BINARYOP, NULL },
    { "\\boxdot",      26, STMARY10,  BINARYOP, NULL },
    { "\\boxslash",    27, STMARY10,  BINARYOP, NULL },
    { "\\boxbslash",       28, STMARY10,  BINARYOP, NULL },
    { "\\boxcircle",       29, STMARY10,  BINARYOP, NULL },
    { "\\boxbox",      30, STMARY10,  BINARYOP, NULL },
    { "\\boxempty",    31, STMARY10,  BINARYOP, NULL },
    { "\\qed",         31, STMARY10,  BINARYOP, NULL },
    { "\\lightning",       32, STMARY10,  ORDINARY, NULL },
    { "\\merge",       33, STMARY10,  BINARYOP, NULL },
    { "\\vartimes",    34, STMARY10,  BINARYOP, NULL },
    { "\\fatsemi",     35, STMARY10,  BINARYOP, NULL },
    { "\\sswarrow",    36, STMARY10,  ARROW,    NULL },
    { "\\ssearrow",    37, STMARY10,  ARROW,    NULL },
    { "\\curlywedgeuparrow", 38, STMARY10,  ARROW,    NULL },
    { "\\curlywedgedownarrow", 39, STMARY10, ARROW,    NULL },
    { "\\fatslash",    40, STMARY10,  BINARYOP, NULL },
    { "\\fatbslash",       41, STMARY10,  BINARYOP, NULL },
    { "\\lbag",        42, STMARY10,  BINARYOP, NULL },
    { "\\rbag",        43, STMARY10,  BINARYOP, NULL },
    { "\\varbigcirc",      44, STMARY10,  BINARYOP, NULL },
    { "\\leftrightarroweq", 45, STMARY10,  ARROW,    NULL },
    { "\\curlyveedownarrow", 46, STMARY10,  ARROW,    NULL },
    { "\\curlyveeuparrow", 47, STMARY10,  ARROW,    NULL },
    { "\\nnwarrow",    48, STMARY10,  ARROW,    NULL },
    { "\\nnearrow",    49, STMARY10,  ARROW,    NULL },
    { "\\leftslice",       50, STMARY10,  BINARYOP, NULL },
    { "\\rightslice",      51, STMARY10,  BINARYOP, NULL },
    { "\\varolessthan",    52, STMARY10,  BINARYOP, NULL },
    { "\\varogreaterthan", 53, STMARY10,  BINARYOP, NULL },
    { "\\varovee",     54, STMARY10,  BINARYOP, NULL },
    { "\\varowedge",       55, STMARY10,  BINARYOP, NULL },
    { "\\talloblong",      56, STMARY10,  BINARYOP, NULL },
    { "\\interleave",      57, STMARY10,  BINARYOP, NULL },
    { "\\obar",        58, STMARY10,  BINARYOP, NULL },
    { "\\oslash",      59, STMARY10,  BINARYOP, NULL },
    { "\\olessthan",       60, STMARY10,  BINARYOP, NULL },
    { "\\ogreaterthan",    61, STMARY10,  BINARYOP, NULL },
    { "\\ovee",        62, STMARY10,  BINARYOP, NULL },
    { "\\owedge",      63, STMARY10,  BINARYOP, NULL },
    { "\\oblong",      64, STMARY10,  BINARYOP, NULL },
    { "\\inplus",      65, STMARY10,  RELATION, NULL },
    { "\\niplus",      66, STMARY10,  RELATION, NULL },
    { "\\nplus",       67, STMARY10,  BINARYOP, NULL },
    { "\\subsetplus",      68, STMARY10,  RELATION, NULL },
    { "\\supsetplus",      69, STMARY10,  RELATION, NULL },
    { "\\subsetpluseq",    70, STMARY10,  RELATION, NULL },
    { "\\supsetpluseq",    71, STMARY10,  RELATION, NULL },
    { "\\Lbag",        72, STMARY10,  OPENING,  NULL },
    { "\\Rbag",        73, STMARY10,  CLOSING,  NULL },
    { "\\llbracket",       74, STMARY10,  OPENING,  NULL },
    { "\\rrbracket",       75, STMARY10,  CLOSING,  NULL },
    { "\\llparenthesis",   76, STMARY10,  OPENING,  NULL },
    { "\\rrparenthesis",   77, STMARY10,  CLOSING,  NULL },
    { "\\binampersand",    78, STMARY10,  OPENING,  NULL },
    { "\\bindnasrepma",    79, STMARY10,  CLOSING,  NULL },
    { "\\trianglelefteqslant", 80, STMARY10, RELATION, NULL },
    { "\\trianglerighteqslant", 81, STMARY10, RELATION,    NULL },
    { "\\ntrianglelefteqslant", 82, STMARY10, RELATION,    NULL },
    { "\\ntrianglerighteqslant", 83, STMARY10, RELATION,   NULL },
    { "\\llfloor",     84, STMARY10,  OPENING,  NULL },
    { "\\rrfloor",     85, STMARY10,  CLOSING,  NULL },
    { "\\llceil",      86, STMARY10,  OPENING,  NULL },
    { "\\rrceil",      87, STMARY10,  CLOSING,  NULL },
    { "\\arrownot",    88, STMARY10,  RELATION, NULL },
    { "\\Arrownot",    89, STMARY10,  RELATION, NULL },
    { "\\Mapstochar",      90, STMARY10,  RELATION, NULL },
    { "\\mapsfromchar",    91, STMARY10,  RELATION, NULL },
    { "\\Mapsfromchar",    92, STMARY10,  RELATION, NULL },
    { "\\leftrightarrowtriangle", 93, STMARY10, BINARYOP,  NULL },
    { "\\leftarrowtriangle", 94, STMARY10,  RELATION, NULL },
    { "\\rightarrowtriangle", 95, STMARY10, RELATION, NULL },
    { "\\bigtriangledown", 96, STMARY10,  OPERATOR, NULL },
    { "\\bigtriangleup",   97, STMARY10,  OPERATOR, NULL },
    { "\\bigcurlyvee",     98, STMARY10,  OPERATOR, NULL },
    { "\\bigcurlywedge",   99, STMARY10,  OPERATOR, NULL },
    { "\\bigsqcap",   100, STMARY10,  OPERATOR, NULL },
    { "\\Bigsqcap",   100, STMARY10,  OPERATOR, NULL },
    { "\\bigbox",     101, STMARY10,  OPERATOR, NULL },
    { "\\bigparallel",    102, STMARY10,  OPERATOR, NULL },
    { "\\biginterleave",  103, STMARY10,  OPERATOR, NULL },
    { "\\bignplus",   112, STMARY10,  OPERATOR, NULL },
    { NULL,     -999,   -999,   -999,       NULL }
};

static mathchardef symbols_cyr10[] = {
    /* ---------------------- C Y R ---------------------------
          symbol     charnum    family    class     function
    -------------------------------------------------------- */
    /* ---
     * undefined: 20,21,28,29,33-59,61,63,64,91,92,93,96,123,124
     * ---------------------------------------------------------- */
    /* --- special characters --- */
    { "\\cyddot",   32, CYR10,   VARIABLE,  NULL },
    /* ---See amsfndoc.dvi Figure 1 Input Conventions for AMS cyrillic--- */
    { "A",      65, CYR10,   VARIABLE,  NULL },
    { "a",      97, CYR10,   VARIABLE,  NULL },
    { "B",      66, CYR10,   VARIABLE,  NULL },
    { "b",      98, CYR10,   VARIABLE,  NULL },
    { "V",      86, CYR10,   VARIABLE,  NULL },
    { "v",      118,    CYR10,   VARIABLE,  NULL },
    { "G",      71, CYR10,   VARIABLE,  NULL },
    { "g",      103,    CYR10,   VARIABLE,  NULL },
    { "D",      68, CYR10,   VARIABLE,  NULL },
    { "d",      100,    CYR10,   VARIABLE,  NULL },
    { "Dj",     6,  CYR10,   VARIABLE,  NULL },
    { "DJ",     6,  CYR10,   VARIABLE,  NULL },
    { "dj",     14, CYR10,   VARIABLE,  NULL },
    { "E",      69, CYR10,   VARIABLE,  NULL },
    { "e",      101,    CYR10,   VARIABLE,  NULL },
    { "\\\"E",      19, CYR10,   VARIABLE,  NULL },
    { "\\\"e",      27, CYR10,   VARIABLE,  NULL },
    { "\\=E",       5,  CYR10,   VARIABLE,  NULL },
    { "\\=e",       13, CYR10,   VARIABLE,  NULL },
    { "Zh",     17, CYR10,   VARIABLE,  NULL },
    { "ZH",     17, CYR10,   VARIABLE,  NULL },
    { "zh",     25, CYR10,   VARIABLE,  NULL },
    { "Z",      90, CYR10,   VARIABLE,  NULL },
    { "z",      122,    CYR10,   VARIABLE,  NULL },
    { "I",      73, CYR10,   VARIABLE,  NULL },
    { "i",      105,    CYR10,   VARIABLE,  NULL },
    { "\\=I",       4,  CYR10,   VARIABLE,  NULL },
    { "\\=\\i",     12, CYR10,   VARIABLE,  NULL },
    { "J",      74, CYR10,   VARIABLE,  NULL },
    { "j",      106,    CYR10,   VARIABLE,  NULL },
    { "\\u I",      18, CYR10,   VARIABLE,  NULL },
    { "\\u\\i",     26, CYR10,   VARIABLE,  NULL },
    { "K",      75, CYR10,   VARIABLE,  NULL },
    { "k",      107,    CYR10,   VARIABLE,  NULL },
    { "L",      76, CYR10,   VARIABLE,  NULL },
    { "l",      108,    CYR10,   VARIABLE,  NULL },
    { "Lj",     1,  CYR10,   VARIABLE,  NULL },
    { "LJ",     1,  CYR10,   VARIABLE,  NULL },
    { "lj",     9,  CYR10,   VARIABLE,  NULL },
    { "M",      77, CYR10,   VARIABLE,  NULL },
    { "m",      109,    CYR10,   VARIABLE,  NULL },
    { "N",      78, CYR10,   VARIABLE,  NULL },
    { "n",      110,    CYR10,   VARIABLE,  NULL },
    { "Nj",     0,  CYR10,   VARIABLE,  NULL },
    { "NJ",     0,  CYR10,   VARIABLE,  NULL },
    { "nj",     8,  CYR10,   VARIABLE,  NULL },
    { "O",      79, CYR10,   VARIABLE,  NULL },
    { "o",      111,    CYR10,   VARIABLE,  NULL },
    { "P",      80, CYR10,   VARIABLE,  NULL },
    { "p",      112,    CYR10,   VARIABLE,  NULL },
    { "R",      82, CYR10,   VARIABLE,  NULL },
    { "r",      114,    CYR10,   VARIABLE,  NULL },
    { "S",      83, CYR10,   VARIABLE,  NULL },
    { "s",      115,    CYR10,   VARIABLE,  NULL },
    { "T",      84, CYR10,   VARIABLE,  NULL },
    { "t",      116,    CYR10,   VARIABLE,  NULL },
    { "\\\'C",      7,  CYR10,   VARIABLE,  NULL },
    { "\\\'c",      15, CYR10,   VARIABLE,  NULL },
    { "U",      85, CYR10,   VARIABLE,  NULL },
    { "u",      117,    CYR10,   VARIABLE,  NULL },
    { "F",      70, CYR10,   VARIABLE,  NULL },
    { "f",      102,    CYR10,   VARIABLE,  NULL },
    { "Kh",     72, CYR10,   VARIABLE,  NULL },
    { "KH",     72, CYR10,   VARIABLE,  NULL },
    { "kh",     104,    CYR10,   VARIABLE,  NULL },
    { "Ts",     67, CYR10,   VARIABLE,  NULL },
    { "TS",     67, CYR10,   VARIABLE,  NULL },
    { "ts",     99, CYR10,   VARIABLE,  NULL },
    { "Ch",     81, CYR10,   VARIABLE,  NULL },
    { "CH",     81, CYR10,   VARIABLE,  NULL },
    { "ch",     113,    CYR10,   VARIABLE,  NULL },
    { "Dzh",        2,  CYR10,   VARIABLE,  NULL },
    { "DZH",        2,  CYR10,   VARIABLE,  NULL },
    { "dzh",        10, CYR10,   VARIABLE,  NULL },
    { "Sh",     88, CYR10,   VARIABLE,  NULL },
    { "SH",     88, CYR10,   VARIABLE,  NULL },
    { "sh",     120,    CYR10,   VARIABLE,  NULL },
    { "Shch",       87, CYR10,   VARIABLE,  NULL },
    { "SHCH",       87, CYR10,   VARIABLE,  NULL },
    { "shch",       119,    CYR10,   VARIABLE,  NULL },
    { "\\Cdprime",  95, CYR10,   VARIABLE,  NULL },
    { "\\cdprime",  127,    CYR10,   VARIABLE,  NULL },
    { "Y",      89, CYR10,   VARIABLE,  NULL },
    { "y",      121,    CYR10,   VARIABLE,  NULL },
    { "\\Cprime",   94, CYR10,   VARIABLE,  NULL },
    { "\\cprime",   126,    CYR10,   VARIABLE,  NULL },
    { "\\`E",       3,  CYR10,   VARIABLE,  NULL },
    { "\\`e",       11, CYR10,   VARIABLE,  NULL },
    { "Yu",     16, CYR10,   VARIABLE,  NULL },
    { "YU",     16, CYR10,   VARIABLE,  NULL },
    { "yu",     24, CYR10,   VARIABLE,  NULL },
    { "Ya",     23, CYR10,   VARIABLE,  NULL },
    { "YA",     23, CYR10,   VARIABLE,  NULL },
    { "ya",     31, CYR10,   VARIABLE,  NULL },
    { "\\Dz",       22, CYR10,   VARIABLE,  NULL },
    { "\\dz",       30, CYR10,   VARIABLE,  NULL },
    { "N0",     125,    CYR10,   VARIABLE,  NULL },
    { "<",      60, CYR10,   VARIABLE,  NULL },
    { ">",      62, CYR10,   VARIABLE,  NULL },
    /* --- trailer record --- */
    { NULL,     -999,   -999,   -999,       NULL }
}; /* --- end-of-symtable[] --- */

mathchardef_table symtables[16] = {
    { NOVALUE,  handlers         },
	{ CMMI10,   symbols_cmmi10   },
	{ CMMIB10,  symbols_cmmib10  },
	{ CMSY10,   symbols_cmsy10   },
	{ CMR10,    symbols_cmr10    },
	{ CMEX10,   symbols_cmex10   },
	{ RSFS10,   symbols_rsfs10   },
	{ BBOLD10,  symbols_bbold10  },
	{ STMARY10, symbols_stmary10 },
	{ CYR10,    symbols_cyr10    },
    { NOVALUE,  NULL             },
    { NOVALUE,  NULL             },
    { NOVALUE,  NULL             },
    { NOVALUE,  NULL             },
    { NOVALUE,  NULL             },
    { NOVALUE,  NULL             }
};

int mimetex_ctx_init(mimetex_ctx *mctx)
{
    int i;
    mctx->msgfp = NULL;
    mctx->msglevel = MSGLEVEL;
    /* ------------------------------------------------------------
    adjustable default values
    ------------------------------------------------------------ */
    /* --- variables for anti-aliasing parameters --- */
    mctx->centerwt    = 8;    /*lowpass matrix center pixel wt */
    mctx->adjacentwt  = 2;  /*lowpass matrix adjacent pixel wt*/
    mctx->cornerwt    = 1;    /*lowpass matrix corner pixel wt */
    mctx->minadjacent = 6;  /* darken if>=adjacent pts black*/
    mctx->maxadjacent = 8;  /* darken if<=adjacent pts black */
    mctx->weightnum   = 1;      /* font wt, */
    mctx->maxaaparams = 4;    /* #entries in table */
    /* set shrinkfactor */
    for (i = 1; i <= 51; i++)
        mctx->patternnumcount0[i] = mctx->patternnumcount1[i] = 0;

    mctx->ispatternnumcount = 1;      /* true to accumulate counts */
    mctx->warninglevel = WARNINGLEVEL;  /* warning level */

    /* ------------------------------------------------------------
    control flags and values
    ------------------------------------------------------------ */
    mctx->recurlevel = 0;     /* inc/decremented in rasterize() */
    mctx->scriptlevel = 0;    /* inc/decremented in rastlimits() */
    mctx->isstring = 0;       /*pixmap is ascii string, not raster*/
    mctx->isligature = 0;     /* true if ligature found */
    mctx->subexprptr = (char *)NULL;  /* ptr within expression to subexpr*/
    mctx->isdisplaystyle = 1;     /* displaystyle mode (forced if 2) */
    mctx->ispreambledollars = 0;  /* displaystyle mode set by $$...$$ */
    mctx->fontnum = 0;        /* cal=1,scr=2,rm=3,it=4,bb=5,bf=6 */
    mctx->fontsize = NORMALSIZE;  /* current size */
    mctx->displaysize = DISPLAYSIZE;  /* use \displaystyle when mctx->fontsize>=*/
    mctx->shrinkfactor = shrinkfactors[mctx->fontsize];
    mctx->unitlength = 1.0;    /* #pixels per unit (may be <1.0) */
    mctx->isnocatspace = 0;   /* >0 to not add space in rastcat()*/
    mctx->smashmargin = SMASHMARGIN;  /* minimum "smash" margin */
    mctx->mathsmashmargin = SMASHMARGIN; /* needed for \text{if $n-m$ even}*/
    mctx->issmashdelta = 1;   /* true if mctx->smashmargin is a delta */
    mctx->isexplicitsmash = 0;    /* true if \smash explicitly given */
    mctx->smashcheck = SMASHCHECK;  /* check if terms safe to smash */
    mctx->isscripted = 0;     /* is (lefthand) term text-scripted*/
    mctx->isdelimscript = 0;      /* is \right delim text-scripted */
    mctx->issmashokay = 0;    /*is leading char okay for smashing*/
    mctx->blanksignal = BLANKSIGNAL;  /*rastsmash signal right-hand blank*/
    mctx->blanksymspace = 0;      /* extra (or too much) space wanted*/
    mctx->istransparent = 1;      /* true sets background transparent*/
    mctx->fgred = 0;
    mctx->fggreen = 0;
    mctx->fgblue = 0;      /* fg r,g,b */
    mctx->bgred = 255;
    mctx->bggreen = 255;
    mctx->bgblue = 255;      /* bg r,g,b */
    mctx->gammacorrection = 1.25; /* gamma correction */
    mctx->isblackonwhite = 1; /*1=black on white,0=reverse*/
    mctx->aaalgorithm = 1;  /* for lp, 1=aalowpass, 2 =aapnm */
    mctx->maxfollow = 8;  /* aafollowline() maxturn parameter*/
    mctx->fgalias = 1;
    mctx->fgonly = 0;
    mctx->bgalias = 0;
    mctx->bgonly = 0;       /* aapnm() params */
    mctx->workingparam = (int *)NULL;  /* working parameter */
    mctx->workingbox = (subraster *)NULL; /*working subraster box*/
    mctx->isreplaceleft = 0;      /* true to replace mctx->leftexpression */
    mctx->leftexpression = (subraster *)NULL; /*rasterized so far*/
    mctx->leftsymdef = NULL; /* mathchardef for preceding symbol*/
    mctx->fraccenterline = NOVALUE; /* baseline for punct. after \frac */
    mctx->fonttable = aafonttable;
    return 0;
}


