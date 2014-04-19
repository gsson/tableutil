/* $Id: conf_parse.y,v 1.50 2005/07/05 21:37:34 gsson Exp $ */
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
#include "types.h"
#include "ip4_range.h"
#include "table_fileop.h"

#define MAXTOKLEN 32

#define TAILQ_REASSIGN(head, head2, field) do {			\
	(head2)->tqh_first = (head)->tqh_first;	\
	(head2)->tqh_last = (head)->tqh_last;	\
	if ((head2)->tqh_first != NULL)	\
		(head2)->tqh_first->field.tqe_prev = &(head2)->tqh_first;	\
	(head)->tqh_first = NULL;	\
	(head)->tqh_last = &(head)->tqh_first;	\
	} while(0)

int yydebug=1;
void yyerror(const char *str);
void parse_commands(FILE *f);
struct variable_list_element *variable_find(char *c);
extern int confline;

void variable_list_init(variable_list_t *list);
void variable_list_destroy(variable_list_t *list);

struct variable *variable_get(char *c);
struct variable *variable_assign(char *c, struct variable *table);
struct variable *variable_dup(struct variable *var);
struct variable *variable_create(void);
void variable_destroy(struct variable *var);
struct variable *variable_insert_range(struct variable *list, const char *address);
struct variable *variable_insert_cidr(struct variable *list, const char *address);
struct variable *variable_insert_single(struct variable *list, const char *address);
struct variable *variable_load_text(char *file);
struct variable *variable_load_p2b(char *file);
struct variable *variable_union(struct variable *lhs, struct variable *rhs);
struct variable *variable_intersect(struct variable *lhs, struct variable *rhs);
struct variable *variable_difference(struct variable *lhs, struct variable *rhs);
struct variable *variable_invert(struct variable *rhs);
void variable_save_cidr(char *file, struct variable *rhs);
void variable_save_range(char *file, struct variable *rhs);

int yylex(void);
int yyparse(void);
int yywrap(void);
void yyrestart(FILE *input_file);
variable_list_t variable_list;

%}


%token <string> VARIABLE WORD FILENAME CIDR RANGE SINGLEIP
%token QUOTE OBRACE EBRACE OPAREN EPAREN SEMICOLON COMMA ASSIGN STDINTOK STDOUTTOK TEXTTOK CIDRTOK RANGETOK
%token LOADTOK SAVETOK UNIONTOK DIFFERENCETOK INTERSECTTOK INVERTTOK P2BTOK
%type <v> table_statement assign table_literal table_literal_entry
%type <v> load_statement union_statement difference_statement 
%type <v> intersection_statement invert_statement
%type <string> file_string

%union {
	char *string;
	struct variable* v;
};

%%

statements:
	|	 
	statements statement SEMICOLON
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

file_string:
	QUOTE FILENAME QUOTE { $$ = $2; }
	;


table_literal_entry:
	CIDR	{ $$ = variable_create(); variable_insert_cidr($$, $1); }
	| RANGE { $$ = variable_create(); variable_insert_range($$, $1); }
	| SINGLEIP { $$ = variable_create(); variable_insert_single($$, $1); }
	| table_literal_entry COMMA CIDR { $$ = variable_insert_cidr($1, $3); }
	| table_literal_entry COMMA RANGE { $$ = variable_insert_range($1, $3); }
	| table_literal_entry COMMA SINGLEIP { $$ = variable_insert_single($1, $3); }
	;
	
table_literal:
	OBRACE EBRACE { $$ = variable_create(); }
	| OBRACE table_literal_entry EBRACE { $$ = $2; }
	;
	 
load_statement:
	LOADTOK OPAREN TEXTTOK COMMA file_string EPAREN { $$ = variable_load_text($5); }
	| LOADTOK OPAREN P2BTOK COMMA file_string EPAREN { $$ = variable_load_p2b($5); }
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
	SAVETOK OPAREN CIDRTOK COMMA file_string COMMA table_statement EPAREN   { variable_save_cidr($5, $7); }
	| SAVETOK OPAREN RANGETOK COMMA file_string COMMA table_statement EPAREN   { variable_save_range($5, $7); }
	| SAVETOK OPAREN CIDRTOK COMMA STDOUTTOK COMMA table_statement EPAREN   { variable_save_cidr(NULL, $7); }
	| SAVETOK OPAREN RANGETOK COMMA STDOUTTOK COMMA table_statement EPAREN   { variable_save_range(NULL, $7); }
	;
	
	
