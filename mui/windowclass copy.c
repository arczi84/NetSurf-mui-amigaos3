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

#include <string.h>

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/utility.h>

#include "desktop/browser.h"
#include "mui/applicationclass.h"
#include "mui/gui.h"
#include "mui/mui.h"
#include "mui/netsurf.h"
#include "libraries/mui.h"

#define  APPLICATION_NAME "NetSurf"

APTR NewObject( struct IClass *classPtr, CONST_STRPTR classID, ULONG tag1, ... );
      
struct Data
{
	APTR windownode;

	APTR active_browser;

	APTR navbar;
	APTR addressbar;
	APTR bgroup;
	APTR findbar;
	APTR statusbar;

	APTR pagetitles;

	APTR lonely_title;
	APTR lonely_friend;

	APTR windowtitle;
};

#if 1

DEFNEW
{
	APTR navbar, bgroup, statusbar, addressbar, searchbar, findbar;

	obj = DoSuperNew(cl, obj,
			MUIA_Window_Width , MUIV_Window_Width_Visible(75),
			MUIA_Window_Height , MUIV_Window_Height_Visible(90),
			MUIA_Window_TopEdge , MUIV_Window_TopEdge_Centered,
			MUIA_Window_LeftEdge, MUIV_Window_LeftEdge_Centered,
			MUIA_Window_AppWindow, TRUE,
			WindowContents, VGroup,
				Child, HGroup,
					Child, navbar = NewObject(getnavigationbargroupclass(), NULL, TAG_DONE),
					Child, searchbar = NewObject(getsearchbargroupclass(), NULL, TAG_DONE),
				End,
				Child, addressbar = NewObject(getaddressbargroupclass(), NULL, TAG_DONE),
				Child, bgroup = VGroup, TAG_DONE),
				Child, findbar = NewObject(getfindtextclass(), NULL, MUIA_ShowMe, FALSE, TAG_DONE),
				Child, statusbar = TextObject, TAG_DONE),
			End,

			TAG_MORE, msg->ops_AttrList);

	if (obj) {
		APTR node;
		GETDATA;

		data->navbar = navbar;
		data->addressbar = addressbar;
		data->bgroup = bgroup;
		data->findbar = findbar;
		data->statusbar = statusbar;

		data->windownode = node = (APTR)GetTagData(MA_Window_Node, NULL, msg->ops_AttrList);

		DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, MUIV_Notify_Application, 2, MM_Application_CloseWindow, node);
		DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, MUIV_EveryTime, obj, 3, MM_Window_MenuAction, MUIV_TriggerValue);
	}

	return (IPTR)obj;
}

