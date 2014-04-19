/* $Id: ip4_range.c,v 1.45 2005/07/08 23:56:12 gsson Exp $ */
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

#include <stdlib.h>
#include <stdio.h>
#include "ip4_range.h"
#include "ip4_cidr.h"

#define MAX(a, b) (((a) > (b))?(a):(b))
#define MIN(a, b) (((a) < (b))?(a):(b))


u_int32_t
ip4_distance(u_int32_t a, u_int32_t b) {
	return (a>b)?(a-b):(b-a);
}

void
ip4_range_list_init(ip4_range_list_t *list) {
	TAILQ_INIT(list);
}

void
ip4_range_list_destroy(ip4_range_list_t *list) {
	struct ip4_range_list_element *element = TAILQ_FIRST(list);
	while (!TAILQ_EMPTY(list)) {
		TAILQ_REMOVE(list, element, ip4_range_list_links);
		free(element);
		element = TAILQ_FIRST(list);
	}
	TAILQ_INIT(list);
}

int
ip4_range_overlap(const struct ip4_range *a, const struct ip4_range *b) {
	if (a->start < b->start) {
		if (a->end >= b->start) {
			return 1;
		}
	}
	else {
		if (b->end >= a->start) {
			return 1;
		}
	}
	return 0;
}

struct ip4_range_list_element *
ip4_range_list_insert_range(ip4_range_list_t *list, const struct ip4_range *range, struct ip4_range_list_element *hint) {
	struct ip4_range_list_element *element;
	struct ip4_range_list_element *tmp_element;
	struct ip4_range_list_element *new_element;

	if (list==NULL || range==NULL) { return NULL; }

	if (TAILQ_EMPTY(list)) {
		new_element = malloc(sizeof(struct ip4_range_list_element));
		if (new_element == NULL) {
			fprintf(stderr, "Allocation error\n");
			exit(-1);
		}
		new_element->range = *range;
		TAILQ_INSERT_HEAD(list, new_element, ip4_range_list_links);
		return new_element;
	}
	
	tmp_element = TAILQ_LAST(list, ip4_range_list);
	if (range->start >= tmp_element->range.start) {
			/* Overlapping or adjacent */
			if (range->start <= tmp_element->range.end+1) {
				tmp_element->range.end = MAX(range->end, tmp_element->range.end);
				return tmp_element;
			}
			
			/* Separate */
			
			new_element = malloc(sizeof(struct ip4_range_list_element));
			if (new_element == NULL) {
				fprintf(stderr, "Allocation error\n");
				exit(-1);
			}
			new_element->range = *range;
			TAILQ_INSERT_TAIL(list, new_element, ip4_range_list_links);
			return new_element;
	}	
	
	if (hint != NULL) {
		element = hint;
	}
	else {
		element = TAILQ_FIRST(list);
	}
	
	for (;element != NULL; element = TAILQ_NEXT(element, ip4_range_list_links)) {
		/* Insert before */
		if (range->start <= element->range.start) {
			if (range->end+1 >= element->range.start) {
				element->range.start = range->start;
				element->range.end = MAX(range->end, element->range.end);
				return element;
			}
			
			new_element = malloc(sizeof(struct ip4_range_list_element));
			if (new_element == NULL) {
				fprintf(stderr, "Allocation error\n");
				exit(-1);
			}
			new_element->range = *range;
			TAILQ_INSERT_BEFORE(element, new_element, ip4_range_list_links);
			return new_element;
		}
	}
	fprintf(stderr, "*dies*\n");
	exit(-1);
}

ip4_range_list_t *
ip4_range_list_dup(const ip4_range_list_t *src, ip4_range_list_t *dst) {
	struct ip4_range_list_element *src_element;
	struct ip4_range_list_element *dst_element;
	
	ip4_range_list_destroy(dst);
	TAILQ_FOREACH(src_element, src, ip4_range_list_links) {
		dst_element = malloc(sizeof(struct ip4_range_list_element));
		if (dst_element == NULL) {
			fprintf(stderr, "Allocation error\n");
			exit(-1);
		}
		dst_element->range = src_element->range;
		TAILQ_INSERT_TAIL(dst, dst_element, ip4_range_list_links);
	}
	
	return dst;
}

void
ip4_range_list_insert_range_raw(ip4_range_list_t *list, u_int32_t start, u_int32_t end, struct ip4_range_list_element *hint) {
	struct ip4_range range;
	range.start = start;
	range.end = end;
	ip4_range_list_insert_range(list, &range, hint);
}