%%

void parse_commands(FILE *f) {
	variable_list_init(&variable_list);
	yyrestart(f);
	yyparse();
	variable_list_destroy(&variable_list);
}

void variable_list_destroy(variable_list_t *list) {
	struct variable_list_element *element;
	element = TAILQ_FIRST(list);
	while (!TAILQ_EMPTY(list)) {
		TAILQ_REMOVE(list, element, variable_list_links);
		variable_destroy(element->v);
		free(element);
		element = TAILQ_FIRST(list);
	}
	TAILQ_INIT(list);
}

void variable_list_init(variable_list_t *list) {
	TAILQ_INIT(list);
}

struct variable_list_element *variable_find(char *c) {
	struct variable_list_element *element;
	
	if (TAILQ_EMPTY(&variable_list)) {
		element = malloc(sizeof(struct variable_list_element));
		if (element == NULL) {
			fprintf(stderr, "Allocation error\n");
			exit(-1);
		}
		element->v = variable_create();
		element->v->name = c;
		element->v->temporary = 0;
		TAILQ_INSERT_HEAD(&variable_list, element, variable_list_links);
		return element;
	}

	TAILQ_FOREACH(element, &variable_list, variable_list_links) {
		if (strncmp(c, element->v->name, MAXTOKLEN) == 0) {
			free(c);
			return element;
		}
		if (strncmp(c, element->v->name, MAXTOKLEN) < 0) {
			break;
		}
	}
	
	if (strncmp(c, TAILQ_LAST(&variable_list, variable_list)->v->name, MAXTOKLEN) > 0) {
		element = malloc(sizeof(struct variable_list_element));
		if (element == NULL) {
			fprintf(stderr, "Allocation error\n");
			exit(-1);
		}
		element->v = variable_create();
		element->v->name = c;
		element->v->temporary = 0;

		TAILQ_INSERT_TAIL(&variable_list, element, variable_list_links);
		return element;		
	}
	
	TAILQ_FOREACH(element, &variable_list, variable_list_links) {
		if (strncmp(c, element->v->name, MAXTOKLEN) < 0) {
			struct variable_list_element *new_element;
			new_element = malloc(sizeof(struct variable_list_element));
			if (new_element == NULL) {
				fprintf(stderr, "Allocation error\n");
				exit(-1);
			}
			new_element->v = variable_create();
			new_element->v->name = c;
			new_element->v->temporary = 0;

			TAILQ_INSERT_BEFORE(element, new_element, variable_list_links);
			return new_element;
		}
	}
	
	fprintf(stderr, "*dies*\n");
	exit(-1);
}

struct variable *variable_dup(struct variable *var) {

	struct variable *new_var;

	if (var->temporary) {
		return var;
	}
	
	new_var = variable_create();
	if (var->name != NULL) {
		new_var->name = strdup(var->name);
	}

	ip4_range_list_dup(&(var->table), &(new_var->table));
	
	return new_var;
}


struct variable *variable_assign(char *c, struct variable *var) {
	struct variable_list_element *element = variable_find(c);

	
	if (var->temporary) {
		ip4_range_list_destroy(&(element->v->table));
		TAILQ_REASSIGN(&(var->table), &(element->v->table), ip4_range_list_links);
		variable_destroy(var);
	}	
	else {
		ip4_range_list_dup(&(var->table), &(element->v->table));
	}
	
	
	return element->v;
}

struct variable *variable_get(char *c) {
	struct variable_list_element *e = NULL;
	if (c != NULL) {
		e = variable_find(c);
		return e->v;
	}
	return NULL;
}

void variable_destroy(struct variable *var) {
	if (var != NULL) {
		ip4_range_list_destroy(&(var->table));
		if (var->name != NULL) {
			free(var->name);
		}
		free(var);
	}
}

struct variable *variable_create() {
	struct variable *var = malloc(sizeof(struct variable));
	if (var == NULL) {
		fprintf(stderr, "Allocation error\n");
		exit(-1);
	}
	TAILQ_INIT(&(var->table));
	var->temporary=1;
	var->name = NULL;
	return var;
}

struct variable *variable_insert_range(struct variable *list, const char *address) {
    int 			a, b, c, d;
    int 			a1, b1, c1, d1;
    