#else
DEFNEW
{
	kprintf(">>>DEBUG: Function:%s\n", __FUNCTION__);
	APTR navbar, bgroup, statusbar, addressbar, searchbar, findbar;


navbar = NewObject(getnavigationbargroupclass(), NULL, TAG_END);
searchbar = NewObject(getsearchbargroupclass(), NULL, TAG_END);
addressbar = NewObject(getaddressbargroupclass(), NULL, TAG_END);
bgroup = VGroup, End;
findbar = NewObject(getfindtextclass(), NULL, MUIA_ShowMe, FALSE, TAG_END);
statusbar = TextObject, MUIA_Text_Contents, "Status", End;

kprintf(">>DEBUG: navbar = %p\n", navbar);
kprintf(">>DEBUG: searchbar = %p\n", searchbar);
kprintf(">>DEBUG: addressbar = %p\n", addressbar);
kprintf(">>DEBUG: bgroup = %p\n", bgroup);
kprintf(">>DEBUG: findbar = %p\n", findbar);
kprintf(">>DEBUG: statusbar = %p\n", statusbar);
// Inspect ops_AttrList
    struct TagItem *tag;
    BOOL hasTagDone = FALSE;
    kprintf("DEBUG: Inspecting ops_AttrList:\n");
    for (tag = msg->ops_AttrList; tag; tag = (struct TagItem *)tag->ti_Data) {
        if (tag->ti_Tag == TAG_DONE) {
            hasTagDone = TRUE;
            break;
        }
        kprintf("  Tag: 0x%08lx  Data: 0x%08lx\n", tag->ti_Tag, tag->ti_Data);
    }
    kprintf("DEBUG: ops_AttrList %s terminated with TAG_DONE\n", hasTagDone ? "is" : "is NOT");

    // Create window with DoSuperNew
    obj = DoSuperNew(cl, obj,
        MUIA_Window_Title, (IPTR)"NetSurf",
        MUIA_Window_Width, MUIV_Window_Width_Visible(75),
        MUIA_Window_Height, MUIV_Window_Height_Visible(90),
        MUIA_Window_TopEdge, MUIV_Window_TopEdge_Centered,
        MUIA_Window_LeftEdge, MUIV_Window_LeftEdge_Centered,
        MUIA_Window_AppWindow, TRUE,
        MUIA_Window_RootObject, MUI_NewObject(MUIC_Group,
            MUIA_Group_Horiz, FALSE,
            Child, MUI_NewObject(MUIC_Group,
                MUIA_Group_Horiz, TRUE,
                Child, navbar,
                Child, searchbar,
                TAG_END),
            Child, addressbar,
            Child, bgroup,
            Child, findbar,
            Child, statusbar,
            TAG_END),
        TAG_MORE, msg->ops_AttrList);

    if (!obj) {
        MUI_Request(NULL, NULL, 0, "Error", "OK", "DoSuperNew failed!");
        kprintf(">>>DEBUG: ERROR: DoSuperNew failed, obj=%p\n", obj);
    } else {
        kprintf(">>>DEBUG: After DoSuperNew: obj=%p\n", obj);
    }

	if (obj) {
		APTR node;
		kprintf(">>>DEBUG: Function:%s\n", __FUNCTION__);	
		GETDATA;	

		data->navbar = navbar;
		data->addressbar = addressbar;
		data->bgroup = bgroup;
		data->findbar = findbar;
		data->statusbar = statusbar;

		data->windownode = node = (APTR)GetTagData(MA_Window_Node, NULL, msg->ops_AttrList);

		DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, MUIV_Notify_Application, 2, MM_Application_CloseWindow, node);
		DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, MUIV_EveryTime, obj, 3, MM_Window_MenuAction, MUIV_TriggerValue);
	}

	return (IPTR)obj;
}
#endif
DEFDISP
{
	kprintf(">>>DEBUG: Function:%s\n", __FUNCTION__);	
	GETDATA;	

	if (data->windowtitle)
		FreeVec(data->windowtitle);

	MUI_DisposeObject(data->lonely_title);

	return DOSUPER;
}

DEFGET
{
	kprintf(">>>DEBUG: Function:%s\n", __FUNCTION__);	
	GETDATA;	

	switch (msg->opg_AttrID) {
	case MA_Browser_Browser:
	#warning we should not need this... some unidentified bug somewhere
	case MA_Browser_Loading:
	case MA_Browser_URL:
		return DoMethodA(data->active_browser, (Msg)msg);
	}

	return DOSUPER;
}

DEFTMETHOD(Window_InsertBookmark)
{
	kprintf(">>>DEBUG: Function:%s\n", __FUNCTION__);	
	GETDATA;	
	STRPTR url, title;

	url = (STRPTR)getv(data->active_browser, MA_Browser_URL);
	title = (STRPTR)getv(data->active_browser, MA_Browser_Title);

	return DoMethod(_app(obj), MM_HotlistWindow_Insert, title, url);
}

