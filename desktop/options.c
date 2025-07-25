/*
 * Copyright 2003 Phil Mellor <monkeyson@users.sourceforge.net>
 * Copyright 2003 John M Bell <jmb202@ecs.soton.ac.uk>
 * Copyright 2004 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2005 Richard Wilson <info@tinct.net>
 *
 * This file is part of NetSurf, http://www.netsurf-browser.org/
 *
 * NetSurf is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * NetSurf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** \file
 * Option reading and saving (implementation).
 *
 * Options are stored in the format key:value, one per line. For bool options,
 * value is "0" or "1".
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>
#include "content/urldb.h"
#include "css/css.h"
#include "desktop/options.h"
#include "desktop/tree.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "utils/url.h"
#include "utils/utils.h"

#if defined(riscos)
#include "riscos/options.h"
#elif defined(nsgtk)
#include "gtk/options.h"
#elif defined(nsbeos)
#include "beos/options.h"
#elif defined(nsamiga)
#include "amiga/options.h"
#elif defined(nsmui)
#include "mui/options.h"
#elif defined(nsfb)
#include "framebuffer/fb_options.h"
#else
#define EXTRA_OPTION_DEFINE
#define EXTRA_OPTION_TABLE
#endif


/** An HTTP proxy should be used. */
bool option_http_proxy = false;
/** Hostname of proxy. */
char *option_http_proxy_host = 0;
/** Proxy port. */
int option_http_proxy_port = 8080;
/** Proxy authentication method. */
int option_http_proxy_auth = OPTION_HTTP_PROXY_AUTH_NONE;
/** Proxy authentication user name */
char *option_http_proxy_auth_user = 0;
/** Proxy authentication password */
char *option_http_proxy_auth_pass = 0;
/** Default font size / 0.1pt. */
int option_font_size = 128;
/** Minimum font size. */
int option_font_min_size = 85;
/** Default sans serif font */
char *option_font_sans;
/** Default serif font */
char *option_font_serif;
/** Default monospace font */
char *option_font_mono;
/** Default cursive font */
char *option_font_cursive;
/** Default fantasy font */
char *option_font_fantasy;
/** Accept-Language header. */
char *option_accept_language = 0;
/** Accept-Charset header. */
char *option_accept_charset = 0;
/** Preferred maximum size of memory cache / bytes. */
int option_memory_cache_size = 2 * 1024 * 1024;
/** Preferred expiry age of disc cache / days. */
int option_disc_cache_age = 28;
/** Whether to block advertisements */
bool option_block_ads = false;
/** Minimum GIF animation delay */
int option_minimum_gif_delay = 10;
/** Whether to send the referer HTTP header */
bool option_send_referer = true;
/** Whether to animate images */
bool option_animate_images = true;
/** How many days to retain URL data for */
int option_expire_url = 28;
/** Default font family */
int option_font_default = CSS_FONT_FAMILY_SANS_SERIF;
/** ca-bundle location */
char *option_ca_bundle = 0;
/** ca-path location */
char *option_ca_path = 0;
/** Cookie file location */
char *option_cookie_file = 0;
/** Cookie jar loaction */
char *option_cookie_jar = 0;
/** Home page location */
char *option_homepage_url = 0;
/** URL completion in url bar */
bool option_url_suggestion = true;
/** default x position of new windows */
int option_window_x = 0;
/** default y position of new windows */
int option_window_y = 0;
/** default width of new windows */
int option_window_width = 0;
/** default height of new windows */
int option_window_height = 0;
/** width of screen when above options were saved */
int option_window_screen_width = 0;
/** height of screen when above options were saved */
int option_window_screen_height = 0;
/** default size of status bar vs. h scroll bar */
#ifdef nsgtk
int option_toolbar_status_width = 400;
#else
int option_toolbar_status_width = 6667;
#endif
/** default window scale */
int option_scale = 100;
/* Whether to reflow web pages while objects are fetching */
bool option_incremental_reflow = true;
/* Minimum time between HTML reflows while objects are fetching */
#ifdef riscos
unsigned int option_min_reflow_period = 100; /* time in cs */
#else
unsigned int option_min_reflow_period = 25; /* time in cs */
#endif
/** top margin of exported page*/
int option_margin_top = DEFAULT_MARGIN_TOP_MM;
/** bottom margin of exported page*/
int option_margin_bottom = DEFAULT_MARGIN_BOTTOM_MM;
/** left margin of exported page*/
int option_margin_left = DEFAULT_MARGIN_LEFT_MM;
/** right margin of exported page*/
int option_margin_right = DEFAULT_MARGIN_RIGHT_MM;
/** scale of exported content*/
int option_export_scale = DEFAULT_EXPORT_SCALE * 100;
/**suppressing images in printed content*/
bool option_suppress_images = false;
/**turning off all backgrounds for printed content*/
bool option_remove_backgrounds = false;
/**turning on content loosening for printed content*/
bool option_enable_loosening = true;
/**compression of PDF documents*/
bool option_enable_PDF_compression = true;
/**setting a password and encoding PDF documents*/
bool option_enable_PDF_password = false;

