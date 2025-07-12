/*
 * mui/plotters.c
 * NetSurf MUI render functions with proper MUI-relative offsets
 *
 * Copyright 2009 Ilkka Lehtoranta <ilkleht@isoveli.org>
 * Modifications 2025 by <Your Name> to fix origin offset
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

#define BITMAP_FLAGS BMF_MINPLANES  /* No VMEM for RTG systems */
#define PATT_DOT    0xAAAA
#define PATT_DASH   0xCCCC
#define PATT_LINE   0xFFFF

struct RenderInfo renderinfo;
static LONG global_pen_a = -1, global_pen_b = -1;

/* Current MUI gadget origin (from mleft/mtop) */
static int origin_x = 0;
static int origin_y = 0;
#define DX (origin_x)
#define DY (origin_y)

/* Convert NetSurf color (0xBBGGRR) to Amiga RGB (0xRRGGBB) */
static ULONG ConvertNetSurfColor(colour ns)
{
    ns &= 0x00FFFFFF;
    UBYTE r = (ns >> 0) & 0xFF;
    UBYTE g = (ns >> 8) & 0xFF;
    UBYTE b = (ns >> 16) & 0xFF;
    return (r << 16) | (g << 8) | b;
}

/* Convert to 16-bit RGB565 */
static UWORD ConvertToRGB565(ULONG c)
{
    UBYTE r = (c >> 16) & 0xFF;
    UBYTE g = (c >> 8) & 0xFF;
    UBYTE b = (c >> 0) & 0xFF;
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}
void SetColorAB1(struct RastPort *rp, ULONG fg, ULONG bg)
{
    if (!rp) return;
    ULONG c1 = ConvertNetSurfColor(fg);
    ULONG c2 = ConvertNetSurfColor(bg);
    ULONG depth = GetBitMapAttr(rp->BitMap, BMA_DEPTH);
    struct ColorMap *cm = renderinfo.screen->ViewPort.ColorMap;

    if (cm == NULL) {
        LOG(("ERROR: No ColorMap available - using ultimate fallback pens"));
        SetAPen(rp, 1);
        SetBPen(rp, 0);
        return;
    }

    LOG(("DEBUG: SetColorAB called - depth=%lu, fg=0x%06lx, bg=0x%06lx", depth, c1, c2));

    if (global_pen_a >= 0 && global_pen_b >= 0) {
        LOG(("DEBUG: Using global pens: A=%ld, B=%ld", global_pen_a, global_pen_b));
        if (depth <= 8) {
            SetRGB4(&renderinfo.screen->ViewPort, global_pen_a,
                    (c1 >> 16)&0xFF, (c1 >> 8)&0xFF, c1&0xFF);
            SetRGB4(&renderinfo.screen->ViewPort, global_pen_b,
                    (c2 >> 16)&0xFF, (c2 >> 8)&0xFF, c2&0xFF);
        } else {
            SetRGB32(&renderinfo.screen->ViewPort, global_pen_a,
                     ((c1 >> 16)&0xFF)<<24, ((c1 >> 8)&0xFF)<<24, (c1&0xFF)<<24);
            SetRGB32(&renderinfo.screen->ViewPort, global_pen_b,
                     ((c2 >> 16)&0xFF)<<24, ((c2 >> 8)&0xFF)<<24, (c2&0xFF)<<24);
        }
        SetAPen(rp, global_pen_a);
        SetBPen(rp, global_pen_b);
    } else {
        LOG(("WARNING: Pens not allocated, using fallback"));
        if (depth > 8 && CyberGfxBase) {  // Hi/truecolor RTG: Use fixed pens + direct SetRGB32
            LOG(("DEBUG: Truecolor mode - using fixed pens 1/0 with SetRGB32"));
            SetAPen(rp, 1);
            SetRGB32(&renderinfo.screen->ViewPort, 1,
                     ((c1 >> 16)&0xFF)<<24, ((c1 >> 8)&0xFF)<<24, (c1&0xFF)<<24);
            SetBPen(rp, 0);
            SetRGB32(&renderinfo.screen->ViewPort, 0,
                     ((c2 >> 16)&0xFF)<<24, ((c2 >> 8)&0xFF)<<24, (c2&0xFF)<<24);
        } else {  // Paletted: Try ObtainBestPen, fallback to fixed if fails
            LONG pen_fg = ObtainBestPen(cm, (c1 >> 16)&0xFF, (c1 >> 8)&0xFF, c1&0xFF,
                                        OBP_Precision, PRECISION_EXACT, TAG_DONE);
            LONG pen_bg = ObtainBestPen(cm, (c2 >> 16)&0xFF, (c2 >> 8)&0xFF, c2&0xFF,
                                        OBP_Precision, PRECISION_EXACT, TAG_DONE);
            if (pen_fg != -1) {
                LOG(("DEBUG: BestPen fg allocated: %ld", pen_fg));
                SetAPen(rp, pen_fg);
                ReleasePen(cm, pen_fg);
            } else {
                LOG(("WARNING: ObtainBestPen fg failed - using fixed pen 1"));
                SetAPen(rp, 1);
            }
            if (pen_bg != -1) {
                LOG(("DEBUG: BestPen bg allocated: %ld", pen_bg));
                SetBPen(rp, pen_bg);
                ReleasePen(cm, pen_bg);
            } else {
                LOG(("WARNING: ObtainBestPen bg failed - using fixed pen 0"));
                SetBPen(rp, 0);
            }
        }
    }
    LOG(("DEBUG: Pens set - APen=%ld, BPen=%ld", rp->FgPen, rp->BgPen));
}
void SetColorAB3(struct RastPort *rp, ULONG fg, ULONG bg)
{
    if (!rp) return;
    ULONG c1 = ConvertNetSurfColor(fg);
    ULONG c2 = ConvertNetSurfColor(bg);
    ULONG depth = GetBitMapAttr(rp->BitMap, BMA_DEPTH);
    struct ColorMap *cm = renderinfo.screen->ViewPort.ColorMap;

    if (global_pen_a >= 0 && global_pen_b >= 0) {
        if (depth <= 8) {  // Tylko dla paletted, SetRGB4/32 dla >8 na pens
            SetRGB4(&renderinfo.screen->ViewPort, global_pen_a,
                    (c1 >> 16)&0xFF, (c1 >> 8)&0xFF, c1&0xFF);
            SetRGB4(&renderinfo.screen->ViewPort, global_pen_b,
                    (c2 >> 16)&0xFF, (c2 >> 8)&0xFF, c2&0xFF);
        } else {
            SetRGB32(&renderinfo.screen->ViewPort, global_pen_a,
                     ((c1 >> 16)&0xFF)<<24, ((c1 >> 8)&0xFF)<<24, (c1&0xFF)<<24);  // Poprawny shift dla SetRGB32 (ARGB format)
            SetRGB32(&renderinfo.screen->ViewPort, global_pen_b,
                     ((c2 >> 16)&0xFF)<<24, ((c2 >> 8)&0xFF)<<24, (c2&0xFF)<<24);
        }
        SetAPen(rp, global_pen_a);
        SetBPen(rp, global_pen_b);
    } else {
        LOG(("WARNING: Pens not allocated, using fallback"));
        if (cm == NULL) {
            LOG(("ERROR: No ColorMap - fallback to fixed pens without color set"));
            SetAPen(rp, 1);
            SetBPen(rp, 0);
            return;
        }
        if (depth > 8 && CyberGfxBase) {  // Hi/truecolor RTG: Użyj fixed pens + SetRGB32
            SetAPen(rp, 1);
            SetRGB32(&renderinfo.screen->ViewPort, 1,
                     ((c1 >> 16)&0xFF)<<24, ((c1 >> 8)&0xFF)<<24, (c1&0xFF)<<24);
            SetBPen(rp, 0);
            SetRGB32(&renderinfo.screen->ViewPort, 0,
                     ((c2 >> 16)&0xFF)<<24, ((c2 >> 8)&0xFF)<<24, (c2&0xFF)<<24);
        } else {  // Paletted: ObtainBestPen
            LONG pen_fg = ObtainBestPen(cm, (c1 >> 16)&0xFF, (c1 >> 8)&0xFF, c1&0xFF,
                                        OBP_Precision, PRECISION_EXACT, TAG_DONE);
            LONG pen_bg = ObtainBestPen(cm, (c2 >> 16)&0xFF, (c2 >> 8)&0xFF, c2&0xFF,
                                        OBP_Precision, PRECISION_EXACT, TAG_DONE);
            if (pen_fg != -1) {
                SetAPen(rp, pen_fg);
                // Nie release tu - release po całym draw call, ale skoro per SetColorAB, release po set
                ReleasePen(cm, pen_fg);
            } else {
                LOG(("ERROR: ObtainBestPen fg failed - using pen 1"));
                SetAPen(rp, 1);
            }
            if (pen_bg != -1) {
                SetBPen(rp, pen_bg);
                ReleasePen(cm, pen_bg);
            } else {
                LOG(("ERROR: ObtainBestPen bg failed - using pen 0"));
                SetBPen(rp, 0);
            }
        }
    }
}
/* Set pens */
 void SetColorAB(struct RastPort *rp, ULONG fg, ULONG bg)
{
    if (!rp) return;
    ULONG c1 = ConvertNetSurfColor(fg);
    ULONG c2 = ConvertNetSurfColor(bg);
    ULONG depth = GetBitMapAttr(rp->BitMap, BMA_DEPTH);

    struct ColorMap *cm = GetColorMap(rp->BitMap);
    if (global_pen_a >= 0 && global_pen_b >= 0) {
        if (depth <= 16) {
            SetRGB4(&renderinfo.screen->ViewPort, global_pen_a,
                    (c1 >> 16)&0xFF, (c1 >> 8)&0xFF, c1&0xFF);
            SetRGB4(&renderinfo.screen->ViewPort, global_pen_b,
                    (c2 >> 16)&0xFF, (c2 >> 8)&0xFF, c2&0xFF);
        } else {
            SetRGB32(&renderinfo.screen->ViewPort, global_pen_a,
                     (c1&0xFF)<<24, ((c1>>8)&0xFF)<<24, ((c1>>16)&0xFF)<<24);
            SetRGB32(&renderinfo.screen->ViewPort, global_pen_b,
                     (c2&0xFF)<<24, ((c2>>8)&0xFF)<<24, ((c2>>16)&0xFF)<<24);
        }
        SetAPen(rp, global_pen_a);
        SetBPen(rp, global_pen_b);
    } else {
        LOG(("WARNING: Pens not allocated"));LOG(("WARNING: Pens not allocated, using fallback"));

            SetAPen(rp, 1);
            SetBPen(rp, 0);
    }
}

