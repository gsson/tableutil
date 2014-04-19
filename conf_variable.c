
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
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "ip4_range.h"
#include "table_fileop.h"
#include "conf_variable.h"


#define MAXTOKLEN 32

#define TAILQ_REASSIGN(head, head2, field) do {			\
	(head2)->tqh_first = (head)->tqh_first;	\
	(head2)->tqh_last = (head)->tqh_last;	\
	if ((head2)->tqh_first != NULL)	\
		(head2)->tqh_first->field.tqe_prev = &(head2)->tqh_first;	\
	(head)->tqh_first = NULL;	\
	(head)->tqh_last = &(head)->tqh_first;	\
	} while(0)

/*
 * Note: The variable_* functions disposes the pointers used as
 * arguments. This kind of non-standard behavior was implemented to try
 * to minimise the memory usage and the amount of memory copies.
 * variable_list_variable_create takes care of the name pointer.
 * 
 * This behaviour has as a side-effect that all 
 */


void
variable_list_destroy(variable_list_t *list) {
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

void
variable_list_init(variable_list_t *list) {
	TAILQ_INIT(list);
}

/* Note: Does not free 'c' */
struct variable *
variable_list_find(variable_list_t *vlist, char *c) {
	struct variable_list_element *element;
	if (TAILQ_EMPTY(vlist)) {
		return NULL;
	}
	
	TAILQ_FOREACH(element, vlist, variable_list_links) {
		if (strncmp(c, element->v->name, MAXTOKLEN) == 0) {
			return element->v;
		}
		if (strncmp(c, element->v->name, MAXTOKLEN) < 0) {
			return NULL;
		}
	}
	return NULL;
}

struct variable *
variable_list_variable_create(variable_list_t *vlist, char *c) {
	struct variable_list_element *element;
	
	if (TAILQ_EMPTY(vlist)) {
		element = malloc(sizeof(struct variable_list_element));
		if (element == NULL) {
			fprintf(stderr, "Allocation error\n");
			exit(-1);
			/* NOTREACHED */
		}
		element->v = variable_create();
		element->v->name = c;
		element->v->temporary = 0;
		TAILQ_INSERT_HEAD(vlist, element, variable_list_links);
		return element->v;
	}

	if (strncmp(c, TAILQ_LAST(vlist, variable_list)->v->name, MAXTOKLEN) > 0) {
		element = malloc(sizeof(struct variable_list_element));
		if (element == NULL) {
			fprintf(stderr, "Allocation error\n");
			exit(-1);
			/* NOTREACHED */
		}
		element->v = variable_create();
		element->v->name = c;
		element->v->temporary = 0;

		TAILQ_INSERT_TAIL(vlist, element, variable_list_links);
		return element->v;		
	}
	
	TAILQ_FOREACH(element, vlist, variable_list_links) {
		if (strncmp(c, element->v->name, MAXTOKLEN) < 0) {
			struct variable_list_element *new_element;
			new_element = malloc(sizeof(struct variable_list_element));
			if (new_element == NULL) {
				fprintf(stderr, "Allocation error\n");
				exit(-1);
				/* NOTREACHED */
			}
			new_element->v = variable_create();
			new_element->v->name = c;
			new_element->v->temporary = 0;

			TAILQ_INSERT_BEFORE(element, new_element, variable_list_links);
			return new_element->v;
		}
	}
	
	fprintf(stderr, "*dies*\n");
	exit(-1);
	/* NOTREACHED */
}

struct variable *
variable_find(variable_list_t *vlist, char *c) {
	struct variable *v;
	v = variable_list_find(vlist, c);
	if (v != NULL) {
		free(c);
		return v;
	}
	
	v = variable_list_variable_create(vlist, c);
	return v;
}

struct variable *
variable_dup(struct variable *var) {

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


struct variable *
variable_assign(variable_list_t *vlist, char *c, struct variable *src) {
	struct variable *dst = variable_find(vlist, c);

	if (src->temporary) {
		ip4_range_list_destroy(&(dst->table));
		TAILQ_REASSIGN(&(src->table), &(dst->table), ip4_range_list_links);
		variable_destroy(src);
	}	
	else {
		ip4_range_list_dup(&(src->table), &(dst->table));
	}
	
	return dst;
}

struct variable *
variable_get(variable_list_t *vlist, char *c) {
	if (c != NULL) {
		struct variable *v;
		v = variable_list_find(vlist, c);
		if (v != NULL) {
			free(c);
			return v;
		}
		
		fprintf(stderr, "Variable '%s' does not exist.\n", c);
		
		return variable_list_variable_create(vlist, c);
	}
	return NULL;
}

void 
variable_destroy(struct variable *var) {
	if (var != NULL) {
		ip4_range_list_destroy(&(var->table));
		if (var->name != NULL) {
			free(var->name);
		}
		free(var);
	}
}

struct variable *
variable_create() {
	struct variable *var = malloc(sizeof(struct variable));
	if (var == NULL) {
		fprintf(stderr, "Allocation error\n");
		exit(-1);
		/* NOTREACHED */
	}
	TAILQ_INIT(&(var->table));
	var->temporary=1;
	var->name = NULL;
	return var;
}

struct variable *
variable_insert_range(struct variable *list, char *address) {
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
	free(address);
	return list;
}

struct variable *
variable_insert_cidr(struct variable *list, char *address) {
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
	free(address);
	return list;
}

struct variable *
variable_insert_hostname(struct variable *list, char *hostname) {

	struct addrinfo *ai_result;
	struct addrinfo *ai;
	struct sockaddr_in *sa;
	
	int result;
	struct addrinfo hint;
	
	memset(&hint, 0, sizeof(struct addrinfo));
	
	hint.ai_family = AF_INET;
	hint.ai_socktype = SOCK_STREAM;
	
	result = getaddrinfo(hostname, NULL, &hint, &ai_result);
	if (result) {
		fprintf(stderr, "Host lookup for host '%s' failed: %s\n", hostname, gai_strerror(result));
		free(hostname);
		return list;
	}
	
	for (ai = ai_result; ai; ai = ai->ai_next) {
		sa = (struct sockaddr_in *) ai->ai_addr;
		ip4_range_list_insert_cidr(&(list->table), ntohl(sa->sin_addr.s_addr), 32, NULL);
	}
	
	freeaddrinfo(ai_result);
	free(hostname);
	return list;
}

struct variable *
variable_insert_single(struct variable *list, char *address) {
    int 			a, b, c, d;
    
	if (sscanf(address, "%d.%d.%d.%d", &a, &b, &c, &d) == 4 &&
			a >= 0 && a < 256 &&
			b >= 0 && b < 256 &&
			c >= 0 && c < 256 &&
			d >= 0 && d < 256) {
			ip4_range_list_insert_cidr(&(list->table), IP(a,b,c,d), 32, NULL);
	}
	free(address);
	return list;
}

struct variable *
variable_load_text(char *file) {
	struct variable *var = NULL;

	var = variable_create();
	ip4_text_load(file, &(var->table));
	if (file != NULL) free(file);
	return var;
}

struct variable *
variable_load_p2b(char *file) {
	struct variable *var = NULL;

	var = variable_create();
	ip4_p2b_load(file, &(var->table));
	if (file != NULL) free(file);
	return var;
}

struct variable *
variable_union(struct variable  *lhs, struct variable  *rhs) {
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

struct variable *
variable_intersect(struct variable  *lhs, struct variable  *rhs) {
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

struct variable *
variable_difference(struct variable  *lhs, struct variable  *rhs) {
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

struct variable *
variable_invert(struct variable *rhs) {
	struct variable *var = NULL;
	
	var = variable_create();
	ip4_range_list_invert(&(rhs->table), &(var->table));
	
	if (rhs->temporary) {
		variable_destroy(rhs);
	}
	
	return var;
}

void
variable_save_cidr(char *file, struct variable *rhs) {
	ip4_cidr_save(file, &(rhs->table));
	
	if (rhs->temporary) {
		variable_destroy(rhs);
	}
	if (file != NULL) free(file);
	return;
}

void 
variable_save_range(char *file, struct variable *rhs) {
	ip4_range_save(file, &(rhs->table));
	
	if (rhs->temporary) {
		variable_destroy(rhs);
	}
	if (file != NULL) free(file);
	return;
}

void 
variable_save_single(char *file, struct variable *rhs) {
	ip4_single_save(file, &(rhs->table));
	
	if (rhs->temporary) {
		variable_destroy(rhs);
	}
	if (file != NULL) free(file);
	return;
}
