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

#include <devices/rawkeycodes.h>
#include <exec/execbase.h>
#include <graphics/rpattr.h>
#include <intuition/pointerclass.h>
#include <devices/printer.h>
#undef NO_INLINE_STDARG
#include <proto/codesets.h>
#include <proto/cybergraphics.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/keymap.h>
#include <proto/intuition.h>
#include <proto/layers.h>
#include <proto/ttengine.h>
#include <proto/utility.h>

#include "desktop/browser.h"
#include "desktop/history_core.h"
#include "desktop/textinput.h"
#include "mui/gui.h"
#include "mui/mui.h"
#include "mui/netsurf.h"
#include "mui/plotters.h"
#include "mui/print.h"
#include "mui/search.h"
#include "mui/utils.h"
#include "utils/log.h"

#include "os3.h"
#define POINTERTYPE_NORMAL 0
#define BITMAP_FLAGS BMF_MINPLANES

LONG global_pen_a = -1;
LONG global_pen_b = -1;

struct Data
{
    struct MUI_EventHandlerNode  ehnode;
    struct gui_window           *context;
    struct browser_window       *browser;

    WORD mouse_state, key_state;
    WORD mwidth, mheight;

    ULONG content_width, content_height;

    ULONG pointertype;
    ULONG mouse_inside;

    BYTE back_available;
    BYTE forward_available;
    BYTE stop_available;
    BYTE reload_available;
    WORD redraw_pending, redraw;
    LONG vleft_old, vtop_old;
    LONG changed;

    LONG loading;
    STRPTR status_text;
    STRPTR url;
    STRPTR title;

    struct RastPort *RastPort;
    struct BitMap   *BitMap;
    struct Layer    *Layer;
    APTR LayerInfo;
    LONG bm_width, bm_height;

    APTR title_obj;
};
void textinput_enable(struct browser_window *bw)
{
    LOG(("DEBUG: textinput_enable called (stub implementation)"));
    // Add AmigaOS 3.x-specific text input enable logic, e.g., MUI text gadget activation
}

void textinput_disable(struct browser_window *bw)
{
    LOG(("DEBUG: textinput_disable called (stub implementation)"));
    // Add AmigaOS 3.x-specific text input disable logic
}
MUI_HOOK(layoutfunc, APTR grp, struct MUI_LayoutMsg *lm)
{
    ULONG rc = MUILM_UNKNOWN;

    switch (lm->lm_Type) {
        case MUILM_MINMAX:
            lm->lm_MinMax.MinWidth = 1;
            lm->lm_MinMax.MinHeight = 1;
            lm->lm_MinMax.DefWidth = 640;
            lm->lm_MinMax.DefHeight = 480;
            lm->lm_MinMax.MaxWidth = MUI_MAXMAX;
            lm->lm_MinMax.MaxHeight = MUI_MAXMAX;
            rc = 0;
            break;

        case MUILM_LAYOUT:
        {
            struct Data *data = INST_DATA(OCLASS(grp), grp);
            lm->lm_Layout.Width = data->content_width;
            lm->lm_Layout.Height = data->content_height;
            rc = TRUE;
        }
            break;
    }

    return rc;
}

STATIC VOID delete_offscreen_bitmap(struct Data *data)
{
    LOG(("DEBUG: delete_offscreen_bitmap called"));
    if (data->Layer) {
        TT_DoneRastPort(data->RastPort);
        DeleteLayer(NULL, data->Layer);
        data->Layer = NULL;
    }

    if (data->BitMap) {
        FreeBitMap(data->BitMap);
        data->BitMap = NULL;
    }

    if (data->LayerInfo) {
        DisposeLayerInfo(data->LayerInfo);
        data->LayerInfo = NULL;
    }

    data->changed = 0;
    data->redraw_pending = 1;
    data->context->redraw = 1;

    data->RastPort = NULL;
    data->context->RastPort = NULL;
    data->context->BitMap = NULL;
    data->context->Layer = NULL;
}

