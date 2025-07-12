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
#include <proto/exec.h>
#include <math.h>

#include "mui/gui.h"
#include "mui/bitmap.h"
#include "mui/font.h"
#include "mui/plotters.h"
#include "mui/utils.h"
#include "utils/utf8.h"
#include "utils/log.h"

#define BITMAP_FLAGS BMF_MINPLANES  // No VMEM for RTG systems
#define PATT_DOT  0xAAAA
#define PATT_DASH 0xCCCC
#define PATT_LINE 0xFFFF

struct RenderInfo renderinfo;
static LONG global_pen_a = -1, global_pen_b = -1;  // Pre-allocated pens

/* Convert NetSurf color (0xBBGGRR) to Amiga RGB (0xRRGGBB) */
static ULONG ConvertNetSurfColor(colour netsurf_color)
{
    netsurf_color &= 0x00FFFFFF;  // Strip alpha
    UBYTE r = (netsurf_color >> 0) & 0xFF;
    UBYTE g = (netsurf_color >> 8) & 0xFF;
    UBYTE b = (netsurf_color >> 16) & 0xFF;
    return (r << 16) | (g << 8) | b;
}

/* Convert Amiga RGB (0xRRGGBB) to 16-bit RGB565 if needed */
static UWORD ConvertToRGB565(ULONG amiga_color)
{
    UBYTE r = (amiga_color >> 16) & 0xFF;
    UBYTE g = (amiga_color >> 8) & 0xFF;
    UBYTE b = (amiga_color >> 0) & 0xFF;
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}

/* Set foreground and background pens, handling 16/32-bit displays */
static void SetColorAB(struct RastPort *rp, ULONG c, ULONG bg)
{
    if (!rp) {
        LOG(("ERROR: RastPort is NULL in SetColorAB"));
        return;
    }

    ULONG fg_amiga = ConvertNetSurfColor(c);
    ULONG bg_amiga = ConvertNetSurfColor(bg);
    ULONG depth = GetBitMapAttr(renderinfo.rp->BitMap, BMA_DEPTH);

    if (global_pen_a >= 0 && global_pen_b >= 0) {
        if (depth <= 16) {
            // 16-bit RGB565
            SetRGB4(&renderinfo.screen->ViewPort, global_pen_a,
                    (fg_amiga >> 16) & 0xFF, (fg_amiga >> 8) & 0xFF, fg_amiga & 0xFF);
            SetRGB4(&renderinfo.screen->ViewPort, global_pen_b,
                    (bg_amiga >> 16) & 0xFF, (bg_amiga >> 8) & 0xFF, bg_amiga & 0xFF);
        } else {
            // 32-bit RGB888
            SetRGB32(&renderinfo.screen->ViewPort, global_pen_a,
                     (fg_amiga & 0xFF) << 24, ((fg_amiga >> 8) & 0xFF) << 24, ((fg_amiga >> 16) & 0xFF) << 24);
            SetRGB32(&renderinfo.screen->ViewPort, global_pen_b,
                     (bg_amiga & 0xFF) << 24, ((bg_amiga >> 8) & 0xFF) << 24, ((bg_amiga >> 16) & 0xFF) << 24);
        }
        SetAPen(rp, global_pen_a);
        SetBPen(rp, global_pen_b);
    } else {
        LOG(("WARNING: Pens not allocated, using defaults"));
        SetAPen(rp, 1);  // Black
        SetBPen(rp, 0);  // White
    }
}

/* Clear the rendering area with a color */
static bool mui_clear(colour c)
{
    if (!renderinfo.rp) {
        LOG(("ERROR: renderinfo.rp is NULL"));
        return false;
    }

    ULONG amiga_color = ConvertNetSurfColor(c);
    if ((c & 0xFFFFFF) == 0xFFFFFF) {
        amiga_color = 0xFFFFFF;  // Force white
    }

    ULONG depth = GetBitMapAttr(renderinfo.rp->BitMap, BMA_DEPTH);
    if (depth <= 16) {
        amiga_color = ConvertToRGB565(amiga_color);
        WritePixelArray(NULL, 0, 0, 0, renderinfo.rp, 0, 0, renderinfo.width, renderinfo.height, RECTFMT_RGB);
        SetAPen(renderinfo.rp, global_pen_a);
        RectFill(renderinfo.rp, 0, 0, renderinfo.width - 1, renderinfo.height - 1);
    } else {
        FillPixelArray(renderinfo.rp, 0, 0, renderinfo.width, renderinfo.height, amiga_color);
    }

    return true;
}

