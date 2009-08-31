lib_LTLIBRARIES = libmimetex.la
libmimetex_la_SOURCES = \
    mimetex.h \
    texfonts.h \
    utils.h \
    aa.c \
    bytemap.c \
    chardef.c \
    mimetex.c \
    raster.c \
    render.c \
    tex.c \
    utils.c

bin_PROGRAMS = mimetex gfuntype
mimetex_SOURCES = driver.c md5.c gifsave.c gifsave.h md5.h
mimetex_LDADD = libmimetex.la -lm
gfuntype_SOURCES = gfuntype.c
gfuntype_LDADD = libmimetex.la -lm