STATIC BOOL create_offscreen_bitmap(APTR obj, struct Data *data, ULONG width, ULONG height)
{
    LOG(("DEBUG: create_offscreen_bitmap called with width=%lu, height=%lu", width, height));

    if (data->BitMap) {
        delete_offscreen_bitmap(data);
    }

    if (!muiRenderInfo(obj)) {
        LOG(("ERROR: muiRenderInfo not available"));
        return FALSE;
    }

    if (!data->LayerInfo) {
        data->LayerInfo = NewLayerInfo();
        if (!data->LayerInfo) {
            LOG(("ERROR: Failed to create LayerInfo"));
            return FALSE;
        }
        LOG(("DEBUG: LayerInfo created: %p", data->LayerInfo));
    }

    struct BitMap *screen_bm = _screen(obj)->RastPort.BitMap;
    if (!screen_bm) {
        LOG(("ERROR: Screen BitMap is NULL"));
        return FALSE;
    }

    data->bm_width = MIN(width, _screen(obj)->Width);
    data->bm_height = MIN(height, _screen(obj)->Height);

    data->BitMap = AllocBitMap(data->bm_width, data->bm_height,
                               GetBitMapAttr(screen_bm, BMA_DEPTH), BITMAP_FLAGS, screen_bm);
    if (!data->BitMap) {
        LOG(("ERROR: Failed to allocate BitMap"));
        return FALSE;
    }
    LOG(("DEBUG: BitMap allocated: %p", data->BitMap));

    data->Layer = CreateUpfrontHookLayer(data->LayerInfo, data->BitMap, 0, 0,
                                        data->bm_width - 1, data->bm_height - 1,
                                        LAYERSIMPLE, LAYERS_NOBACKFILL, NULL);
    if (!data->Layer) {
        LOG(("ERROR: Failed to create Layer"));
        FreeBitMap(data->BitMap);
        data->BitMap = NULL;
        return FALSE;
    }
    LOG(("DEBUG: Layer created: %p", data->Layer));

    data->RastPort = data->Layer->rp;
    data->context->RastPort = data->RastPort;
    data->context->BitMap = data->BitMap;
    data->context->Layer = data->Layer;

    return TRUE;
}

STATIC VOID render_content_to_bitmap(struct Data *data)
{
    LOG(("DEBUG: render_content_to_bitmap called"));
    if (!data->context || !data->context->bw || !data->context->bw->current_content || !data->RastPort) {
        LOG(("ERROR: Invalid context, browser, content, or RastPort"));
        return;
    }

    struct content *c = data->context->bw->current_content;

    renderinfo.rp = data->RastPort;
    renderinfo.width = data->bm_width;
    renderinfo.height = data->bm_height;

    current_redraw_browser = data->context->bw;
    plot = muiplot;

    SetRast(data->RastPort, 0); // Clear with background color (black)

    content_redraw(c, 0, 0, data->bm_width, data->bm_height,
                   0, 0, c->width, c->height,
                   data->context->bw->scale, 0xffffff);

    current_redraw_browser = NULL;
}

DEFSMETHOD(Browser_GetBitMap)
{
    GETDATA;
    struct RastPort *rp;

    LOG(("DEBUG: Browser_GetBitMap called, width=%d, height=%d", msg->width, msg->height));

    if (msg->width <= 0 || msg->height <= 0) {
        LOG(("ERROR: Invalid dimensions: %dx%d", msg->width, msg->height));
        return 0;
    }

    if (data->changed) {
        delete_offscreen_bitmap(data);
    }

    rp = data->RastPort;
    if (!rp && muiRenderInfo(obj)) {
        if (!create_offscreen_bitmap(obj, data, msg->width, msg->height)) {
            LOG(("WARNING: Falling back to window RastPort"));
            rp = _rp(obj);
            if (rp) {
                data->RastPort = rp;
                data->context->RastPort = rp;
                data->context->BitMap = rp->BitMap;
                data->context->Layer = rp->Layer;
            } else {
                LOG(("ERROR: Window RastPort is NULL"));
                return 0;
            }
        }
    }

    LOG(("DEBUG: Browser_GetBitMap returning rp = %p", rp));
    return (IPTR)rp;
}