/* Fetcher configuration */
/** Maximum simultaneous active fetchers */
int option_max_fetchers = 24;
/** Maximum simultaneous active fetchers per host.
 * (<=option_max_fetchers else it makes no sense)
 * Note that rfc2616 section 8.1.4 says that there should be no more than
 * two keepalive connections per host. None of the main browsers follow this
 * as it slows page fetches down considerably.
 * See https://bugzilla.mozilla.org/show_bug.cgi?id=423377#c4
 */
int option_max_fetchers_per_host = 5;
/** Maximum number of inactive fetchers cached.
 * The total number of handles netsurf will therefore have open
 * is this plus option_max_fetchers.
 */
int option_max_cached_fetch_handles = 6;
/** Suppress debug output from cURL. */
bool option_suppress_curl_debug = true;

/** Whether to allow target="_blank" */
bool option_target_blank = true;

/** Whether second mouse button opens in new tab */
bool option_button_2_tab = true;

EXTRA_OPTION_DEFINE


struct {
	const char *key;
	enum { OPTION_BOOL, OPTION_INTEGER, OPTION_STRING } type;
	void *p;
} option_table[] = {
	{ "http_proxy",		OPTION_BOOL,	&option_http_proxy },
	{ "http_proxy_host",	OPTION_STRING,	&option_http_proxy_host },
	{ "http_proxy_port",	OPTION_INTEGER,	&option_http_proxy_port },
	{ "http_proxy_auth",	OPTION_INTEGER,	&option_http_proxy_auth },
	{ "http_proxy_auth_user",
				OPTION_STRING,	&option_http_proxy_auth_user },
	{ "http_proxy_auth_pass",
				OPTION_STRING,	&option_http_proxy_auth_pass },
	{ "font_size",		OPTION_INTEGER,	&option_font_size },
	{ "font_min_size",	OPTION_INTEGER,	&option_font_min_size },
	{ "font_sans",		OPTION_STRING,	&option_font_sans },
	{ "font_serif",		OPTION_STRING,	&option_font_serif },
	{ "font_mono",		OPTION_STRING,	&option_font_mono },
	{ "font_cursive",	OPTION_STRING,	&option_font_cursive },
	{ "font_fantasy",	OPTION_STRING,	&option_font_fantasy },
	{ "accept_language",	OPTION_STRING,	&option_accept_language },
	{ "accept_charset",	OPTION_STRING,	&option_accept_charset },
	{ "memory_cache_size",	OPTION_INTEGER,	&option_memory_cache_size },
	{ "disc_cache_age",	OPTION_INTEGER,	&option_disc_cache_age },
	{ "block_advertisements",
				OPTION_BOOL,	&option_block_ads },
	{ "minimum_gif_delay",	OPTION_INTEGER,	&option_minimum_gif_delay },
	{ "send_referer",	OPTION_BOOL,	&option_send_referer },
	{ "animate_images",	OPTION_BOOL,	&option_animate_images },
	{ "expire_url",		OPTION_INTEGER,	&option_expire_url },
	{ "font_default",	OPTION_INTEGER,	&option_font_default },
	{ "ca_bundle",		OPTION_STRING,	&option_ca_bundle },
	{ "ca_path",		OPTION_STRING,	&option_ca_path },
	{ "cookie_file",	OPTION_STRING,	&option_cookie_file },
	{ "cookie_jar",		OPTION_STRING,	&option_cookie_jar },
        { "homepage_url",	OPTION_STRING,	&option_homepage_url },
	{ "url_suggestion",	OPTION_BOOL,	&option_url_suggestion },
	{ "window_x",		OPTION_INTEGER,	&option_window_x },
	{ "window_y",		OPTION_INTEGER,	&option_window_y },
	{ "window_width",	OPTION_INTEGER,	&option_window_width },
	{ "window_height",	OPTION_INTEGER,	&option_window_height },
	{ "window_screen_width",
				OPTION_INTEGER,	&option_window_screen_width },
	{ "window_screen_height",
				OPTION_INTEGER,	&option_window_screen_height },
	{ "toolbar_status_size",
				OPTION_INTEGER,	&option_toolbar_status_width },
	{ "scale",		OPTION_INTEGER,	&option_scale },
	{ "incremental_reflow",	OPTION_BOOL,	&option_incremental_reflow },
	{ "min_reflow_period",	OPTION_INTEGER,	&option_min_reflow_period },
	/* Fetcher options */
	{ "max_fetchers",	OPTION_INTEGER,	&option_max_fetchers },
	{ "max_fetchers_per_host",
				OPTION_INTEGER, &option_max_fetchers_per_host },
	{ "max_cached_fetch_handles",
			OPTION_INTEGER, &option_max_cached_fetch_handles },
	{ "suppress_curl_debug",OPTION_BOOL,	&option_suppress_curl_debug },
	{ "target_blank",	OPTION_BOOL,	&option_target_blank },
	{ "button_2_tab",	OPTION_BOOL,	&option_button_2_tab },
	/* PDF / Print options*/
	{ "margin_top",		OPTION_INTEGER,	&option_margin_top},
	{ "margin_bottom",	OPTION_INTEGER,	&option_margin_bottom},
	{ "margin_left",	OPTION_INTEGER,	&option_margin_left},
	{ "margin_right",	OPTION_INTEGER,	&option_margin_right},
 	{ "export_scale",	OPTION_INTEGER,	&option_export_scale},
	{ "suppress_images",	OPTION_BOOL,	&option_suppress_images},
	{ "remove_backgrounds",	OPTION_BOOL,	&option_remove_backgrounds},
	{ "enable_loosening",	OPTION_BOOL,	&option_enable_loosening},
 	{ "enable_PDF_compression",
 				OPTION_BOOL,	&option_enable_PDF_compression},
 	{ "enable_PDF_password",
 				OPTION_BOOL,	&option_enable_PDF_password},
	EXTRA_OPTION_TABLE
};

