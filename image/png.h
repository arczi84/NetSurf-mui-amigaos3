/*
 * Copyright 2003 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2008 Daniel Silverstone <dsilvers@netsurf-browser.org>
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

#ifndef _NETSURF_RISCOS_PNG_H_
#define _NETSURF_RISCOS_PNG_H_

#include "utils/config.h"

#ifdef WITH_PNG

#include "css/css.h"

#include <stdbool.h>
#include <png.h>

struct content;
struct bitmap;

struct content_png_data {
	png_structp png;
	png_infop info;
	int interlace;
        struct bitmap *bitmap;	/**< Created NetSurf bitmap */
        unsigned char *bitbuffer; /**< Bitmap buffer */
        size_t rowstride, bpp; /**< Bitmap rowstride and bpp */
        size_t rowbytes; /**< Number of bytes per row */
};

bool nspng_create(struct content *c, const char *params[]);
bool nspng_process_data(struct content *c, char *data, unsigned int size);
bool nspng_convert(struct content *c, int width, int height);
void nspng_destroy(struct content *c);
bool nspng_redraw(struct content *c, int x, int y,
		int width, int height,
		int clip_x0, int clip_y0, int clip_x1, int clip_y1,
		float scale, colour background_colour);
bool nspng_redraw_tiled(struct content *c, int x, int y, int width, int height,
                        int clip_x0, int clip_y0, int clip_x1, int clip_y1,
                        float scale, colour background_colour,
                        bool repeat_x, bool repeat_y);
#endif

#endif