/* Draw a rectangle */
static bool mui_rectangle(int x0, int y0, int width, int height, int line_width, colour c, bool dotted, bool dashed)
{
    if (!renderinfo.rp) {
        LOG(("ERROR: renderinfo.rp is NULL"));
        return false;
    }

    struct RastPort *rp = renderinfo.rp;
    ULONG ptrn = rp->LinePtrn, pw = rp->PenWidth, ph = rp->PenHeight;

    rp->PenWidth = line_width;
    rp->PenHeight = line_width;
    rp->LinePtrn = dotted ? PATT_DOT : dashed ? PATT_DASH : PATT_LINE;

    SetColorAB(rp, c, 0xFFFFFF);  // White background
    Move(rp, x0, y0);
    Draw(rp, x0 + width - 1, y0);
    Draw(rp, x0 + width - 1, y0 + height - 1);
    Draw(rp, x0, y0 + height - 1);
    Draw(rp, x0, y0);

    rp->PenWidth = pw;
    rp->PenHeight = ph;
    rp->LinePtrn = ptrn;

    return true;
}

/* Clip the rendering area */
static bool mui_clip(int x0, int y0, int x1, int y1)
{
    if (!renderinfo.rp || !renderinfo.rp->Layer) {
        LOG(("ERROR: RastPort or Layer is NULL"));
        return false;
    }

    struct RastPort *rp = renderinfo.rp;
    static struct Rectangle rect;
    struct Region *reg = InstallClipRegion(rp->Layer, NULL);

    if (!reg) {
        reg = NewRegion();
        if (!reg) {
            LOG(("ERROR: NewRegion failed"));
            return false;
        }
    } else {
        ClearRectRegion(reg, &rect);
    }

    rect.MinX = x0;
    rect.MinY = y0;
    rect.MaxX = x1 - 1;
    rect.MaxY = y1 - 1;
    OrRectRegion(reg, &rect);

    reg = InstallClipRegion(rp->Layer, reg);
    if (reg) {
        DisposeRegion(reg);
    }

    return true;
}

/* Draw a line */
static bool mui_line(int x0, int y0, int x1, int y1, int width, colour c, bool dotted, bool dashed)
{
    if (!renderinfo.rp) {
        LOG(("ERROR: renderinfo.rp is NULL"));
        return false;
    }

    struct RastPort *rp = renderinfo.rp;
    ULONG ptrn = rp->LinePtrn, pw = rp->PenWidth, ph = rp->PenHeight;

    rp->PenWidth = width;
    rp->PenHeight = width;
    rp->LinePtrn = dotted ? PATT_DOT : dashed ? PATT_DASH : PATT_LINE;

    SetColorAB(rp, c, 0xFFFFFF);
    Move(rp, x0, y0);
    Draw(rp, x1, y1);

    rp->PenWidth = pw;
    rp->PenHeight = ph;
    rp->LinePtrn = ptrn;

    return true;
}

/* Draw a filled polygon */
static bool mui_polygon(int *p, unsigned int n, colour fill)
{
    if (!renderinfo.rp || n < 3) {
        LOG(("ERROR: Invalid polygon data or RastPort is NULL"));
        return false;
    }

    struct RastPort *rp = renderinfo.rp;
    SetColorAB(rp, fill, 0xFFFFFF);

    struct AreaInfo areaInfo;
    struct TmpRas tmpRas;
    UBYTE buffer[512];  // Buffer for AreaInfo
    InitArea(&areaInfo, buffer, sizeof(buffer) / 5);
    InitTmpRas(&tmpRas, AllocMem(renderinfo.width * renderinfo.height, MEMF_FAST), renderinfo.width);

    rp->AreaInfo = &areaInfo;
    rp->TmpRas = &tmpRas;

    AreaMove(rp, p[0], p[1]);
    for (unsigned int i = 1; i < n; i++) {
        AreaDraw(rp, p[i * 2], p[i * 2 + 1]);
    }
    AreaEnd(rp);

    rp->AreaInfo = NULL;
    rp->TmpRas = NULL;
    FreeMem(tmpRas.RasPtr, renderinfo.width * renderinfo.height);

    return true;
}