#define option_table_entries (sizeof option_table / sizeof option_table[0])


static void options_load_tree_directory(xmlNode *ul, struct node *directory);
static void options_load_tree_entry(xmlNode *li, struct node *directory);
xmlNode *options_find_tree_element(xmlNode *node, const char *name);
bool options_save_tree_directory(struct node *directory, xmlNode *node);
bool options_save_tree_entry(struct node *entry, xmlNode *node);


/**
 * Read options from a file.
 *
 * \param  path  name of file to read options from
 *
 * Option variables corresponding to lines in the file are updated. Missing
 * options are unchanged. If the file fails to open, options are unchanged.
 */

void options_read(const char *path)
{
	char s[100];
	FILE *fp;

	fp = fopen(path, "r");
	if (!fp) {
		LOG(("failed to open file '%s'", path));
		return;
	}

	while (fgets(s, 100, fp)) {
		char *colon, *value;
		unsigned int i;

		if (s[0] == 0 || s[0] == '#')
			continue;
		colon = strchr(s, ':');
		if (colon == 0)
			continue;
		s[strlen(s) - 1] = 0;  /* remove \n at end */
		*colon = 0;  /* terminate key */
		value = colon + 1;

		for (i = 0; i != option_table_entries; i++) {
			if (strcasecmp(s, option_table[i].key) != 0)
				continue;

			switch (option_table[i].type) {
				case OPTION_BOOL:
					*((bool *) option_table[i].p) =
							value[0] == '1';
					break;

				case OPTION_INTEGER:
					*((int *) option_table[i].p) =
							atoi(value);
					break;

				case OPTION_STRING:
					free(*((char **) option_table[i].p));
					*((char **) option_table[i].p) =
							strdup(value);
					break;
			}
			break;
		}
	}

	fclose(fp);

	if (option_font_size < 50)
		option_font_size = 50;
	if (1000 < option_font_size)
		option_font_size = 1000;
	if (option_font_min_size < 10)
		option_font_min_size = 10;
	if (500 < option_font_min_size)
		option_font_min_size = 500;

	if (option_memory_cache_size < 0)
		option_memory_cache_size = 0;
}


