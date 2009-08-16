lib_LTLIBRARIES = libmimetex.la
libmimetex_la_SOURCES = \
    mimetex.h \
    texfonts.h \
    utils.h \
    aa.c \
    bytemap.c \
    chardef.c \
    cstruct.c \
    mimetex.c \
    pbmpgm.c \
    raster.c \
    render.c \
    tex.c \
    utils.c \
    xbm.c

bin_PROGRAMS = mimetex gfuntype
mimetex_SOURCES = driver.c md5.c gifsave.c gifsave.h md5.h
mimetex_LDADD = libmimetex.la
gfuntype_SOURCES = gfuntype.c
gfuntype_LDADD = libmimetex.la

