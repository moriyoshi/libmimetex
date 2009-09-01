/* $Id: gifsave.c,v 1.2 1998/07/05 16:29:56 sverrehu Exp $ */
/**************************************************************************
 *
 *  FILE            gifsave.c
 *
 *  DESCRIPTION     Routines to create a GIF-file. See README for
 *                  a description.
 *
 *                  The functions were originally written using Borland's
 *                  C-compiler on an IBM PC -compatible computer, but they
 *                  are compiled and tested on Linux and SunOS as well.
 *
 *  WRITTEN BY      Sverre H. Huseby <sverrehu@online.no>
 *
 **************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gifsave.h"

/**************************************************************************
 *                                                                        *
 *                       P R I V A T E    D A T A                         *
 *                                                                        *
 **************************************************************************/

/* used by IO-routines */

/* used by routines maintaining an LZW string table */
#define RES_CODES 2

#define HASH_FREE 0xFFFF
#define NEXT_FIRST 0xFFFF

#define MAXBITS 12
#define MAXSTR (1 << MAXBITS)

#define HASHSIZE 9973
#define HASHSTEP 2039

#define HASH(index, lastbyte) (((lastbyte << 8) ^ index) % HASHSIZE)

#if !defined(MAXGIFSZ)		/* " */
  #define MAXGIFSZ 131072	/* " max #bytes comprising gif image */
#endif				/* " */

/* used in the main routines */
typedef struct {
    Word LocalScreenWidth,
         LocalScreenHeight;
    Byte GlobalColorTableSize : 3,
         SortFlag             : 1,
         ColorResolution      : 3,
         GlobalColorTableFlag : 1;
    Byte BackgroundColorIndex;
    Byte PixelAspectRatio;
} ScreenDescriptor;

typedef struct {
    Byte Separator;
    Word LeftPosition,
         TopPosition;
    Word Width,
         Height;
    Byte LocalColorTableSize : 3,
         Reserved            : 2,
         SortFlag            : 1,
         InterlaceFlag       : 1,
         LocalColorTableFlag : 1;
} ImageDescriptor;

typedef struct {
    Word ImageHeight,
         ImageWidth,
         ImageLeft,
         ImageTop,
         RelPixX, RelPixY;   /* used by InputByte() -function */
    int(*GetPixel)(void *ctx, int x, int y);
    void *getPixelCtx;
} GIFCreationContext;

/**************************************************************************
 *                                                                        *
 *                   P R I V A T E    F U N C T I O N S                   *
 *                                                                        *
 **************************************************************************/

/*========================================================================*
 =                         Routines to do file IO                         =
 *========================================================================*/


/*-------------------------------------------------------------------------
 *
 *  NAME          Write
 *
 *  DESCRIPTION   Output bytes to the current OutFile.
 *
 *  INPUT         buf     pointer to buffer to write
 *                len     number of bytes to write
 *
 *  RETURNS       GIF_OK       - OK
 *                GIF_ERRWRITE - Error writing to the file
 */
static int
Write(GIFContext *ctx, const void *buf, unsigned len)
{
    if ( ctx->OutBuffer == NULL )			/* (added by j.forkosh) */
      {	if (fwrite(buf, sizeof(Byte), len, ctx->OutFile) < len)
	  return GIF_ERRWRITE; }
    else					/* (added by j.forkosh) */
      {	if ( ctx->gifSize + len <= ctx->maxgifSize )	/* " */
	  memcpy(ctx->OutBuffer + ctx->gifSize, buf, len); }	/* " */
    ctx->gifSize += len;				/* " */
    return GIF_OK;
}



/*-------------------------------------------------------------------------
 *
 *  NAME          WriteByte
 *
 *  DESCRIPTION   Output one byte to the current OutFile.
 *
 *  INPUT         b       byte to write
 *
 *  RETURNS       GIF_OK       - OK
 *                GIF_ERRWRITE - Error writing to the file
 */
