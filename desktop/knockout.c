/*
 * Copyright 2006 Richard Wilson <info@tinct.net>
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
 * Knockout rendering (implementation).
 *
 * Knockout rendering is an optimisation which is particularly for
 * unaccelerated screen redraw. It tries to avoid plotting the same area more
 * than once.
 *
 * If the object is to plot two overlapping rectangles (one large, one small),
 * such as:
 *
 *   +-----------------+
 *   |#################|
 *   |####+-------+####|
 *   |####|:::::::|####|
 *   |####|:::::::|####|
 *   |####|:::::::|####|
 *   |####+-------+####|
 *   |#################|
 *   +-----------------+
 *
 * Without knockout rendering we plot the bottom rectangle and then the top one:
 *
 *   +-----------------+                 +-----------------+
 *   |#################|                 |#################|
 *   |#################|                 |####+-------+####|
 *   |#################|                 |####|:::::::|####|
 *   |#################|    and then,    |####|:::::::|####|
 *   |#################|                 |####|:::::::|####|
 *   |#################|                 |####+-------+####|
 *   |#################|                 |#################|
 *   +-----------------+                 +-----------------+
 *
 * With knockoout rendering, the bottom rectangle is split up into smaller
 * ones and each pixel is just plotted once:
 *
 *   +-----------------+
 *   |#################|
 *   +----+-------+----+
 *   |####|:::::::|####|
 *   |####|:::::::|####|
 *   |####|:::::::|####|
 *   +----+-------+----+
 *   |#################|
 *   +-----------------+
 */

#define NDEBUG
#include <assert.h>
#include <string.h>
#include "desktop/knockout.h"
#include "desktop/plotters.h"
#include "image/bitmap.h"
#include "utils/log.h"

#define KNOCKOUT_ENTRIES 3072	/* 40 bytes each */
#define KNOCKOUT_BOXES 768	/* 28 bytes each */
#define KNOCKOUT_POLYGONS 3072	/* 4 bytes each */


struct knockout_box;
struct knockout_entry;


static void knockout_set_plotters(void);
static void knockout_calculate(int x0, int y0, int x1, int y1, struct knockout_box *box);
static bool knockout_plot_fill_recursive(struct knockout_box *box, colour c);
static bool knockout_plot_bitmap_tile_recursive(struct knockout_box *box,
		struct knockout_entry *entry);

static bool knockout_plot_clg(colour c);
static bool knockout_plot_rectangle(int x0, int y0, int width, int height,
		int line_width, colour c, bool dotted, bool dashed);
static bool knockout_plot_line(int x0, int y0, int x1, int y1, int width,
		colour c, bool dotted, bool dashed);
static bool knockout_plot_polygon(const int *p, unsigned int n, colour fill);
static bool knockout_plot_fill(int x0, int y0, int x1, int y1, colour c);
static bool knockout_plot_clip(int clip_x0, int clip_y0,
		int clip_x1, int clip_y1);
static bool knockout_plot_text(int x, int y, const struct css_style *style,
		const char *text, size_t length, colour bg, colour c);
static bool knockout_plot_disc(int x, int y, int radius, colour colour, bool filled);
static bool knockout_plot_arc(int x, int y, int radius, int angle1, int angle2,
		colour c);
static bool knockout_plot_bitmap(int x, int y, int width, int height,
		struct bitmap *bitmap, colour bg, struct content *content);
static bool knockout_plot_bitmap_tile(int x, int y, int width, int height,
		struct bitmap *bitmap, colour bg,
		bool repeat_x, bool repeat_y, struct content *content);
static bool knockout_plot_flush(void);
static bool knockout_plot_group_start(const char *name);
static bool knockout_plot_group_end(void);
static bool knockout_plot_path(const float *p, unsigned int n, colour fill,
		float width, colour c, const float transform[6]);


const struct plotter_table knockout_plotters = {
	knockout_plot_clg,
	knockout_plot_rectangle,
	knockout_plot_line,
	knockout_plot_polygon,
	knockout_plot_fill,
	knockout_plot_clip,
	knockout_plot_text,
	knockout_plot_disc,
	knockout_plot_arc,
	knockout_plot_bitmap,
	knockout_plot_bitmap_tile,
	knockout_plot_group_start,
	knockout_plot_group_end,
	knockout_plot_flush,
	knockout_plot_path,
	true
};


