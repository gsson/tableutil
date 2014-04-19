/* $Id: table_parse.y,v 1.6 2005/07/08 23:45:49 gsson Exp $ */
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "types.h"
#include "ip4_range.h"
#include "table_parse.h"

#define YYSTYPE char *

extern int table_lineno;

int table_lex(void);
int table_parse(void);
void table_restart(FILE *input_file);
void table_error(const char *str);

ip4_range_list_t *table_parse_target_list;

%}


%token CIDR RANGE SINGLEIP HOSTNAME
%token NEWLINE

%start lines
%%

lines:
	lines line NEWLINE
	| lines NEWLINE
	|
	;

line:
	CIDR	{
			int a, b, c, d;
			int p;
			if (sscanf($1, "%d.%d.%d.%d/%d", &a, &b, &c, &d, &p) == 5 &&
				a >= 0 && a < 256 &&
				b >= 0 && b < 256 &&
				c >= 0 && c < 256 &&
				d >= 0 && d < 256 &&
				p >= 0 && p < 33) {
				ip4_range_list_insert_cidr(table_parse_target_list, IP(a,b,c,d), p, NULL);
			}
			free($1);
		}
	| RANGE {
			int a, b, c, d;
			int a1, b1, c1, d1;

			if (sscanf($1, "%d.%d.%d.%d-%d.%d.%d.%d", &a, &b, &c, &d, &a1, &b1, &c1, &d1 ) == 8 &&
				a >= 0 && a < 256 &&
				b >= 0 && b < 256 &&
				c >= 0 && c < 256 &&
				d >= 0 && d < 256 &&
				a1 >= 0 && a1 < 256 &&
				b1 >= 0 && b1 < 256 &&
				c1 >= 0 && c1 < 256 &&
				d1 >= 0 && d1 < 256) {
				ip4_range_list_insert_range_raw( table_parse_target_list, IP(a,b,c,d), IP(a1, b1, c1, d1), NULL);
			}
			free($1);
		}
	| SINGLEIP {
			int a, b, c, d;
		    
			if (sscanf($1, "%d.%d.%d.%d", &a, &b, &c, &d) == 4 &&
					a >= 0 && a < 256 &&
					b >= 0 && b < 256 &&
					c >= 0 && c < 256 &&
					d >= 0 && d < 256) {
					ip4_range_list_insert_cidr(table_parse_target_list, IP(a,b,c,d), 32, NULL);
			}
			free($1);
		}
	| HOSTNAME {
			struct addrinfo *ai_result;
			struct addrinfo *ai;
			struct sockaddr_in *sa;
			
			int result;
			struct addrinfo hint;
			
			memset(&hint, 0, sizeof(struct addrinfo));
			
			hint.ai_family = AF_INET;
			hint.ai_socktype = SOCK_STREAM;
			
			result = getaddrinfo($1, NULL, &hint, &ai_result);
			if (result) {
				fprintf(stderr, "Host lookup for host '%s' failed: %s\n", $1, gai_strerror(result));
			}
			else {
				for (ai = ai_result; ai; ai = ai->ai_next) {
					sa = (struct sockaddr_in *) ai->ai_addr;
					ip4_range_list_insert_cidr(table_parse_target_list, ntohl(sa->sin_addr.s_addr), 32, NULL);
				}
				
				freeaddrinfo(ai_result);
			}
			free($1);
		}
	
	;
	
%%

void
table_parse_file(const char *file, ip4_range_list_t *list) {
	FILE *f;
	if (file == NULL) {
		f = stdin;
	}
	else {
		f = fopen(file, "r");
	}
	
	if (f == NULL) {
		fprintf(stderr, "Error opening file %s.\n", file);
		return;
	}
	table_parse_target_list = list;
	table_restart(f);
	table_parse();
	fclose(f);
}

void
table_error(const char *str) {
	fprintf(stderr,"error: %s on line: %d\n",str, table_lineno);
}