/* Fill a rectangular area */
static bool mui_fill(int x0, int y0, int x1, int y1, colour c)
{
    if (!renderinfo.rp) {
        LOG(("ERROR: renderinfo.rp is NULL"));
        return false;
    }

    ULONG amiga_color = ConvertNetSurfColor(c);
    ULONG depth = GetBitMapAttr(renderinfo.rp->BitMap, BMA_DEPTH);

    if (depth <= 16) {
        SetColorAB(renderinfo.rp, c, 0xFFFFFF);
        RectFill(renderinfo.rp, x0, y0, x1 - 1, y1 - 1);
    } else {
        FillPixelArray(renderinfo.rp, x0, y0, x1 - x0 + 1, y1 - y0 + 1, amiga_color);
    }

    return true;
}

/* Render text */
static bool mui_text(int x, int y, const struct css_style *style, const char *text, size_t length, colour bg, colour c)
{

    struct RastPort *rp = renderinfo.rp;

    if (!renderinfo.rp)
    {
        LOG(("ERROR: renderinfo.rp is NULL"));
        //return false;
    }
    if (!text)
    {
        LOG(("ERROR: text is NULL"));
        //return false;
    }
    
    if (length == 0)
    {
        LOG(("ERROR: length is 0"));
       // return false;
    }

    APTR tfont = mui_open_font(rp, style);
    if (!tfont) {
        LOG(("ERROR: Failed to open font"));
        return false;
    }

    ULONG baseline, fontheight;
    TT_GetAttrs(rp, TT_FontHeight, &fontheight, TT_FontBaseline, &baseline, TAG_DONE);
    SetColorAB(rp, c, bg);
    Move(rp, x, y + baseline);
    length = utf8_codepoints(text, length);
    TT_Text(rp, (STRPTR)text, length);

    mui_close_font(rp, tfont);
    return true;
}

/* Draw a disc (circle) */
static bool mui_disc(int x, int y, int radius, colour c, bool filled)
{
    if (!renderinfo.rp) {
        LOG(("ERROR: renderinfo.rp is NULL"));
        return false;
    }

    SetColorAB(renderinfo.rp, c, 0xFFFFFF);
    if (filled) {
        // Approximate filled circle with AreaEllipse
        struct AreaInfo areaInfo;
        struct TmpRas tmpRas;
        UBYTE buffer[512];
        InitArea(&areaInfo, buffer, sizeof(buffer) / 5);
        InitTmpRas(&tmpRas, AllocMem(renderinfo.width * renderinfo.height, MEMF_FAST), renderinfo.width);

        renderinfo.rp->AreaInfo = &areaInfo;
        renderinfo.rp->TmpRas = &tmpRas;

        AreaEllipse(renderinfo.rp, x, y, radius, radius);
        AreaEnd(renderinfo.rp);

        renderinfo.rp->AreaInfo = NULL;
        renderinfo.rp->TmpRas = NULL;
        FreeMem(tmpRas.RasPtr, renderinfo.width * renderinfo.height);
    } else {
        DrawEllipse(renderinfo.rp, x, y, radius, radius);
    }

    return true;
}

/* Draw an arc */
static bool mui_arc(int x, int y, int radius, int angle1, int angle2, colour c)
{
    if (!renderinfo.rp) {
        LOG(("ERROR: renderinfo.rp is NULL"));
        return false;
    }

    SetColorAB(renderinfo.rp, c, 0xFFFFFF);
    // Approximate arc with a series of lines
    int steps = 32;
    double angle_step = (angle2 - angle1) * M_PI / (180.0 * steps);
    double start_rad = angle1 * M_PI / 180.0;

    Move(renderinfo.rp, x + radius * cos(start_rad), y + radius * sin(start_rad));
    for (int i = 1; i <= steps; i++) {
        double angle = start_rad + i * angle_step;
        Draw(renderinfo.rp, x + radius * cos(angle), y + radius * sin(angle));
    }

    return true;
}