DEFSMETHOD(Window_MenuAction)
{
	kprintf(">>>DEBUG: Function:%s\n", __FUNCTION__);	
	GETDATA;	
	APTR app = _app(obj) ? _app(obj) : application;

	switch (msg->action) {
	//case MNA_NEW_WINDOW      : DoMethod(app, MM_Application_NewWindow); break;
	case MNA_RELOAD          : DoMethod(obj, MM_Window_Navigate, NAV_RELOAD); break;

	case MNA_SAVE_AS_TEXT    : DoMethod(app, MM_Application_SaveDocument, SAVEDOC_HTML); break;
	case MNA_SAVE_AS_SOURCE  : DoMethod(app, MM_Application_SaveDocument, SAVEDOC_SOURCE); break;
	case MNA_SAVE_AS_PDF     : DoMethod(app, MM_Application_SaveDocument, SAVEDOC_PDF); break;
	case MNA_PRINT           : DoMethod(data->active_browser, MM_Browser_Print); break;

	case MNA_ABOUT           : DoMethod(app, MM_Application_About); break;
	case MNA_FIND            : set(data->findbar, MUIA_ShowMe, TRUE); break;
	case MNA_DOWNLOADS_WINDOW: DoMethod(app, MM_Application_OpenWindow, WINDOW_DOWNLOADS); break;
	case MNA_HOTLIST_WINDOW  : DoMethod(app, MM_Application_OpenWindow, WINDOW_HOTLIST); break;
	case MNA_MUI_SETTINGS    : DoMethod(app, MUIM_Application_OpenConfigWindow, 0, NULL); break;
	case MNA_NETSURF_SETTINGS: DoMethod(app, MM_Application_OpenWindow, WINDOW_SETTINGS); break;

	case MNA_NEW_PAGE:
	{
		#if MULTITHREADED
		struct worknode *node;
		STRPTR page;
		ULONG length;

		page = (STRPTR)getv(app, MA_Application_Homepage);
		length = strlen(page) + 1;

		node = AllocMem(sizeof(*node) + length, MEMF_ANY);

		if (node) {
			node->size = sizeof(*node) + length;
			bcopy(page, node->data, length);

			netsurf_add_job(NULL, JOB_NEW_PAGE, node);
		}
		#else
		browser_window_create((char *)getv(app, MA_Application_Homepage), 0, 0, true, false);
		#endif
	}
		break;

	case MNA_CLOSE_PAGE:
	{
		#if MULTITHREADED
		APTR bw = (APTR)getv(data->active_browser, MA_Browser_Browser);
		netsurf_add_simple_job(bw, JOB_DESTROY, 0, 0, 0);
		#else
	//}
		//break;
		//#if 0
		if (data->pagetitles) {
			DoMethod(data->pagetitles, MUIM_Title_Close, NULL);
		} else {
			DoMethod(app, MUIM_Application_PushMethod, app, 2, MM_Application_CloseWindow, data->windownode);
		}
		#endif
	}
		break;
		//#endif

	case MNA_CLOSE_WINDOW:
		if (data->pagetitles) {
			ULONG count = getv(data->pagetitles, MUIA_Group_ChildCount);

			if (count > 1) {
				if (!MUI_RequestA(_app(obj), obj, 0, "NetSurf", "_Yes|*_No", "Do you want to close all pages?", NULL))
					break;
			}
		}

		DoMethod(app, MUIM_Application_PushMethod, app, 2, MM_Application_CloseWindow, data->windownode);
		break;
	}

	return 0;
}

DEFSMETHOD(Window_RemovePage)
{
	kprintf(">>>DEBUG: Function:%s\n", __FUNCTION__);	
	GETDATA;	

	if (data->pagetitles == NULL || getv(data->pagetitles, MUIA_Group_ChildCount) == 1) {
		DoMethod(application, MUIM_Application_PushMethod, application,
				2, MM_Application_CloseWindow, data->windownode);
	} else {
		APTR title = (APTR)getv(msg->browser, MA_Browser_TitleObj);

		DoMethod(data->bgroup, MUIM_Group_InitChange);
		DoMethod(data->bgroup, OM_REMMEMBER, msg->browser);
		DoMethod(data->pagetitles, MUIM_Group_InitChange);
		DoMethod(data->pagetitles, OM_REMMEMBER, title);
		DoMethod(data->pagetitles, MUIM_Group_ExitChange);
		DoMethod(data->bgroup, MUIM_Group_ExitChange);

		MUI_DisposeObject(title);
		MUI_DisposeObject(msg->browser);
	}

	return 0;
}