typedef enum {
	KNOCKOUT_PLOT_CLG,		/* translated to _FILL */
	KNOCKOUT_PLOT_RECTANGLE,
	KNOCKOUT_PLOT_LINE,
	KNOCKOUT_PLOT_POLYGON,
	KNOCKOUT_PLOT_FILL,		/* knockout, knocked out */
	KNOCKOUT_PLOT_CLIP,
	KNOCKOUT_PLOT_TEXT,
	KNOCKOUT_PLOT_DISC,
	KNOCKOUT_PLOT_ARC,
	KNOCKOUT_PLOT_BITMAP,		/* knockout */
	KNOCKOUT_PLOT_BITMAP_TILE,	/* knockout, knocked out */
	KNOCKOUT_PLOT_GROUP_START,
	KNOCKOUT_PLOT_GROUP_END,
} knockout_type;


struct knockout_box {
	struct {
		int x0;
		int y0;
		int x1;
		int y1;
	} bbox;
	bool deleted;			/* box has been deleted, ignore */
	struct knockout_box *child;
	struct knockout_box *next;
};


struct knockout_entry {
	knockout_type type;
	struct knockout_box *box;	/* relating series of knockout clips */
	union {
		struct {
			colour c;
		} clg;
		struct {
			int x0;
			int y0;
			int width;
			int height;
			int line_width;
			colour c;
			bool dotted;
			bool dashed;
		} rectangle;
		struct {
			int x0;
			int y0;
			int x1;
			int y1;
			int width;
			colour c;
			bool dotted;
			bool dashed;
		} line;
		struct {
			int *p;
			unsigned int n;
			colour fill;
		} polygon;
		struct {
			int x0;
			int y0;
			int x1;
			int y1;
			colour c;
		} fill;
		struct {
			int x0;
			int y0;
			int x1;
			int y1;
		} clip;
		struct {
			int x;
			int y;
			const struct css_style *style;
			const char *text;
			size_t length;
			colour bg;
			colour c;
		} text;
		struct {
			int x;
			int y;
			int radius;
			colour colour;
			bool filled;
		} disc;
		struct {
			int x;
			int y;
			int radius;
			int angle1;
			int angle2;
			colour c;
		} arc;
		struct {
			int x;
			int y;
			int width;
			int height;
			struct bitmap *bitmap;
			colour bg;
			struct content *content;
		} bitmap;
		struct {
			int x;
			int y;
			int width;
			int height;
			struct bitmap *bitmap;
			colour bg;
			bool repeat_x;
			bool repeat_y;
			struct content *content;
		} bitmap_tile;
		struct {
			const char *name;
		} group_start;
	} data;
};


static struct knockout_entry knockout_entries[KNOCKOUT_ENTRIES];
static struct knockout_box knockout_boxes[KNOCKOUT_BOXES];
static int knockout_polygons[KNOCKOUT_POLYGONS];
static int knockout_entry_cur = 0;
static int knockout_box_cur = 0;
static int knockout_polygon_cur = 0;
static struct knockout_box *knockout_list = NULL;

static struct plotter_table real_plot;

static int clip_x0_cur;
static int clip_y0_cur;
static int clip_x1_cur;
static int clip_y1_cur;
static int nested_depth = 0;

/**
 * Start a knockout plotting session
 *
 * \param  plotter  the plotter to use
 * \return  true on success, false otherwise
 */
bool knockout_plot_start(struct plotter_table *plotter)
{
  	/* check if we're recursing */
  	if (nested_depth++ > 0) {
  	  	/* we should already have the knockout renderer as default */
  		assert(plotter->clg == knockout_plotters.clg);
  		return true;
  	}

	/* end any previous sessions */
	if (knockout_entry_cur > 0)
		knockout_plot_end();

	/* take over the plotter */
	real_plot = *plotter;
	knockout_set_plotters();
	return true;
}


/**
 * End a knockout plotting session
 *
 * \return  true on success, false otherwise
 */
bool knockout_plot_end(void)
{
	/* only output when we've finished any nesting */
	if (--nested_depth == 0)
		return knockout_plot_flush();

	assert(nested_depth > 0);
	return true;
}


/**
 * Flush the current knockout session to empty the buffers
 *
 * \return  true on success, false otherwise
 */
