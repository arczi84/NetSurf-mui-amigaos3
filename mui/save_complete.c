/*
 * Copyright 2004 John M Bell <jmb202@ecs.soton.ac.uk>
 * Copyright 2004-2007 James Bursa <bursa@users.sourceforge.net>
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
 * Save HTML document with dependencies (implementation).
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <exec/memory.h>/MEMF_ANY

#include <regex.h>
#include <libxml/HTMLtree.h>
#include <libxml/parserInternals.h>

#include <proto/dos.h>
#include <proto/icon.h>

#include "css/css.h"
#include "mui/extrasrc.h"
#include "mui/save_complete.h"
#include "mui/utils.h"
#include "render/box.h"
#include "utils/log.h"
#include "utils/url.h"
#include "utils/utils.h"
#include "utils/config.h"

regex_t save_complete_import_re;

/** An entry in save_complete_list. */
struct save_complete_entry {
	struct content *content;
	struct save_complete_entry *next; /**< Next entry in list */
};

/** List of urls seen and saved so far. */
static struct save_complete_entry *save_complete_list = 0;

static bool save_complete_html(struct content *c, const char *path,
		bool index);
static bool save_imported_sheets(struct content *c, const char *path);
static char * rewrite_stylesheet_urls(const char *source, unsigned int size,
		int *osize, const char *base);
static bool rewrite_document_urls(xmlDoc *doc, const char *base);
static bool rewrite_urls(xmlNode *n, const char *base);
static bool rewrite_url(xmlNode *n, const char *attr, const char *base);
static bool save_complete_list_add(struct content *content);
static struct content * save_complete_list_find(const char *url);
static bool save_complete_list_check(struct content *content);
/* static void save_complete_list_dump(void); */
static bool save_complete_inventory(const char *path);

/**
 * Save an HTML page with all dependencies.
 *
 * \param  c     CONTENT_HTML to save
 * \param  path  directory to save to (must exist)
 * \return  true on success, false on error and error reported
 */

bool save_complete(struct content *c, const char *path)
{
	bool result;

	result = save_complete_html(c, path, true);

	if (result)
		result = save_complete_inventory(path);

	/* free save_complete_list */
	while (save_complete_list) {
		struct save_complete_entry *next = save_complete_list->next;
		free(save_complete_list);
		save_complete_list = next;
	}

	return result;
}


/**
 * Save an HTML page with all dependencies, recursing through imported pages.
 *
 * \param  c      CONTENT_HTML to save
 * \param  path   directory to save to (must exist)
 * \param  index  true to save as "index"
 * \return  true on success, false on error and error reported
 */