static int
WriteByte(GIFContext *ctx, Byte b)
{
    if ( ctx->OutBuffer == NULL )			/* (added by j.forkosh) */
      {	if (putc(b, ctx->OutFile) == EOF)
	  return GIF_ERRWRITE; }
    else					/* (added by j.forkosh) */
      {	if ( ctx->gifSize < ctx->maxgifSize )		/* " */
	  ctx->OutBuffer[ctx->gifSize] = b; }		/* " */
    ctx->gifSize++;					/* " */
    return GIF_OK;
}



/*-------------------------------------------------------------------------
 *
 *  NAME          WriteWord
 *
 *  DESCRIPTION   Output one word (2 bytes with byte-swapping, like on
 *                the IBM PC) to the current OutFile.
 *
 *  INPUT         w       word to write
 *
 *  RETURNS       GIF_OK       - OK
 *                GIF_ERRWRITE - Error writing to the file
 */
static int
WriteWord(GIFContext *ctx, Word w)
{
    if ( ctx->OutBuffer == NULL )			/* (added by j.forkosh) */
      {	if (putc(w & 0xFF, ctx->OutFile) == EOF)
	  return GIF_ERRWRITE;
	if (putc((w >> 8), ctx->OutFile) == EOF)
	  return GIF_ERRWRITE; }
    else					/* (added by j.forkosh) */
      if ( ctx->gifSize+1 < ctx->maxgifSize )		/* " */
	{ ctx->OutBuffer[ctx->gifSize] = (Byte)(w & 0xFF);  /* " */
	  ctx->OutBuffer[ctx->gifSize+1] = (Byte)(w >> 8); }  /* " */
    ctx->gifSize += 2;				/* " */
    return GIF_OK;
}



/*-------------------------------------------------------------------------
 *
 *  NAME          Close
 *
 *  DESCRIPTION   Close current OutFile.
 */
static void
Close(GIFContext *ctx)
{
    if ( ctx->OutFile )			/* (added by j.forkosh) */
      fclose(ctx->OutFile);
    ctx->OutBuffer = NULL;				/* (added by j.forkosh) */
    ctx->OutFile = NULL;				/* " */
}





/*========================================================================*
 =                                                                        =
 =                      Routines to write a bit-file                      =
 =                                                                        =
 *========================================================================*/

/*-------------------------------------------------------------------------
 *
 *  NAME          InitBitFile
 *
 *  DESCRIPTION   Initiate for using a bitfile. All output is sent to
 *                  the current OutFile using the I/O-routines above.
 */
static void
InitBitFile(GIFContext *ctx)
{
    ctx->Buffer[ctx->Index = 0] = 0;
    ctx->BitsLeft = 8;
}



/*-------------------------------------------------------------------------
 *
 *  NAME          ResetOutBitFile
 *
 *  DESCRIPTION   Tidy up after using a bitfile
 *
 *  RETURNS       0 - OK, -1 - error
 */
static int
ResetOutBitFile(GIFContext *ctx)
{
    Byte numbytes;

    /* how much is in the buffer? */
    numbytes = ctx->Index + (ctx->BitsLeft == 8 ? 0 : 1);

    /* write whatever is in the buffer to the file */
    if (numbytes) {
        if (WriteByte(ctx, numbytes) != GIF_OK)
            return -1;

        if (Write(ctx, ctx->Buffer, numbytes) != GIF_OK)
            return -1;

        ctx->Buffer[ctx->Index = 0] = 0;
        ctx->BitsLeft = 8;
    }
    return 0;
}



/*-------------------------------------------------------------------------
 *
 *  NAME          WriteBits
 *
 *  DESCRIPTION   Put the given number of bits to the outfile.
 *
 *  INPUT         bits    bits to write from (right justified)
 *                numbits number of bits to write
 *
 *  RETURNS       bits written, or -1 on error.
 *
 */