STATIC VOID doset(APTR obj, struct Data *data, struct TagItem *tags)
{
    struct TagItem *tag, *tstate;
    STRPTR p;

    tstate = tags;
    while ((tag = NextTagItem(&tstate)) != NULL) {
        IPTR tdata = tag->ti_Data;
        switch (tag->ti_Tag) {
            case MA_Browser_Loading:
                data->loading = tdata;
                break;

            case MA_Browser_Pointer:
                if (data->pointertype != tdata) {
                    data->pointertype = tdata;
                }
                break;

            case MA_Browser_StatusText:
                data->status_text = (APTR)tdata;
                break;

            case MA_Browser_Title:
                if (data->title) {
                    free(data->title);
                    data->title = NULL;
                }
                p = (APTR)tdata;
                if (p) {
                    p = DupStr(p);
                    if (!p) {
                        LOG(("ERROR: Failed to duplicate title string"));
                    }
                }
                data->title = p;
                break;

            case MA_Browser_URL:
                if (data->url) {
                    free(data->url);
                    data->url = NULL;
                }
                p = (APTR)tdata;
                if (p) {
                    p = DupStr(p);
                    if (!p) {
                        LOG(("ERROR: Failed to duplicate URL string"));
                    }
                }
                data->url = p;
                break;
        }
    }
}

DEFNEW
{
    obj = DoSuperNew(cl, obj,
                     MUIA_Group_LayoutHook, (IPTR)&layoutfunc_hook,
                     MUIA_FillArea, FALSE,
                     InnerSpacing(0, 0),
                     TAG_MORE, msg->ops_AttrList);

    if (obj) {
        GETDATA;
        data->ehnode.ehn_Object = obj;
        data->ehnode.ehn_Class = cl;
        data->ehnode.ehn_Priority = 1;
        data->ehnode.ehn_Flags = MUI_EHF_GUIMODE;

        data->mwidth = 640; // Default for AmigaOS 3.x
        data->mheight = 480;
        data->pointertype = POINTERTYPE_NORMAL;

        data->context = (APTR)FindTagItem(MA_Browser_Context, msg->ops_AttrList)->ti_Data;
        data->browser = data->context ? data->context->bw : NULL;
        data->title_obj = (APTR)FindTagItem(MA_Browser_TitleObj, msg->ops_AttrList)->ti_Data;

        if (!data->browser) {
            LOG(("ERROR: Invalid browser context"));
            MUI_DisposeObject(obj);
            return 0;
        }
    }

    return (IPTR)obj;
}

DEFDISP
{
    GETDATA;

    if (data->url) {
        free(data->url);
        data->url = NULL;
    }
    if (data->title) {
        free(data->title);
        data->title = NULL;
    }
    delete_offscreen_bitmap(data);

    return DOSUPER;
}

DEFMMETHOD(Setup)
{
    GETDATA;
    IPTR rc;

    rc = DOSUPER;

    if (rc) {
        LOG(("DEBUG: Setup called, allocating pens"));
        if (global_pen_a < 0) {
            global_pen_a = ObtainPen(_screen(obj)->ViewPort.ColorMap, -1, 0, 0, 0, PEN_EXCLUSIVE);
            LOG(("DEBUG: global_pen_a allocated: %d", global_pen_a));
        }
        if (global_pen_b < 0) {
            global_pen_b = ObtainPen(_screen(obj)->ViewPort.ColorMap, -1, 0, 0, 0, PEN_EXCLUSIVE);
            LOG(("DEBUG: global_pen_b allocated: %d", global_pen_b));
        }
    }

    return rc;
}

DEFMMETHOD(Cleanup)
{
    GETDATA;

    if (global_pen_a >= 0) {
        ReleasePen(_screen(obj)->ViewPort.ColorMap, global_pen_a);
        global_pen_a = -1;
    }
    if (global_pen_b >= 0) {
        ReleasePen(_screen(obj)->ViewPort.ColorMap, global_pen_b);
        global_pen_b = -1;
    }

    data->changed = 1;
    delete_offscreen_bitmap(data);

    return DOSUPER;
}