bool save_complete_html(struct content *c, const char *path, bool index)
{
	char spath[256];
	unsigned int i;
	htmlParserCtxtPtr parser;
	BPTR fh = 0;

	if (c->type != CONTENT_HTML)
		return false;

	if (save_complete_list_check(c))
		return true;

	/* save stylesheets, ignoring the base and adblocking sheets */
	for (i = STYLESHEET_STYLE; i != c->data.html.stylesheet_count; i++) {
		struct content *css = c->data.html.stylesheet_content[i];
		char *source;
		int source_len;

		if (!css)
			continue;
		if (save_complete_list_check(css))
			continue;

		if (i != STYLESHEET_STYLE) {
			if (!save_complete_list_add(css)) {
				warn_user("NoMemory", 0);
				return false;
			}
		}

		if (!save_imported_sheets(css, path))
			return false;

		if (i == STYLESHEET_STYLE)
			continue; /* don't save <style> elements */

		snprintf(spath, sizeof spath, "%s/%x", path,
				(unsigned int) css);
		source = rewrite_stylesheet_urls(css->source_data,
				css->source_size, &source_len, css->url);
		if (!source) {
			warn_user("NoMemory", 0);
			return false;
		}

/*
		error = xosfile_save_stamped(spath, 0xf79, source,
					source + source_len);
*/

		if(fh = Open(spath,MODE_NEWFILE))
		{
			Write(fh,source,source_len);
			Close(fh);
			SetComment(spath,c->url);
		}

		free(source);
/*
		if (error) {
			LOG(("xosfile_save_stamped: 0x%x: %s",
					error->errnum, error->errmess));
			warn_user("SaveError", error->errmess);
			return false;
		}
*/
	}

	/* save objects */
	for (i = 0; i != c->data.html.object_count; i++) {
		struct content *obj = c->data.html.object[i].content;

		/* skip difficult content types */
		if (!obj || obj->type >= CONTENT_OTHER || !obj->source_data)
			continue;
		if (save_complete_list_check(obj))
			continue;

		if (!save_complete_list_add(obj)) {
			warn_user("NoMemory", 0);
			return false;
		}

		if (obj->type == CONTENT_HTML) {
			if (!save_complete_html(obj, path, false))
				return false;
			continue;
		}

		snprintf(spath, sizeof spath, "%s/%x", path,
				(unsigned int) obj);
/*
		error = xosfile_save_stamped(spath,
				ro_content_filetype(obj),
				obj->source_data,
				obj->source_data + obj->source_size);
		if (error) {
			LOG(("xosfile_save_stamped: 0x%x: %s",
					error->errnum, error->errmess));
			warn_user("SaveError", error->errmess);
			return false;
		}
*/
		if(fh = Open(spath,MODE_NEWFILE))
		{
			Write(fh,obj->source_data,obj->source_size);
			Close(fh);
			SetComment(spath,obj->url);
		}

	}

	/*save_complete_list_dump();*/

	/* make a copy of the document tree */
	parser = htmlCreateMemoryParserCtxt(c->source_data, c->source_size);
	if (!parser) {
		warn_user("NoMemory", 0);
		return false;
	}
	/* set parser charset */
	if (c->data.html.encoding) {
		xmlCharEncodingHandler *enc_handler;
		enc_handler =
			xmlFindCharEncodingHandler(c->data.html.encoding);
		if (enc_handler) {
			xmlCtxtResetLastError(parser);
			if (xmlSwitchToEncoding(parser, enc_handler)) {
				xmlFreeDoc(parser->myDoc);
				htmlFreeParserCtxt(parser);
				warn_user("MiscError",
						"Encoding switch failed");
				return false;
			}
		}
	}

	htmlParseDocument(parser);

	/* rewrite all urls we know about */
	if (!rewrite_document_urls(parser->myDoc, c->data.html.base_url)) {
		xmlFreeDoc(parser->myDoc);
		htmlFreeParserCtxt(parser);
		warn_user("NoMemory", 0);
		return false;
	}

	/* save the html file out last of all */
	if (index) {
		struct DiskObject *dobj;

		dobj = GetDiskObject("PROGDIR:NetSurf");

		if (dobj) {
			BPTR lock;

			lock = Lock(path, ACCESS_READ);

			if (lock) {
				BPTR progdirlock;
				STRPTR deftool;
				ULONG length = 0; /* shut up stupid gcc */

				lock = CurrentDir(lock);

				progdirlock = GetProgramDir();
				deftool = NULL;

				if (progdirlock) {
					STRPTR s = GetNameFromLock(progdirlock);

					if (s) {
						STRPTR p;

						length = strlen(s) + sizeof("NetSurf") + 4;

						deftool = AllocMem(length, MEMF_ANY);

						if (deftool) {
							strcpy(deftool, s);
							#warning fixme
							AddPart(deftool, "NetSurf", length);
							dobj->do_DefaultTool = deftool;
						}

						FreeVecTaskPooled(s);
					}
				}

				PutDiskObject("index", dobj);
				UnLock(CurrentDir(lock));

				if (deftool)
					FreeMem(deftool, length);
			}

			FreeDiskObject(dobj);
		}
	} else {
		snprintf(spath, sizeof spath, "%s/%x", path, (unsigned int)c);
	}

	errno = 0;
	if (htmlSaveFileFormat(spath, parser->myDoc, 0, 0) == -1) {
		if (errno)
			warn_user("SaveError", strerror(errno));
		else
			warn_user("SaveError", "htmlSaveFileFormat failed");
		return false;
	}

/*
	error = xosfile_set_type(spath, 0xfaf);
	if (error) {
		LOG(("xosfile_set_type: 0x%x: %s",
				error->errnum, error->errmess));
		warn_user("SaveError", error->errmess);
		return false;
	}
*/

	xmlFreeDoc(parser->myDoc);
	htmlFreeParserCtxt(parser);

	return true;
}


/**
 * Save stylesheets imported by a CONTENT_CSS.
 *
 * \param  c     a CONTENT_CSS
 * \param  path  path to save to
 * \return  true on success, false on error and error reported
 */