static bool mui_clear(colour c)
{
    struct RastPort *rp = renderinfo.rp;
    if (!rp) return false;
    ULONG depth = GetBitMapAttr(rp->BitMap, BMA_DEPTH);
    ULONG col = ConvertNetSurfColor(c);

    if (depth <= 16) {
        col = ConvertToRGB565(col);
        RectFill(rp,
                 DX, DY,
                 DX + renderinfo.width -1,
                 DY + renderinfo.height -1);
    } else {
        FillPixelArray(rp,
                       DX, DY,
                       renderinfo.width,
                       renderinfo.height,
                       col);
    }
    return true;
}

static bool mui_rectangle(int x0,int y0,int w,int h,
                          int lw,colour col,bool dot,bool dash)
{
    struct RastPort *rp = renderinfo.rp;
    if (!rp) return false;

    x0 += DX; y0 += DY;
    ULONG oldPtrn = rp->LinePtrn;
    UBYTE oldPW = rp->PenWidth;
    rp->PenWidth = lw;
    rp->PenHeight = lw;
    rp->LinePtrn = dot?PATT_DOT:(dash?PATT_DASH:PATT_LINE);

    SetColorAB(rp, col, 0xFFFFFF);
    Move(rp, x0, y0);
    Draw(rp, x0 + w -1, y0);
    Draw(rp, x0 + w -1, y0 + h -1);
    Draw(rp, x0,      y0 + h -1);
    Draw(rp, x0,      y0);

    rp->LinePtrn = oldPtrn;
    rp->PenWidth  = oldPW;
    rp->PenHeight = oldPW;
    return true;
}

