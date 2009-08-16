#ifndef GIFSAVE_H
#define GIFSAVE_H
/* prototypes for... */
int   GIF_Create(), GIF_CompressImage(), GIF_Close();
/* ...gifsave enntry points */
void  GIF_SetColor(), GIF_SetTransparent();

/* ---
 * info needed when gif image returned in memory buffer
 * ------------------------------------------------------------ */
extern int gifSize;
extern int maxgifSize;

#endif /* GIFSAVE_H */