STATIC VOID setup_browser_notifications(APTR obj, struct Data *data, APTR bw)
{
	kprintf(">>>DEBUG: setup_browser_notifications called for bw = %lx\n", bw);
	DoMethod(bw, MUIM_Notify, MA_Browser_Loading, MUIV_EveryTime,
			data->navbar, 3, MUIM_Set, MA_TransferAnim_Animate,
			MUIV_NotTriggerValue);
	DoMethod(bw, MUIM_Notify, MA_Browser_StatusText, MUIV_EveryTime,
			data->statusbar, 3, MUIM_Set, MUIA_Text_Contents,
			MUIV_TriggerValue);
	DoMethod(bw, MUIM_Notify, MA_Browser_URL, MUIV_EveryTime,
			data->addressbar, 3, MUIM_Set, MUIA_String_Contents,
			MUIV_TriggerValue);
			kprintf(">>>DEBUG: Browser notifications for MA_Browser_URL set up for bw = %lx\n", bw);
	DoMethod(bw, MUIM_Notify, MA_Browser_Title, MUIV_EveryTime, obj, 2,
			MM_Window_SetTitle, MUIV_TriggerValue);
			kprintf(">>>DEBUG: Browser notifications after MA_Browser_Title set up for bw = %lx\n", bw);
	DoMethod(bw, MUIM_Notify, MA_Browser_BackAvailable, MUIV_EveryTime,
			data->navbar, 3, MUIM_Set, MA_Navigation_BackEnabled,
			MUIV_TriggerValue);
			kprintf(">>>DEBUG: Browser notifications for MA_Browser_BackAvailable set up for bw = %lx\n", bw);
	DoMethod(bw, MUIM_Notify, MA_Browser_ForwardAvailable, MUIV_EveryTime,
			data->navbar, 3, MUIM_Set,
			MA_Navigation_ForwardEnabled, MUIV_TriggerValue);
	DoMethod(bw, MUIM_Notify, MA_Browser_ReloadAvailable, MUIV_EveryTime,
			data->navbar, 3, MUIM_Set, MA_Navigation_ReloadEnabled,
			MUIV_TriggerValue);
	DoMethod(bw, MUIM_Notify, MA_Browser_StopAvailable, MUIV_EveryTime,
			data->navbar, 3, MUIM_Set, MA_Navigation_StopEnabled,
			MUIV_TriggerValue);
	kprintf(">>>DEBUG: Browser notifications set up for bw = %lx\n", bw);
}
#if 1
DEFSMETHOD(Window_AddPage)
{
    kprintf(">>>DEBUG: Function: %s\n", __FUNCTION__);

    GETDATA;
    APTR bwgroup = NULL, bw = NULL, title = NULL;
    LONG ok = FALSE;

    kprintf(">>>DEBUG: Creating title object\n");

    title = TextObject,
        MUIA_Text_Contents, "NetSurf",
        MUIA_UserData, msg->context,
    End;

    kprintf(">>>DEBUG: Creating browser object\n");

    bwgroup = ScrollgroupObject,
        MUIA_Scrollgroup_AutoBars, TRUE,
        MUIA_FillArea, FALSE,
        MUIA_Scrollgroup_Contents, bw = NewObject(getbrowserclass(), NULL,
            MA_Browser_Context, msg->context,
            MA_Browser_TitleObj, title,
        TAG_END),
    TAG_END);
	if(bwgroup == NULL)
	{
		kprintf(">>>DEBUG: Failed to create bwgroup, bw or title object\n");

	}
	if (bw == NULL)
	{
		kprintf(">>>DEBUG: Failed to create bw object\n");
	}
    kprintf(">>>DEBUG: bwgroup = %lx, bw = %lx, title = %lx\n", bwgroup, bw, title);

    if (bwgroup && title)
    {
        APTR maingroup = data->bgroup;

        DoMethod(maingroup, MUIM_Group_InitChange);

        if (data->pagetitles)
        {
            kprintf(">>>DEBUG: Using pagetitles group\n");

            DoMethod(data->pagetitles, MUIM_Group_InitChange);
            DoMethod(data->pagetitles, OM_ADDMEMBER, title);
            DoMethod(data->bgroup, OM_ADDMEMBER, bwgroup);
            DoMethod(data->pagetitles, MUIM_Group_ExitChange);
            ok = TRUE;
        }
        else if (data->lonely_title == NULL)
        {
            kprintf(">>>DEBUG: Using lonely_title slot\n");

            data->lonely_title = title;
            data->lonely_friend = bwgroup;
            DoMethod(maingroup, OM_ADDMEMBER, bwgroup); //<< to było zakomentowane
			kprintf(">>>DEBUG: Added bwgroup to maingroup\n");
            ok = TRUE;
        }
        else
        {
            kprintf(">>>DEBUG: Creating new page group and titles\n");

            APTR group, titles;

            group = VGroup,
                MUIA_Background, MUII_RegisterBack,
                MUIA_Frame, MUIV_Frame_Register,
                MUIA_Group_PageMode, TRUE,
                Child, titles = NewObject(gettitleclass(), NULL,
                    MUIA_CycleChain, 1,
                    MUIA_Title_Closable, TRUE,
                TAG_END),
            End;

            if (group)
            {
                kprintf(">>>DEBUG: New group created: %lx\n", group);

                data->bgroup = group;
                data->pagetitles = titles;

                if (data->lonely_title)
                {
                    kprintf(">>>DEBUG: Migrating lonely_title to new group\n");

                    DoMethod(group, MUIM_Notify,
                        MUIA_Group_ActivePage,
                        MUIV_EveryTime, obj, 2,
                        MM_Window_ActivePage,
                        MUIV_TriggerValue);

                    DoMethod(maingroup, OM_REMMEMBER, data->lonely_friend);
                    DoMethod(titles, OM_ADDMEMBER, data->lonely_title);
                    DoMethod(group, OM_ADDMEMBER, data->lonely_friend);
                    DoMethod(maingroup, OM_ADDMEMBER, group);
                    data->lonely_title = NULL;
                }

                DoMethod(titles, OM_ADDMEMBER, title);
                DoMethod(group, OM_ADDMEMBER, bwgroup);
                ok = TRUE;
            }
            else
            {
                kprintf(">>>DEBUG: Failed to create group object\n");
            }
        }

		kprintf(">>>DEBUG: Group change completed\n");
        DoMethod(maingroup, MUIM_Group_ExitChange);
		kprintf(">>>DEBUG: Exiting group change\n");

		//set(group, MUIA_Window_Title, APPLICATION_NAME);
		//kprintf(">>>DEBUG: Setting window title to %s\n", APPLICATION_NAME);
    }
    else
    {
        kprintf(">>>DEBUG: Missing bwgroup or title\n");
    }

    if (ok)
    {
        kprintf(">>>DEBUG: Page created successfully\n");

        data->active_browser = bw;

        if (data->bgroup)
        {
            set(data->bgroup, MUIA_Group_ActivePage, MUIV_Group_ActivePage_Last);
        }

        DoMethod(bw, MUIM_Notify, MA_Browser_Title, MUIV_EveryTime,
            title, 3, MUIM_Set, MUIA_Text_Contents, MUIV_TriggerValue);

        setup_browser_notifications(obj, data, bw);
    }
    else
    {
        kprintf(">>>DEBUG: Page creation failed, disposing objects\n");
        MUI_DisposeObject(bwgroup);
        MUI_DisposeObject(title);
        bw = NULL;
    }

    kprintf(">>>DEBUG: Returning bw = %lx\n", bw);

    return (IPTR)bw;
}
#else //  original code?
DEFSMETHOD(Window_AddPage)
{
	kprintf(">>>DEBUG: Function:%s\n", __FUNCTION__);	
	GETDATA;		
	APTR bwgroup, bw, title;
	LONG ok;

	title = TextObject,
			MUIA_Text_Contents, "NetSurf",
			MUIA_UserData, msg->context,
		End;

	bwgroup = ScrollgroupObject,
			MUIA_Scrollgroup_AutoBars, TRUE,
			MUIA_FillArea, FALSE,
			MUIA_Scrollgroup_Contents, bw = NewObject(getbrowserclass(), NULL,
				MA_Browser_Context, msg->context,
				MA_Browser_TitleObj, title,
			TAG_END),
		TAG_END);

	ok = FALSE;

	if (bwgroup && title) {
		APTR maingroup = data->bgroup;

		DoMethod(maingroup, MUIM_Group_InitChange);

		if (data->pagetitles) {
			DoMethod(data->pagetitles, MUIM_Group_InitChange);
			DoMethod(data->pagetitles, OM_ADDMEMBER, title);
			DoMethod(data->bgroup, OM_ADDMEMBER, bwgroup);
			DoMethod(data->pagetitles, MUIM_Group_ExitChange);
			ok = TRUE;
		} else if (data->lonely_title == NULL) {
			data->lonely_title = title;
			data->lonely_friend = bwgroup;
			DoMethod(maingroup, OM_ADDMEMBER, bwgroup);
			ok = TRUE;
		} else {
			APTR group, titles;

			group = VGroup,
					MUIA_Background, MUII_RegisterBack,
					MUIA_Frame, MUIV_Frame_Register,
					MUIA_Group_PageMode, TRUE,
					Child, titles = NewObject(gettitleclass(), NULL,
						MUIA_CycleChain, 1,
						MUIA_Title_Closable, TRUE,
					TAG_END),
				End;

			if (group) {
				data->bgroup = group;
				data->pagetitles = titles;

				if (data->lonely_title) {
					DoMethod(group, MUIM_Notify,
							MUIA_Group_ActivePage,
							MUIV_EveryTime, obj, 2,
							MM_Window_ActivePage,
							MUIV_TriggerValue);

					DoMethod(maingroup, OM_REMMEMBER, data->lonely_friend);
					DoMethod(titles, OM_ADDMEMBER, data->lonely_title);
					DoMethod(group, OM_ADDMEMBER, data->lonely_friend);
					DoMethod(maingroup, OM_ADDMEMBER, group);
					data->lonely_title = NULL;
				}

				DoMethod(titles, OM_ADDMEMBER, title);
				DoMethod(group, OM_ADDMEMBER, bwgroup);
				ok = TRUE;
			}
		}

		DoMethod(maingroup, MUIM_Group_ExitChange);

		//set(group, MUIA_Window_Title, APPLICATION_NAME);
	}

	if (ok) {
		data->active_browser = bw;

		if (data->bgroup)
			set(data->bgroup, MUIA_Group_ActivePage,
					MUIV_Group_ActivePage_Last);

		DoMethod(bw, MUIM_Notify, MA_Browser_Title, MUIV_EveryTime,
				title, 3, MUIM_Set, MUIA_Text_Contents,
				MUIV_TriggerValue);

		setup_browser_notifications(obj, data, bw);
	} else {
		MUI_DisposeObject(bwgroup);
		MUI_DisposeObject(title);
		bw = NULL;
	}

	return (IPTR)bw;
}
#endif
DEFSMETHOD(Window_ActivePage)
{
	kprintf(">>>DEBUG: Function:%s\n", __FUNCTION__);	
	GETDATA;	
	ULONG idx;

	idx = 0;

	FORCHILD(data->bgroup, MUIA_Group_ChildList) {
		if (data->pagetitles == child)
			continue;

		if (msg->pagenum == idx) {
			APTR old = data->active_browser;

			if (old) {
				DoMethod(old, MUIM_KillNotify, MA_Browser_Loading);
				DoMethod(old, MUIM_KillNotify, MA_Browser_StatusText);
				DoMethod(old, MUIM_KillNotify, MA_Browser_URL);
				DoMethod(old, MUIM_KillNotifyObj, MA_Browser_Title, obj);
				DoMethod(old, MUIM_KillNotify, MA_Browser_BackAvailable);
				DoMethod(old, MUIM_KillNotify, MA_Browser_ForwardAvailable);
				DoMethod(old, MUIM_KillNotify, MA_Browser_ReloadAvailable);
				DoMethod(old, MUIM_KillNotify, MA_Browser_StopAvailable);
			}

			setup_browser_notifications(obj, data, child);

			data->active_browser = child;

			DoMethod(data->addressbar, MUIM_NoNotifySet, MUIA_String_Contents, getv(child, MA_Browser_URL));
			DoMethod(data->statusbar, MUIM_NoNotifySet, MUIA_String_Contents, getv(child, MA_Browser_StatusText));
			kprintf(">>>DEBUG: Window_ActivePage: set status text to %s\n", (STRPTR)getv(child, MA_Browser_StatusText));
			DoMethod(obj, MM_Window_SetTitle, getv(child, MA_Browser_Title));
			kprintf(">>>DEBUG: Window_ActivePage: set title to %s\n", (STRPTR)getv(child, MA_Browser_Title));
			SetAttrs(data->navbar,
				MA_TransferAnim_Animate, getv(child, MA_Browser_Loading),
				MA_Navigation_BackEnabled, getv(child, MA_Browser_BackAvailable),
				MA_Navigation_ForwardEnabled, getv(child, MA_Browser_ForwardAvailable),
				MA_Navigation_ReloadEnabled, getv(child, MA_Browser_ReloadAvailable),
				MA_Navigation_StopEnabled, getv(child, MA_Browser_StopAvailable),
			TAG_DONE);

			break;
		}

		idx++;
	}
	NEXTCHILD

	return 0;
}
#if 1
DEFSMETHOD(Window_SetTitle)
{
	kprintf(">>>DEBUG: Function:%s\n", __FUNCTION__);	
	GETDATA;

	STRPTR newtitle, title;
	ULONG length;

	if (!msg->title) {
		kprintf(">>>DEBUG: msg->title is NULL\n");
		return 0;
	}

	length = strlen("NetSurf: ") + strlen(msg->title) + 1;
	newtitle = AllocVec(length, MEMF_CLEAR);  // lepsze niż malloc dla AmigaOS
	title = data->windowtitle;

	if (newtitle) {
		sprintf(newtitle, "NetSurf: %s", msg->title);
		kprintf(">>>DEBUG: Before Window_SetTitle: %s\n", newtitle);
		set(obj, MUIA_Window_Title, newtitle);

		if (title)
			FreeVec(title);

		data->windowtitle = newtitle;
		kprintf(">>>DEBUG: After Window_SetTitle: %s\n", newtitle);
	} else {
		kprintf(">>>DEBUG: AllocVec failed\n");
	}

	return 0;
}
#else
/*
DEFSMETHOD(Window_SetTitle)
{
	kprintf(">>>DEBUG: Function:%s\n", __FUNCTION__);	
	GETDATA;	
	STRPTR newtitle, title;
	ULONG length;

	length = sizeof("NetSurf: %s") + strlen(msg->title);
	newtitle = malloc(length); //AllocVecTaskPooled(length);
	title = data->windowtitle;

	if (newtitle) {
		sprintf("NetSurf: %s", NULL, newtitle, msg->title);
		kprintf(">>>DEBUG: Window_SetTitle: %s\n", newtitle);
		set(obj, MUIA_Window_Title, newtitle);

		if (title)
			FreeVec(title);

		data->windowtitle = newtitle;
	}

	return 0;
}
*/
DEFSMETHOD(Window_SetTitle)
{
	STRPTR dummy = "Hello World";
	ULONG len = strlen("NetSurf: ") + strlen(dummy) + 1;
	STRPTR buf = AllocVec(len, MEMF_CLEAR);

	if (buf) {
		snprintf(buf, len, "NetSurf: %s", dummy);
		set(obj, MUIA_Window_Title, buf);
		kprintf(">>>TITLE='%s'\n", buf);
		FreeVec(buf);
	}
}
#endif
DEFSMETHOD(Window_Navigate)
{
	kprintf(">>>DEBUG: Function:%s\n", __FUNCTION__);	
	GETDATA;	

	switch (msg->Navigate) {
	case NAV_BACK   : return DoMethod(data->active_browser, MM_Browser_Back);
	case NAV_FORWARD: return DoMethod(data->active_browser, MM_Browser_Forward);
	case NAV_RELOAD : return DoMethod(data->active_browser, MM_Browser_Reload);
	case NAV_STOP   : return DoMethod(data->active_browser, MM_Browser_Stop);
	}

	return 0;
}