static bool mui_clip(int x0,int y0,int x1,int y1)
{
    struct RastPort *rp = renderinfo.rp;
    if (!rp || !rp->Layer) return false;
    static struct Rectangle R;
    struct Region *reg = InstallClipRegion(rp->Layer, NULL);
    if (!reg) reg = NewRegion();
    else ClearRectRegion(reg, &R);

    R.MinX = x0 + DX;
    R.MinY = y0 + DY;
    R.MaxX = x1-1 + DX;
    R.MaxY = y1-1 + DY;
    OrRectRegion(reg, &R);
    reg = InstallClipRegion(rp->Layer, reg);
    if (reg) DisposeRegion(reg);
    return true;
}

static bool mui_line(int x0,int y0,int x1,int y1,
                     int lw,colour col,bool dot,bool dash)
{
    struct RastPort *rp = renderinfo.rp;
    if (!rp) return false;

    x0 += DX; y0 += DY;
    x1 += DX; y1 += DY;
    ULONG oldPtrn = rp->LinePtrn;
    UBYTE oldPW = rp->PenWidth;
    rp->PenWidth  = lw;
    rp->PenHeight = lw;
    rp->LinePtrn  = dot?PATT_DOT:(dash?PATT_DASH:PATT_LINE);

    SetColorAB(rp, col, 0xFFFFFF);
    Move(rp, x0, y0);
    Draw(rp, x1, y1);

    rp->LinePtrn  = oldPtrn;
    rp->PenWidth  = oldPW;
    rp->PenHeight = oldPW;
    return true;
}

