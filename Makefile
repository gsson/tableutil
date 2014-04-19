# $Id: Makefile,v 1.24 2005/07/08 22:57:31 gsson Exp $

PREFIX?=/usr/local
INSTALL_BIN?=${PREFIX}/bin
INSTALL_MAN?=${PREFIX}/man/man1

INSTALL=/usr/bin/install
GROFF?=`which groff`

# Use these CFLAGS for debugging
#CFLAGS= -g

CFLAGS+=-pedantic -Wall -Wstrict-prototypes -Wmissing-prototypes -ansi
LDFLAGS+=-pedantic -Wall -ansi -lz

.PHONY: all clean html obj

TARGET=tableutil
OBJECTS=tableutil.o table_fileop.o ip4_cidr.o ip4_range.o \
	conf_variable.o lex.conf_parse.o yacc.conf_parse.tab.o\
	lex.table_parse.o yacc.table_parse.tab.o

MAN=tableutil.1

all: ${TARGET}

install: ${TARGET}
	${INSTALL} -g bin -o root -m 755 ${TARGET} ${INSTALL_BIN}
	${INSTALL} -g bin -o root -m 644 ${MAN} ${INSTALL_MAN}

clean:
	rm -f ${TARGET}
	rm -f ${OBJECTS}
	rm -f yacc.conf_parse.tab.c yacc.conf_parse.tab.h lex.conf_parse.c
	rm -f yacc.table_parse.tab.c yacc.table_parse.tab.h lex.table_parse.c

obj: ${OBJECTS}

html:
	${GROFF} -Thtml -mandoc tableutil.1 > tableutil.html

${TARGET}: ${OBJECTS}
	${CC} ${LDFLAGS} $> -o $@


.c.o:
	${CC} ${CFLAGS} -c $< -o $@

lex.conf_parse.c: conf_parse.l yacc.conf_parse.tab.h
	${LEX} -t -Pconf_ conf_parse.l > lex.conf_parse.c

yacc.conf_parse.tab.c yacc.conf_parse.tab.h: conf_parse.y
	${YACC} -pconf_ -byacc.conf_parse -d conf_parse.y

lex.table_parse.c: table_parse.l yacc.table_parse.tab.h
	${LEX} -t -Ptable_ table_parse.l > lex.table_parse.c

yacc.table_parse.tab.c yacc.table_parse.tab.h: table_parse.y
	${YACC} -ptable_ -byacc.table_parse -d table_parse.y
	