	if (sscanf(address, "%d.%d.%d.%d-%d.%d.%d.%d", &a, &b, &c, &d, &a1, &b1, &c1, &d1 ) == 8 &&
			a >= 0 && a < 256 &&
			b >= 0 && b < 256 &&
			c >= 0 && c < 256 &&
			d >= 0 && d < 256 &&
			a1 >= 0 && a1 < 256 &&
			b1 >= 0 && b1 < 256 &&
			c1 >= 0 && c1 < 256 &&
			d1 >= 0 && d1 < 256) {
			ip4_range_list_insert_range_raw(&(list->table), IP(a,b,c,d), IP(a1,b1,c1,d1), NULL);
	}    	
	return list;
}

struct variable *variable_insert_cidr(struct variable *list, const char *address) {
    int 			a, b, c, d;
    int 			p;
    
	if (sscanf(address, "%d.%d.%d.%d/%d", &a, &b, &c, &d, &p) == 5 &&
			a >= 0 && a < 256 &&
			b >= 0 && b < 256 &&
			c >= 0 && c < 256 &&
			d >= 0 && d < 256 &&
			p >= 0 && p < 33) {
			ip4_range_list_insert_cidr(&(list->table), IP(a,b,c,d), p, NULL);
	}
	return list;
}

struct variable *variable_insert_single(struct variable *list, const char *address) {
    int 			a, b, c, d;
    
	if (sscanf(address, "%d.%d.%d.%d", &a, &b, &c, &d) == 4 &&
			a >= 0 && a < 256 &&
			b >= 0 && b < 256 &&
			c >= 0 && c < 256 &&
			d >= 0 && d < 256) {
			ip4_range_list_insert_cidr(&(list->table), IP(a,b,c,d), 32, NULL);
	}
	
	return list;
}

struct variable *variable_load_text(char *file) {
	struct variable *var = NULL;
	var = variable_create();
	ip4_text_load(file, &(var->table));
	if (file != NULL) free(file);
	return var;
}

struct variable *variable_load_p2b(char *file) {
	struct variable *var = NULL;
	var = variable_create();
	ip4_p2b_load(file, &(var->table));
	if (file != NULL) free(file);
	return var;
}

struct variable *variable_union(struct variable  *lhs, struct variable  *rhs) {
	struct variable *var = NULL;
	
	var = variable_create();
	ip4_range_list_union(&(lhs->table), &(rhs->table), &(var->table));
	
	if (lhs->temporary) {
		variable_destroy(lhs);
	}
	if (rhs->temporary) {
		variable_destroy(rhs);
	}
	
	return var;
}

struct variable *variable_intersect(struct variable  *lhs, struct variable  *rhs) {
	struct variable *var = NULL;
	
	var = variable_create();
	ip4_range_list_intersect(&(lhs->table), &(rhs->table), &(var->table));
	
	if (lhs->temporary) {
		variable_destroy(lhs);
	}
	if (rhs->temporary) {
		variable_destroy(rhs);
	}
	
	return var;
}

struct variable *variable_difference(struct variable  *lhs, struct variable  *rhs) {
	struct variable *var = NULL;
	
	var = variable_create();
	ip4_range_list_difference(&(lhs->table), &(rhs->table), &(var->table));
	
	if (lhs->temporary) {
		variable_destroy(lhs);
	}
	if (rhs->temporary) {
		variable_destroy(rhs);
	}
	
	return var;
}

struct variable *variable_invert(struct variable *rhs) {
	struct variable *var = NULL;
	
	var = variable_create();
	ip4_range_list_invert(&(rhs->table), &(var->table));
	
	if (rhs->temporary) {
		variable_destroy(rhs);
	}
	
	return var;
}

void variable_save_cidr(char *file, struct variable *rhs) {
	ip4_cidr_save(file, &(rhs->table));
	
	if (rhs->temporary) {
		variable_destroy(rhs);
	}
	if (file != NULL) free(file);
	return;
}

void variable_save_range(char *file, struct variable *rhs) {
	ip4_range_save(file, &(rhs->table));
	
	if (rhs->temporary) {
		variable_destroy(rhs);
	}
	if (file != NULL) free(file);
	return;
}

void yyerror(const char *str) {
	fprintf(stderr,"error: %s on line: %d\n",str, confline);
}

int yywrap() {
	return 1;
}


