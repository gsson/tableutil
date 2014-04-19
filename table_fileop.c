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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <zlib.h>
#include <sys/types.h>
#include <ctype.h>

#include "table_parse.h"
#include "table_fileop.h"
#include "ip4_range.h"

const char p2b_magic[]="\xFF\xFF\xFF\xFFP2B";
#define P2B_MAGIC_SIZE (sizeof(p2b_magic)-1)

int
ip4_p2b_load(const char *name, ip4_range_list_t *list) {
	char magic_buf[P2B_MAGIC_SIZE];
	int version;
	int c;
	int i;
	u_int32_t count;
	u_int32_t name_index;
	u_int32_t start;
	u_int32_t end;
	
	gzFile file;
	if (name == NULL) {
		file = gzdopen(STDIN_FILENO, "r");
	}
	else {
		file = gzopen(name, "r");
	}

	if (file == NULL) {
		fprintf(stderr, "Error opening file '%s'.\n", name);
		return -1;
	}
	if (gzread(file, magic_buf, P2B_MAGIC_SIZE) != P2B_MAGIC_SIZE) {
		fprintf(stderr, "Error reading file '%s'.\n", name);
		return -1;
	}
	if (memcmp(p2b_magic, magic_buf, P2B_MAGIC_SIZE) != 0) {
		fprintf(stderr, "Bad magic in file '%s'.\n", name);
		return -1;
	}
	version = gzgetc(file);
	
	if (version == 1 || version == 2) {
		while (!gzeof(file)) {
			c = gzgetc(file);
			while (!gzeof(file) && c != '\0') {
				c = gzgetc(file);
			}
			gzread(file, &start, sizeof(u_int32_t));
			gzread(file, &end, sizeof(u_int32_t));
			start=ntohl(start);
			end=ntohl(end);	
			ip4_range_list_insert_range_raw(list, start, end, NULL);
		}
	}
	else if (version == 3){
		gzread(file, &count, sizeof(u_int32_t));
		count=ntohl(count);
		/* Skip names */
		for (i = 0; i < count; i++) {
			c = gzgetc(file);
			while (!gzeof(file) && c != '\0') {
				c = gzgetc(file);
			}
		}
		
		gzread(file, &count, sizeof(u_int32_t));
		count=ntohl(count);
		for (i = 0; i < count; i++) {
			gzread(file, &name_index, sizeof(u_int32_t));
			gzread(file, &start, sizeof(u_int32_t));
			gzread(file, &end, sizeof(u_int32_t));
			start=ntohl(start);
			end=ntohl(end);	
			ip4_range_list_insert_range_raw(list, start, end, NULL);
		}		
	}
	else {
		fprintf(stderr, "Unsupported P2B-version (%d) in file '%s'.\n", version, name);
		return -1;
	}
		
	gzclose(file);

	return 0;
}

#define TOKEN_SIZE 256

int
ip4_text_load(const char *name, ip4_range_list_t *list) {
	table_parse_file(name, list);
	
#if 0
	char token[TOKEN_SIZE];
    static char     ch = ' ';
    int             i = 0;
    int 			a, b, c, d, p;
    int 			a1, b1, c1, d1;

	gzFile file;
	if (name == NULL) {
		file = gzdopen(STDIN_FILENO, "r");
	}
	else {
		file = gzopen(name, "r");
	}
	if (file == NULL) {
		fprintf(stderr, "Error opening file '%s'.\n", name);
		return -1;
	}

	for (;;) {
		for (;;) {
			/* Skip Whitespace */
			while (!gzeof(file) && isspace(ch)) {
				ch = gzgetc(file);
			}
			
			/* Remove from '#' until end of line */
			if (ch == '#') {
				while (!gzeof(file) && ch != '\n') {
					ch = gzgetc(file);
				}
			}
			else {
				break;
			}
		}
			
		if (gzeof(file)) {
			ch = ' ';
			break;
		}
		
		i = 0;
		token[i++] = ch;
		ch = gzgetc(file);
		while (!gzeof(file) && !isspace(ch)) {
			if (i < TOKEN_SIZE)
				token[i++] = ch;
			ch = gzgetc(file);
		}
		
		if (i >= TOKEN_SIZE) {
			fprintf(stderr, "Syntax error in '%s'.\n", name);
			gzclose(file);
			return -1;
		}
		token[i] = '\0';
		        
		if (sscanf(token, "%d.%d.%d.%d-%d.%d.%d.%d", &a, &b, &c, &d, &a1, &b1, &c1, &d1 ) == 8 &&
			a >= 0 && a < 256 &&
			b >= 0 && b < 256 &&
			c >= 0 && c < 256 &&
			d >= 0 && d < 256 &&
			a1 >= 0 && a1 < 256 &&
			b1 >= 0 && b1 < 256 &&
			c1 >= 0 && c1 < 256 &&
			d1 >= 0 && d1 < 256) {
			ip4_range_list_insert_range_raw(list, IP(a,b,c,d), IP(a1,b1,c1,d1), NULL);
		}
		else if (sscanf(token, "%d.%d.%d.%d/%d", &a, &b, &c, &d, &p) == 5 &&
			a >= 0 && a < 256 &&
			b >= 0 && b < 256 &&
			c >= 0 && c < 256 &&
			d >= 0 && d < 256 &&
			p >= 0 && p < 33) {
			ip4_range_list_insert_cidr(list, IP(a,b,c,d), p, NULL);
		}
		else if (sscanf(token, "%d.%d.%d.%d", &a, &b, &c, &d) == 4 &&
			a >= 0 && a < 256 &&
			b >= 0 && b < 256 &&
			c >= 0 && c < 256 &&
			d >= 0 && d < 256) {
			ip4_range_list_insert_range_raw(list, IP(a,b,c,d), IP(a,b,c,d), NULL);
		}
		else {
			fprintf(stderr, "Syntax error in '%s'.\n", name);
			gzclose(file);
			return -1;
		}
	}
	gzclose(file);
#endif
	return 0;
}

int
ip4_cidr_save(const char *name, ip4_range_list_t *list) {
	FILE *file;
	if (name == NULL) {
		file = stdout;
	}
	else {
		file = fopen(name, "w");
		if (file == NULL) {
			fprintf(stderr, "Error opening file '%s'.\n", name);
			return -1;
		}
		
	}
	ip4_range_list_output_cidr(file, list);
	fclose(file);
	return 0;
}

int
ip4_range_save(const char *name, ip4_range_list_t *list) {
	FILE *file;
	if (name == NULL) {
		file = stdout;
	}
	else {
		file = fopen(name, "w");
		if (file == NULL) {
			fprintf(stderr, "Error opening file '%s'.\n", name);
			return -1;
		}
	}
	ip4_range_list_output_range(file, list);
	fclose(file);
	return 0;
}

int
ip4_single_save(const char *name, ip4_range_list_t *list) {
	FILE *file;
	if (name == NULL) {
		file = stdout;
	}
	else {
		file = fopen(name, "w");
		if (file == NULL) {
			fprintf(stderr, "Error opening file '%s'.\n", name);
			return -1;
		}
	}
	ip4_range_list_output_single(file, list);
	fclose(file);
	return 0;
}