DEFMMETHOD(Show)
{
    ULONG rc;
    GETDATA;

    LOG(("DEBUG: Browser Show called"));
    if ((rc = DOSUPER)) {
        ULONG mwidth = _mwidth(obj);
        ULONG mheight = _mheight(obj);

        LOG(("DEBUG: Show successful, _win(obj) = %p, muiRenderInfo = %p", _win(obj), muiRenderInfo(obj)));

        if (data->mwidth != mwidth || data->mheight != mheight) {
            data->mwidth = mwidth;
            data->mheight = mheight;
            data->changed = 1;
            browser_window_reformat(data->browser, mwidth, mheight);
        }

        data->ehnode.ehn_Events = IDCMP_RAWKEY | IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE;
        DoMethod(_win(obj), MUIM_Window_AddEventHandler, &data->ehnode);
        LOG(("DEBUG: Show completed successfully"));
    }

    return rc;
}

DEFMMETHOD(Hide)
{
    GETDATA;
    DoMethod(_win(obj), MUIM_Window_RemEventHandler, &data->ehnode);
    return DOSUPER;
}

DEFTMETHOD(Browser_Print)
{
    GETDATA;

    LOG(("DEBUG: Browser_Print called"));
    if (data->RastPort && data->context->bw->current_content) {
        struct IORequest *req = CreateIORequest(NULL, sizeof(struct IODRPReq));
        if (req) {
            if (OpenDevice("printer.device", 0, req, 0) == 0) {
                print_doc(data->RastPort, data->bm_width, data->bm_height);
                CloseDevice(req);
            } else {
                LOG(("ERROR: Failed to open printer.device"));
            }
            DeleteIORequest(req);
        } else {
            LOG(("ERROR: Failed to create IO request for printing"));
        }
    } else {
        LOG(("ERROR: No RastPort or content available for printing"));
    }

    return 0;
}

STATIC VOID update_buttons(APTR obj, struct Data *data)
{
    data->back_available = data->browser->history ? history_back_available(data->browser->history) : FALSE;
    data->forward_available = data->browser->history ? history_forward_available(data->browser->history) : FALSE;

    SetAttrs(obj,
             MA_Browser_BackAvailable, data->back_available,
             MA_Browser_ForwardAvailable, data->forward_available,
             TAG_DONE);
}

DEFSMETHOD(Browser_SetContentSize)
{
    GETDATA;

    data->content_width = msg->width;
    data->content_height = msg->height;

    data->vleft_old = 0;
    data->vtop_old = 0;

    if (data->mwidth != data->content_width || data->mheight != data->content_height) {
        APTR parent = (APTR)getv(obj, MUIA_Parent);
        data->changed = 1;

        if (parent) {
            DoMethod(parent, MUIM_Group_InitChange);
            DoMethod(parent, MUIM_Group_ExitChange);
        }
    }

    update_buttons(obj, data);

    return SetAttrs(obj, MA_Browser_ReloadAvailable, TRUE, MA_Browser_StopAvailable, TRUE, TAG_DONE);
}

DEFSMETHOD(Browser_SetContentType)
{
    GETDATA;

    LOG(("DEBUG: Browser_SetContentType called, type=%d", msg->type));
    if (msg->type <= CONTENT_CSS) {
        LOG(("DEBUG: Enabling textual content handling"));
        if (data->context && data->context->bw) {
            textinput_enable(data->context->bw);
        }
    } else {
        LOG(("DEBUG: Enabling graphical content handling"));
        if (data->context && data->context->bw) {
            textinput_disable(data->context->bw);
        }
    }

    return 0;
}

DEFTMETHOD(Browser_Back)
{
    GETDATA;
    if (data->browser->history) {
        history_back(data->browser, data->browser->history);
        update_buttons(obj, data);
    }
    return 0;
}

DEFTMETHOD(Browser_Forward)
{
    GETDATA;
    if (data->browser->history) {
        history_forward(data->browser, data->browser->history);
        update_buttons(obj, data);
    }
    return 0;
}