bool save_imported_sheets(struct content *c, const char *path)
{
	char spath[256];
	unsigned int j;
	char *source;
	int source_len;
	BPTR fh = 0;

	for (j = 0; j != c->data.css.import_count; j++) {
		struct content *css = c->data.css.import_content[j];

		if (!css)
			continue;
		if (save_complete_list_check(css))
			continue;

		if (!save_complete_list_add(css)) {
			warn_user("NoMemory", 0);
			return false;
		}

		if (!save_imported_sheets(css, path))
			return false;

		snprintf(spath, sizeof spath, "%s/%x", path,
				(unsigned int) css);
		source = rewrite_stylesheet_urls(css->source_data,
				css->source_size, &source_len, css->url);
		if (!source) {
			warn_user("NoMemory", 0);
			return false;
		}

		if(fh = Open(spath,MODE_NEWFILE))
		{
			Write(fh,source,source_len);
			Close(fh);
			SetComment(spath,c->url);
		}
/*
		error = xosfile_save_stamped(spath, 0xf79, source,
					source + source_len);
*/
		free(source);
/*
		if (error) {
			LOG(("xosfile_save_stamped: 0x%x: %s",
					error->errnum, error->errmess));
			warn_user("SaveError", error->errmess);
			return false;
		}
*/
	}

	return true;
}


/**
 * Initialise the save_complete module.
 */

void save_complete_init(void)
{
	/* Match an @import rule - see CSS 2.1 G.1. */
	regcomp_wrapper(&save_complete_import_re,
			"@import"		/* IMPORT_SYM */
			"[ \t\r\n\f]*"		/* S* */
			/* 1 */
			"("			/* [ */
			/* 2 3 */
			"\"(([^\"]|[\\]\")*)\""	/* STRING (approximated) */
			"|"
			/* 4 5 */
			"'(([^']|[\\]')*)'"
			"|"			/* | */
			"url\\([ \t\r\n\f]*"	/* URI (approximated) */
			     /* 6 7 */
			     "\"(([^\"]|[\\]\")*)\""
			     "[ \t\r\n\f]*\\)"
			"|"
			"url\\([ \t\r\n\f]*"
			    /* 8 9 */
			     "'(([^']|[\\]')*)'"
			     "[ \t\r\n\f]*\\)"
			"|"
			"url\\([ \t\r\n\f]*"
			   /* 10 */
			     "([^) \t\r\n\f]*)"
			     "[ \t\r\n\f]*\\)"
			")",			/* ] */
			REG_EXTENDED | REG_ICASE);
}


/**
 * Rewrite stylesheet \@import rules for save complete.
 *
 * @param  source  stylesheet source
 * @param  size    size of source
 * @param  osize   updated with the size of the result
 * @param  base    url of stylesheet
 * @return  converted source, or 0 on out of memory
 */