/**
 * Save options to a file.
 *
 * \param  path  name of file to write options to
 *
 * Errors are ignored.
 */

void options_write(const char *path)
{
	unsigned int i;
	FILE *fp;

	fp = fopen(path, "w");
	if (!fp) {
		LOG(("failed to open file '%s' for writing", path));
		return;
	}

	for (i = 0; i != option_table_entries; i++) {
		fprintf(fp, "%s:", option_table[i].key);
		switch (option_table[i].type) {
			case OPTION_BOOL:
				fprintf(fp, "%c", *((bool *) option_table[i].p) ?
						'1' : '0');
				break;

			case OPTION_INTEGER:
				fprintf(fp, "%i", *((int *) option_table[i].p));
				break;

			case OPTION_STRING:
				if (*((char **) option_table[i].p))
					fprintf(fp, "%s", *((char **) option_table[i].p));
				break;
		}
		fprintf(fp, "\n");
	}

	fclose(fp);
}

/**
 * Dump user options to stderr
 */
void options_dump(void)
{
	unsigned int i;

	for (i = 0; i != option_table_entries; i++) {
		fprintf(stderr, "%s:", option_table[i].key);
		switch (option_table[i].type) {
			case OPTION_BOOL:
				fprintf(stderr, "%c",
					*((bool *) option_table[i].p) ?
						'1' : '0');
				break;

			case OPTION_INTEGER:
				fprintf(stderr, "%i",
					*((int *) option_table[i].p));
				break;

			case OPTION_STRING:
				if (*((char **) option_table[i].p))
					fprintf(stderr, "%s",
						*((char **) option_table[i].p));
				break;
		}
		fprintf(stderr, "\n");
	}
}

/**
 * Loads a hotlist as a tree from a specified file.
 *
 * \param  filename  name of file to read
 * \return the hotlist file represented as a tree, or NULL on failure
 */
struct tree *options_load_tree(const char *filename) {
	xmlDoc *doc;
	xmlNode *html, *body, *ul;
	struct tree *tree;

	doc = htmlParseFile(filename, "iso-8859-1");
	if (!doc) {
		warn_user("HotlistLoadError", messages_get("ParsingFail"));
		return NULL;
	}

	html = options_find_tree_element((xmlNode *) doc, "html");
	body = options_find_tree_element(html, "body");
	ul = options_find_tree_element(body, "ul");
	if (!ul) {
		xmlFreeDoc(doc);
		warn_user("HotlistLoadError",
				"(<html>...<body>...<ul> not found.)");
		return NULL;
	}

	tree = calloc(sizeof(struct tree), 1);
	if (!tree) {
		xmlFreeDoc(doc);
		warn_user("NoMemory", 0);
		return NULL;
	}
	tree->root = tree_create_folder_node(NULL, "Root");
	if (!tree->root) {
		free(tree);
		xmlFreeDoc(doc);

		return NULL;
	}

