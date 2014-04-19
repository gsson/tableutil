/* $Id: types.h,v 1.11 2005/07/05 21:15:46 gsson Exp $ */
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

#ifndef _TYPES_H_
#define _TYPES_H_

#include <sys/types.h>
#include <sys/queue.h>


#define IP(a, b, c, d) ((a << 24) | (b << 16) | (c << 8) | d)

struct ip4_range {
	u_int32_t start;
	u_int32_t end;
};

struct ip4_range_list_element {
	TAILQ_ENTRY(ip4_range_list_element) ip4_range_list_links;
	struct ip4_range range;
};

typedef TAILQ_HEAD(ip4_range_list, ip4_range_list_element) ip4_range_list_t;

struct variable {
	int temporary;
	char *name;
	ip4_range_list_t table;
};

struct variable_list_element {
	TAILQ_ENTRY(variable_list_element) variable_list_links;
	struct variable *v;
};

typedef TAILQ_HEAD(variable_list, variable_list_element) variable_list_t;


#endif /*_TYPES_H_*/