char * rewrite_stylesheet_urls(const char *source, unsigned int size,
		int *osize, const char *base)
{
	char *res;
	const char *url;
	char *url2;
	char buf[20];
	unsigned int offset = 0;
	int url_len = 0;
	struct content *content;
	int m;
	unsigned int i;
	unsigned int imports = 0;
	regmatch_t match[11];
	url_func_result result;

	/* count number occurences of @import to (over)estimate result size */
	/* can't use strstr because source is not 0-terminated string */
	for (i = 0; 7 < size && i != size - 7; i++) {
		if (source[i] == '@' &&
				tolower(source[i + 1]) == 'i' &&
				tolower(source[i + 2]) == 'm' &&
				tolower(source[i + 3]) == 'p' &&
				tolower(source[i + 4]) == 'o' &&
				tolower(source[i + 5]) == 'r' &&
				tolower(source[i + 6]) == 't')
			imports++;
	}

	res = malloc(size + imports * 20);
	if (!res)
		return 0;
	*osize = 0;

	while (offset < size) {
		m = regexec(&save_complete_import_re, source + offset,
				11, match, 0);
		if (m)
			break;

		/*for (unsigned int i = 0; i != 11; i++) {
			if (match[i].rm_so == -1)
				continue;
			fprintf(stderr, "%i: '%.*s'\n", i,
					match[i].rm_eo - match[i].rm_so,
					source + offset + match[i].rm_so);
		}*/

		url = 0;
		if (match[2].rm_so != -1) {
			url = source + offset + match[2].rm_so;
			url_len = match[2].rm_eo - match[2].rm_so;
		} else if (match[4].rm_so != -1) {
			url = source + offset + match[4].rm_so;
			url_len = match[4].rm_eo - match[4].rm_so;
		} else if (match[6].rm_so != -1) {
			url = source + offset + match[6].rm_so;
			url_len = match[6].rm_eo - match[6].rm_so;
		} else if (match[8].rm_so != -1) {
			url = source + offset + match[8].rm_so;
			url_len = match[8].rm_eo - match[8].rm_so;
		} else if (match[10].rm_so != -1) {
			url = source + offset + match[10].rm_so;
			url_len = match[10].rm_eo - match[10].rm_so;
		}
		assert(url);

		url2 = strndup(url, url_len);
		if (!url2) {
			free(res);
			return 0;
		}
		result = url_join(url2, base, (char**)&url);
		free(url2);
		if (result == URL_FUNC_NOMEM) {
			free(res);
			return 0;
		}

		/* copy data before match */
		memcpy(res + *osize, source + offset, match[0].rm_so);
		*osize += match[0].rm_so;

		if (result == URL_FUNC_OK) {
			content = save_complete_list_find(url);
			if (content) {
				/* replace import */
				snprintf(buf, sizeof buf, "@import '%x'",
						(unsigned int) content);
				memcpy(res + *osize, buf, strlen(buf));
				*osize += strlen(buf);
			} else {
				/* copy import */
				memcpy(res + *osize, source + offset + match[0].rm_so,
					match[0].rm_eo - match[0].rm_so);
				*osize += match[0].rm_eo - match[0].rm_so;
			}
		}
		else {
			/* copy import */
			memcpy(res + *osize, source + offset + match[0].rm_so,
				match[0].rm_eo - match[0].rm_so);
			*osize += match[0].rm_eo - match[0].rm_so;
		}

		assert(0 < match[0].rm_eo);
		offset += match[0].rm_eo;
	}

	/* copy rest of source */
	if (offset < size) {
		memcpy(res + *osize, source + offset, size - offset);
		*osize += size - offset;
	}

	return res;
}


/**
 * Rewrite URLs in a HTML document to be relative.
 *
 * \param  doc   root of the document tree
 * \param  base  base url of document
 * \return  true on success, false on out of memory
 */

bool rewrite_document_urls(xmlDoc *doc, const char *base)
{
	xmlNode *node;

	for (node = doc->children; node; node = node->next)
		if (node->type == XML_ELEMENT_NODE)
			if (!rewrite_urls(node, base))
				return false;

	return true;
}


/**
 * Traverse tree, rewriting URLs as we go.
 *
 * \param  n     xmlNode of type XML_ELEMENT_NODE to rewrite
 * \param  base  base url of document
 * \return  true on success, false on out of memory
 *
 * URLs in the tree rooted at element n are rewritten.
 */

bool rewrite_urls(xmlNode *n, const char *base)
{
	xmlNode *child;

	assert(n->type == XML_ELEMENT_NODE);

	/**
	 * We only need to consider the following cases:
	 *
	 * Attribute:      Elements:
	 *
	 * 1)   data         <object>
	 * 2)   href         <a> <area> <link>
	 * 3)   src          <script> <input> <frame> <iframe> <img>
	 * 4)   n/a          <style>
	 * 5)   n/a          any <base> tag
	 * 6)   background   any (except those above)
	 */
	if (!n->name) {
		/* ignore */
	}
	/* 1 */
	else if (strcmp(n->name, "object") == 0) {
		if (!rewrite_url(n, "data", base))
			return false;
	}
	/* 2 */
	else if (strcmp(n->name, "a") == 0 ||
			strcmp(n->name, "area") == 0 ||
			strcmp(n->name, "link") == 0) {
		if (!rewrite_url(n, "href", base))
			return false;
	}
	/* 3 */
	else if (strcmp(n->name, "frame") == 0 ||
			strcmp(n->name, "iframe") == 0 ||
			strcmp(n->name, "input") == 0 ||
			strcmp(n->name, "img") == 0 ||
			strcmp(n->name, "script") == 0) {
		if (!rewrite_url(n, "src", base))
			return false;
	}
	/* 4 */
	else if (strcmp(n->name, "style") == 0) {
		unsigned int len;
		xmlChar *content;

		for (child = n->children; child != 0; child = child->next) {
			/* Get current content */
			content = xmlNodeGetContent(child);
			if (!content)
				/* unfortunately we don't know if this is
				 * due to memory exhaustion, or because
				 * there is no content for this node */
				continue;

			/* Rewrite @import rules */
			char *rewritten = rewrite_stylesheet_urls(
					content,
					strlen((char*)content),
					&len, base);
			xmlFree(content);
			if (!rewritten)
				return false;

			/* set new content */
			xmlNodeSetContentLen(child,
					(const xmlChar*)rewritten,
					len);
		}

		return true;
	}
	/* 5 */
	else if (strcmp(n->name, "base") == 0) {
		/* simply remove any <base> tags from the document */
		xmlUnlinkNode(n);
		xmlFreeNode(n);
		/* base tags have no content, so there's no point recursing
		 * additionally, we've just destroyed this node, so trying
		 * to recurse would result in bad things happening */
		return true;
	}
	/* 6 */
	else {
	        if (!rewrite_url(n, "background", base))
	                return false;
	}

	/* now recurse */
	for (child = n->children; child;) {
		/* we must extract the next child now, as if the current
		 * child is a <base> element, it will be removed from the
		 * tree (see 5, above), thus preventing extraction of the
		 * next child */
		xmlNode *next = child->next;
		if (child->type == XML_ELEMENT_NODE) {
			if (!rewrite_urls(child, base))
				return false;
		}
		child = next;
	}

	return true;
}