bool knockout_plot_flush(void)
{
	int i;
	bool success = true;
	struct knockout_box *box;

	/* debugging information */
	LOG(("Entries are %i/%i, %i/%i, %i/%i",
			knockout_entry_cur, KNOCKOUT_ENTRIES,
			knockout_box_cur, KNOCKOUT_BOXES,
			knockout_polygon_cur, KNOCKOUT_POLYGONS));

	/* release our plotter */
	plot = real_plot;

	for (i = 0; i < knockout_entry_cur; i++) {
		switch (knockout_entries[i].type) {
			case KNOCKOUT_PLOT_CLG:
				success &= plot.clg(
						knockout_entries[i].data.clg.c);
				break;
			case KNOCKOUT_PLOT_RECTANGLE:
				success &= plot.rectangle(
						knockout_entries[i].data.rectangle.x0,
						knockout_entries[i].data.rectangle.y0,
						knockout_entries[i].data.rectangle.width,
						knockout_entries[i].data.rectangle.height,
						knockout_entries[i].data.rectangle.line_width,
						knockout_entries[i].data.rectangle.c,
						knockout_entries[i].data.rectangle.dotted,
						knockout_entries[i].data.rectangle.dashed);
				break;
			case KNOCKOUT_PLOT_LINE:
				success &= plot.line(
						knockout_entries[i].data.line.x0,
						knockout_entries[i].data.line.y0,
						knockout_entries[i].data.line.x1,
						knockout_entries[i].data.line.y1,
						knockout_entries[i].data.line.width,
						knockout_entries[i].data.line.c,
						knockout_entries[i].data.line.dotted,
						knockout_entries[i].data.line.dashed);
				break;
			case KNOCKOUT_PLOT_POLYGON:
				success &= plot.polygon(
						knockout_entries[i].data.polygon.p,
						knockout_entries[i].data.polygon.n,
						knockout_entries[i].data.polygon.fill);
				break;
			case KNOCKOUT_PLOT_FILL:
				box = knockout_entries[i].box->child;
				if (box)
					success &= knockout_plot_fill_recursive(box,
							knockout_entries[i].data.fill.c);
				else if (!knockout_entries[i].box->deleted)
					success &= plot.fill(
							knockout_entries[i].data.fill.x0,
							knockout_entries[i].data.fill.y0,
							knockout_entries[i].data.fill.x1,
							knockout_entries[i].data.fill.y1,
							knockout_entries[i].data.fill.c);
				break;
			case KNOCKOUT_PLOT_CLIP:
				success &= plot.clip(
						knockout_entries[i].data.clip.x0,
						knockout_entries[i].data.clip.y0,
						knockout_entries[i].data.clip.x1,
						knockout_entries[i].data.clip.y1);
				break;
			case KNOCKOUT_PLOT_TEXT:
				success &= plot.text(
						knockout_entries[i].data.text.x,
						knockout_entries[i].data.text.y,
						knockout_entries[i].data.text.style,
						knockout_entries[i].data.text.text,
						knockout_entries[i].data.text.length,
						knockout_entries[i].data.text.bg,
						knockout_entries[i].data.text.c);
				break;
			case KNOCKOUT_PLOT_DISC:
				success &= plot.disc(
						knockout_entries[i].data.disc.x,
						knockout_entries[i].data.disc.y,
						knockout_entries[i].data.disc.radius,
						knockout_entries[i].data.disc.colour,
						knockout_entries[i].data.disc.filled);
				break;
			case KNOCKOUT_PLOT_ARC:
				success &= plot.arc(
						knockout_entries[i].data.arc.x,
						knockout_entries[i].data.arc.y,
						knockout_entries[i].data.arc.radius,
						knockout_entries[i].data.arc.angle1,
						knockout_entries[i].data.arc.angle2,
						knockout_entries[i].data.arc.c);
				break;
			case KNOCKOUT_PLOT_BITMAP:
				success &= plot.bitmap(
						knockout_entries[i].data.bitmap.x,
						knockout_entries[i].data.bitmap.y,
						knockout_entries[i].data.bitmap.width,
						knockout_entries[i].data.bitmap.height,
						knockout_entries[i].data.bitmap.bitmap,
						knockout_entries[i].data.bitmap.bg,
						knockout_entries[i].data.bitmap.content);
				break;
			case KNOCKOUT_PLOT_BITMAP_TILE:
				box = knockout_entries[i].box->child;
				if (box) {
					success &= knockout_plot_bitmap_tile_recursive(box,
							&knockout_entries[i]);
				} else if (!knockout_entries[i].box->deleted) {
					success &= plot.bitmap_tile(
							knockout_entries[i].data.
									bitmap_tile.x,
							knockout_entries[i].data.
									bitmap_tile.y,
							knockout_entries[i].data.
									bitmap_tile.width,
							knockout_entries[i].data.
									bitmap_tile.height,
							knockout_entries[i].data.
									bitmap_tile.bitmap,
							knockout_entries[i].data.
									bitmap_tile.bg,
							knockout_entries[i].data.
									bitmap_tile.repeat_x,
							knockout_entries[i].data.
									bitmap_tile.repeat_y,
							knockout_entries[i].data.
									bitmap_tile.content);
				}
				break;
			case KNOCKOUT_PLOT_GROUP_START:
				success &= plot.group_start(
						knockout_entries[i].data.group_start.name);
				break;
			case KNOCKOUT_PLOT_GROUP_END:
				success &= plot.group_end();
				break;
		}
	}

	knockout_entry_cur = 0;
	knockout_box_cur = 0;
	knockout_polygon_cur = 0;
	knockout_list = NULL;

	/* re-instate knockout plotters if we are still active */
	if (nested_depth > 0)
		knockout_set_plotters();
	return success;
}


