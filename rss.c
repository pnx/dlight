/* rss.c
 *
 *   Copyright (C) 2011       Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *   MA 02110-1301, USA.
 */
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "rss.h"

/* Sidestep warnings about signedness (xmlChar = unsigned char) */
#define xmlStrcmp(a, b) xmlStrcmp((xmlChar *)a, (xmlChar *)b)
#define xmlGetProp(n, p) xmlGetProp(n, (xmlChar *) p)

struct __walk_info {
	xmlNodePtr current;
};

struct __rssdoc {
	xmlDocPtr  xmldoc;
	xmlNodePtr channel;
	xmlNodePtr firstitem;
};

struct __rss {
	struct __rssdoc doc;
	struct __walk_info info;
};

static xmlNodePtr getchild(xmlNodePtr node, const char *name) {

	if (node) {
		xmlNodePtr it;

		for(it = node->children; it; it = it->next) {
			if (!xmlStrcmp(it->name, name))
				return it;
		}
	}
	return NULL;
}

static const char* getnodetext(xmlNodePtr node) {

	if (node) {
		if (node->type == XML_ELEMENT_NODE)
			node = node->children;
		if (node->type == XML_TEXT_NODE)
			return (const char *) node->content;
	}
	return "";
}

static int validate(struct __rssdoc *doc) {

	xmlChar *attr;
	xmlNodePtr node;

	if (!doc->xmldoc)
		return -1;

	node = doc->xmldoc->children;

	if (xmlStrcmp(node->name, "rss"))
		return -1;
	attr = xmlGetProp(node, "version");
	if (!attr)
		return -1;

	/* get channel node */
	node = xmlFirstElementChild(node);

	if (!node || xmlStrcmp(node->name, "channel"))
		return -1;
	doc->channel = node;

	/* get first item */
	node = getchild(node, "item");
	if (!node)
		return -1;

	doc->firstitem = node;

	while(node) {
		if (xmlStrcmp(node->name, "item"))
			return -1;

		node = xmlNextElementSibling(node);
	}
	return 0;
}

rss_t rss_parse(void *buf, size_t size) {

	rss_t rss = malloc(sizeof(struct __rss));

	rss->doc.xmldoc = xmlReadMemory(buf, size, "noname.xml", NULL, 0);

	if (validate(&rss->doc) < 0) {
		rss_free(rss);
		return NULL;
	}

	rss->info.current = rss->doc.firstitem;

	return rss;
}

void rss_free(rss_t r) {

	if (!r)
		return;
	if (r->doc.xmldoc)
		xmlFreeDoc(r->doc.xmldoc);
	free(r);
}

int rss_walk_next(rss_t rss, struct rss_item *item) {

	if (rss && rss->info.current) {
		/* fill item */
		xmlNodePtr cur = rss->info.current;
		item->title = getnodetext(getchild(cur, "title"));
		item->link = getnodetext(getchild(cur, "link"));

		rss->info.current = xmlNextElementSibling(rss->info.current);
		return 1;
	}
	return 0;
}

int rss_walk_reset(rss_t rss) {

	if (rss) {
		rss->info.current = rss->doc.firstitem;
		return 1;
	}
	return 0;
}
