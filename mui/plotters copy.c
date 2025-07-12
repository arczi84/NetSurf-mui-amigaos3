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

#include <cybergraphx/cybergraphics.h>
#include <intuition/intuition.h>
#include <graphics/rpattr.h>
#include <proto/cybergraphics.h>
#include <proto/graphics.h>
#include <proto/layers.h>
#undef NO_INLINE_STDARG
#include <proto/ttengine.h>

#include "mui/gui.h"
#include "mui/bitmap.h"
#include "mui/font.h"
#include "mui/plotters.h"
#include "mui/utils.h"
#include "utils/utf8.h"
#include "utils/log.h"

#define USE_VMEM 0

#if USE_VMEM
#define BITMAP_FLAGS BMF_DISPLAYABLE | BMF_MINPLANES
#else
#define BITMAP_FLAGS BMF_MINPLANES
#endif

#define PATT_DOT  0xAAAA
#define PATT_DASH 0xCCCC
#define PATT_LINE 0xFFFF

struct RenderInfo renderinfo;

/* The bitmap format is ABGR */
#define ABGR_TO_ARGB(abgr) (((abgr >> 16) & 0xff) | (abgr & 0xff00) | ((abgr & 0xff) << 16))
#undef ABGR_TO_ARGB
#define ABGR_TO_ARGB(abgr) \
    (((abgr & 0xff) << 16) | ((abgr >> 8) & 0xff00) | ((abgr >> 16) & 0xff))
// Lub lepiej - dodaj funkcję konwersji
STATIC ULONG ConvertNetSurfColor(colour netsurf_color)
{
    LOG(("DEBUG: Converting NetSurf color 0x%08x", netsurf_color));
    
    // Usuń alfa kanał jeśli istnieje
    netsurf_color &= 0x00ffffff;
    
    // NetSurf używa formatu 0xBBGGRR, Amiga potrzebuje 0xRRGGBB
    UBYTE r = (netsurf_color >> 0) & 0xff;   // Red z LSB
    UBYTE g = (netsurf_color >> 8) & 0xff;   // Green ze środka  
    UBYTE b = (netsurf_color >> 16) & 0xff;  // Blue z MSB
    
    ULONG amiga_color = (r << 16) | (g << 8) | b;
    
    LOG(("DEBUG: Converted 0x%06x -> 0x%06x (R=%02x G=%02x B=%02x)", 
         netsurf_color, amiga_color, r, g, b));
    
    return amiga_color;
}
#if defined(__MORPHOS__)
STATIC CONST IPTR alphatags[] = { BLTBMA_USESOURCEALPHA, TRUE, TAG_DONE };
#endif

#if defined(__MORPHOS__)
STATIC VOID SetColor(struct RastPort *rp, ULONG c)
{
	SetRPAttrs(rp, RPTAG_PenMode, FALSE, RPTAG_FgColor, ABGR_TO_ARGB(c), TAG_DONE);
}

STATIC VOID SetColorAB(struct RastPort *rp, ULONG c, ULONG bg)
{
	SetRPAttrs(rp, RPTAG_PenMode, FALSE, RPTAG_FgColor, ABGR_TO_ARGB(c), RPTAG_BgColor, ABGR_TO_ARGB(bg), TAG_DONE);
}
#else
STATIC VOID SetPenColor(struct RastPort *rp, ULONG abgr, LONG pen)
{
	ULONG r, g, b;

	r = abgr & 0xff;
	g = abgr & 0xff00;
	b = abgr & 0xff0000;

	r |= r << 8 | r << 16 | r << 24;
	g |= g >> 8 | g << 8  | r << 16;
	b |= b << 8 | b >> 8  | b >> 16;

	SetRGB32(&renderinfo.screen->ViewPort, pen, r, g, b);
	SetAPen(rp, pen);
}
#if !defined(__MORPHOS__)
STATIC VOID SetColor(struct RastPort *rp, ULONG c)
{
    LOG(("DEBUG: SetColor called with color 0x%08x", c));
    
    ULONG amiga_color = ConvertNetSurfColor(c);
    
    if (global_pen_a >= 0) {
        SetRGB32(&renderinfo.screen->ViewPort, global_pen_a,
                 (amiga_color & 0xff) << 24,        // Red
                 ((amiga_color >> 8) & 0xff) << 24, // Green
                 ((amiga_color >> 16) & 0xff) << 24); // Blue
        
        SetAPen(rp, global_pen_a);
        LOG(("DEBUG: Set pen %d to color 0x%06x", global_pen_a, amiga_color));
    } else {
        LOG(("WARNING: global_pen_a not allocated"));
        SetAPen(rp, 1);
    }
    
    LOG(("DEBUG: SetColor completed"));
}
#endif
STATIC VOID SetColor1(struct RastPort *rp, ULONG c)
{
	SetPenColor(rp, c, global_pen_a);
}

