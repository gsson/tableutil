# $Id: Makefile,v 1.14 2005/07/05 21:08:42 gsson Exp $

PREFIX?=/usr/local
INSTALL_BIN?=${INSTALL_PREFIX}/bin
INSTALL_MAN?=${INSTALL_PREFIX}/man/man1

INSTALL=/usr/bin/install

# Use these CFLAGS for debugging
#CFLAGS= -g -pedantic -Wall -ansi

CFLAGS+=-pedantic -Wall -Wstrict-prototypes -Wmissing-prototypes -ansi
LDFLAGS+=-pedantic -Wall -ansi -lz

.PHONY: all clean obj

TARGET=tableutil
OBJECTS=tableutil.o table_fileop.o ip4_cidr.o ip4_range.o lex.yy.o y.tab.o
MAN=tableutil.1

all: ${TARGET}

install: ${TARGET}
	${INSTALL} -g bin -o root -m 755 ${TARGET} ${INSTALL_BIN}
	${INSTALL} -g bin -o root -m 644 ${MAN} ${INSTALL_MAN}

clean:
	rm -f ${TARGET}
	rm -f ${OBJECTS}
	rm -f y.tab.h y.tab.c lex.yy.c

${TARGET}: ${OBJECTS}
	${CC} ${LDFLAGS} $> -o $@
	
obj: ${OBJECTS}

.c.o:
	${CC} ${CFLAGS} -c $< -o $@

lex.yy.c: conf_parse.l y.tab.h
	${LEX} conf_parse.l

y.tab.c y.tab.h: conf_parse.y
	${YACC} -d conf_parse.y
	 