static int
WriteBits(GIFContext *ctx, int bits, int numbits)
{
    int  bitswritten = 0;
    Byte numbytes = 255;

    do {
        /* if the buffer is full, write it */
        if ((ctx->Index == 254 && !ctx->BitsLeft) || ctx->Index > 254) {
            if (WriteByte(ctx, numbytes) != GIF_OK)
                return -1;

            if (Write(ctx, ctx->Buffer, numbytes) != GIF_OK)
                return -1;

            ctx->Buffer[ctx->Index = 0] = 0;
            ctx->BitsLeft = 8;
        }

        /* now take care of the two specialcases */
        if (numbits <= ctx->BitsLeft) {
            ctx->Buffer[ctx->Index] |= (bits & ((1 << numbits) - 1)) << (8 - ctx->BitsLeft);
            bitswritten += numbits;
            ctx->BitsLeft -= numbits;
            numbits = 0;
        } else {
            ctx->Buffer[ctx->Index] |= (bits & ((1 << ctx->BitsLeft) - 1)) << (8 - ctx->BitsLeft);
            bitswritten += ctx->BitsLeft;
            bits >>= ctx->BitsLeft;
            numbits -= ctx->BitsLeft;

            ctx->Buffer[++ctx->Index] = 0;
            ctx->BitsLeft = 8;
        }
    } while (numbits);

    return bitswritten;
}



/*========================================================================*
 =                Routines to maintain an LZW-string table                =
 *========================================================================*/

/*-------------------------------------------------------------------------
 *
 *  NAME          FreeStrtab
 *
 *  DESCRIPTION   Free arrays used in string table routines
 */
static void
FreeStrtab(GIFContext *ctx)
{
    if (ctx->StrHsh) {
        free(ctx->StrHsh);
        ctx->StrHsh = NULL;
    }
    if (ctx->StrNxt) {
        free(ctx->StrNxt);
        ctx->StrNxt = NULL;
    }
    if (ctx->StrChr) {
        free(ctx->StrChr);
        ctx->StrChr = NULL;
    }
}



/*-------------------------------------------------------------------------
 *
 *  NAME          AllocStrtab
 *
 *  DESCRIPTION   Allocate arrays used in string table routines
 *
 *  RETURNS       GIF_OK     - OK
 *                GIF_OUTMEM - Out of memory
 */
static int
AllocStrtab(GIFContext *ctx)
{
    /* just in case */
    FreeStrtab(ctx);

    if ((ctx->StrChr = (Byte *) malloc(MAXSTR * sizeof(Byte))) == 0) {
        FreeStrtab(ctx);
        return GIF_OUTMEM;
    }
    if ((ctx->StrNxt = (Word *) malloc(MAXSTR * sizeof(Word))) == 0) {
        FreeStrtab(ctx);
        return GIF_OUTMEM;
    }
    if ((ctx->StrHsh = (Word *) malloc(HASHSIZE * sizeof(Word))) == 0) {
        FreeStrtab(ctx);
        return GIF_OUTMEM;
    }
    return GIF_OK;
}



/*-------------------------------------------------------------------------
 *
 *  NAME          AddCharString
 *
 *  DESCRIPTION   Add a string consisting of the string of index plus
 *                the byte b.
 *
 *                If a string of length 1 is wanted, the index should
 *                be 0xFFFF.
 *
 *  INPUT         index   index to first part of string, or 0xFFFF is
 *                        only 1 byte is wanted
 *                b       last byte in new string
 *
 *  RETURNS       Index to new string, or 0xFFFF if no more room
 */
static Word
AddCharString(GIFContext *ctx, Word index, Byte b)
{
    Word hshidx;

    /* check if there is more room */
    if (ctx->NumStrings >= MAXSTR)
        return 0xFFFF;

    /* search the string table until a free position is found */
    hshidx = HASH(index, b);
    while (ctx->StrHsh[hshidx] != 0xFFFF)
        hshidx = (hshidx + HASHSTEP) % HASHSIZE;

    /* insert new string */
    ctx->StrHsh[hshidx] = ctx->NumStrings;
    ctx->StrChr[ctx->NumStrings] = b;
    ctx->StrNxt[ctx->NumStrings] = (index != 0xFFFF) ? index : NEXT_FIRST;

    return ctx->NumStrings++;
}