/**
 * Rewrite an URL in a HTML document.
 *
 * \param  n     The node to modify
 * \param  attr  The html attribute to modify
 * \param  base  base url of document
 * \return  true on success, false on out of memory
 */

bool rewrite_url(xmlNode *n, const char *attr, const char *base)
{
	char *url, *data;
	char rel[20];
	struct content *content;
	url_func_result res;

	if (!xmlHasProp(n, (const xmlChar *) attr))
		return true;

	data = xmlGetProp(n, (const xmlChar *) attr);
	if (!data)
		return false;

	res = url_join(data, base, &url);
	xmlFree(data);
	if (res == URL_FUNC_NOMEM)
		return false;
	else if (res == URL_FUNC_OK) {
		content = save_complete_list_find(url);
		if (content) {
			/* found a match */
			free(url);
			snprintf(rel, sizeof rel, "%x",
						(unsigned int) content);
			if (!xmlSetProp(n, (const xmlChar *) attr,
							(xmlChar *) rel))
				return false;
		} else {
			/* no match found */
			if (!xmlSetProp(n, (const xmlChar *) attr,
							(xmlChar *) url)) {
				free(url);
				return false;
			}
			free(url);
		}
	}

	return true;
}


/**
 * Add a content to the save_complete_list.
 *
 * \param  content  content to add
 * \return  true on success, false on out of memory
 */

bool save_complete_list_add(struct content *content)
{
	struct save_complete_entry *entry;
	entry = malloc(sizeof (*entry));
	if (!entry)
		return false;
	entry->content = content;
	entry->next = save_complete_list;
	save_complete_list = entry;
	return true;
}


/**
 * Look up a url in the save_complete_list.
 *
 * \param  url  url to find
 * \return  content if found, 0 otherwise
 */

struct content * save_complete_list_find(const char *url)
{
	struct save_complete_entry *entry;
	for (entry = save_complete_list; entry; entry = entry->next)
		if (strcmp(url, entry->content->url) == 0)
			return entry->content;
	return 0;
}


/**
 * Look up a content in the save_complete_list.
 *
 * \param  content  pointer to content
 * \return  true if the content is in the save_complete_list
 */

bool save_complete_list_check(struct content *content)
{
	struct save_complete_entry *entry;
	for (entry = save_complete_list; entry; entry = entry->next)
		if (entry->content == content)
			return true;
	return false;
}


/**
 * Create the inventory file listing original URLs.
 */

bool save_complete_inventory(const char *path)
{
	STRPTR spath;
	ULONG len;
	bool rc;

	len = strlen(path) + sizeof("Inventory") + 4;
	spath = AllocMem(len, MEMF_ANY);
	rc = false;

	if (spath)
	{
		BPTR fh;

		strcpy(spath, path);
		AddPart(spath, "Inventory", len);

		fh = Open(spath, MODE_NEWFILE);

		if (fh)
		{
			if (fh)
			{
				struct save_complete_entry *entry;

				for (entry = save_complete_list; entry; entry = entry->next)
					FPrintf(fh, "%lx %s\n", (unsigned int) entry->content, entry->content->url);

				Close(fh);

				rc = TRUE;
			}
			else
			{
				LOG(("fopen(): errno = %i", errno));
				warn_user("SaveError", strerror(errno));
			}
		}
	}

	return rc;
}