STATIC VOID SetColorAB_org(struct RastPort *rp, ULONG c, ULONG bg)
{
	SetPenColor(rp, c, global_pen_a);
	SetPenColor(rp, bg, global_pen_b);
}
#if defined(__MORPHOS__)
STATIC VOID SetColorAB(struct RastPort *rp, ULONG c, ULONG bg)
{
	SetRPAttrs(rp, RPTAG_PenMode, FALSE, RPTAG_FgColor, ABGR_TO_ARGB(c), RPTAG_BgColor, ABGR_TO_ARGB(bg), TAG_DONE);
}
#else
#if !defined(__MORPHOS__)
STATIC VOID SetColorAB(struct RastPort *rp, ULONG c, ULONG bg)
{
    LOG(("DEBUG: SetColorAB called with c=0x%08x, bg=0x%08x", c, bg));
    
    // Konwertuj kolory NetSurf na Amiga
    ULONG fg_amiga = ConvertNetSurfColor(c);
    ULONG bg_amiga = ConvertNetSurfColor(bg);
    
    LOG(("DEBUG: Converted colors: fg=0x%06x, bg=0x%06x", fg_amiga, bg_amiga));
    
    if (global_pen_a >= 0 && global_pen_b >= 0) {
        // Ustaw kolory w palecie
        SetRGB32(&renderinfo.screen->ViewPort, global_pen_a, 
                 (fg_amiga & 0xff) << 24,       // Red
                 ((fg_amiga >> 8) & 0xff) << 24,  // Green  
                 ((fg_amiga >> 16) & 0xff) << 24); // Blue
                 
        SetRGB32(&renderinfo.screen->ViewPort, global_pen_b,
                 (bg_amiga & 0xff) << 24,       // Red
                 ((bg_amiga >> 8) & 0xff) << 24,  // Green
                 ((bg_amiga >> 16) & 0xff) << 24); // Blue
        
        SetAPen(rp, global_pen_a);
        SetBPen(rp, global_pen_b);
        
        LOG(("DEBUG: Set pens: fg=%d, bg=%d", global_pen_a, global_pen_b));
    } else {
        LOG(("WARNING: Pens not allocated, using defaults"));
        SetAPen(rp, 1);  // Czarny tekst
        SetBPen(rp, 0);  // Białe tło
    }
    
    LOG(("DEBUG: SetColorAB completed"));
}
#endif
#endif
#endif
static bool mui_clear(colour c)
{
    LOG(("DEBUG: mui_clear called with color 0x%08x", c));
    
    if (!renderinfo.rp) {
        LOG(("ERROR: renderinfo.rp is NULL!"));
        return false;
    }
    
    // Konwertuj kolor
    ULONG amiga_color = ConvertNetSurfColor(c);
    
    // Sprawdź czy to biały kolor
    if ((c & 0xffffff) == 0xffffff) {
        LOG(("DEBUG: Clearing with white color"));
        amiga_color = 0xffffff;  // Wymusimy biały
    }
    
    LOG(("DEBUG: FillPixelArray with color 0x%06x", amiga_color));
    FillPixelArray(renderinfo.rp, 0, 0, renderinfo.width, renderinfo.height, amiga_color);
    
    LOG(("DEBUG: mui_clear completed"));
    return true;
}
static bool mui_clear1(colour c)
{
	LOG(("DEBUG: mui_clear called with color 0x%08x", c));
	FillPixelArray(renderinfo.rp, 0, 0, renderinfo.width, renderinfo.height, ABGR_TO_ARGB(c));
	LOG(("DEBUG: mui_clear completed"));
	return true;
}
static bool mui_rectangle(int x0, int y0, int width, int height, int line_width, colour c, bool dotted, bool dashed)
{
	LOG(("DEBUG: mui_rectangle called (%d,%d) %dx%d", x0, y0, width, height));
	
	struct RastPort *rp;
	ULONG ptrn, pw, ph;

	rp = renderinfo.rp;
	
	if (!rp) {
		LOG(("ERROR: renderinfo.rp is NULL in mui_rectangle!"));
		return false;
	}

	ptrn = rp->LinePtrn;
	pw = rp->PenWidth;
	ph = rp->PenHeight;

	rp->PenWidth = line_width;
	rp->PenHeight = line_width;

	rp->LinePtrn = PATT_LINE;

	if(dotted) rp->LinePtrn = PATT_DOT;
	if(dashed) rp->LinePtrn = PATT_DASH;

	SetColor(rp, c);

	LOG(("DEBUG: Drawing rectangle at (%d,%d) with size %dx%d", x0, y0, width, height));
	Move(rp, x0, y0);
	Draw(rp, x0 + width, y0);
	Draw(rp, x0 + width, y0 + height);
	Draw(rp, x0, y0 + height);
	Draw(rp, x0, y0);

	rp->PenWidth = pw;
	rp->PenHeight = ph;
	rp->LinePtrn = ptrn;

	LOG(("DEBUG: mui_rectangle completed"));
	return true;
}

