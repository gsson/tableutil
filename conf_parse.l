/* $Id: conf_parse.l,v 1.34 2005/10/06 19:39:55 gsson Exp $ */
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
#include <unistd.h>
#include "types.h"
#include "yacc.conf_parse.tab.h"

int conf_lex(void);

%}

%option yylineno
%option noyywrap
%option nounput

IP	[[:digit:]]{1,3}(\.[[:digit:]]{1,3}){3}

%%

load/[^[:alnum:]]	return LOADTOK;
save/[^[:alnum:]]	return SAVETOK;
union/[^[:alnum:]]	return UNIONTOK;
difference/[^[:alnum:]]	return DIFFERENCETOK;
intersect/[^[:alnum:]]	return INTERSECTTOK;
invert/[^[:alnum:]]	return INVERTTOK;
stdin/[^[:alnum:]]	return STDINTOK;
stdout/[^[:alnum:]]	return STDOUTTOK;
text/[^[:alnum:]]	return TEXTTOK;
cidr/[^[:alnum:]]	return CIDRTOK;
single/[^[:alnum:]]	return SINGLETOK;
range/[^[:alnum:]]	return RANGETOK;
p2b/[^[:alnum:]]	return P2BTOK;

{IP}\/[[:digit:]]{1,2}        conf_lval.string=strdup(conf_text); return CIDR;
{IP}-{IP}	conf_lval.string=strdup(conf_text); return RANGE;
{IP}        conf_lval.string=strdup(conf_text); return SINGLEIP;
[[:alpha:]][[:alnum:]\-]*(\.[[:alpha:]][[:alnum:]\-]*)*	conf_lval.string=strdup(conf_text); return HOSTNAME;

\$[[:alpha:]][[:alnum:]_]+    conf_lval.string=strdup(conf_text); return VARIABLE;
\"[^\"\n]*\"		{ int l = strlen(conf_text)-2; conf_lval.string=malloc(l+1); strncpy(conf_lval.string, conf_text+1,l); conf_lval.string[l] = '\0'; return STRING; }
\{                      return OBRACE;
\}                      return EBRACE;
\(                      return OPAREN;
\)                      return EPAREN;
=                       return ASSIGN;
;                       return SEMICOLON;
,                       return COMMA;

\n                      /* Ignore EOL */
\r			/* Ignore EOL */
"//".*\n		/* Ignore C++-style comments */
"#".*\n                 /* Ignore sh-style comments */
[ \t]+                  /* Ignore whitespace */
%%
