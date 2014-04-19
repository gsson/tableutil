/* $Id: conf_parse.y,v 1.60 2005/07/08 22:57:31 gsson Exp $ */
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "types.h"
#include "conf_parse.h"
#include "conf_variable.h"



extern int conf_lineno;


int conf_lex(void);
int conf_parse(void);
void conf_restart(FILE *input_file);
void conf_error(const char *str);



%}


%token <string> VARIABLE STRING CIDR RANGE SINGLEIP HOSTNAME
%token QUOTE OBRACE EBRACE OPAREN EPAREN SEMICOLON COMMA ASSIGN
%token STDINTOK STDOUTTOK TEXTTOK CIDRTOK RANGETOK
%token LOADTOK SAVETOK UNIONTOK DIFFERENCETOK INTERSECTTOK INVERTTOK P2BTOK
%type <v> table_statement assign table_literal table_literal_entry
%type <v> load_statement union_statement difference_statement 
%type <v> intersection_statement invert_statement

%union {
	char *string;
	struct variable* v;
};
%start statements
%%

statements:
	statements statement SEMICOLON
	|
	;


statement:
	table_statement
	| void_statement
	;

assign:
	VARIABLE ASSIGN table_statement	{ variable_assign($1, $3); $$ = $3; }
	;
	
table_statement:
	VARIABLE	{ $$ = variable_get($1); }
	| assign
	| table_literal
	| load_statement
	| union_statement
	| difference_statement
	| intersection_statement
	| invert_statement
	;

void_statement:
	save_statement
	;

table_literal_entry:
	CIDR	{ $$ = variable_create(); variable_insert_cidr($$, $1); }
	| RANGE { $$ = variable_create(); variable_insert_range($$, $1); }
	| SINGLEIP { $$ = variable_create(); variable_insert_single($$, $1); }
	| HOSTNAME { $$ = variable_create(); variable_insert_hostname($$, $1); }
	| table_literal_entry COMMA CIDR { $$ = variable_insert_cidr($1, $3); }
	| table_literal_entry COMMA RANGE { $$ = variable_insert_range($1, $3); }
	| table_literal_entry COMMA SINGLEIP { $$ = variable_insert_single($1, $3); }
	| table_literal_entry COMMA HOSTNAME { $$ = variable_insert_hostname($1, $3); }
	;
	
table_literal:
	OBRACE EBRACE { $$ = variable_create(); }
	| OBRACE table_literal_entry EBRACE { $$ = $2; }
	;
	 
load_statement:
	LOADTOK OPAREN TEXTTOK COMMA STRING EPAREN { $$ = variable_load_text($5); }
	| LOADTOK OPAREN P2BTOK COMMA STRING EPAREN { $$ = variable_load_p2b($5); }
	| LOADTOK OPAREN TEXTTOK COMMA STDINTOK EPAREN { $$ = variable_load_text(NULL); }
	| LOADTOK OPAREN P2BTOK COMMA STDINTOK EPAREN { $$ = variable_load_p2b(NULL); }
	;
	
union_statement:
	UNIONTOK OPAREN table_statement COMMA table_statement EPAREN  { $$= variable_union($3, $5); }
	;
	
difference_statement:
	DIFFERENCETOK OPAREN table_statement COMMA table_statement EPAREN { $$ = variable_difference($3, $5); }
	;
	
intersection_statement:
	INTERSECTTOK OPAREN table_statement COMMA table_statement EPAREN  { $$ = variable_intersect($3, $5); }
	;
	
invert_statement:
	INVERTTOK OPAREN table_statement EPAREN  { $$ = variable_invert($3); }
	;
	
save_statement:
	SAVETOK OPAREN CIDRTOK COMMA STRING COMMA table_statement EPAREN   { variable_save_cidr($5, $7); }
	| SAVETOK OPAREN RANGETOK COMMA STRING COMMA table_statement EPAREN   { variable_save_range($5, $7); }
	| SAVETOK OPAREN CIDRTOK COMMA STDOUTTOK COMMA table_statement EPAREN   { variable_save_cidr(NULL, $7); }
	| SAVETOK OPAREN RANGETOK COMMA STDOUTTOK COMMA table_statement EPAREN   { variable_save_range(NULL, $7); }
	;
	
	
%%

void
conf_parse_file(const char *file) {
	FILE *f = fopen(file, "r");
	if (f == NULL) {
		fprintf(stderr, "Error opening file %s.\n", file);
		return;
	}
	variable_list_init(&variable_list);
	conf_restart(f);
	conf_parse();
	variable_list_destroy(&variable_list);
	fclose(f);
}

void
conf_parse_str(const char *c) {
	int fd[2];
	FILE *f;
	int len;
	if (pipe(fd)) {
		fprintf(stderr, "Error opening pipe.\n");
		return;
	}
	len = strlen(c);
	if (write(fd[1], c, len) != len) {
		fprintf(stderr, "Error writing string to pipe.\n");
		return;
	}
	close(fd[1]);
	f = fdopen(fd[0], "r");
	variable_list_init(&variable_list);
	conf_restart(f);
	conf_parse();
	variable_list_destroy(&variable_list);
	fclose(f);
}

void
conf_error(const char *str) {
	fprintf(stderr,"error: %s on line: %d\n",str, conf_lineno);
}