static bool mui_clip1(int x0, int y0, int x1, int y1)
{
	LOG(("DEBUG: mui_clip called (%d,%d) to (%d,%d)", x0, y0, x1, y1));
	
	STATIC struct Rectangle rect;
	struct RastPort *rp;
	struct Region *reg;

	rp = renderinfo.rp;
	
	if (!rp) {
		LOG(("ERROR: renderinfo.rp is NULL in mui_clip!"));
		return false;
	}
	
	if (!rp->Layer) {
		LOG(("WARNING: rp->Layer is NULL - clipping disabled"));
		// Jeśli nie ma Layer, po prostu zignoruj clipping
		return true;
	}

	LOG(("DEBUG: About to call InstallClipRegion"));
	reg = InstallClipRegion(rp->Layer, NULL);
	LOG(("DEBUG: InstallClipRegion returned %p", reg));

	if (!reg)
	{
		LOG(("DEBUG: Creating new region"));
		reg = NewRegion();
		if (!reg) {
			LOG(("ERROR: NewRegion failed!"));
			return false;
		}
	}
	else
	{
		LOG(("DEBUG: Clearing existing region"));
		ClearRectRegion(reg, &rect);
	}

	rect.MinX = x0;
	rect.MinY = y0;
	rect.MaxX = x1-1;
	rect.MaxY = y1-1;

	LOG(("DEBUG: Setting rectangle (%d,%d) to (%d,%d)", rect.MinX, rect.MinY, rect.MaxX, rect.MaxY));
	OrRectRegion(reg, &rect);

	LOG(("DEBUG: Installing clip region"));
	reg = InstallClipRegion(rp->Layer, reg);

	if (reg) {
		LOG(("DEBUG: Disposing old region"));
		DisposeRegion(reg);
	}

	LOG(("DEBUG: mui_clip completed"));
	return true;
}
static bool mui_clip(int x0, int y0, int x1, int y1)
{
	LOG(("DEBUG: mui_clip called (%d,%d) to (%d,%d)", x0, y0, x1, y1));
	
	STATIC struct Rectangle rect;
	struct RastPort *rp;
	struct Region *reg;

	rp = renderinfo.rp;
	
	if (!rp) {
		LOG(("ERROR: renderinfo.rp is NULL in mui_clip!"));
		return false;
	}
	
	if (!rp->Layer) {
		LOG(("ERROR: rp->Layer is NULL in mui_clip!"));
		return false;
	}

	LOG(("DEBUG: About to call InstallClipRegion"));
	reg = InstallClipRegion(rp->Layer, NULL);
	LOG(("DEBUG: InstallClipRegion returned %p", reg));

	if (!reg)
	{
		LOG(("DEBUG: Creating new region"));
		reg = NewRegion();
		if (!reg) {
			LOG(("ERROR: NewRegion failed!"));
			return false;
		}
	}
	else
	{
		LOG(("DEBUG: Clearing existing region"));
		ClearRectRegion(reg, &rect);
	}

	rect.MinX = x0;
	rect.MinY = y0;
	rect.MaxX = x1-1;
	rect.MaxY = y1-1;

	LOG(("DEBUG: Setting rectangle (%d,%d) to (%d,%d)", rect.MinX, rect.MinY, rect.MaxX, rect.MaxY));
	OrRectRegion(reg, &rect);

	LOG(("DEBUG: Installing clip region"));
	reg = InstallClipRegion(rp->Layer, reg);

	if (reg) {
		LOG(("DEBUG: Disposing old region"));
		DisposeRegion(reg);
	}

	LOG(("DEBUG: mui_clip completed"));
	return true;
}


