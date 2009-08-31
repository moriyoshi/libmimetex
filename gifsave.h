#ifndef GIFSAVE_H
#define GIFSAVE_H

enum GIF_Code {
    GIF_OK = 0,
    GIF_ERRCREATE,
    GIF_ERRWRITE,
    GIF_OUTMEM
};

typedef unsigned Word;          /* at least two bytes (16 bits) */
typedef unsigned char Byte;     /* exactly one byte (8 bits) */

typedef struct {
    int  BitsPrPrimColor,    /* bits pr primary color */
         NumColors;          /* number of colors in color table */
    int  TransparentColorIndex; /* (added by j.forkosh) */
    Byte *ColorTable;
    Word ScreenHeight, ScreenWidth;
    int gifSize;
    int maxgifSize;
    FILE *OutFile;    /* file to write to */
    Byte *OutBuffer;	/* (added by j.forkosh) */

    /* used when writing to a file bitwise */
    Byte Buffer[256];        /* there must be one more than `needed' */
    int  Index,              /* current byte in buffer */
         BitsLeft;           /* bits left to fill in current byte. These
                                     * are right-justified */
    Byte *StrChr;
    Word *StrNxt,
         *StrHsh,
         NumStrings;
} GIFContext;

GIFContext* GIF_Create(FILE *fp, void *buffer, int buffer_size,
        int width, int height, int numcolors, int colorres);
void GIF_SetColor(GIFContext *ctx, int colornum, int red, int green, int blue);
void GIF_SetTransparent(GIFContext *ctx, int colornum);	/* (added by j.forkosh) */
int  GIF_CompressImage(GIFContext *ctx, int left, int top, int width, int height,
		       int (*getpixel)(void *ctx, int x, int y), void *cctx);
int  GIF_Close(GIFContext *);

#endif /* GIFSAVE_H */