/*-------------------------------------------------------------------------
 *
 *  NAME          FindCharString
 *
 *  DESCRIPTION   Find index of string consisting of the string of index
 *                plus the byte b.
 *
 *                If a string of length 1 is wanted, the index should
 *                be 0xFFFF.
 *
 *  INPUT         index   index to first part of string, or 0xFFFF is
 *                        only 1 byte is wanted
 *                b       last byte in string
 *
 *  RETURNS       Index to string, or 0xFFFF if not found
 */
static Word
FindCharString(GIFContext *ctx, Word index, Byte b)
{
    Word hshidx, nxtidx;

    /* check if index is 0xFFFF. in that case we need only return b,
     * since all one-character strings has their bytevalue as their
     * index */
    if (index == 0xFFFF)
        return b;

    /* search the string table until the string is found, or we find
     * HASH_FREE. in that case the string does not exist. */
    hshidx = HASH(index, b);
    while ((nxtidx = ctx->StrHsh[hshidx]) != 0xFFFF) {
        if (ctx->StrNxt[nxtidx] == index && ctx->StrChr[nxtidx] == b)
            return nxtidx;
        hshidx = (hshidx + HASHSTEP) % HASHSIZE;
    }

    /* no match is found */
    return 0xFFFF;
}



/*-------------------------------------------------------------------------
 *
 *  NAME          ClearStrtab
 *
 *  DESCRIPTION   Mark the entire table as free, enter the 2**codesize
 *                one-byte strings, and reserve the RES_CODES reserved
 *                codes.
 *
 *  INPUT         codesize
 *                        number of bits to encode one pixel
 */
static void
ClearStrtab(GIFContext *ctx, int codesize)
{
    int q, w;
    Word *wp;

    /* no strings currently in the table */
    ctx->NumStrings = 0;

    /* mark entire hashtable as free */
    wp = ctx->StrHsh;
    for (q = 0; q < HASHSIZE; q++)
        *wp++ = HASH_FREE;

    /* insert 2**codesize one-character strings, and reserved codes */
    w = (1 << codesize) + RES_CODES;
    for (q = 0; q < w; q++)
        AddCharString(ctx, 0xFFFF, q);
}



/*========================================================================*
 =                        LZW compression routine                         =
 *========================================================================*/

/*-------------------------------------------------------------------------
 *
 *  NAME          LZW_Compress
 *
 *  DESCRIPTION   Perform LZW compression as specified in the
 *                GIF-standard.
 *
 *  INPUT         codesize
 *                         number of bits needed to represent
 *                         one pixelvalue.
 *                inputbyte
 *                         function that fetches each byte to compress.
 *                         must return -1 when no more bytes.
 *
 *  RETURNS       GIF_OK     - OK
 *                GIF_OUTMEM - Out of memory
 */
static int
LZW_Compress(GIFContext *ctx, int codesize, int (*inputbyte)(void*), void *ibctx)
{
    register int c;
    register Word index;
    int  clearcode, endofinfo, numbits, limit, errcode;
    Word prefix = 0xFFFF;

    /* set up the given outfile */
    InitBitFile(ctx);

    /* set up variables and tables */
    clearcode = 1 << codesize;
    endofinfo = clearcode + 1;

    numbits = codesize + 1;
    limit = (1 << numbits) - 1;

    if ((errcode = AllocStrtab(ctx)) != GIF_OK)
        return errcode;
    ClearStrtab(ctx, codesize);

    /* first send a code telling the unpacker to clear the stringtable */
    WriteBits(ctx, clearcode, numbits);

    /* pack image */
    while ((c = inputbyte(ibctx)) != -1) {
        /* now perform the packing. check if the prefix + the new
         *  character is a string that exists in the table */
        if ((index = FindCharString(ctx, prefix, c)) != 0xFFFF) {
            /* the string exists in the table. make this string the
             * new prefix.  */
            prefix = index;
        } else {
            /* the string does not exist in the table. first write
             * code of the old prefix to the file. */
            WriteBits(ctx, prefix, numbits);

            /* add the new string (the prefix + the new character) to
             * the stringtable */
            if (AddCharString(ctx, prefix, c) > limit) {
                if (++numbits > 12) {
                    WriteBits(ctx, clearcode, numbits - 1);
                    ClearStrtab(ctx, codesize);
                    numbits = codesize + 1;
                }
                limit = (1 << numbits) - 1;
            }

            /* set prefix to a string containing only the character
             * read. since all possible one-character strings exists
             * int the table, there's no need to check if it is found. */
            prefix = c;
        }
    }

    /* end of info is reached. write last prefix. */
    if (prefix != 0xFFFF)
        WriteBits(ctx, prefix, numbits);

    /* erite end of info -mark, flush the buffer, and tidy up */
    WriteBits(ctx, endofinfo, numbits);
    ResetOutBitFile(ctx);
    FreeStrtab(ctx);

    return GIF_OK;
}