DEFTMETHOD(Browser_Reload)
{
    GETDATA;
    browser_window_reload(data->browser, data->key_state & BROWSER_MOUSE_MOD_1 ? TRUE : FALSE);
    return 0;
}

DEFTMETHOD(Browser_Stop)
{
    GETDATA;
    browser_window_stop(data->browser);
    return 0;
}

DEFSMETHOD(Browser_Find)
{
    GETDATA;
    start_search(msg->flags & MF_Browser_Find_Previous ? FALSE : TRUE,
                 msg->flags & MF_Browser_Find_CaseSensitive ? TRUE : FALSE,
                 msg->string);
    return 0;
}

DEFSMETHOD(Browser_Go)
{
    GETDATA;
    LOG(("DEBUG: Browser_Go: %s", msg->url));
    browser_window_go(data->browser, msg->url, NULL, TRUE);
    return 0;
}

DEFGET
{
    GETDATA;

    switch (msg->opg_AttrID) {
        case MA_Browser_BackAvailable:
            *msg->opg_Storage = data->back_available;
            return TRUE;

        case MA_Browser_Box:
        {
            struct IBox *box = (APTR)msg->opg_Storage;
            box->Left = getv(obj, MUIA_Virtgroup_Left);
            box->Top = getv(obj, MUIA_Virtgroup_Top);
            box->Width = data->mwidth;
            box->Height = data->mheight;
            return FALSE;
        }

        case MA_Browser_Browser:
            *msg->opg_Storage = (IPTR)data->browser;
            return FALSE;

        case MA_Browser_ForwardAvailable:
            *msg->opg_Storage = data->forward_available;
            return TRUE;

        case MA_Browser_Loading:
            *msg->opg_Storage = data->loading;
            return TRUE;

        case MA_Browser_ReloadAvailable:
            *msg->opg_Storage = (IPTR)data->reload_available;
            return TRUE;

        case MA_Browser_StatusText:
            *msg->opg_Storage = (IPTR)data->status_text;
            return TRUE;

        case MA_Browser_StopAvailable:
            *msg->opg_Storage = (IPTR)data->stop_available;
            return TRUE;

        case MA_Browser_Title:
            *msg->opg_Storage = (IPTR)data->title;
            return TRUE;

        case MA_Browser_TitleObj:
            *msg->opg_Storage = (IPTR)data->title_obj;
            return FALSE;

        case MA_Browser_URL:
            *msg->opg_Storage = (IPTR)data->url;
            return TRUE;
    }

    return DOSUPER;
}

DEFSET
{
    GETDATA;
    doset(obj, data, msg->ops_AttrList);
    return DOSUPER;
}