static bool mui_line(int x0, int y0, int x1, int y1, int width, colour c, bool dotted, bool dashed)
{
	struct RastPort *rp;
	ULONG ptrn, pw, ph;

	rp  = renderinfo.rp;

	ptrn = rp->LinePtrn;
	pw = rp->PenWidth;
	ph = rp->PenHeight;

	rp->PenWidth = width;
	rp->PenHeight = width;

	rp->LinePtrn = PATT_LINE;
	if(dotted) rp->LinePtrn = PATT_DOT;
	if(dashed) rp->LinePtrn = PATT_DASH;

	SetColor(rp, c);
	Move(rp, x0, y0);
	Draw(rp, x1, y1);

	rp->PenWidth = pw;
	rp->PenHeight = ph;
	rp->LinePtrn = ptrn;

	return true;
}

static bool mui_polygon(int *p, unsigned int n, colour fill)
{

	return true;
}   
static bool mui_fill(int x0, int y0, int x1, int y1, colour c)
{
    LOG(("DEBUG: mui_fill called (%d,%d) to (%d,%d) color 0x%08x", x0, y0, x1, y1, c));
    
    if (!renderinfo.rp) {
        LOG(("ERROR: renderinfo.rp is NULL!"));
        return false;
    }
    
    ULONG amiga_color = ConvertNetSurfColor(c);
    
    LOG(("DEBUG: FillPixelArray with converted color 0x%06x", amiga_color));
    FillPixelArray(renderinfo.rp, x0, y0, x1 - x0 + 1, y1 - y0 + 1, amiga_color);
    
    LOG(("DEBUG: mui_fill completed"));
    return true;
}
static bool mui_fill1(int x0, int y0, int x1, int y1, colour c)
{
	LOG(("DEBUG: mui_fill called (%d,%d) to (%d,%d) color 0x%08x", x0, y0, x1, y1, c));
	FillPixelArray(renderinfo.rp, x0, y0, x1 - x0 + 1, y1 - y0 + 1, ABGR_TO_ARGB(c));
	LOG(("DEBUG: mui_fill completed"));
	return true;
}
            
