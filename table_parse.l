/* $Id: table_parse.l,v 1.3 2005/10/06 19:26:52 gsson Exp $ */
/*
 * Copyright (c) 2005 Henrik Gustafsson <henrik.gustafsson@fnord.se>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

%{
#include <string.h>
#include <stdio.h>
#include "types.h"
#include "yacc.table_parse.tab.h"

int table_lex(void);

extern char *table_lval;

%}

%option yylineno
%option noyywrap
%option nounput

IP	[[:digit:]]{1,3}(\.[[:digit:]]{1,3}){3}

%%

{IP}\/[[:digit:]]{1,2}        table_lval=strdup(table_text); return CIDR;
{IP}-{IP}	table_lval=strdup(table_text); return RANGE;
{IP}        table_lval=strdup(table_text); return SINGLEIP;
[[:alpha:]][[:alnum:]\-]*(\.[[:alpha:]][[:alnum:]\-]*)*	table_lval=strdup(table_text); return HOSTNAME;
\r\n			return NEWLINE;
\n                      return NEWLINE;
\r			return NEWLINE;

\#.*		        /* Ignore sh-style comments */
[ \t]+                  /* ignore whitespace */

%%
