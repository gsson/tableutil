/* $Id: conf_variable.h,v 1.3 2005/07/07 19:02:06 gsson Exp $ */

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

#ifndef _CONF_VARIABLE_H_
#define _CONF_VARIABLE_H_

#include "types.h"

extern variable_list_t variable_list;

void variable_list_init(variable_list_t *list);
void variable_list_destroy(variable_list_t *list);
struct variable *variable_list_find(variable_list_t *vlist, char *c);
struct variable *variable_list_variable_create(variable_list_t *vlist, char *c);

struct variable *variable_find(char *c);
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
variable_list_t variable_list;

#endif /*_CONF_VARIABLE_H_*/