/*========================================================================*
 =                              Other routines                            =
 *========================================================================*/

/*-------------------------------------------------------------------------
 *
 *  NAME          BitsNeeded
 *
 *  DESCRIPTION   Calculates number of bits needed to store numbers
 *                between 0 and n - 1
 *
 *  INPUT         n       number of numbers to store (0 to n - 1)
 *
 *  RETURNS       Number of bits needed
 */
static int
BitsNeeded(Word n)
{
    int ret = 1;

    if (!n--)
        return 0;
    while (n >>= 1)
        ++ret;
    return ret;
}



/*-------------------------------------------------------------------------
 *
 *  NAME          InputByte
 *
 *  DESCRIPTION   Get next pixel from image. Called by the
 *                LZW_Compress()-function
 *
 *  RETURNS       Next pixelvalue, or -1 if no more pixels
 */
static int
InputByte(void *_ctx)
{
    GIFCreationContext *ctx = _ctx;
    int ret;

    if (ctx->RelPixY >= ctx->ImageHeight)
        return -1;
    ret = ctx->GetPixel(ctx->getPixelCtx, ctx->ImageLeft + ctx->RelPixX, ctx->ImageTop + ctx->RelPixY);
    if (++ctx->RelPixX >= ctx->ImageWidth) {
        ctx->RelPixX = 0;
        ++ctx->RelPixY;
    }
    return ret;
}



/*-------------------------------------------------------------------------
 *
 *  NAME          WriteScreenDescriptor
 *
 *  DESCRIPTION   Output a screen descriptor to the current GIF-file
 *
 *  INPUT         sd      pointer to screen descriptor to output
 *
 *  RETURNS       GIF_OK       - OK
 *                GIF_ERRWRITE - Error writing to the file
 */
static int
WriteScreenDescriptor(GIFContext *ctx, ScreenDescriptor *sd)
{
    Byte tmp;

    if (WriteWord(ctx, sd->LocalScreenWidth) != GIF_OK)
        return GIF_ERRWRITE;
    if (WriteWord(ctx, sd->LocalScreenHeight) != GIF_OK)
        return GIF_ERRWRITE;
    tmp = (sd->GlobalColorTableFlag << 7)
          | (sd->ColorResolution << 4)
          | (sd->SortFlag << 3)
          | sd->GlobalColorTableSize;
    if (WriteByte(ctx, tmp) != GIF_OK)
        return GIF_ERRWRITE;
    if (WriteByte(ctx, sd->BackgroundColorIndex) != GIF_OK)
        return GIF_ERRWRITE;
    if (WriteByte(ctx, sd->PixelAspectRatio) != GIF_OK)
        return GIF_ERRWRITE;

    return GIF_OK;
}



/*-------------------------------------------------------------------------
 *
 *  NAME          WriteTransparentColorIndex (added by j.forkosh)
 *
 *  DESCRIPTION   Output a graphic extension block setting transparent
 *		  colormap index
 *
 *  INPUT         colornum       colormap index of color to be transparent
 *
 *  RETURNS       GIF_OK       - OK
 *                GIF_ERRWRITE - Error writing to the file
 */