void ip4_range_list_insert_cidr(ip4_range_list_t *list, u_int32_t address, u_int8_t prefix, struct ip4_range_list_element *hint) {
	struct ip4_range range;
	range.start = address;
	range.end = ip4_cidr_top_address(address, prefix);
	ip4_range_list_insert_range(list, &range, hint);
}

ip4_range_list_t *
ip4_range_list_union(const ip4_range_list_t *lhs, const ip4_range_list_t *rhs, ip4_range_list_t *result) {
	struct ip4_range_list_element *lhs_element;
	struct ip4_range_list_element *rhs_element;
	
	if (lhs == NULL || rhs == NULL || result == NULL) {
		return result;
	}

	lhs_element = TAILQ_FIRST(lhs);
	rhs_element = TAILQ_FIRST(rhs);

	ip4_range_list_destroy(result);

	/*
	 * Try to keep them sorted. Will make insertion faster.
	 */	
	while (lhs_element != NULL || rhs_element != NULL) {
		while (lhs_element != NULL && (rhs_element == NULL || lhs_element->range.start <= rhs_element->range.start)) {
			ip4_range_list_insert_range(result, &(lhs_element->range), NULL);
			lhs_element = TAILQ_NEXT(lhs_element, ip4_range_list_links);
		}
		while (rhs_element != NULL && (lhs_element == NULL || rhs_element->range.start <= lhs_element->range.start)) {
			ip4_range_list_insert_range(result, &(rhs_element->range), NULL);
			rhs_element = TAILQ_NEXT(rhs_element, ip4_range_list_links);
		}
	}

	return result;
}

ip4_range_list_t *
ip4_range_list_intersect(const ip4_range_list_t *lhs, const ip4_range_list_t *rhs, ip4_range_list_t *result) {
	struct ip4_range_list_element *lhs_element;
	struct ip4_range_list_element *rhs_element;
	
	if (lhs == NULL || rhs == NULL || result == NULL) {
		return result;
	}
	
	lhs_element = TAILQ_FIRST(lhs);
	rhs_element = TAILQ_FIRST(rhs);

	ip4_range_list_destroy(result);
	
	/*
	 * FIXME: Way too many comparisions...
	 */
	 
	while (lhs_element != NULL && rhs_element != NULL) {
		if (ip4_range_overlap(&(lhs_element->range), &(rhs_element->range))) {

			ip4_range_list_insert_range_raw(result, MAX(lhs_element->range.start, rhs_element->range.start), MIN(lhs_element->range.end, rhs_element->range.end), NULL);
			
			if (lhs_element->range.end > rhs_element->range.end) {
				rhs_element = TAILQ_NEXT(rhs_element, ip4_range_list_links);
			}
			else if (lhs_element->range.end < rhs_element->range.end) {
				lhs_element = TAILQ_NEXT(lhs_element, ip4_range_list_links);
			}
			else {
				rhs_element = TAILQ_NEXT(rhs_element, ip4_range_list_links);
				lhs_element = TAILQ_NEXT(lhs_element, ip4_range_list_links);
			}
		}
		else {
			if (lhs_element->range.start > rhs_element->range.end) {
				rhs_element = TAILQ_NEXT(rhs_element, ip4_range_list_links);
			}
			else if (rhs_element->range.start > lhs_element->range.end) {
				lhs_element = TAILQ_NEXT(lhs_element, ip4_range_list_links);
			}
		}
	}

	return result;
}

ip4_range_list_t *
ip4_range_list_difference(const ip4_range_list_t *lhs, const ip4_range_list_t *rhs, ip4_range_list_t *result) {
	struct ip4_range_list_element *lhs_element;
	struct ip4_range_list_element *rhs_element;
	struct ip4_range range;
	
	if (lhs == NULL || rhs == NULL || result == NULL) {
		return result;
	}
	
	/*
	 * FIXME: Ugly :/
	 */

	lhs_element = TAILQ_FIRST(lhs);
	rhs_element = TAILQ_FIRST(rhs);

	ip4_range_list_destroy(result);
	if (lhs_element != NULL) {
		range = lhs_element->range;
		while (lhs_element != NULL) {
			
			if (rhs_element != NULL) {
				while (rhs_element != NULL && rhs_element->range.end < range.start) {
					rhs_element = TAILQ_NEXT(rhs_element, ip4_range_list_links);
				}
			}
			
			if (rhs_element != NULL) {
				if (ip4_range_overlap(&range, &(rhs_element->range))) {
					/* Merge difference */
					if (range.start < rhs_element->range.start) {
						ip4_range_list_insert_range_raw(result, range.start, rhs_element->range.start-1, NULL);
					}
					
					if (range.end > rhs_element->range.end) {
						range.start = rhs_element->range.end + 1;
						rhs_element = TAILQ_NEXT(rhs_element, ip4_range_list_links);
					}
					else {
						lhs_element = TAILQ_NEXT(lhs_element, ip4_range_list_links);
						if (lhs_element != NULL) {
							range = lhs_element->range;
						}
					}
				}
				else {
					ip4_range_list_insert_range(result, &range, NULL);			
					lhs_element = TAILQ_NEXT(lhs_element, ip4_range_list_links);
					if (lhs_element != NULL) {
						range = lhs_element->range;
					}
				}
			}
			else {
				ip4_range_list_insert_range(result, &range, NULL);			
				lhs_element = TAILQ_NEXT(lhs_element, ip4_range_list_links);
				if (lhs_element != NULL) {
					range = lhs_element->range;
				}
			}		
		}
	}
	return result;
}

