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

#ifndef _IP4_RANGE_H_
#define _IP4_RANGE_H_

#include <sys/types.h>
#include "types.h"



struct ip4_range_list_element *ip4_range_list_insert_range(ip4_range_list_t *list, const struct ip4_range *range, struct ip4_range_list_element *hint);
void ip4_range_list_insert_range_raw(ip4_range_list_t *list, u_int32_t start, u_int32_t end, struct ip4_range_list_element *hint);
void ip4_range_list_insert_cidr(ip4_range_list_t *list, u_int32_t address, u_int8_t prefix, struct ip4_range_list_element *hint);
void ip4_range_list_init(ip4_range_list_t *list);
void ip4_range_list_destroy(ip4_range_list_t *list);
void ip4_range_list_output_cidr(FILE *f, ip4_range_list_t *list);
void ip4_range_list_output_range(FILE *f, ip4_range_list_t *list);
void ip4_range_list_output_single(FILE *f, ip4_range_list_t *list);

ip4_range_list_t *ip4_range_list_union(const ip4_range_list_t *lhs, const ip4_range_list_t *rhs, ip4_range_list_t *result);
ip4_range_list_t *ip4_range_list_difference(const ip4_range_list_t *lhs, const ip4_range_list_t *rhs, ip4_range_list_t *result);
ip4_range_list_t *ip4_range_list_intersect(const ip4_range_list_t *lhs, const ip4_range_list_t *rhs, ip4_range_list_t *result);
ip4_range_list_t *ip4_range_list_invert(const ip4_range_list_t *rhs, ip4_range_list_t *result);
ip4_range_list_t *ip4_range_list_assign(const ip4_range_list_t *rhs, ip4_range_list_t *result);
ip4_range_list_t *ip4_range_list_dup(const ip4_range_list_t *src, ip4_range_list_t *dst);

u_int32_t ip4_distance(u_int32_t a, u_int32_t b);
void ip4_range_output(FILE *f, u_int32_t start, u_int32_t end);
void ip4_range_output_single(FILE *f, u_int32_t start, u_int32_t end);
int ip4_range_overlap(const struct ip4_range *a, const struct ip4_range *b);

#endif /*_IP4_RANGE_H_*/