/* Draw a bitmap */
static bool mui_bitmap(int x, int y, int width, int height, struct bitmap *bm, colour bg, struct content *content)
{
    return false;  // Not implemented in this example
    if (!renderinfo.rp || !bm || width <= 0 || height <= 0) {
        LOG(("ERROR: Invalid bitmap data or RastPort is NULL"));
        return false;
    }

    struct BitMap *tbm = bm->bitmap;
    ULONG bmwidth = bm->width, bmheight = bm->height;
    ULONG depth = GetBitMapAttr(renderinfo.rp->BitMap, BMA_DEPTH);
    ULONG bitmap_depth = (depth <= 16) ? 16 : 32;

    if (!tbm) {
        tbm = bm->bitmap = AllocBitMap(bmwidth, bmheight, bitmap_depth, BITMAP_FLAGS, renderinfo.rp->BitMap);
        if (!tbm) {
            LOG(("ERROR: AllocBitMap failed"));
            return false;
        }
        bm->update = 1;
    }

    if (bm->update) {
        struct RastPort trp;
        InitRastPort(&trp);
        trp.BitMap = tbm;
        bm->update = 0;

        if (depth <= 16) {
            // Convert RGBA to RGB565 for 16-bit displays
            UWORD *rgb565_data = AllocMem(bmwidth * bmheight * 2, MEMF_ANY);
            if (rgb565_data) {
                for (ULONG i = 0; i < bmwidth * bmheight; i++) {
                    ULONG rgba = ((ULONG *)bm->pixdata)[i];
                    UBYTE r = (rgba >> 24) & 0xFF;
                    UBYTE g = (rgba >> 16) & 0xFF;
                    UBYTE b = (rgba >> 8) & 0xFF;
                    rgb565_data[i] = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
                }
                WritePixelArray(rgb565_data, 0, 0, bmwidth * 2, &trp, 0, 0, bmwidth, bmheight, RECTFMT_RGB);
                FreeMem(rgb565_data, bmwidth * bmheight * 2);
            }
        } else {
            WritePixelArray(bm->pixdata, 0, 0, bm->modulo, &trp, 0, 0, bmwidth, bmheight, RECTFMT_RGBA);
        }
    }

    struct BitMap *scaledbm = NULL;
    if (bmwidth != (ULONG)width || bmheight != (ULONG)height) {
        scaledbm = AllocBitMap(width, height, bitmap_depth, BITMAP_FLAGS, tbm);
        if (scaledbm) {
            struct BitScaleArgs bsa = {
                .bsa_SrcX = 0, .bsa_SrcY = 0,
                .bsa_SrcWidth = bmwidth, .bsa_SrcHeight = bmheight,
                .bsa_DestX = 0, .bsa_DestY = 0,
                .bsa_XSrcFactor = bmwidth, .bsa_XDestFactor = width,
                .bsa_YSrcFactor = bmheight, .bsa_YDestFactor = height,
                .bsa_SrcBitMap = tbm, .bsa_DestBitMap = scaledbm,
                .bsa_Flags = 0
            };
            BitMapScale(&bsa);
            tbm = scaledbm;
        } else {
            LOG(("WARNING: Failed to allocate scaled bitmap"));
        }
    }

    BltBitMapRastPort(tbm, 0, 0, renderinfo.rp, x, y, width, height, 0xC0);
    if (scaledbm) {
        FreeBitMap(scaledbm);
    }

    bitmap_flush(bm, FALSE);
    return true;
}

