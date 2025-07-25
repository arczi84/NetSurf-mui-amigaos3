#ifndef MUI_FONT_H
#define MUI_FONT_H

/*
 * Copyright 2009 Ilkka Lehtoranta <ilkleht@isoveli.org>
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

#include "css/css.h"

struct RastPort;

void font_init(void);
void font_cleanup(void);
void font_cache_check(void);
APTR mui_open_font(struct RastPort *rp, const struct css_style *style);
void mui_close_font(struct RastPort *rp, APTR tfont);

#endif /* MUI_FONT_H */