static bool mui_text(int x, int y, const struct css_style *style, const char *text, size_t length, colour bg, colour c)
{
	LOG(("DEBUG: mui_text called at (%d,%d) length=%d", x, y, length));
	
	if (length && text && *text != '\0')
	{
		struct RastPort *rp;
		APTR tfont;

		rp = renderinfo.rp;
		
		if (!rp) {
			LOG(("ERROR: renderinfo.rp is NULL in mui_text!"));
			return false;
		}

		LOG(("DEBUG: Opening font"));
		tfont = mui_open_font(rp, style);
		LOG(("DEBUG: mui_open_font returned %p", tfont));
		if (tfont)
		{
			ULONG baseline, fontheight;
			LOG(("DEBUG: mui_open_font succeeded, font=%p", tfont));
			TT_GetAttrs(rp, TT_FontHeight, &fontheight, TT_FontBaseline, &baseline, TAG_DONE);
			LOG(("DEBUG: Setting color"));
			SetColorAB(rp, c, bg);
			LOG(("DEBUG: Font height %ld, baseline %ld", fontheight, baseline));	
			Move(rp, x, y);
			/* TTEngine uses number of glyphs but NetSurf gives us number of bytes */
			length = utf8_codepoints(text, length);
			LOG(("DEBUG: Number of glyphs to draw: %d", length));
			LOG(("DEBUG: Drawing text, glyphs=%d", length));
			TT_Text(rp, (STRPTR)text, length);

			mui_close_font(rp, tfont);
			LOG(("DEBUG: Font closed"));
		}
		else
		{
			LOG(("ERROR: Failed to open font"));
		}
	}

	LOG(("DEBUG: mui_text completed"));
	return true;
}
static bool mui_disc(int x, int y, int radius, colour c, bool filled)
{
	LOG(("DEBUG: mui_disc called at (%d,%d) radius=%d color=0x%08x filled=%d", x, y, radius, c, filled));
	#if defined(__MORPHOS__)
	SetColor(renderinfo.rp, c);
	#endif
	DrawEllipse(renderinfo.rp, x, y, radius, radius);

	if (filled)
	{
		// Flood() fill ? Nah...
	}

	return true;
}

static bool mui_arc(int x, int y, int radius, int angle1, int angle2, colour c)
{
	return true;
}

static bool mui_bitmap(int x, int y, int width, int height, struct bitmap *bm, colour bg, struct content *content)
{
	LOG(("DEBUG: mui_bitmap called at (%d,%d) size %dx%d", x, y, width, height));
	if (width && height)
	{
		struct BitMap *tbm;
		ULONG bmwidth, bmheight;

		tbm = bm->bitmap;
		bmwidth = bm->width;
		bmheight = bm->height;

		if (tbm == NULL)
		{
			bm->bitmap = tbm = AllocBitMap(bmwidth, bmheight, 32,  BITMAP_FLAGS, renderinfo.rp->BitMap);
			bm->update = 1;
		}

		if (tbm)
		{
			struct BitMap *scaledbm = NULL;

			if (bm->update)
			{
				struct RastPort trp;

				InitRastPort(&trp);
				trp.BitMap = tbm;
				bm->update = 0;

				WritePixelArray(bm->pixdata, 0, 0, bm->modulo, &trp, 0, 0, bmwidth, bmheight, RECTFMT_RGBA);
			}

			if (bmwidth != width || bmheight != height)
			{
				scaledbm = AllocBitMap(width, height, 32, BITMAP_FLAGS, tbm);

				if (scaledbm)
				{
					struct BitScaleArgs bsa;
					APTR tmp;

					bsa.bsa_SrcX = 0;
					bsa.bsa_SrcY = 0;
					bsa.bsa_SrcWidth = bmwidth;
					bsa.bsa_SrcHeight = bmheight;
					bsa.bsa_DestX = 0;
					bsa.bsa_DestY = 0;
					bsa.bsa_XSrcFactor = bmwidth;
					bsa.bsa_XDestFactor = width;
					bsa.bsa_YSrcFactor = bmheight;
					bsa.bsa_YDestFactor = height;
					bsa.bsa_SrcBitMap = tbm;
					bsa.bsa_DestBitMap = scaledbm;
					bsa.bsa_Flags = 0;

					BitMapScale(&bsa);

					tbm = scaledbm;
				}
			}

			if (bm->opaque)
			{
				BltBitMapRastPort(tbm, 0, 0, renderinfo.rp, x, y, width, height, 0xc0);
			}
			else
			{
				#if defined(__MORPHOS__)
				BltBitMapRastPortAlpha(tbm, 0, 0, renderinfo.rp, x, y, width, height, (APTR)&alphatags);
				#else
				BltBitMapRastPort(tbm, 0, 0, renderinfo.rp, x, y, width, height, 0xc0);
				#endif
			}

			if (scaledbm)
			{
				FreeBitMap(scaledbm);
			}

			bitmap_flush(bm, FALSE);
		}
	}

	return true;
}

