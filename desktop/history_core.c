/*
 * Copyright 2006 James Bursa <bursa@users.sourceforge.net>
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
 * Browser history tree (implementation).
 */

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "content/content.h"
#include "content/urldb.h"
#include "css/css.h"
#include "desktop/gui.h"
#include "desktop/history_core.h"
#include "desktop/plotters.h"
#include "image/bitmap.h"
#include "render/font.h"
#include "utils/log.h"
#include "utils/url.h"
#include "utils/utils.h"


#define WIDTH 100
#define HEIGHT 86
#define RIGHT_MARGIN 50
#define BOTTOM_MARGIN 30

struct history_page {
	char *url;    /**< Page URL, never 0. */
	char *frag_id; /** Fragment identifier, or 0. */
	char *title;  /**< Page title, never 0. */
};

/** A node in the history tree. */
struct history_entry {
  	struct history_page page;
	struct history_entry *back;  /**< Parent. */
	struct history_entry *next;  /**< Next sibling. */
	struct history_entry *forward;  /**< First child. */
	struct history_entry *forward_pref;  /**< Child in direction of
	                                          current entry. */
	struct history_entry *forward_last;  /**< Last child. */
	unsigned int children;  /**< Number of children. */
	int x;  /**< Position of node. */
	int y;  /**< Position of node. */
	struct bitmap *bitmap;  /**< Thumbnail bitmap, or 0. */
};

/** History tree for a window. */
struct history {
	/** First page in tree (page that window opened with). */
	struct history_entry *start;
	/** Current position in tree. */
	struct history_entry *current;
	/** Width of layout. */
	int width;
	/** Height of layout. */
	int height;
};

static struct history_entry *history_clone_entry(struct history *history,
		struct history_entry *entry);
static void history_free_entry(struct history_entry *entry);
static void history_go(struct browser_window *bw, struct history *history,
		struct history_entry *entry, bool new_window);
static void history_layout(struct history *history);
static int history_layout_subtree(struct history *history,
		struct history_entry *entry, int x, int y, bool shuffle);
static bool history_redraw_entry(struct history *history,
		struct history_entry *entry,
		int x0, int y0, int x1, int y1,
		int x, int y, bool clip);
static struct history_entry *history_find_position(struct history_entry *entry,
		int x, int y);


/**
 * Create a new history tree for a window.
 *
 * \return  pointer to an opaque history structure, 0 on failure.
 */

struct history *history_create(void)
{
	struct history *history;

	history = calloc(1, sizeof *history);
	if (!history) {
		warn_user("NoMemory", 0);
		return 0;
	}
	history->width = RIGHT_MARGIN / 2;
	history->height = BOTTOM_MARGIN / 2;
	return history;
}


/**
 * Clone a history tree
 *
 * \param  history  opaque history structure, as returned by history_create()
 *
 * \return  pointer to an opaque history structure, 0 on failure.
 */

struct history *history_clone(struct history *history)
{
	struct history *new_history;

	if (!history->start)
		return history_create();

	new_history = malloc(sizeof *history);
	if (!new_history)
		return 0;
	memcpy(new_history, history, sizeof *history);

	new_history->start = history_clone_entry(new_history,
			new_history->start);
	if (!history->start) {
		LOG(("Insufficient memory to clone history"));
		warn_user("NoMemory", 0);
		history_destroy(new_history);
		return 0;
	}

	return new_history;
}


/**
 * Clone a history entry
 *
 * \param  history  opaque history structure, as returned by history_create()
 * \param  start    entry to clone
 *
 * \return  a cloned history entry, or 0 on error
 */

struct history_entry *history_clone_entry(struct history *history,
		struct history_entry *entry)
{
	struct history_entry *child;
	struct history_entry *new_child;
	struct history_entry *prev = NULL;
	struct history_entry *new_entry;

	assert(entry->page.url);
	assert(entry->page.title);

	/* clone the entry */
	new_entry = malloc(sizeof *entry);
	if (!new_entry)
		return 0;
	memcpy(new_entry, entry, sizeof *entry);
	new_entry->page.url = strdup(entry->page.url);
	if (entry->page.frag_id)
		new_entry->page.frag_id = strdup(entry->page.frag_id);
	new_entry->page.title = strdup(entry->page.title);
	if (!new_entry->page.url || !new_entry->page.title ||
			((entry->page.frag_id) && (!new_entry->page.frag_id))) {
		free(new_entry->page.url);
		free(new_entry->page.title);
		free(new_entry->page.frag_id);
		free(new_entry);
		return 0;
	}

	/* update references */
	if (history->current == entry)
		history->current = new_entry;

