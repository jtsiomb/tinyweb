/* tinyweb - tiny web server library and daemon
 * Author: John Tsiombikas <nuclear@member.fsf.org>
 *
 * This program is placed in the public domain. Feel free to use it any
 * way you like. Mentions and retaining this attribution header will be
 * appreciated, but not required.
 */
#include <stdlib.h>
#include <string.h>
#include "mime.h"
#include "rbtree.h"

/* TODO: do proper content detection */
struct mime_type {
	const char *suffix, *type;
};

static struct mime_type def_types[] = {
	{"txt", "text/plain"},
	{"htm", "text/html"},
	{"html", "text/html"},
	{"png", "image/png"},
	{"jpg", "image/jpeg"},
	{"jpeg", "image/jpeg"},
	{"gif", "image/gif"},
	{"bmp", "image/bmp"},
	{"cgi", 0},
	{0, 0}
};

static int init_types(void);
static void del_func(struct rbnode *node, void *cls);

static struct rbtree *types;

static int init_types(void)
{
	int i;

	if(types) return 0;

	if(!(types = rb_create(RB_KEY_STRING))) {
		return -1;
	}
	rb_set_delete_func(types, del_func, 0);

	for(i=0; def_types[i].suffix; i++) {
		add_mime_type(def_types[i].suffix, def_types[i].type);
	}
	return 0;
}

static void del_func(struct rbnode *node, void *cls)
{
	free(node->key);
	free(node->data);
}

int add_mime_type(const char *suffix, const char *type)
{
	init_types();

	return rb_insert(types, strdup(suffix), type ? strdup(type) : 0);
}

const char *mime_type(const char *path)
{
	const char *suffix;

	init_types();

	if((suffix = strrchr(path, '.'))) {
		struct rbnode *node = rb_find(types, (void*)(suffix + 1));
		if(node) {
			return node->data;
		}
	}
	return "text/plain";
}