static bool mui_bitmap_tile(int x, int y, int width, int height, struct bitmap *bm, colour bg, bool repeat_x, bool repeat_y, struct content *content)
{
	LOG(("DEBUG: mui_bitmap_tile called at (%d,%d) size %dx%d", x, y, width, height));
	struct BitMap *tbm;
	ULONG bmwidth, bmheight;

	tbm = bm->bitmap;
	bmwidth = bm->width;
	bmheight = bm->height;

	if (tbm == NULL)
	{
		bm->bitmap = tbm = AllocBitMap(bmwidth, bmheight, 32,  BITMAP_FLAGS, renderinfo.rp->BitMap);
		bm->update = 1;
	}

	if (tbm)
	{
		struct RastPort *rp;
		ULONG max_width, max_height, xf;

		rp = renderinfo.rp;

		max_width =  (repeat_x ? renderinfo.width : width);
		max_height = (repeat_y ? renderinfo.height : height);

		if (repeat_x && x < -bmwidth)
		{
			while (x < -bmwidth)
				x += bmwidth;
		}

		if (repeat_y && y < -bmheight)
		{
			while (y < -bmheight)
				y += bmheight;
		}

		if (bm->update)
		{
			struct RastPort rp;

			InitRastPort(&rp);
			rp.BitMap = tbm;
			bm->update = 0;

			WritePixelArray(bm->pixdata, 0, 0, bm->modulo, &rp, 0, 0, bmwidth, bmheight, RECTFMT_RGBA);
		}

		for (xf = 0; xf < max_width; xf += bmwidth)
		{
			ULONG yf, wf, hf;

			wf = bmwidth;

			if (width > xf + bmwidth)
				wf = width - (xf + bmwidth);

			for (yf = 0; yf < max_height; yf += bmheight)
			{
				hf = bmheight;

				if (height > yf + bmheight)
					hf = height - (yf + bmheight);

				if (bm->opaque)
				{
					BltBitMapRastPort(tbm, 0, 0, rp, x + xf, y + yf, wf, hf, 0xc0);
				}
				else
				{
					#if defined(__MORPHOS__)
					BltBitMapRastPortAlpha(tbm, 0, 0, rp, x + xf, y + yf, wf, hf, (APTR)&alphatags);
					#else
					BltBitMapRastPort(tbm, 0, 0, rp, x + xf, y + yf, wf, hf, 0xc0);
					#endif
				}
			}
		}

		bitmap_flush(bm, FALSE);
	}

	return true;
}

static bool mui_group_start(const char *name)
{
	return false;
}

static bool mui_group_end(void)
{
	return false;
}

static bool mui_flush(void)
{
	return true;
}

static bool mui_path(float *p, unsigned int n, colour fill, float width, colour c, float *transform)
{
	return true;
}

struct plotter_table plot;

const struct plotter_table muiplot =
{
	mui_clear,
	mui_rectangle,
	mui_line,
	mui_polygon,
	mui_fill,
	mui_clip,
	mui_text,
	mui_disc,
	mui_arc,
	mui_bitmap,
	mui_bitmap_tile,
	NULL, //mui_group_start,
	NULL, //mui_group_end,
	NULL, //mui_flush, // optional
	mui_path,
	false // option_knockout
};