	/* recurse for all children */
	for (child = new_entry->forward; child; child = child->next) {
		new_child = history_clone_entry(history, child);
		if (new_child)
			new_child->back = new_entry;
		if (prev)
			prev->next = new_child;
		if (new_entry->forward == child)
			new_entry->forward = new_child;
		if (new_entry->forward_pref == child)
			new_entry->forward_pref = new_child;
		if (new_entry->forward_last == child)
			new_entry->forward_last = new_child;
		if (!new_child)
			return 0;
		prev = new_child;
	}
	return new_entry;
}


/**
 * Insert a url into the history tree.
 *
 * \param  history  opaque history structure, as returned by history_create()
 * \param  content  content to add to history
 * \param  frag_id  fragment identifier
 *
 * The page is added after the current entry and becomes current.
 */

void history_add(struct history *history, struct content *content,
		char *frag_id)
{
	url_func_result res;
	struct history_entry *entry;
	char *url;
	char *title;
	struct bitmap *bitmap;

	assert(history);
	assert(content);

	/* allocate space */
	entry = malloc(sizeof *entry);

	res = url_normalize(content->url, &url);
	if (res != URL_FUNC_OK) {
		warn_user("NoMemory", 0);
		return;
	}

	title = strdup(content->title ? content->title : url);

	if (!entry || !url || !title) {
		warn_user("NoMemory", 0);
		free(entry);
		free(url);
		free(title);
		return;
	}

	entry->page.url = url;
	entry->page.frag_id = frag_id ? strdup(frag_id) : 0;
	entry->page.title = title;
	entry->back = history->current;
	entry->next = 0;
	entry->forward = entry->forward_pref = entry->forward_last = 0;
	entry->children = 0;
	entry->bitmap = 0;
	if (history->current) {
		if (history->current->forward_last)
			history->current->forward_last->next = entry;
		else
			history->current->forward = entry;
		history->current->forward_pref = entry;
		history->current->forward_last = entry;
		history->current->children++;
	} else {
		history->start = entry;
	}
	history->current = entry;

	/* if we have a thumbnail, don't update until the page has finished
	 * loading */
	bitmap = urldb_get_thumbnail(url);
	if (!bitmap) {
		bitmap = bitmap_create(WIDTH, HEIGHT,
				BITMAP_NEW | BITMAP_CLEAR_MEMORY |
				BITMAP_OPAQUE | BITMAP_PERSISTENT);
		if (!bitmap) {
			warn_user("NoMemory", 0);
			return;
		}
		if (thumbnail_create(content, bitmap, url) == false) {
			/* Thumbnailing failed. Ignore it silently */
			bitmap_destroy(bitmap);
			bitmap = NULL;
		}
	}
	entry->bitmap = bitmap;

	history_layout(history);
}


/**
 * Update the thumbnail for the current entry.
 *
 * \param  history  opaque history structure, as returned by history_create()
 * \param  content  content for current entry
 */

void history_update(struct history *history, struct content *content)
{
	char *title;
	char *url;
	url_func_result res;

	if (!history || !history->current || !history->current->bitmap)
		return;

	assert(history->current->page.url);
	assert(history->current->page.title);

	if (content->title) {
		title = strdup(content->title);
		if (!title) {
			warn_user("NoMemory", 0);
			return;
		}
	} else {
		res = url_normalize(content->url, &url);
		if (res != URL_FUNC_OK) {
			warn_user("NoMemory", 0);
			return;
		}
		title = url;
	}

	assert(title);
	free(history->current->page.title);
	history->current->page.title = title;

	thumbnail_create(content, history->current->bitmap, 0);
}


/**
 * Free a history structure.
 *
 * \param  history  opaque history structure, as returned by history_create()
 */

void history_destroy(struct history *history)
{
	if (!history)
		return;
	history_free_entry(history->start);
	free(history);
}


/**
 * Free an entry in the tree recursively.
 */

void history_free_entry(struct history_entry *entry)
{
	if (!entry)
		return;
	history_free_entry(entry->forward);
	history_free_entry(entry->next);
	free(entry->page.url);
	if (entry->page.frag_id)
		free(entry->page.frag_id);
	free(entry->page.title);
	free(entry);
}


/**
 * Go back in the history.
 *
 * \param  bw       browser window
 * \param  history  history of the window
 */

void history_back(struct browser_window *bw, struct history *history)
{
	if (!history || !history->current || !history->current->back)
		return;
	history_go(bw, history, history->current->back, false);
}


/**
 * Go forward in the history.
 *
 * \param  bw       browser window
 * \param  history  history of the window
 */