/**
 * Override the current plotters with the knockout plotters
 */
void knockout_set_plotters(void)
{
	plot = knockout_plotters;
	if (!real_plot.group_start)
		plot.group_start = NULL;
	if (!real_plot.group_end)
		plot.group_end = NULL;
}


/**
 * Knockout a section of previous rendering
 *
 * \param  x0	the left edge of the removal box
 * \param  y0	the bottom edge of the removal box
 * \param  x1	the right edge of the removal box
 * \param  y1	the top edge of the removal box
 * \param  box  the parent box set to consider, or NULL for top level
*/
void knockout_calculate(int x0, int y0, int x1, int y1, struct knockout_box *owner)
{
	struct knockout_box *box;
	struct knockout_box *parent;
	struct knockout_box *prev = NULL;
	int nx0, ny0, nx1, ny1;

	if (owner == NULL)
		box = knockout_list;
	else
		box = owner->child;

	for (parent = box; parent; parent = parent->next) {
	  	/* permanently delink deleted nodes */
		if (parent->deleted) {
		  	if (prev) {
		  	  	/* not the first valid element: just skip future */
		  		prev->next = parent->next;
		  	} else {
		  	  	if (owner) {
		  	  		/* first valid element: update child reference */
		  	  		owner->child = parent->next;
		  	  		/* have we deleted all child nodes? */
		  	  		if (!owner->child)
		  	  			owner->deleted = true;
		  	  	} else {
		  	  	  	/* we are the head of the list */
		  	  	  	knockout_list = parent->next;
		  	  	}
		  	}
			continue;
		} else {
		  	prev = parent;
		}

		/* get the parent dimensions */
	  	nx0 = parent->bbox.x0;
	  	ny0 = parent->bbox.y0;
	  	nx1 = parent->bbox.x1;
	  	ny1 = parent->bbox.y1;

		/* reject non-overlapping boxes */
		if ((nx0 >= x1) || (nx1 <= x0) || (ny0 >= y1) || (ny1 <= y0))
			continue;

		/* check for a total knockout */
		if ((x0 <= nx0) && (x1 >= nx1) && (y0 <= ny0) && (y1 >= ny1)) {
			parent->deleted = true;
			continue;
		}

		/* has the box been replaced by children? */
		if (parent->child) {
			knockout_calculate(x0, y0, x1, y1, parent);
		} else {
			/* we need a maximum of 4 child boxes */
			if (knockout_box_cur + 4 >= KNOCKOUT_BOXES) {
				knockout_plot_flush();
				return;
			}

			/* clip top */
			if (y1 < ny1) {
			  	knockout_boxes[knockout_box_cur].bbox.x0 = nx0;
			  	knockout_boxes[knockout_box_cur].bbox.y0 = y1;
			  	knockout_boxes[knockout_box_cur].bbox.x1 = nx1;
			  	knockout_boxes[knockout_box_cur].bbox.y1 = ny1;
			  	knockout_boxes[knockout_box_cur].deleted = false;
			  	knockout_boxes[knockout_box_cur].child = NULL;
			  	knockout_boxes[knockout_box_cur].next = parent->child;
				parent->child = &knockout_boxes[knockout_box_cur++];
				ny1 = y1;
			}
			/* clip bottom */
			if (y0 > ny0) {
			  	knockout_boxes[knockout_box_cur].bbox.x0 = nx0;
			  	knockout_boxes[knockout_box_cur].bbox.y0 = ny0;
			  	knockout_boxes[knockout_box_cur].bbox.x1 = nx1;
			  	knockout_boxes[knockout_box_cur].bbox.y1 = y0;
			  	knockout_boxes[knockout_box_cur].deleted = false;
			  	knockout_boxes[knockout_box_cur].child = NULL;
			  	knockout_boxes[knockout_box_cur].next = parent->child;
				parent->child = &knockout_boxes[knockout_box_cur++];
				ny0 = y0;
			}
			/* clip right */
			if (x1 < nx1) {
			  	knockout_boxes[knockout_box_cur].bbox.x0 = x1;
			  	knockout_boxes[knockout_box_cur].bbox.y0 = ny0;
			  	knockout_boxes[knockout_box_cur].bbox.x1 = nx1;
			  	knockout_boxes[knockout_box_cur].bbox.y1 = ny1;
			  	knockout_boxes[knockout_box_cur].deleted = false;
			  	knockout_boxes[knockout_box_cur].child = NULL;
			  	knockout_boxes[knockout_box_cur].next = parent->child;
				parent->child = &knockout_boxes[knockout_box_cur++];
				nx1 = x1;
			}
			/* clip left */
			if (x0 > nx0) {
			  	knockout_boxes[knockout_box_cur].bbox.x0 = nx0;
			  	knockout_boxes[knockout_box_cur].bbox.y0 = ny0;
			  	knockout_boxes[knockout_box_cur].bbox.x1 = x0;
			  	knockout_boxes[knockout_box_cur].bbox.y1 = ny1;
			  	knockout_boxes[knockout_box_cur].deleted = false;
			  	knockout_boxes[knockout_box_cur].child = NULL;
			  	knockout_boxes[knockout_box_cur].next = parent->child;
				parent->child = &knockout_boxes[knockout_box_cur++];
				//nx0 = x0;
			}
		}
	}
}


