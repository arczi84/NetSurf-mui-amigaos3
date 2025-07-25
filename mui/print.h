#ifndef MUI_PRINT_H
#define MUI_PRINT_H

/*
 * Copyright 2009 Ilkka Lehtoranta
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

enum
{
	PRTTAG_BitMap = TAG_USER,
	PRTTAG_ARGB,
	PRTTAG_RGB,
	PRTTAG_Width,
	PRTTAG_Height,
	PRTTAG_Modulo,
	PRTTAG_ColorMap,
};

VOID print_doc(struct RastPort *rp, ULONG width, ULONG height);

#endif /* MUI_PRINT_H */