void history_forward(struct browser_window *bw, struct history *history)
{
	if (!history || !history->current || !history->current->forward_pref)
		return;
	history_go(bw, history, history->current->forward_pref, false);
}


/**
 * Check whether it is pssible to go back in the history.
 *
 * \param  history  history of the window
 * \return true if the history can go back, false otherwise
 */

bool history_back_available(struct history *history)
{
	return (history && history->current && history->current->back);
}


/**
 * Check whether it is pssible to go forwards in the history.
 *
 * \param  history  history of the window
 * \return true if the history can go forwards, false otherwise
 */

bool history_forward_available(struct history *history)
{
	return (history && history->current && history->current->forward_pref);
}


/**
 * Open a history entry in the specified browser window
 *
 * \param  bw          browser window
 * \param  history     history containing entry
 * \param  entry       entry to open
 * \param  new_window  open entry in new window
 */

void history_go(struct browser_window *bw, struct history *history,
		struct history_entry *entry, bool new_window)
{
	char *url;
	struct history_entry *current;

//	LOG(("%p %p %p", bw, history, entry));
//	LOG(("%s %s %s",
//		entry->page.url, entry->page.title, entry->page.frag_id));

	if (entry->page.frag_id) {
		url = malloc(strlen(entry->page.url) +
				strlen(entry->page.frag_id) + 5);
		if (!url) {
			warn_user("NoMemory", 0);
			return;
		}
		sprintf(url, "%s#%s", entry->page.url, entry->page.frag_id);
	}
	else
		url = entry->page.url;

	if (new_window) {
		current = history->current;
		history->current = entry;
		browser_window_create(url, bw, 0, false, false);
		history->current = current;
	} else {
		history->current = entry;
		browser_window_go(bw, url, 0, false);
	}

	if (entry->page.frag_id)
		free(url);
}


/**
 * Compute node positions.
 *
 * \param  history  history to layout
 *
 * Each node's x and y are filled in.
 */

void history_layout(struct history *history)
{
	time_t t = time(0);
	struct tm *tp = localtime(&t);
	bool shuffle = tp->tm_mon == 3 && tp->tm_mday == 1;

	if (!history)
		return;

	history->width = 0;
	if (history->start)
		history->height = history_layout_subtree(history,
			history->start, RIGHT_MARGIN / 2, BOTTOM_MARGIN / 2,
			shuffle);
	else
		history->height = 0;
	if (shuffle) {
		history->width = 600 + WIDTH;
		history->height = 400 + HEIGHT;
	}
	history->width += RIGHT_MARGIN / 2;
	history->height += BOTTOM_MARGIN / 2;
}


/**
 * Recursively position a subtree.
 *
 * \param  history  history being laid out
 * \param  entry    subtree to position
 * \param  x        x position for entry
 * \param  y        smallest available y
 * \param  shuffle  shuffle layout
 * \return  greatest y used by subtree
 */

int history_layout_subtree(struct history *history,
		struct history_entry *entry, int x, int y, bool shuffle)
{
	struct history_entry *child;
	int y1 = y;

	if (history->width < x + WIDTH)
		history->width = x + WIDTH;

	if (!entry->forward) {
		entry->x = x;
		entry->y = y;
		if (shuffle) {
			entry->x = rand() % 600;
			entry->y = rand() % 400;
		}
		return y + HEIGHT;
	}

	/* layout child subtrees below each other */
	for (child = entry->forward; child; child = child->next) {
		y1 = history_layout_subtree(history, child,
				x + WIDTH + RIGHT_MARGIN, y1, shuffle);
		if (child->next)
			y1 += BOTTOM_MARGIN;
	}

	/* place ourselves in the middle */
	entry->x = x;
	entry->y = (y + y1) / 2 - HEIGHT / 2;
	if (shuffle) {
		entry->x = rand() % 600;
		entry->y = rand() % 400;
	}

	return y1;
}


/**
 * Get the dimensions of a history.
 *
 * \param  history  history to measure
 * \param  width    updated to width
 * \param  height   updated to height
 */

void history_size(struct history *history, int *width, int *height)
{
	*width = history->width;
	*height = history->height;
}


/**
 * Redraw a history.
 *
 * \param  history  history to render
 *
 * The current plotter is used.
 */

bool history_redraw(struct history *history)
{
	if (!history->start)
		return true;
	return history_redraw_entry(history, history->start, 0, 0, 0, 0, 0, 0, false);
}

/**
 * Redraw part of a history.
 *
 * \param  history  history to render
 * \param  x0       left X co-ordinate of redraw area
 * \param  y0       top Y co-ordinate of redraw area
 * \param  x1       right X co-ordinate of redraw area
 * \param  y1       lower Y co-ordinate of redraw area
 * \param  x        start X co-ordinate on plot canvas
 * \param  y        start Y co-ordinate on plot canvas
 *
 * The current plotter is used.
 */