/* Tile a bitmap */
static bool mui_bitmap_tile(int x, int y, int width, int height, struct bitmap *bm, colour bg, bool repeat_x, bool repeat_y, struct content *content)
{
    if (!renderinfo.rp || !bm || width <= 0 || height <= 0) {
        LOG(("ERROR: Invalid bitmap data or RastPort is NULL"));
        return false;
    }

    struct BitMap *tbm = bm->bitmap;
    ULONG bmwidth = bm->width, bmheight = bm->height;
    ULONG depth = GetBitMapAttr(renderinfo.rp->BitMap, BMA_DEPTH);
    ULONG bitmap_depth = (depth <= 16) ? 16 : 32;

    if (!tbm) {
        tbm = bm->bitmap = AllocBitMap(bmwidth, bmheight, bitmap_depth, BITMAP_FLAGS, renderinfo.rp->BitMap);
        if (!tbm) {
            LOG(("ERROR: AllocBitMap failed"));
            return false;
        }
        bm->update = 1;
    }

    if (bm->update) {
        struct RastPort trp;
        InitRastPort(&trp);
        trp.BitMap = tbm;
        bm->update = 0;

        if (depth <= 16) {
            UWORD *rgb565_data = AllocMem(bmwidth * bmheight * 2, MEMF_ANY);
            if (rgb565_data) {
                for (ULONG i = 0; i < bmwidth * bmheight; i++) {
                    ULONG rgba = ((ULONG *)bm->pixdata)[i];
                    UBYTE r = (rgba >> 24) & 0xFF;
                    UBYTE g = (rgba >> 16) & 0xFF;
                    UBYTE b = (rgba >> 8) & 0xFF;
                    rgb565_data[i] = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
                }
                WritePixelArray(rgb565_data, 0, 0, bmwidth * 2, &trp, 0, 0, bmwidth, bmheight, RECTFMT_RGB);
                FreeMem(rgb565_data, bmwidth * bmheight * 2);
            }
        } else {
            WritePixelArray(bm->pixdata, 0, 0, bm->modulo, &trp, 0, 0, bmwidth, bmheight, RECTFMT_RGBA);
        }
    }

    struct RastPort *rp = renderinfo.rp;
    ULONG max_width = repeat_x ? renderinfo.width : width;
    ULONG max_height = repeat_y ? renderinfo.height : height;

    if (repeat_x && x < -(int)bmwidth) {
        while (x < -(int)bmwidth) x += bmwidth;
    }
    if (repeat_y && y < -(int)bmheight) {
        while (y < -(int)bmheight) y += bmheight;
    }

    for (ULONG xf = 0; xf < max_width; xf += bmwidth) {
        ULONG wf = (width - xf < bmwidth) ? width - xf : bmwidth;
        for (ULONG yf = 0; yf < max_height; yf += bmheight) {
            ULONG hf = (height - yf < bmheight) ? height - yf : bmheight;
            BltBitMapRastPort(tbm, 0, 0, rp, x + xf, y + yf, wf, hf, 0xC0);
        }
    }

    bitmap_flush(bm, FALSE);
    return true;
}

/* Start a group (stub, not needed for basic rendering) */
static bool mui_group_start(const char *name)
{
    LOG(("DEBUG: mui_group_start called with name %s", name));
    return true;
}

/* End a group (stub) */
static bool mui_group_end(void)
{
    LOG(("DEBUG: mui_group_end called"));
    return true;
}

/* Flush rendering (no-op for immediate rendering) */
static bool mui_flush(void)
{
    LOG(("DEBUG: mui_flush called"));
    return true;
}

/* Draw a path */
static bool mui_path(float *p, unsigned int n, colour fill, float width, colour c, float *transform)
{
    if (!renderinfo.rp || n < 2) {
        LOG(("ERROR: Invalid path data or RastPort is NULL"));
        return false;
    }

    struct RastPort *rp = renderinfo.rp;
    if (fill != 0xFFFFFFFF) {
        // Filled path (treat as polygon)
        SetColorAB(rp, fill, 0xFFFFFF);
        struct AreaInfo areaInfo;
        struct TmpRas tmpRas;
        UBYTE buffer[512];
        InitArea(&areaInfo, buffer, sizeof(buffer) / 5);
        void *ras_ptr = AllocMem(renderinfo.width * renderinfo.height, MEMF_FAST);
        if (!ras_ptr) {
            LOG(("ERROR: Failed to allocate TmpRas"));
            return false;
        }
        InitTmpRas(&tmpRas, ras_ptr, renderinfo.width);

        rp->AreaInfo = &areaInfo;
        rp->TmpRas = &tmpRas;

        AreaMove(rp, (int)p[0], (int)p[1]);
        for (unsigned int i = 1; i < n; i++) {
            AreaDraw(rp, (int)p[i * 2], (int)p[i * 2 + 1]);
        }
        AreaEnd(rp);

        rp->AreaInfo = NULL;
        rp->TmpRas = NULL;
        FreeMem(ras_ptr, renderinfo.width * renderinfo.height);
    } else {
        // Outline path
        ULONG ptrn = rp->LinePtrn, pw = rp->PenWidth, ph = rp->PenHeight;
        rp->PenWidth = (int)width;
        rp->PenHeight = (int)width;
        rp->LinePtrn = PATT_LINE;

        SetColorAB(rp, c, 0xFFFFFF);
        Move(rp, (int)p[0], (int)p[1]);
        for (unsigned int i = 1; i < n; i++) {
            Draw(rp, (int)p[i * 2], (int)p[i * 2 + 1]);
        }

        rp->PenWidth = pw;
        rp->PenHeight = ph;
        rp->LinePtrn = ptrn;
    }

    return true;
}
struct plotter_table plot;
/* Plotter table */
const struct plotter_table muiplot = {
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
    mui_group_start,
    mui_group_end,
    mui_flush,
    mui_path,
    false  // option_knockout
};