bool knockout_plot_fill_recursive(struct knockout_box *box, colour c)
{
	bool success = true;
	struct knockout_box *parent;

	for (parent = box; parent; parent = parent->next) {
		if (parent->deleted)
			continue;
		if (parent->child)
			knockout_plot_fill_recursive(parent->child, c);
		else
			success &= plot.fill(parent->bbox.x0,
					parent->bbox.y0,
					parent->bbox.x1,
					parent->bbox.y1,
					c);
	}
	return success;
}


bool knockout_plot_bitmap_tile_recursive(struct knockout_box *box,
		struct knockout_entry *entry)
{
	bool success = true;
	struct knockout_box *parent;

	for (parent = box; parent; parent = parent->next) {
		if (parent->deleted)
			continue;
		if (parent->child)
			knockout_plot_bitmap_tile_recursive(parent->child, entry);
		else {
			success &= plot.clip(parent->bbox.x0,
					parent->bbox.y0,
					parent->bbox.x1,
					parent->bbox.y1);
			success &= plot.bitmap_tile(entry->data.bitmap_tile.x,
					entry->data.bitmap_tile.y,
					entry->data.bitmap_tile.width,
					entry->data.bitmap_tile.height,
					entry->data.bitmap_tile.bitmap,
					entry->data.bitmap_tile.bg,
					entry->data.bitmap_tile.repeat_x,
					entry->data.bitmap_tile.repeat_y,
					entry->data.bitmap_tile.content);
		}
	}
	return success;
}

bool knockout_plot_clg(colour c)
{
	return knockout_plot_fill(clip_x0_cur, clip_y0_cur, clip_x1_cur, clip_y1_cur, c);
}


bool knockout_plot_rectangle(int x0, int y0, int width, int height,
		int line_width, colour c, bool dotted, bool dashed)
{
	knockout_entries[knockout_entry_cur].data.rectangle.x0 = x0;
	knockout_entries[knockout_entry_cur].data.rectangle.y0 = y0;
	knockout_entries[knockout_entry_cur].data.rectangle.width = width;
	knockout_entries[knockout_entry_cur].data.rectangle.height = height;
	knockout_entries[knockout_entry_cur].data.rectangle.line_width = line_width;
	knockout_entries[knockout_entry_cur].data.rectangle.c = c;
	knockout_entries[knockout_entry_cur].data.rectangle.dotted = dotted;
	knockout_entries[knockout_entry_cur].data.rectangle.dashed = dashed;
	knockout_entries[knockout_entry_cur].type = KNOCKOUT_PLOT_RECTANGLE;
	if (++knockout_entry_cur >= KNOCKOUT_ENTRIES)
		knockout_plot_flush();
	return true;
}