static bool mui_polygon(int *pts,unsigned n,colour fill)
{
    struct RastPort *rp = renderinfo.rp;
    if (!rp || n<3) return false;

    SetColorAB(rp, fill, 0xFFFFFF);
    struct AreaInfo A;
    struct TmpRas T;
    UBYTE buf[512];
    InitArea(&A, buf, sizeof(buf)/5);
    InitTmpRas(&T, AllocMem(renderinfo.width*renderinfo.height, MEMF_FAST), renderinfo.width);
    rp->AreaInfo = &A;
    rp->TmpRas   = &T;
    AreaMove(rp, pts[0]+DX, pts[1]+DY);
    for (unsigned i=1; i<n; i++) {
        AreaDraw(rp, pts[i*2]+DX, pts[i*2+1]+DY);
    }
    AreaEnd(rp);
    rp->AreaInfo = NULL;
    rp->TmpRas   = NULL;
    FreeMem(T.RasPtr, renderinfo.width*renderinfo.height);
    return true;
}

static bool mui_fill1(int x0,int y0,int x1,int y1,colour col)
{
    struct RastPort *rp = renderinfo.rp;
    if (!rp) return false;
    x0 += DX; y0 += DY;
    x1 += DX; y1 += DY;
    ULONG depth = GetBitMapAttr(rp->BitMap, BMA_DEPTH);
    if (depth<=16) {
        SetColorAB(rp, col, 0xFFFFFF);
        RectFill(rp, x0, y0, x1-1, y1-1);
    } else {
        FillPixelArray(rp, x0, y0, x1-x0+1, y1-y0+1, ConvertNetSurfColor(col));
    }
    return true;
}
static bool mui_fill(int x0,int y0,int x1,int y1,colour col)
{
    struct RastPort *rp = renderinfo.rp;
    if (!rp) return false;
    x0 += DX; y0 += DY;
    x1 += DX; y1 += DY;
    ULONG depth = GetBitMapAttr(rp->BitMap, BMA_DEPTH);
    ULONG ami_col = ConvertNetSurfColor(col);

    if (depth > 8 && CyberGfxBase) {  // Direct CGX dla truecolor
        FillPixelArray(rp, x0, y0, x1 - x0, y1 - y0, ami_col);
        LOG(("DEBUG: mui_fill using FillPixelArray"));
    } else {
        SetColorAB(rp, col, 0xFFFFFF);  // Stare, dla paletted
        RectFill(rp, x0, y0, x1-1, y1-1);
    }
    return true;
}
static bool mui_text(int x,int y,const struct css_style *st,const char*txt,size_t len,colour bg,colour fg)
{
    struct RastPort *rp = renderinfo.rp;
    if (!rp || !txt || len==0) {
        LOG(("ERROR: Invalid params in mui_text"));
        return false;
    }
    APTR f = mui_open_font(rp, st);
    if (!f) {
        LOG(("ERROR: Font open failed in mui_text"));
        return false;
    }
    ULONG base;
    TT_GetAttrs(rp, TT_FontBaseline, &base, TAG_DONE);
    SetColorAB(rp, fg, bg);
    Move(rp, x+DX, y+DY+base);
    len = utf8_codepoints(txt, len);
    LOG(("DEBUG: Drawing text '%s' (len=%lu) at (%d,%d) with fg=0x%06x bg=0x%06x.", txt, len, x+DX, y+DY+base, fg, bg));

    // Force fallback do system Text - wyłącz TT_Text na OS3
    LOG(("DEBUG: Skipping TT_Text - direct fallback to system Text"));
    struct TextFont *sys_font = rp->Font;
    if (sys_font) {
        Move(rp, x+DX, y+DY + sys_font->tf_Baseline);
        if (len < 256) {  // Small text: fixed buffer na stacku, by uniknąć alloc hang
            char ami_buf[256];
            ULONG buf_len = 0;
            ULONG octets;
            WCHAR wc;
            CONST_STRPTR p = txt;

            while ((octets = UTF8_Decode(p, &wc)) && buf_len < 255) {
                if (wc > 255) wc = '?';
                ami_buf[buf_len++] = wc;
                p += octets;
            }
            ami_buf[buf_len] = '\0';
            Text(rp, ami_buf, buf_len);
            LOG(("DEBUG: System Text drawn with fixed buffer"));
        } else {  // Large: użyj utf8_to_amiga, ale truncate jeśli > max
            STRPTR ami_txt = utf8_to_amiga(txt);
            if (ami_txt) {
                Text(rp, ami_txt, strlen(ami_txt));
                free(ami_txt);
                LOG(("DEBUG: System Text drawn with utf8_to_amiga"));
            } else {
                LOG(("ERROR: utf8_to_amiga failed - no text drawn"));
            }
        }
    } else {
        LOG(("ERROR: No system font - text skipped"));
    }

    mui_close_font(rp, f);
    return true;
}
static bool mui_text_hangs(int x,int y,const struct css_style *st,const char*txt,size_t len,colour bg,colour fg)
{
    struct RastPort *rp = renderinfo.rp;
    if (!rp || !txt || len==0) {
        LOG(("ERROR: Invalid params in mui_text"));
        return false;
    }
    APTR f = mui_open_font(rp, st);
    if (!f) {
        LOG(("ERROR: Font open failed in mui_text"));
        return false;
    }
    ULONG base;
    TT_GetAttrs(rp, TT_FontBaseline, &base, TAG_DONE);
    SetColorAB(rp, fg, bg);
    Move(rp, x+DX, y+DY+base);
    len = utf8_codepoints(txt, len);
    LOG(("DEBUG: Drawing text '%s' (len=%lu) at (%d,%d) with fg=0x%06x bg=0x%06x.", txt, len, x+DX, y+DY+base, fg, bg));

    // Check mem przed TT_Text
    ULONG avail_fast = AvailMem(MEMF_FAST | MEMF_TOTAL);  // FAST, skoro masz 2GB
    if (avail_fast < 1024 * 20) {  // Min 20KB FAST na glyph
        LOG(("WARNING: Low FAST mem (%lu) - fallback to system Text", avail_fast));
        struct TextFont *sys_font = rp->Font;
        if (sys_font) {
            Move(rp, x+DX, y+DY + sys_font->tf_Baseline);
            STRPTR ami_txt = utf8_to_amiga(txt);  // Konwersja na Amiga charset
            if (ami_txt) {
                Text(rp, ami_txt, strlen(ami_txt));  // Byte len dla ASCII
                free(ami_txt);  // Free po użyciu
                LOG(("DEBUG: System Text drawn with utf8_to_amiga"));
            } else {
                LOG(("ERROR: utf8_to_amiga failed - no text drawn"));
            }
        }
    } else {
        TT_Text(rp, (STRPTR)txt, len);
        LONG err = IoErr();
        LOG(("DEBUG: TT_Text completed, error: %ld", err));
        if (err != 0) {
            LOG(("WARNING: TT_Text failed (%ld) - fallback to system Text", err));
            struct TextFont *sys_font = rp->Font;
            if (sys_font) {
                Move(rp, x+DX, y+DY + sys_font->tf_Baseline);
                STRPTR ami_txt = utf8_to_amiga(txt);  // Konwersja
                if (ami_txt) {
                    Text(rp, ami_txt, strlen(ami_txt));
                    free(ami_txt);
                    LOG(("DEBUG: System Text drawn as fallback with utf8_to_amiga"));
                } else {
                    LOG(("ERROR: utf8_to_amiga failed in fallback"));
                }
            }
        }
    }

    mui_close_font(rp, f);
    return true;
}
static bool mui_text_works(int x,int y,const struct css_style *st,const char*txt,size_t len,colour bg,colour fg)
{
    struct RastPort *rp = renderinfo.rp;
    if (!rp || !txt || len==0) {
        LOG(("ERROR: Invalid params in mui_text"));
        return false;
    }
    APTR f = mui_open_font(rp, st);
    if (!f) {
        LOG(("ERROR: Font open failed in mui_text"));
        return false;
    }
    
    ULONG base;
    TT_GetAttrs(rp, TT_FontBaseline, &base, TAG_DONE);
    SetColorAB(rp, fg, bg);
    Move(rp, x+DX, y+DY+base);
    len = utf8_codepoints(txt, len);
    LOG(("DEBUG: Drawing text '%s' (len=%lu) at (%d,%d) with fg=0x%06x bg=0x%06x", txt, len, x+DX, y+DY+base, fg, bg));
    TT_Text(rp, (STRPTR)txt, len);
    LOG(("DEBUG: TT_Text completed, error: %ld", IoErr()));
    if (f) {  // tfont OK
    // TT_Text jak wcześniej
    LONG err = IoErr();
    if (err != 0) {
        LOG(("WARNING: TT_Text failed (%ld) - fallback to system Text", err));
        struct TextFont *sys_font = rp->Font;  // Użyj default rp Font
        if (sys_font) {
            Move(rp, x+DX, y+DY + sys_font->tf_Baseline);
            Text(rp, txt, len);
            LOG(("DEBUG: System Text drawn"));
        }
    }
}
    mui_close_font(rp, f);
    return true;
}
static bool mui_text1(int x,int y,const struct css_style *st,const char*txt,size_t len,colour bg,colour fg)
{
    struct RastPort *rp = renderinfo.rp;
    if (!rp || !txt || len==0) return false;
    APTR f = mui_open_font(rp, st);
    if (!f) return false;
    ULONG base;
    TT_GetAttrs(rp, TT_FontBaseline, &base, TAG_DONE);
    SetColorAB(rp, fg, bg);
    Move(rp, x+DX, y+DY+base);
    len = utf8_codepoints(txt, len);
    TT_Text(rp, (STRPTR)txt, len);
    mui_close_font(rp, f);
    return true;
}