ip4_range_list_t *
ip4_range_list_invert(const ip4_range_list_t *rhs, ip4_range_list_t *result) {
	struct ip4_range_list_element *rhs_element;
	u_int32_t start;
	
	if (rhs == NULL || result == NULL) {
		return result;
	}
	
	rhs_element = TAILQ_FIRST(rhs);
	ip4_range_list_destroy(result);
	
	if (rhs_element == NULL) {
		ip4_range_list_insert_range_raw(result, 0x00000000, 0xFFFFFFFF, NULL);
		return result;
	}
	
	if (rhs_element->range.start > 0) {
		ip4_range_list_insert_range_raw(result, 0, rhs_element->range.start-1, NULL);
	}

	start = rhs_element->range.end + 1;
	rhs_element = TAILQ_NEXT(rhs_element, ip4_range_list_links);
	while (rhs_element != NULL) {
		ip4_range_list_insert_range_raw(result, start + 1, rhs_element->range.start - 1, NULL);
		start = rhs_element->range.end;
		rhs_element = TAILQ_NEXT(rhs_element, ip4_range_list_links);
	}

	if (start < 0xFFFFFFFF) {
		ip4_range_list_insert_range_raw(result, start+1, 0xFFFFFFFF, NULL);
	}

	return result;
}

void
ip4_range_list_output_cidr(FILE *f, ip4_range_list_t *list) {
	struct ip4_range_list_element *element;
	u_int32_t diff_address;
	u_int8_t diff_prefix;
	
	u_int32_t cidr_address;
	u_int8_t cidr_prefix;
	
	TAILQ_FOREACH(element, list, ip4_range_list_links) {
		diff_address = ip4_distance(element->range.start, element->range.end);
		diff_prefix = 32 - ip4_cidr_suffix(diff_address+1);
		
		if (diff_address == 0) {
			cidr_address = element->range.start;
			cidr_prefix = 32;
			ip4_cidr_output(f, cidr_address, cidr_prefix);
			fprintf(f, "\n");
		}
		else if (diff_address == 0xFFFFFFFF) {
			cidr_address = 0;
			cidr_prefix = 0;
			ip4_cidr_output(f, cidr_address, cidr_prefix);
			fprintf(f, "\n");
		}
		else {
			cidr_address = element->range.start;
			cidr_prefix = ip4_cidr_prefix(cidr_address);
			while (cidr_address <= element->range.end && cidr_address >= element->range.start) {
				
				if (cidr_prefix < diff_prefix)
					cidr_prefix = diff_prefix;
					
				ip4_cidr_output(f, cidr_address, cidr_prefix);
				fprintf(f, "\n");
				
				cidr_address = ip4_cidr_top_address(cidr_address, cidr_prefix)+1;
				cidr_prefix = ip4_cidr_prefix(cidr_address);
				
				diff_address = ip4_distance(cidr_address, element->range.end);
				diff_prefix = 32 - ip4_cidr_suffix(diff_address+1);
			}
		}
	}	
	
}

void
ip4_range_list_output_range(FILE *f, ip4_range_list_t *list) {
	struct ip4_range_list_element *element;

	TAILQ_FOREACH(element, list, ip4_range_list_links) {
		ip4_range_output(f, element->range.start, element->range.end);
		fprintf(f, "\n");
	}	
	
}

void ip4_range_output(FILE *f, u_int32_t start, u_int32_t end) {
        fprintf(f, "%d.%d.%d.%d", start >> 24 & 255, start >> 16 & 255, start >> 8 & 255, start & 255);
        fprintf(f, "-");
        fprintf(f, "%d.%d.%d.%d", end >> 24 & 255, end >> 16 & 255, end >> 8 & 255, end & 255);
}