	options_load_tree_directory(ul, tree->root);
	tree->root->expanded = true;
	tree_initialise(tree);

	xmlFreeDoc(doc);
	return tree;
}


/**
 * Parse a directory represented as a ul.
 *
 * \param  ul         xmlNode for parsed ul
 * \param  directory  directory to add this directory to
 */
void options_load_tree_directory(xmlNode *ul, struct node *directory) {
	char *title;
	struct node *dir;
	xmlNode *n;

	assert(ul);
	assert(directory);

	for (n = ul->children; n; n = n->next) {
		/* The ul may contain entries as a li, or directories as
		 * an h4 followed by a ul. Non-element nodes may be present
		 * (eg. text, comments), and are ignored. */

		if (n->type != XML_ELEMENT_NODE)
			continue;

		if (strcmp((const char *) n->name, "li") == 0) {
			/* entry */
			options_load_tree_entry(n, directory);

		} else if (strcmp((const char *) n->name, "h4") == 0) {
			/* directory */
			title = (char *) xmlNodeGetContent(n);
			if (!title) {
				warn_user("HotlistLoadError", "(Empty <h4> "
						"or memory exhausted.)");
				return;
			}

			for (n = n->next;
					n && n->type != XML_ELEMENT_NODE;
					n = n->next)
				;
			if (!n || strcmp((const char *) n->name, "ul") != 0) {
				/* next element isn't expected ul */
				free(title);
				warn_user("HotlistLoadError", "(Expected "
						"<ul> not present.)");
				return;
			}

			dir = tree_create_folder_node(directory, title);
			if (!dir) {
				free(title);

				return;
			}
			options_load_tree_directory(n, dir);
		}
	}
}


/**
 * Parse an entry represented as a li.
 *
 * \param  li         xmlNode for parsed li
 * \param  directory  directory to add this entry to
 */
void options_load_tree_entry(xmlNode *li, struct node *directory) {
	char *url = NULL, *url1 = NULL;
	char *title = NULL;
	struct node *entry;
	xmlNode *n;
	const struct url_data *data;
	url_func_result res;

	for (n = li->children; n; n = n->next) {
		/* The li must contain an "a" element */
		if (n->type == XML_ELEMENT_NODE &&
				strcmp((const char *) n->name, "a") == 0) {
			url1 = (char *) xmlGetProp(n, (const xmlChar *) "href");
			title = (char *) xmlNodeGetContent(n);
		}
	}

	if (!url1 || !title) {
		warn_user("HotlistLoadError", "(Missing <a> in <li> or "
				"memory exhausted.)");
		return;
	}

	/* We're loading external input. 
	 * This may be garbage, so attempt to normalise
	 */
	res = url_normalize(url1, &url);
	if (res != URL_FUNC_OK) {
		LOG(("Failed normalising '%s'", url1));

		if (res == URL_FUNC_NOMEM)
			warn_user("NoMemory", NULL);

		xmlFree(url1);
		xmlFree(title);

		return;
	}

	/* No longer need this */
	xmlFree(url1);

	data = urldb_get_url_data(url);
	if (!data) {
		/* No entry in database, so add one */
		urldb_add_url(url);
		/* now attempt to get url data */
		data = urldb_get_url_data(url);
	}
	if (!data) {
		xmlFree(title);
		free(url);

		return;
	}

	/* Make this URL persistent */
	urldb_set_url_persistence(url, true);

	if (!data->title)
		urldb_set_url_title(url, title);

	entry = tree_create_URL_node(directory, url, data, title);

	xmlFree(title);
	free(url);
}


/**
 * Search the children of an xmlNode for an element.
 *
 * \param  node  xmlNode to search children of, or 0
 * \param  name  name of element to find
 * \return  first child of node which is an element and matches name, or
 *          0 if not found or parameter node is 0
 */
xmlNode *options_find_tree_element(xmlNode *node, const char *name) {
	xmlNode *n;
	if (!node)
		return 0;
	for (n = node->children;
			n && !(n->type == XML_ELEMENT_NODE &&
			strcmp((const char *) n->name, name) == 0);
			n = n->next)
		;
	return n;
}