static int
WriteTransparentColorIndex(GIFContext *ctx, int colornum)
{
    if ( colornum < 0 ) return GIF_OK;		/*no transparent color set*/
    if (WriteByte(ctx, (Byte)(0x21)) != GIF_OK)	/*magic:Extension Introducer*/
        return GIF_ERRWRITE;
    if (WriteByte(ctx, (Byte)(0xf9)) != GIF_OK)     /*magic:Graphic Control Label*/
        return GIF_ERRWRITE;
    if (WriteByte(ctx, (Byte)(4)) != GIF_OK)		/* #bytes in block */
        return GIF_ERRWRITE;
    if (WriteByte(ctx, (Byte)(1)) != GIF_OK)        /*transparent index indicator*/
        return GIF_ERRWRITE;
    if (WriteWord(ctx, (Word)(0)) != GIF_OK)		/* delay time */
        return GIF_ERRWRITE;
    if (WriteByte(ctx, (Byte)(colornum)) != GIF_OK)	/* transparent color index */
        return GIF_ERRWRITE;
    if (WriteByte(ctx, (Byte)(0)) != GIF_OK)        /* terminator */
        return GIF_ERRWRITE;

    return GIF_OK;
}



/*-------------------------------------------------------------------------
 *
 *  NAME          WriteImageDescriptor
 *
 *  DESCRIPTION   Output an image descriptor to the current GIF-file
 *
 *  INPUT         id      pointer to image descriptor to output
 *
 *  RETURNS       GIF_OK       - OK
 *                GIF_ERRWRITE - Error writing to the file
 */
static int
WriteImageDescriptor(GIFContext *ctx, ImageDescriptor *id)
{
    Byte tmp;

    if (WriteByte(ctx, id->Separator) != GIF_OK)
        return GIF_ERRWRITE;
    if (WriteWord(ctx, id->LeftPosition) != GIF_OK)
        return GIF_ERRWRITE;
    if (WriteWord(ctx, id->TopPosition) != GIF_OK)
        return GIF_ERRWRITE;
    if (WriteWord(ctx, id->Width) != GIF_OK)
        return GIF_ERRWRITE;
    if (WriteWord(ctx, id->Height) != GIF_OK)
        return GIF_ERRWRITE;
    tmp = (id->LocalColorTableFlag << 7)
          | (id->InterlaceFlag << 6)
          | (id->SortFlag << 5)
          | (id->Reserved << 3)
          | id->LocalColorTableSize;
    if (WriteByte(ctx, tmp) != GIF_OK)
        return GIF_ERRWRITE;

    return GIF_OK;
}



/**************************************************************************
 *                                                                        *
 *                    P U B L I C    F U N C T I O N S                    *
 *                                                                        *
 **************************************************************************/

/*-------------------------------------------------------------------------
 *
 *  NAME          GIF_Create
 *
 *  DESCRIPTION   Create a GIF-file, and write headers for both screen
 *                and image.
 *
 *  INPUT         filename
 *                        name of file to create (including extension)
 *                width   number of horisontal pixels on screen
 *                height  number of vertical pixels on screen
 *                numcolors
 *                        number of colors in the colormaps
 *                colorres
 *                        color resolution. Number of bits for each
 *                        primary color
 *
 *  RETURNS       Context
 */