bool history_redraw_rectangle(struct history *history,
	int x0, int y0, int x1, int y1,
	int x, int y)
{
	if (!history->start)
		return true;
	return history_redraw_entry(history, history->start,
		x0, y0, x1, y1, x, y, true);
}

/**
 * Recursively redraw a history_entry.
 *
 * \param  history        history containing the entry
 * \param  history_entry  entry to render
 */

bool history_redraw_entry(struct history *history,
		struct history_entry *entry,
		int x0, int y0, int x1, int y1,
		int x, int y, bool clip)
{
	size_t char_offset;
	int actual_x;
	struct history_entry *child;
	colour c = entry == history->current ? 0x0000ff : 0x333333;
	int tailsize = 5;
	int xoffset = x - x0;
	int yoffset = y - y0;

	if (clip) {
		if(!plot.clip(x0 + xoffset, y0 + yoffset, x1 + xoffset, y1 + yoffset))
			return false;
	}

	if (!plot.bitmap(entry->x + xoffset, entry->y + yoffset, WIDTH, HEIGHT,
			entry->bitmap, 0xffffff, NULL))
		return false;
	if (!plot.rectangle(entry->x - 1 + xoffset, entry->y - 1 + yoffset,
			WIDTH + 1, HEIGHT + 1,
			entry == history->current ? 2 : 1, c, false, false))
		return false;

	if (!nsfont.font_position_in_string(&css_base_style, entry->page.title,
			strlen(entry->page.title), WIDTH,
			&char_offset, &actual_x))
		return false;
	if (!plot.text(entry->x + xoffset, entry->y + HEIGHT + 12 + yoffset,
			&css_base_style, entry->page.title, char_offset, 0xffffff, c))
		return false;

	for (child = entry->forward; child; child = child->next) {
		if (!plot.line(entry->x + WIDTH + xoffset,
				entry->y + HEIGHT / 2 + yoffset,
		      	entry->x + WIDTH + tailsize + xoffset,
				entry->y + HEIGHT / 2 + yoffset, 1,
				0x333333, false, false))
			return false;
		if (!plot.line(entry->x + WIDTH + tailsize + xoffset,
				entry->y + HEIGHT / 2 + yoffset,
				child->x - tailsize +xoffset,
				child->y + HEIGHT / 2 + yoffset, 1,
				0x333333, false, false))
			return false;
		if (!plot.line(child->x - tailsize + xoffset,
				child->y + HEIGHT / 2 + yoffset,
				child->x + xoffset, child->y + HEIGHT / 2 + yoffset, 1,
				0x333333, false, false))
			return false;
		if (!history_redraw_entry(history, child, x0, y0, x1, y1, x, y, clip))
			return false;
	}

	return true;
}


/**
 * Handle a mouse click in a history.
 *
 * \param  bw          browser window containing history
 * \param  history     history that was clicked in
 * \param  x           click coordinate
 * \param  y           click coordinate
 * \param  new_window  open a new window instead of using bw
 * \return  true if action was taken, false if click was not on an entry
 */

bool history_click(struct browser_window *bw, struct history *history,
		int x, int y, bool new_window)
{
	struct history_entry *entry;

	entry = history_find_position(history->start, x, y);
	if (!entry)
		return false;
	if (entry == history->current)
		return false;

	history_go(bw, history, entry, new_window);

	return true;
}


/**
 * Determine the URL of the entry at a position.
 *
 * \param  history  history to search
 * \param  x        coordinate
 * \param  y        coordinate
 * \return  URL, or 0 if no entry at (x, y)
 */

const char *history_position_url(struct history *history, int x, int y)
{
	struct history_entry *entry;

	entry = history_find_position(history->start, x, y);
	if (!entry)
		return 0;

	return entry->page.url;
}


/**
 * Find the history entry at a position.
 *
 * \param  entry  entry to search from
 * \param  x      coordinate
 * \param  y      coordinate
 * \return  an entry if found, 0 if none
 */

struct history_entry *history_find_position(struct history_entry *entry,
		int x, int y)
{
	struct history_entry *child;
	struct history_entry *found;

	if (!entry)
		return 0;

	if (entry->x <= x && x <= entry->x + WIDTH &&
			entry->y <= y && y <= entry->y + HEIGHT)
		return entry;

	for (child = entry->forward; child; child = child->next) {
		found = history_find_position(child, x, y);
		if (found)
			return found;
	}

	return 0;
}