bool knockout_plot_line(int x0, int y0, int x1, int y1, int width,
		colour c, bool dotted, bool dashed)
{
	knockout_entries[knockout_entry_cur].data.line.x0 = x0;
	knockout_entries[knockout_entry_cur].data.line.y0 = y0;
	knockout_entries[knockout_entry_cur].data.line.x1 = x1;
	knockout_entries[knockout_entry_cur].data.line.y1 = y1;
	knockout_entries[knockout_entry_cur].data.line.width = width;
	knockout_entries[knockout_entry_cur].data.line.c = c;
	knockout_entries[knockout_entry_cur].data.line.dotted = dotted;
	knockout_entries[knockout_entry_cur].data.line.dashed = dashed;
	knockout_entries[knockout_entry_cur].type = KNOCKOUT_PLOT_LINE;
	if (++knockout_entry_cur >= KNOCKOUT_ENTRIES)
		knockout_plot_flush();
	return true;
}


bool knockout_plot_polygon(const int *p, unsigned int n, colour fill)
{
  	bool success = true;
  	int *dest;

	/* ensure we have sufficient room even when flushed */
	if (n * 2 >= KNOCKOUT_POLYGONS) {
		knockout_plot_flush();
		success = real_plot.polygon(p, n, fill);
		return success;
	}

	/* ensure we have enough room right now */
	if (knockout_polygon_cur + n * 2 >= KNOCKOUT_POLYGONS)
		knockout_plot_flush();

	/* copy our data */
	dest = &(knockout_polygons[knockout_polygon_cur]);
	memcpy(dest, p, n * 2 * sizeof(int));
	knockout_polygon_cur += n * 2;
	knockout_entries[knockout_entry_cur].data.polygon.p = dest;
	knockout_entries[knockout_entry_cur].data.polygon.n = n;
	knockout_entries[knockout_entry_cur].data.polygon.fill = fill;
	knockout_entries[knockout_entry_cur].type = KNOCKOUT_PLOT_POLYGON;
	if (++knockout_entry_cur >= KNOCKOUT_ENTRIES)
		knockout_plot_flush();
	return true;
}


bool knockout_plot_path(const float *p, unsigned int n, colour fill,
		float width, colour c, const float transform[6])
{
	knockout_plot_flush();
	return real_plot.path(p, n, fill, width, c, transform);
}


bool knockout_plot_fill(int x0, int y0, int x1, int y1, colour c)
{
	int kx0, ky0, kx1, ky1;

	/* get our bounds */
 	kx0 = (x0 > clip_x0_cur) ? x0 : clip_x0_cur;
 	ky0 = (y0 > clip_y0_cur) ? y0 : clip_y0_cur;
  	kx1 = (x1 < clip_x1_cur) ? x1 : clip_x1_cur;
  	ky1 = (y1 < clip_y1_cur) ? y1 : clip_y1_cur;
 	if ((kx0 > clip_x1_cur) || (kx1 < clip_x0_cur) ||
 			(ky0 > clip_y1_cur) || (ky1 < clip_y0_cur))
 		return true;

	/* fills both knock out and get knocked out */
	knockout_calculate(kx0, ky0, kx1, ky1, NULL);
	knockout_boxes[knockout_box_cur].bbox.x0 = x0;
	knockout_boxes[knockout_box_cur].bbox.y0 = y0;
	knockout_boxes[knockout_box_cur].bbox.x1 = x1;
	knockout_boxes[knockout_box_cur].bbox.y1 = y1;
	knockout_boxes[knockout_box_cur].deleted = false;
	knockout_boxes[knockout_box_cur].child = NULL;
	knockout_boxes[knockout_box_cur].next = knockout_list;
	knockout_list = &knockout_boxes[knockout_box_cur];
	knockout_entries[knockout_entry_cur].box = &knockout_boxes[knockout_box_cur];
	knockout_entries[knockout_entry_cur].data.fill.x0 = x0;
	knockout_entries[knockout_entry_cur].data.fill.y0 = y0;
	knockout_entries[knockout_entry_cur].data.fill.x1 = x1;
	knockout_entries[knockout_entry_cur].data.fill.y1 = y1;
	knockout_entries[knockout_entry_cur].data.fill.c = c;
	knockout_entries[knockout_entry_cur].type = KNOCKOUT_PLOT_FILL;
	if ((++knockout_entry_cur >= KNOCKOUT_ENTRIES) ||
			(++knockout_box_cur >= KNOCKOUT_BOXES))
		knockout_plot_flush();
	return true;
}


