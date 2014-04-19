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
#include "ip4_cidr.h"

const u_int32_t prefix_mask[] = {
	0x00000000,
	0x80000000,
	0xC0000000,
	0xE0000000,
	0xF0000000,
	0xF8000000,
	0xFC000000,
	0xFE000000,
	0xFF000000,
	0xFF800000,
	0xFFC00000,
	0xFFE00000,
	0xFFF00000,
	0xFFF80000,
	0xFFFC0000,
	0xFFFE0000,
	0xFFFF0000,
	0xFFFF8000,
	0xFFFFC000,
	0xFFFFE000,
	0xFFFFF000,
	0xFFFFF800,
	0xFFFFFC00,
	0xFFFFFE00,
	0xFFFFFF00,
	0xFFFFFF80,
	0xFFFFFFC0,
	0xFFFFFFE0,
	0xFFFFFFF0,
	0xFFFFFFF8,
	0xFFFFFFFC,
	0xFFFFFFFE,
	0xFFFFFFFF
};

u_int32_t
ip4_cidr_top_address(u_int32_t address, u_int8_t prefix) {
	return (address & prefix_mask[prefix]) | ~prefix_mask[prefix];
}


int
ip4_cidr_suffix(u_int32_t address) {
	int i;
	for(i=1; i < 32+1; i++){
		if(address & prefix_mask[i]) return (32-i);
	}
	return 0;
}

int
ip4_cidr_prefix(u_int32_t address) {
	int i;
	for (i = 32; i >= 0; i--) {
		if (address & ~prefix_mask[i]) return (i+1);
	}
	return 0;
}

void ip4_cidr_output(FILE *f, u_int32_t address, u_int8_t prefix)
{
        fprintf(f, "%d.%d.%d.%d/%d", address >> 24 & 255, address >> 16 & 255, address >> 8 & 255, address & 255, prefix);
}