GIFContext *
GIF_Create(FILE *fp, void *buffer, int buffer_size, int width, int height,
	   int numcolors, int colorres)
{
    GIFContext *retval;
    int q, tabsize;
    Byte *bp;
    ScreenDescriptor SD;

    if ((retval = malloc(sizeof(GIFContext))) == NULL) {
        return NULL;
    }

    memset(retval, sizeof(GIFContext), 0);

    retval->TransparentColorIndex = -1;
    retval->OutFile = fp;
    retval->OutBuffer = buffer;
    retval->maxgifSize = buffer_size;

    /* initiate variables for new GIF-file */
    retval->NumColors = numcolors ? (1 << BitsNeeded(numcolors)) : 0;
    retval->BitsPrPrimColor = colorres;
    retval->ScreenHeight = height;
    retval->ScreenWidth = width;

    /* write GIF signature */
    if ((Write(retval, "GIF87a", 6)) != GIF_OK) {
        free(retval);
        return NULL;
    }

    /* initiate and write screen descriptor */
    SD.LocalScreenWidth = width;
    SD.LocalScreenHeight = height;
    if (retval->NumColors) {
        SD.GlobalColorTableSize = BitsNeeded(retval->NumColors) - 1;
        SD.GlobalColorTableFlag = 1;
    } else {
        SD.GlobalColorTableSize = 0;
        SD.GlobalColorTableFlag = 0;
    }
    SD.SortFlag = 0;
    SD.ColorResolution = colorres - 1;
    SD.BackgroundColorIndex = 0;
    SD.PixelAspectRatio = 0;
    if (WriteScreenDescriptor(retval, &SD) != GIF_OK) {
        free(retval);
        return NULL;
    }

    /* allocate color table */
    if (retval->NumColors) {
        tabsize = retval->NumColors * 3;
        if ((retval->ColorTable = (Byte *) malloc(tabsize * sizeof(Byte))) == NULL) {
            free(retval);

            return NULL;
        } else {
            bp = retval->ColorTable;
            for (q = 0; q < tabsize; q++)
                *bp++ = 0;
        }
    }

    retval->StrHsh = NULL;
    retval->StrChr = NULL;
    retval->StrNxt = NULL;
    return retval;
}



/*-------------------------------------------------------------------------
 *
 *  NAME          GIF_SetColor
 *
 *  DESCRIPTION   Set red, green and blue components of one of the
 *                colors. The color components are all in the range
 *                [0, (1 << BitsPrPrimColor) - 1]
 *
 *  INPUT         colornum
 *                        color number to set. [0, NumColors - 1]
 *                red     red component of color
 *                green   green component of color
 *                blue    blue component of color
 */
void
GIF_SetColor(GIFContext *ctx, int colornum, int red, int green, int blue)
{
    long maxcolor;
    Byte *p;

    maxcolor = (1L << ctx->BitsPrPrimColor) - 1L;
    p = ctx->ColorTable + colornum * 3;
    *p++ = (Byte) ((red * 255L) / maxcolor);
    *p++ = (Byte) ((green * 255L) / maxcolor);
    *p++ = (Byte) ((blue * 255L) / maxcolor);
}



/*-------------------------------------------------------------------------
 *
 *  NAME          GIF_SetTransparent (added by j.forkosh)
 *
 *  DESCRIPTION   Set colormap index of color to be transparent
 *
 *  INPUT         colornum
 *                        color number to set transparent. [0, NumColors - 1]
 */
void
GIF_SetTransparent(GIFContext *ctx, int colornum)
{
    ctx->TransparentColorIndex = colornum;
}



/*-------------------------------------------------------------------------
 *
 *  NAME          GIF_CompressImage
 *
 *  DESCRIPTION   Compress an image into the GIF-file previousely
 *                created using GIF_Create(). All color values should
 *                have been specified before this function is called.
 *
 *                The pixels are retrieved using a user defined callback
 *                function. This function should accept two parameters,
 *                x and y, specifying which pixel to retrieve. The pixel
 *                values sent to this function are as follows:
 *
 *                    x : [ImageLeft, ImageLeft + ImageWidth - 1]
 *                    y : [ImageTop, ImageTop + ImageHeight - 1]
 *
 *                The function should return the pixel value for the
 *                point given, in the interval [0, NumColors - 1]
 *
 *  INPUT         left    screen-relative leftmost pixel x-coordinate
 *                        of the image
 *                top     screen-relative uppermost pixel y-coordinate
 *                        of the image
 *                width   width of the image, or -1 if as wide as
 *                        the screen
 *                height  height of the image, or -1 if as high as
 *                        the screen
 *                getpixel
 *                        address of user defined callback function.
 *                        (see above)
 *
 *  RETURNS       GIF_OK       - OK
 *                GIF_OUTMEM   - Out of memory
 *                GIF_ERRWRITE - Error writing to the file
 */