DEFMMETHOD(HandleEvent)
{
    struct IntuiMessage *imsg = msg->imsg;
    ULONG rc = 0;

    if (imsg) {
        GETDATA;
        ULONG mouse_inside = 0;
        LONG MouseX = imsg->MouseX - _mleft(obj);
        LONG MouseY = imsg->MouseY - _mtop(obj);

        if (imsg->Class == IDCMP_RAWKEY) {
            mouse_inside = data->mouse_inside;
            switch (imsg->Code) {
                case RAWKEY_UP:
                    browser_window_key_press(data->browser, KEY_UP);
                    break;
                case RAWKEY_DOWN:
                    browser_window_key_press(data->browser, KEY_DOWN);
                    break;
                case RAWKEY_LEFT:
                    browser_window_key_press(data->browser, KEY_LEFT);
                    break;
                case RAWKEY_RIGHT:
                    browser_window_key_press(data->browser, KEY_RIGHT);
                    break;
                case RAWKEY_ESCAPE:
                    browser_window_key_press(data->browser, 27);
                    break;
                case RAWKEY_LSHIFT:
                    data->key_state = BROWSER_MOUSE_MOD_1;
                    break;
                case RAWKEY_LSHIFT + 0x80:
                    data->key_state = 0;
                    break;
                case RAWKEY_CONTROL:
                    data->key_state = BROWSER_MOUSE_MOD_2;
                    break;
                case RAWKEY_CONTROL + 0x80:
                    data->key_state = 0;
                    break;
                default:
                    if (imsg->Code < 0x80) {
                        ULONG ucs4 = 0;
                        struct InputEvent ie;
                        TEXT buffer[4];

                        ie.ie_Class = IECLASS_RAWKEY;
                        ie.ie_SubClass = 0;
                        ie.ie_Code = imsg->Code;
                        ie.ie_Qualifier = imsg->Qualifier;
                        ie.ie_EventAddress = NULL;

                        if (MapRawKey(&ie, (STRPTR)&buffer, sizeof(buffer), NULL) == 1) {
                            ucs4 = buffer[0];
                            if (CodesetsBase) {
                                APTR cset = CodesetsFindA("UTF-32", NULL);
                                if (cset) {
                                    ULONG *dst = (APTR)CodesetsConvertStr(CSA_Source, &buffer, CSA_SourceLen, 1, CSA_DestCodeset, cset, TAG_DONE);
                                    if (dst) {
                                        ucs4 = *dst;
                                        CodesetsFreeA(dst, NULL);
                                    }
                                }
                            }
                        }
                        browser_window_key_press(data->browser, ucs4);
                    }
                    break;
            }
            rc = MUI_EventHandlerRC_Eat;
        } else if (imsg->Class == IDCMP_MOUSEBUTTONS) {
            if (MouseX >= 0 && MouseY >= 0 && MouseX < data->mwidth && MouseY < data->mheight) {
                LONG button = imsg->Code & IECODE_LBUTTON ? BROWSER_MOUSE_PRESS_1 : BROWSER_MOUSE_PRESS_2;
                LONG click = imsg->Code & IECODE_LBUTTON ? BROWSER_MOUSE_CLICK_1 : BROWSER_MOUSE_CLICK_2;
                mouse_inside = 1;
                MouseX += getv(obj, MUIA_Virtgroup_Left);
                MouseY += getv(obj, MUIA_Virtgroup_Top);

                switch (imsg->Code) {
                    case SELECTDOWN:
                    case MIDDLEDOWN:
                        browser_window_mouse_click(data->browser, button | data->key_state, MouseX, MouseY);
                        data->mouse_state = button;
                        rc = MUI_EventHandlerRC_Eat;
                        break;
                    case SELECTUP:
                    case MIDDLEUP:
                        if (data->mouse_state & button) {
                            browser_window_mouse_click(data->browser, click | data->key_state, MouseX, MouseY);
                        } else {
                            browser_window_mouse_drag_end(data->browser, 0, MouseX, MouseY);
                        }
                        data->mouse_state = 0;
                        rc = MUI_EventHandlerRC_Eat;
                        break;
                }
            }
        } else if (imsg->Class == IDCMP_MOUSEMOVE) {
            if (MouseX >= 0 && MouseY >= 0 && MouseX < data->mwidth && MouseY < data->mheight) {
                mouse_inside = 1;
                MouseX += getv(obj, MUIA_Virtgroup_Left);
                MouseY += getv(obj, MUIA_Virtgroup_Top);

                if (data->mouse_state & BROWSER_MOUSE_PRESS_1) {
                    browser_window_mouse_track(data->browser, BROWSER_MOUSE_DRAG_1 | data->key_state, MouseX, MouseY);
                    data->mouse_state = BROWSER_MOUSE_HOLDING_1 | BROWSER_MOUSE_DRAG_ON;
                } else if (data->mouse_state & BROWSER_MOUSE_PRESS_2) {
                    browser_window_mouse_track(data->browser, BROWSER_MOUSE_DRAG_2 | data->key_state, MouseX, MouseY);
                    data->mouse_state = BROWSER_MOUSE_HOLDING_2 | BROWSER_MOUSE_DRAG_ON;
                } else {
                    browser_window_mouse_track(data->browser, data->key_state, MouseX, MouseY);
                }
                rc = MUI_EventHandlerRC_Eat;
            }
        }

        if (data->mouse_inside != mouse_inside) {
            data->mouse_inside = mouse_inside;
        }
    }

    return rc;
}

