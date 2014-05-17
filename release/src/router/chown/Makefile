# This file is part of Mongoose project, http://code.google.com/p/mongoose
# $Id: Makefile 473 2009-09-02 11:20:06Z valenok $

include ../common.mak

CFLAGS = -Os -Wall $(EXTRACFLAGS)

all:
	$(CC) chown.c util.c -o chown $(CFLAGS)
	$(CC) chgrp.c util.c -o chgrp $(CFLAGS)

clean:
	rm -rf *.o *.core $(PROG) *.obj *.so $(PROG).txt *.dSYM *.tgz