int
GIF_CompressImage(GIFContext *ctx, int left, int top, int width, int height,
		  int (*getpixel)(void *ctx, int x, int y), void *getPixelCtx)
{
    int codesize, errcode;
    ImageDescriptor ID;
    GIFCreationContext cctx;

    if (width < 0) {
        width = ctx->ScreenWidth;
        left = 0;
    }
    if (height < 0) {
        height = ctx->ScreenHeight;
        top = 0;
    }
    if (left < 0)
        left = 0;
    if (top < 0)
        top = 0;

    /* write global colortable if any */
    if (ctx->NumColors)
        if ((Write(ctx, ctx->ColorTable, ctx->NumColors * 3)) != GIF_OK)
            return GIF_ERRWRITE;

    /* write graphic extension block with transparent color index */
    if ( ctx->TransparentColorIndex >= 0 )     /* (added by j.forkosh) */
      if ( WriteTransparentColorIndex(ctx, ctx->TransparentColorIndex)
      !=   GIF_OK ) return GIF_ERRWRITE;

    /* initiate and write image descriptor */
    ID.Separator = ',';
    ID.LeftPosition = cctx.ImageLeft = left;
    ID.TopPosition = cctx.ImageTop = top;
    ID.Width = cctx.ImageWidth = width;
    ID.Height = cctx.ImageHeight = height;
    ID.LocalColorTableSize = 0;
    ID.Reserved = 0;
    ID.SortFlag = 0;
    ID.InterlaceFlag = 0;
    ID.LocalColorTableFlag = 0;

    if (WriteImageDescriptor(ctx, &ID) != GIF_OK)
        return GIF_ERRWRITE;

    /* write code size */
    codesize = BitsNeeded(ctx->NumColors);
    if (codesize == 1)
        ++codesize;
    if (WriteByte(ctx, codesize) != GIF_OK)
        return GIF_ERRWRITE;

    /* perform compression */
    cctx.RelPixX = cctx.RelPixY = 0;
    cctx.GetPixel = getpixel;
    cctx.getPixelCtx = getPixelCtx;
    if ((errcode = LZW_Compress(ctx, codesize, InputByte, &cctx)) != GIF_OK)
        return errcode;

    /* write terminating 0-byte */
    if (WriteByte(ctx, 0) != GIF_OK)
        return GIF_ERRWRITE;

    return GIF_OK;
}



/*-------------------------------------------------------------------------
 *
 *  NAME          GIF_Close
 *
 *  DESCRIPTION   Close the GIF-file
 *
 *  RETURNS       GIF_OK       - OK
 *                GIF_ERRWRITE - Error writing to file
 */
int
GIF_Close(GIFContext *ctx)
{
    ImageDescriptor ID;

    /* initiate and write ending image descriptor */
    ID.Separator = ';';
    ID.LeftPosition = 0;	/* (added by j.forkosh) */
    ID.TopPosition = 0;		/* " initialize entire ID structure */
    ID.Width = 0;		/* " and ditto for other ID.x=0; below */
    ID.Height = 0;
    ID.LocalColorTableSize = 0;
    ID.Reserved = 0;
    ID.SortFlag = 0;
    ID.InterlaceFlag = 0;
    ID.LocalColorTableFlag = 0;

    if (WriteImageDescriptor(ctx, &ID) != GIF_OK)
        return GIF_ERRWRITE;

    /* close file */
    Close(ctx);

    /* release color table */
    if (ctx->ColorTable) {
        free(ctx->ColorTable);
        ctx->ColorTable = NULL;
    }

    return GIF_OK;
}
/* --- end-of-file gifsave.c --- */