static bool mui_disc(int x,int y,int r,colour col,bool fill)
{
    struct RastPort *rp = renderinfo.rp;
    if (!rp) 
    return false;
    SetColorAB(rp, col, 0xFFFFFF);
    x+=DX; y+=DY;
    if (fill) {
        struct AreaInfo A; struct TmpRas T; UBYTE b[512];
        InitArea(&A,b,sizeof(b)/5);
        InitTmpRas(&T,AllocMem(renderinfo.width*renderinfo.height,MEMF_FAST),renderinfo.width);
        rp->AreaInfo=&A; rp->TmpRas=&T;
        AreaEllipse(rp,x,y,r,r);
        AreaEnd(rp);
        rp->AreaInfo=NULL; rp->TmpRas=NULL; FreeMem(T.RasPtr,renderinfo.width*renderinfo.height);
    } else {
        DrawEllipse(rp,x,y,r,r);
    }
    return true;
}

static bool mui_arc(int x,int y,int r,int a1,int a2,colour col)
{
    struct RastPort *rp=renderinfo.rp;
    if(!rp)return false;
    SetColorAB(rp,col,0xFFFFFF);
    x+=DX; y+=DY;
    int steps=32; double st=a1*M_PI/180.0,step=(a2-a1)*M_PI/(180.0*steps);
    Move(rp,x+r*cos(st),y+r*sin(st));
    for(int i=1;i<=steps;i++){ double ang=st+i*step; Draw(rp,x+r*cos(ang),y+r*sin(ang)); }
    return true;
}

