/* $Id: tableutil.c,v 1.23 2005/07/05 21:15:46 gsson Exp $ */
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "table_fileop.h"
#include "types.h"
#include "ip4_range.h"

void usage(void);
void parse_commands(FILE *f);

const char arg_p2b[]="p2b";
const char arg_text[]="text";

int
main(int argc, const char *argv[]) {
	FILE *f;
	ip4_range_list_t *range_list;
	int load_ok = 0;
	
	if (argc < 2 || argc > 3) {
		usage();
	}
	if (argc == 3) {
		range_list = malloc(sizeof(ip4_range_list_t));
		ip4_range_list_init(range_list);
		
		if (!strcmp(arg_p2b, argv[1])) {
			if (!ip4_p2b_load(argv[2], range_list)) {
				load_ok = 1;
			}
		}
		else if (!strcmp(arg_text, argv[1])) {
			if (!ip4_text_load(argv[2], range_list)) {
				load_ok = 1;
			}
		}
		else {
			usage();
		}
		ip4_range_list_output_cidr(stdout, range_list);
		ip4_range_list_destroy(range_list);
		free(range_list);
	}
	else {
		f = fopen(argv[1], "r");
		parse_commands(f);
	}

	return 0;
}


void usage(void) {
	extern char *__progname;

	fprintf(stderr, "usage: %s type table\n", __progname);
	fprintf(stderr, "       %s scriptfile\n", __progname);
	exit(1);
}
