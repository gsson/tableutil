/* $Id: tableutil.c,v 1.27 2005/07/07 18:19:11 gsson Exp $ */
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
void conf_parse_file(const char *file);
void conf_parse_str(const char *str);
void quickconvert(const char *type, const char *infile);

const char arg_p2b[]="p2b";
const char arg_text[]="text";

void quickconvert(const char *type, const char *file) {
	ip4_range_list_t range_list;
	int load_ok = 0;

	ip4_range_list_init(&range_list);
	
	if (!strcmp(arg_p2b, type)) {
		if (!ip4_p2b_load(file, &range_list)) {
			load_ok = 1;
		}
	}
	else if (!strcmp(arg_text, type)) {
		if (!ip4_text_load(file, &range_list)) {
			load_ok = 1;
		}
	}
	else {
		usage();
	}
	
	if (load_ok) {
		ip4_range_list_output_cidr(stdout, &range_list);
	}
	
	ip4_range_list_destroy(&range_list);
}

int
main(int argc, char *const *argv) {
	int ch;
	char t = 0;
	if (argc < 3 || argc > 4) {
		usage();
	}
	
	while ((ch = getopt(argc, argv, "cfq")) != -1) {
		switch (ch) {
		case 'c':
		case 'f':
		case 'q': {
			if (t) usage();
			t = ch;
			break;
		}
		default: {
			usage();
			break;
		}
		}
	}
	
	argc -=optind;
	argv +=optind;
	
	switch (t) {
	case 'c': {
		conf_parse_str(argv[0]);
		return 0;
	}
	case 'f': {
		conf_parse_file(argv[0]);
		return 0;
	}
	case 'q': {
		quickconvert(argv[0], argv[1]);
		return 0;
	}
	}
	return 0;
}


void usage(void) {
	extern char *__progname;

	fprintf(stderr, "usage: %s -q type table\n", __progname);
	fprintf(stderr, "       %s -c commands\n", __progname);
	fprintf(stderr, "       %s -f file\n", __progname);
	exit(1);
}
