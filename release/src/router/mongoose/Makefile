# This file is part of Mongoose project, http://code.google.com/p/mongoose
# $Id: Makefile 473 2009-09-02 11:20:06Z valenok $

include ../common.mak

PROG=	mongoose
#CFLAGS=		-W -Wall -std=c99 -pedantic -O2 $(COPT)
CFLAGS = -Os -Wall $(EXTRACFLAGS)
#CFLAGS += -DNO_CGI -DNO_SSL
CFLAGS += -I$(SRCBASE) -I$(SRCBASE)/include -I. -I$(TOP)/shared
CFLAGS += -ldl -pthread
#CFLAGS += -L$(TOP)/nvram -lnvram -L$(TOP)/shared -lshared
LIB=		_$(PROG).so

all:
	$(CC) mongoose.c -shared -fPIC -fpic -o $(LIB) $(CFLAGS)
	$(CC) mongoose.c main.c -o $(PROG) $(CFLAGS)
	#$(CC) hello.c -o hello $(LINFLAGS)

# Possible COPT values: (in brackets are rough numbers for 'gcc -O2' on i386)
# -DHAVE_MD5		- use system md5 library (-2kb)
# -DNDEBUG		- strip off all debug code (-5kb)
# -DDEBUG		- build debug version (very noisy) (+7kb)
# -DNO_CGI		- disable CGI support (-5kb)
# -DNO_SSL		- disable SSL functionality (-2kb)
# -DCONFIG_FILE=\"file\" - use `file' as the default config file
# -DHAVE_STRTOUI64	- use system strtoui64() function for strtoull()
# -DSSL_LIB=\"libssl.so.<version>\" - use system versioned SSL shared object
# -DCRYPTO_LIB=\"libcrypto.so.<version>\" - use system versioned CRYPTO so