bool knockout_plot_clip(int clip_x0, int clip_y0,
		int clip_x1, int clip_y1)
{
	if (clip_x1 < clip_x0 || clip_y0 > clip_y1) {
		LOG(("bad clip rectangle %i %i %i %i",
				clip_x0, clip_y0, clip_x1, clip_y1));
		return false;
	}

	/* memorise clip for bitmap tiling */
	clip_x0_cur = clip_x0;
	clip_y0_cur = clip_y0;
	clip_x1_cur = clip_x1;
	clip_y1_cur = clip_y1;

	knockout_entries[knockout_entry_cur].data.clip.x0 = clip_x0;
	knockout_entries[knockout_entry_cur].data.clip.y0 = clip_y0;
	knockout_entries[knockout_entry_cur].data.clip.x1 = clip_x1;
	knockout_entries[knockout_entry_cur].data.clip.y1 = clip_y1;
	knockout_entries[knockout_entry_cur].type = KNOCKOUT_PLOT_CLIP;
	if (++knockout_entry_cur >= KNOCKOUT_ENTRIES)
		knockout_plot_flush();
	return true;
}


bool knockout_plot_text(int x, int y, const struct css_style *style,
		const char *text, size_t length, colour bg, colour c)
{
	knockout_entries[knockout_entry_cur].data.text.x = x;
	knockout_entries[knockout_entry_cur].data.text.y = y;
	knockout_entries[knockout_entry_cur].data.text.style = style;
	knockout_entries[knockout_entry_cur].data.text.text = text;
	knockout_entries[knockout_entry_cur].data.text.length = length;
	knockout_entries[knockout_entry_cur].data.text.bg = bg;
	knockout_entries[knockout_entry_cur].data.text.c = c;
	knockout_entries[knockout_entry_cur].type = KNOCKOUT_PLOT_TEXT;
	if (++knockout_entry_cur >= KNOCKOUT_ENTRIES)
		knockout_plot_flush();
	return true;
}


bool knockout_plot_disc(int x, int y, int radius, colour colour, bool filled)
{
	knockout_entries[knockout_entry_cur].data.disc.x = x;
	knockout_entries[knockout_entry_cur].data.disc.y = y;
	knockout_entries[knockout_entry_cur].data.disc.radius = radius;
	knockout_entries[knockout_entry_cur].data.disc.colour = colour;
	knockout_entries[knockout_entry_cur].data.disc.filled = filled;
	knockout_entries[knockout_entry_cur].type = KNOCKOUT_PLOT_DISC;
	if (++knockout_entry_cur >= KNOCKOUT_ENTRIES)
		knockout_plot_flush();
	return true;
}

bool knockout_plot_arc(int x, int y, int radius, int angle1, int angle2, colour c)
{
	knockout_entries[knockout_entry_cur].data.arc.x = x;
	knockout_entries[knockout_entry_cur].data.arc.y = y;
	knockout_entries[knockout_entry_cur].data.arc.radius = radius;
	knockout_entries[knockout_entry_cur].data.arc.angle1 = angle1;
	knockout_entries[knockout_entry_cur].data.arc.angle2 = angle2;
	knockout_entries[knockout_entry_cur].data.arc.c = c;
	knockout_entries[knockout_entry_cur].type = KNOCKOUT_PLOT_ARC;
	if (++knockout_entry_cur >= KNOCKOUT_ENTRIES)
		knockout_plot_flush();
	return true;
}

bool knockout_plot_bitmap(int x, int y, int width, int height,
		struct bitmap *bitmap, colour bg, struct content *content)
{
	int kx0, ky0, kx1, ky1;

	/* opaque bitmaps knockout, but don't get knocked out */
	if (bitmap_get_opaque(bitmap)) {
		/* get our bounds */
  		kx0 = (x > clip_x0_cur) ? x : clip_x0_cur;
  		ky0 = (y > clip_y0_cur) ? y : clip_y0_cur;
  		kx1 = (x + width < clip_x1_cur) ? x + width : clip_x1_cur;
  		ky1 = (y + height< clip_y1_cur) ? y + height: clip_y1_cur;
 		if ((kx0 > clip_x1_cur) || (kx1 < clip_x0_cur) ||
 				(ky0 > clip_y1_cur) || (ky1 < clip_y0_cur))
 			return true;
		knockout_calculate(kx0, ky0, kx1, ky1, NULL);
	}
	knockout_entries[knockout_entry_cur].data.bitmap.x = x;
	knockout_entries[knockout_entry_cur].data.bitmap.y = y;
	knockout_entries[knockout_entry_cur].data.bitmap.width = width;
	knockout_entries[knockout_entry_cur].data.bitmap.height = height;
	knockout_entries[knockout_entry_cur].data.bitmap.bitmap = bitmap;
	knockout_entries[knockout_entry_cur].data.bitmap.bg = bg;
	knockout_entries[knockout_entry_cur].data.bitmap.content = content;
	knockout_entries[knockout_entry_cur].type = KNOCKOUT_PLOT_BITMAP;
	if (++knockout_entry_cur >= KNOCKOUT_ENTRIES)
		knockout_plot_flush();
	return true;
}