/* Browser dispatchers */
DEFSMETHOD(Browser_Go)
{
	kprintf(">>>DEBUG: Function:%s\n", __FUNCTION__);	
	GETDATA;	
	return DoMethodA(data->active_browser, (Msg)msg);
}

DEFSMETHOD(Browser_Find)
{
	kprintf(">>>DEBUG: Function:%s\n", __FUNCTION__);	
	GETDATA;	
	return DoMethodA(data->active_browser, (Msg)msg);
}

DEFSMETHOD(FindText_DisableButtons)
{
	kprintf(">>>DEBUG: Function:%s\n", __FUNCTION__);	
	GETDATA;	
	return DoMethodA(data->findbar, (Msg)msg);
}

BEGINMTABLE
DECNEW
DECDISP
DECGET
DECSMETHOD(Browser_Find)
DECSMETHOD(Browser_Go)
DECSMETHOD(FindText_DisableButtons)
DECSMETHOD(Window_ActivePage)
DECSMETHOD(Window_AddPage)
DECSMETHOD(Window_InsertBookmark)
DECSMETHOD(Window_Navigate)
DECSMETHOD(Window_MenuAction)
DECSMETHOD(Window_RemovePage)
DECSMETHOD(Window_SetTitle)
ENDMTABLE

DECSUBCLASS_NC(MUIC_Window, windowclass)