DEFMMETHOD(Backfill)
{
    GETDATA;

    LOG(("DEBUG: Backfill called"));
    if (data->BitMap && data->RastPort) {
        BltBitMapRastPort(data->BitMap, 0, 0, _rp(obj),
                          _mleft(obj), _mtop(obj),
                          data->mwidth, data->mheight, 0xc0);
    }

    return 0;
}

DEFTMETHOD(Browser_Redraw)
{
    GETDATA;

    LOG(("DEBUG: Browser_Redraw called"));
    data->redraw_pending = 0;

    if (!data->BitMap || data->changed) {
        if (create_offscreen_bitmap(obj, data, data->mwidth, data->mheight)) {
            render_content_to_bitmap(data);
            data->changed = 0;
        } else {
            LOG(("WARNING: Using window RastPort for redraw"));
            data->RastPort = _rp(obj);
            if (data->RastPort) {
                render_content_to_bitmap(data);
            }
        }
    }

    MUI_Redraw(obj, MADF_DRAWUPDATE);

    if (!data->reload_available) {
        data->reload_available = 1;
        set(obj, MA_Browser_ReloadAvailable, TRUE);
    }

    update_buttons(obj, data);
    LOG(("DEBUG: Browser_Redraw completed"));
}

DEFMMETHOD(Draw)
{
    DOSUPER;

    if (msg->flags & (MADF_DRAWOBJECT | MADF_DRAWUPDATE)) {
        GETDATA;

        if (!data->BitMap || data->changed || data->redraw_pending) {
            if (!data->redraw_pending) {
                data->redraw_pending = 1;
                LOG(("DEBUG: Scheduling redraw"));
                DoMethod(_app(obj), MUIM_Application_PushMethod, obj,
                         1 | MUIV_PushMethod_Delay(1) | MUIF_PUSHMETHOD_SINGLE, MM_Browser_Redraw);
                FillPixelArray(_rp(obj), _mleft(obj), _mtop(obj), data->mwidth, data->mheight, 0xffffff);
            }
        } else {
            LONG vleft = getv(obj, MUIA_Virtgroup_Left);
            LONG vtop = getv(obj, MUIA_Virtgroup_Top);

            BltBitMapRastPort(data->BitMap, vleft, vtop, _rp(obj),
                              _mleft(obj), _mtop(obj),
                              MIN(data->mwidth, data->bm_width),
                              MIN(data->mheight, data->bm_height), 0xc0);

            if (data->bm_width < data->mwidth) {
                FillPixelArray(_rp(obj), _mleft(obj) + data->bm_width, _mtop(obj),
                               data->mwidth - data->bm_width, data->mheight, 0xffffff);
            }
            if (data->bm_height < data->mheight) {
                FillPixelArray(_rp(obj), _mleft(obj), _mtop(obj) + data->bm_height,
                               data->mwidth, data->mheight - data->bm_height, 0xffffff);
            }
        }
    }

    return 0;
}

BEGINMTABLE
DECNEW
DECDISP
DECGET
DECSET
DECMMETHOD(Backfill)
DECMMETHOD(Cleanup)
DECMMETHOD(Draw)
DECMMETHOD(Hide)
DECMMETHOD(Setup)
DECMMETHOD(Show)
DECMMETHOD(HandleEvent)
DECSMETHOD(Browser_Back)
DECSMETHOD(Browser_Find)
DECSMETHOD(Browser_Forward)
DECSMETHOD(Browser_GetBitMap)
DECSMETHOD(Browser_Go)
DECSMETHOD(Browser_Print)
DECSMETHOD(Browser_Redraw)
DECSMETHOD(Browser_Reload)
DECSMETHOD(Browser_SetContentSize)
DECSMETHOD(Browser_SetContentType)
DECSMETHOD(Browser_Stop)
ENDMTABLE

DECSUBCLASS_NC(MUIC_Virtgroup, browserclass)