static bool mui_bitmap(int x,int y,int w,int h,struct bitmap*b,colour bg,struct content*c){return false;}
static bool mui_bitmap_tile(int x,int y,int w,int h,struct bitmap*b,colour bg,bool rx,bool ry,struct content*c){return false;}

static bool mui_group_start(const char *name)
{
    /* use MUI-provided mleft/mtop from renderinfo */
    origin_x = renderinfo.origin_x;
    origin_y = renderinfo.origin_y;
    return mui_clip(0,0,renderinfo.width,renderinfo.height);
}

static bool mui_group_end(void){return true;}
static bool mui_flush(void){return true;}

static bool mui_path(float*pts,unsigned n,colour fill,float w,colour col,float*tr)
{
    struct RastPort*rp=renderinfo.rp; if(!rp||n<2)return false;
    if(fill!=0xFFFFFFFF){
        struct AreaInfo A; struct TmpRas T; UBYTE b[512];
        InitArea(&A,b,sizeof(b)/5);
        void* ras=AllocMem(renderinfo.width*renderinfo.height,MEMF_FAST);
        InitTmpRas(&T,ras,renderinfo.width);
        rp->AreaInfo=&A; rp->TmpRas=&T;
        AreaMove(rp,(int)pts[0]+DX,(int)pts[1]+DY);
        for(unsigned i=1;i<n;i++)AreaDraw(rp,(int)pts[2*i]+DX,(int)pts[2*i+1]+DY);
        AreaEnd(rp); rp->AreaInfo=NULL; rp->TmpRas=NULL; FreeMem(ras,renderinfo.width*renderinfo.height);
    } else {
        ULONG oldP=rp->LinePtrn; UBYTE oldW=rp->PenWidth;
        rp->PenWidth=(int)w; rp->PenHeight=(int)w; rp->LinePtrn=PATT_LINE;
        SetColorAB(rp,col,0xFFFFFF);
        Move(rp,(int)pts[0]+DX,(int)pts[1]+DY);
        for(unsigned i=1;i<n;i++)Draw(rp,(int)pts[2*i]+DX,(int)pts[2*i+1]+DY);
        rp->LinePtrn=oldP; rp->PenWidth=oldW; rp->PenHeight=oldW;
    }
    return true;
}
struct plotter_table plot;
const struct plotter_table muiplot = {
    mui_clear, mui_rectangle, mui_line, mui_polygon,
    mui_fill, mui_clip, mui_text, mui_disc,
    mui_arc, mui_bitmap, mui_bitmap_tile,
    mui_group_start, mui_group_end, mui_flush, mui_path, false
};