/**
 * Perform a save to a specified file
 *
 * /param  filename  the file to save to
 */
bool options_save_tree(struct tree *tree, const char *filename, const char *page_title) {
	int res;
	xmlDoc *doc;
	xmlNode *html, *head, *title, *body;

	/* Unfortunately the Browse Hotlist format is invalid HTML,
	 * so this is a lie. */
	doc = htmlNewDoc(
		(const xmlChar *) "http://www.w3.org/TR/html4/strict.dtd",
		(const xmlChar *) "-//W3C//DTD HTML 4.01//EN");
	if (!doc) {
		warn_user("NoMemory", 0);
		return false;
	}

	html = xmlNewNode(NULL, (const xmlChar *) "html");
	if (!html) {
		warn_user("NoMemory", 0);
		xmlFreeDoc(doc);
		return false;
	}
	xmlDocSetRootElement(doc, html);

	head = xmlNewChild(html, NULL, (const xmlChar *) "head", NULL);
	if (!head) {
		warn_user("NoMemory", 0);
		xmlFreeDoc(doc);
		return false;
	}

	title  = xmlNewTextChild(head, NULL, (const xmlChar *) "title",
			(const xmlChar *) page_title);
	if (!title) {
		warn_user("NoMemory", 0);
		xmlFreeDoc(doc);
		return false;
	}

	body = xmlNewChild(html, NULL, (const xmlChar *) "body", NULL);
	if (!body) {
		warn_user("NoMemory", 0);
		xmlFreeDoc(doc);
		return false;
	}

	if (!options_save_tree_directory(tree->root, body)) {
		warn_user("NoMemory", 0);
		xmlFreeDoc(doc);
		return false;
	}

	doc->charset = XML_CHAR_ENCODING_UTF8;
	res = htmlSaveFileEnc(filename, doc, "iso-8859-1");
	if (res == -1) {
		warn_user("HotlistSaveError", 0);
		xmlFreeDoc(doc);
		return false;
	}

	xmlFreeDoc(doc);
	return true;
}


/**
 * Add a directory to the HTML tree for saving.
 *
 * \param  directory  hotlist directory to add
 * \param  node       node to add ul to
 * \return  true on success, false on memory exhaustion
 */
bool options_save_tree_directory(struct node *directory, xmlNode *node) {
	struct node *child;
	xmlNode *ul, *h4;

	ul = xmlNewChild(node, NULL, (const xmlChar *) "ul", NULL);
	if (!ul)
		return false;

	for (child = directory->child; child; child = child->next) {
		if (!child->folder) {
			/* entry */
			if (!options_save_tree_entry(child, ul))
				return false;
		} else {
			/* directory */
			/* invalid HTML */
			h4 = xmlNewTextChild(ul, NULL,
					(const xmlChar *) "h4",
					(const xmlChar *) child->data.text);
			if (!h4)
				return false;

			if (!options_save_tree_directory(child, ul))
				return false;
		}	}

	return true;
}


/**
 * Add an entry to the HTML tree for saving.
 *
 * The node must contain a sequence of node_elements in the following order:
 *
 * \param  entry  hotlist entry to add
 * \param  node   node to add li to
 * \return  true on success, false on memory exhaustion
 */
bool options_save_tree_entry(struct node *entry, xmlNode *node) {
	xmlNode *li, *a;
	xmlAttr *href;
	struct node_element *element;

	li = xmlNewChild(node, NULL, (const xmlChar *) "li", NULL);
	if (!li)
		return false;

	a = xmlNewTextChild(li, NULL, (const xmlChar *) "a",
			(const xmlChar *) entry->data.text);
	if (!a)
		return false;

	element = tree_find_element(entry, TREE_ELEMENT_URL);
	if (!element)
		return false;
	href = xmlNewProp(a, (const xmlChar *) "href",
			(const xmlChar *) element->text);
	if (!href)
		return false;
	return true;
}