bool knockout_plot_bitmap_tile(int x, int y, int width, int height,
		struct bitmap *bitmap, colour bg,
		bool repeat_x, bool repeat_y, struct content *content)
{
	int kx0, ky0, kx1, ky1;

	/* get our bounds */
  	kx0 = clip_x0_cur;
  	ky0 = clip_y0_cur;
  	kx1 = clip_x1_cur;
  	ky1 = clip_y1_cur;
  	if (!repeat_x) {
  		if (x > kx0)
  			kx0 = x;
  		if (x + width < kx1)
  			kx1 = x + width;
 		if ((kx0 > clip_x1_cur) || (kx1 < clip_x0_cur))
  			return true;
  	}
  	if (!repeat_y) {
  		if (y > ky0)
  			ky0 = y;
  		if (y + height < ky1)
  			ky1 = y + height;
  		if ((ky0 > clip_y1_cur) || (ky1 < clip_y0_cur))
  			return true;
  	}

	/* tiled bitmaps both knock out and get knocked out */
	if (bitmap_get_opaque(bitmap))
		knockout_calculate(kx0, ky0, kx1, ky1, NULL);
	knockout_boxes[knockout_box_cur].bbox.x0 = kx0;
	knockout_boxes[knockout_box_cur].bbox.y0 = ky0;
	knockout_boxes[knockout_box_cur].bbox.x1 = kx1;
	knockout_boxes[knockout_box_cur].bbox.y1 = ky1;
	knockout_boxes[knockout_box_cur].deleted = false;
	knockout_boxes[knockout_box_cur].child = NULL;
	knockout_boxes[knockout_box_cur].next = knockout_list;
	knockout_list = &knockout_boxes[knockout_box_cur];
	knockout_entries[knockout_entry_cur].box = &knockout_boxes[knockout_box_cur];
	knockout_entries[knockout_entry_cur].data.bitmap_tile.x = x;
	knockout_entries[knockout_entry_cur].data.bitmap_tile.y = y;
	knockout_entries[knockout_entry_cur].data.bitmap_tile.width = width;
	knockout_entries[knockout_entry_cur].data.bitmap_tile.height = height;
	knockout_entries[knockout_entry_cur].data.bitmap_tile.bitmap = bitmap;
	knockout_entries[knockout_entry_cur].data.bitmap_tile.bg = bg;
	knockout_entries[knockout_entry_cur].data.bitmap_tile.repeat_x = repeat_x;
	knockout_entries[knockout_entry_cur].data.bitmap_tile.repeat_y = repeat_y;
	knockout_entries[knockout_entry_cur].data.bitmap_tile.content = content;
	knockout_entries[knockout_entry_cur].type = KNOCKOUT_PLOT_BITMAP_TILE;
	if ((++knockout_entry_cur >= KNOCKOUT_ENTRIES) ||
			(++knockout_box_cur >= KNOCKOUT_BOXES))
		knockout_plot_flush();
	return knockout_plot_clip(clip_x0_cur, clip_y0_cur, clip_x1_cur, clip_y1_cur);
}

bool knockout_plot_group_start(const char *name)
{
	knockout_entries[knockout_entry_cur].data.group_start.name = name;
	knockout_entries[knockout_entry_cur].type = KNOCKOUT_PLOT_GROUP_START;
	if (++knockout_entry_cur >= KNOCKOUT_ENTRIES)
		knockout_plot_flush();
	return true;
}

bool knockout_plot_group_end(void)
{
	knockout_entries[knockout_entry_cur].type = KNOCKOUT_PLOT_GROUP_END;
	if (++knockout_entry_cur >= KNOCKOUT_ENTRIES)
		knockout_plot_flush();
	return true;
}
