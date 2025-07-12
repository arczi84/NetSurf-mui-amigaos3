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

#include <cybergraphx/cybergraphics.h>
#include <exec/execbase.h>
#include <exec/resident.h>
//#include <exec/system.h> //Only on MorphOS 
#include <intuition/pointerclass.h>
#include <workbench/startup.h>
#include <proto/asyncio.h>
#include <proto/codesets.h>
#include <proto/cybergraphics.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/icon.h>
#include <proto/intuition.h>
#include <proto/layers.h>
#include <proto/locale.h>
#include <proto/openurl.h>
#include <proto/ttengine.h>
#include <proto/utility.h>
#include <proto/keymap.h>

#include <proto/bsdsocket.h> //arczi

#ifdef WITH_HUBBUB
#include <hubbub/hubbub.h>
#endif

#include "content/urldb.h"
#include "desktop/gui.h"
#include "desktop/netsurf.h"
#include "desktop/options.h"
#include "desktop/selection.h"
#include "desktop/textinput.h"
#include "mui/applicationclass.h"
#include "mui/bitmap.h"
#include "mui/clipboard.h"
#include "mui/cookies.h"
#include "mui/fetch_file.h"
#include "mui/font.h"
#include "mui/gui.h"
#include "mui/history.h"
#include "mui/methodstack.h"
#include "mui/mui.h"
#include "mui/netsurf.h"
#include "mui/options.h"
#include "mui/plotters.h"
#include "mui/save_complete.h"
#include "mui/schedule.h"
#include "mui/transferanimclass.h"
#include "mui/utils.h"
#include "render/form.h"
#include "utils/messages.h"
#include "utils/url.h"
#include "utils/utf8.h"
#include "utils/utils.h"
#include "utils/log.h"

#define kprintf

APTR global_obj = NULL;


char *default_stylesheet_url;
char *adblock_stylesheet_url;

#if defined(__MORPHOS__)
struct Library *ZBase;
struct Library *JFIFBase;
#endif

struct Library *TTEngineBase;
struct Library *IconBase;
struct IntuitionBase *IntuitionBase;
struct Library *SocketBase;
struct Library *AsyncIOBase;
struct Library *OpenURLBase;

struct Library *CyberGfxBase;
struct Library *MUIMasterBase;
//Added for AmigaOS 3.x
struct Library *AslBase, *LayersBase;
struct UtilityBase *UtilityBase = NULL;
struct GfxBase *GfxBase = NULL;
struct Library *IFFParseBase = NULL;
struct LocaleBase *LocaleBase = NULL;
struct Library *KeymapBase = NULL;
/////////
#if !defined(__MORPHOS2__)
struct Library *CodesetsBase;
#endif

#ifdef WITH_HUBBUB
static void *myrealloc(void *ptr, size_t len, void *pw)
{
	return realloc(ptr, len);
}
#endif

STATIC struct MinList download_list = { (APTR)&download_list.mlh_Tail, NULL, (APTR)&download_list };
STATIC struct MinList window_list = { (APTR)&window_list.mlh_Tail, NULL, (APTR)&window_list };
STATIC LONG process_priority;

#if defined(__MORPHOS2__)
LONG altivec_accelerated;
#endif

/*********************************************************************/

static void abort_downloads(void)
{
	struct gui_download_window *dw, *next;

	ITERATELISTSAFE(dw, next, &download_list)
	{
		CloseAsync(dw->fh);
		FreeMem(dw->dl, sizeof(struct download));
		FreeMem(dw, sizeof(*dw));
	}
}

static void gui_delete_windowlist(void)
{
	struct gui_window *g, *next;

	ITERATELISTSAFE(g, next, &window_list)
	{
		gui_window_destroy(g);
	}
}

static void cleanup(void)
{
	netsurf_cleanup();
	mui_schedule_finalise();
	methodstack_cleanup();

	gui_delete_windowlist();
	bitmap_cleanup();
	transferanimclass_unload();

#if 0
	if (option_url_file && option_url_file[0])
		urldb_save(option_url_file);

	urldb_save_cookies(APPLICATION_COOKIES_FILE);
#endif

	mui_global_history_save();

	classes_cleanup();

	font_cleanup();

#ifdef WITH_HUBBUB
	hubbub_finalise(myrealloc, NULL);
#endif

	mui_clipboard_free();

	/* When NetSurf is aborted we must cleanup everything manually */
	mui_fetch_file_finalise(NULL);

	abort_downloads();

	#if !defined(__MORPHOS2__)
	CloseLibrary(CodesetsBase);
	#endif

	#if defined(__MORPHOS__)
	CloseLibrary(JFIFBase);
	CloseLibrary(ZBase);
	#endif

	CloseLibrary(OpenURLBase);
	CloseLibrary(AsyncIOBase);
	//CloseLibrary(SocketBase);
	CloseLibrary(TTEngineBase);
	CloseLibrary((struct Library *)IntuitionBase);
	CloseLibrary(IconBase);
	CloseLibrary(CyberGfxBase);
	CloseLibrary((struct Library *)GfxBase);
	CloseLibrary(MUIMasterBase);

	CloseLibrary((struct Library*)UtilityBase);
	CloseLibrary(IFFParseBase);
	//CloseLibrary(LocaleBase);
	CloseLibrary(LayersBase);
	CloseLibrary(KeymapBase);
	
	#if defined(__MORPHOS__)
	NewSetTaskAttrsA(NULL, &process_priority, sizeof(ULONG), TASKINFOTYPE_PRI, NULL);
	#else
	SetTaskPri(SysBase->ThisTask, process_priority);
	#endif
}
static LONG startup(void)
{
    #if defined(__MORPHOS2__)
    #define TTENGINE_VERSION 8
    #else
    #define TTENGINE_VERSION 7
    #endif

    LOG(("DEBUG: Starting library initialization\n"));

    // Open muimaster.library
    MUIMasterBase = OpenLibrary("muimaster.library", 20);
    LOG(("DEBUG: muimaster.library %s\n", MUIMasterBase ? "opened" : "FAILED to open"));
    if (!MUIMasterBase) goto cleanup;

    // Open cybergraphics.library
    CyberGfxBase = OpenLibrary("cybergraphics.library", 39);
    LOG(("DEBUG: cybergraphics.library %s\n", CyberGfxBase ? "opened" : "FAILED to open"));
    if (!CyberGfxBase) goto cleanup;

	// Open GFXBase
	GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 39);
	LOG(("DEBUG: graphics.library %s\n", GfxBase ? "opened" : "FAILED to open"));
	if (!GfxBase) goto cleanup;

    #ifdef __MORPHOS__
    // Open z.library
    ZBase = OpenLibrary("z.library", 51);
    LOG(("DEBUG: z.library %s\n", ZBase ? "opened" : "FAILED to open"));
    if (!ZBase) goto cleanup;

    // Open jfif.library
    JFIFBase = OpenLibrary("jfif.library", 0);
    LOG(("DEBUG: jfif.library %s\n", JFIFBase ? "opened" : "FAILED to open"));
    if (!JFIFBase) goto cleanup;
    #endif

    // Open asl.library
    AslBase = OpenLibrary("asl.library", 37);
    LOG(("DEBUG: asl.library %s\n", AslBase ? "opened" : "FAILED to open"));
    if (!AslBase) goto cleanup;

    // Open ttengine.library
    TTEngineBase = OpenLibrary("ttengine.library", TTENGINE_VERSION);
    LOG(("DEBUG: ttengine.library %s\n", TTEngineBase ? "opened" : "FAILED to open"));
    if (!TTEngineBase) goto cleanup;

    // Open icon.library
    IconBase = OpenLibrary("icon.library", 0);
    LOG(("DEBUG: icon.library %s\n", IconBase ? "opened" : "FAILED to open"));
    if (!IconBase) goto cleanup;

    // Open intuition.library
    IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 36);
    LOG(("DEBUG: intuition.library %s\n", IntuitionBase ? "opened" : "FAILED to open"));
    if (!IntuitionBase) goto cleanup;

    // Open bsdsocket.library
	//if (!SocketBase)
    //SocketBase = OpenLibrary("bsdsocket.library", 0);
    //LOG(("DEBUG: bsdsocket.library %s\n", SocketBase ? "opened" : "FAILED to open"));
   // if (!SocketBase) goto cleanup;

    // Open asyncio.library
    AsyncIOBase = OpenLibrary("asyncio.library", 0);
    LOG(("DEBUG: asyncio.library %s\n", AsyncIOBase ? "opened" : "FAILED to open"));
    if (!AsyncIOBase) goto cleanup;

    // Open utility.library
    UtilityBase = (struct UtilityBase *)OpenLibrary("utility.library", 37);
    LOG(("DEBUG: utility.library %s\n", UtilityBase ? "opened" : "FAILED to open"));
    if (!UtilityBase) goto cleanup;

    // Open iffparse.library
    IFFParseBase = OpenLibrary("iffparse.library", 0);
    LOG(("DEBUG: iffparse.library %s\n", IFFParseBase ? "opened" : "FAILED to open"));
    if (!IFFParseBase) goto cleanup;

	LayersBase = OpenLibrary("layers.library", 0);
	LOG(("DEBUG: layers.library %s\n", LayersBase ? "opened" : "FAILED to open"));
	if (!LayersBase) {
		LOG(("DEBUG: layers.library not available\n"));
		goto cleanup;
	}
    // Open locale.library
   // LocaleBase = OpenLibrary("locale.library", 37);
    //LOG(("DEBUG: locale.library %s\n", LocaleBase ? "opened" : "FAILED to open"));
   // if (!LocaleBase) goto cleanup;

	KeymapBase = OpenLibrary("keymap.library", 0);
	LOG(("DEBUG: keymap.library %s\n", KeymapBase ? "opened" : "FAILED to open"));
	if (!KeymapBase) {
		LOG(("DEBUG: keymap.library not available\n"));
		goto cleanup;
	}
	    // Initialize classes
    LOG(("DEBUG: Initializing classes...\n"));
    if (!classes_init()) {
        LOG(("DEBUG: classes_init FAILED\n"));
        goto cleanup;
    }
    LOG(("DEBUG: classes_init succeeded\n"));

    // Initialize mui_schedule
    LOG(("DEBUG: Initializing mui_schedule...\n"));
    if (!mui_schedule_init()) {
        LOG(("DEBUG: mui_schedule_init FAILED\n"));
        goto cleanup;
    }
    LOG(("DEBUG: mui_schedule_init succeeded\n"));

    // Initialize mui_clipboard
    LOG(("DEBUG: Initializing mui_clipboard...\n"));
    if (!mui_clipboard_init()) {
        LOG(("DEBUG: mui_clipboard_init FAILED\n"));
        goto cleanup;
    }
    LOG(("DEBUG: mui_clipboard_init succeeded\n"));

    // Initialize methodstack
    LOG(("DEBUG: Initializing methodstack...\n"));
    methodstack_init();
    LOG(("DEBUG: methodstack_init succeeded\n"));

    #ifdef __MORPHOS2__
    // Check MorphOS version
    LOG(("DEBUG: Checking MorphOS version...\n"));
    struct Resident *res = FindResident("MorphOS");
    if (!res || res->rt_Version < 2 || (res->rt_Version == 2 && res->rt_Revision < 2)) {
        LOG(("DEBUG: MorphOS 2.2 or newer required!\n"));
        goto cleanup;
    }
    LOG(("DEBUG: MorphOS version check passed\n"));

    // Check AltiVec
    LOG(("DEBUG: Checking AltiVec support...\n"));
    NewGetSystemAttrsA(&altivec_accelerated, sizeof(altivec_accelerated), SYSTEMINFOTYPE_PPC_ALTIVEC, NULL);
    LOG(("DEBUG: AltiVec check completed (accelerated=%d))\n", altivec_accelerated);
    #endif

    // Load transferanimclass
    LOG(("DEBUG: Loading transferanimclass...\n"));
    transferanimclass_load(); //arczi
    LOG(("DEBUG: transferanimclass loaded\n"));

    LOG(("DEBUG: Startup completed successfully\n"));
    return TRUE;

cleanup:
    LOG(("DEBUG: Cleaning up due to initialization failure\n"));
	if (KeymapBase) CloseLibrary(KeymapBase);
   // if (LocaleBase) CloseLibrary((struct Library *)LocaleBase);
    if (IFFParseBase) CloseLibrary(IFFParseBase);
    if (UtilityBase) CloseLibrary((struct Library *)UtilityBase);
    if (AsyncIOBase) CloseLibrary(AsyncIOBase);
    if (SocketBase) CloseLibrary(SocketBase);
    if (IntuitionBase) CloseLibrary((struct Library *)IntuitionBase);
    if (IconBase) CloseLibrary(IconBase);
    if (TTEngineBase) CloseLibrary(TTEngineBase);
    if (AslBase) CloseLibrary(AslBase);
    #ifdef __MORPHOS__
    if (JFIFBase) CloseLibrary(JFIFBase);
    if (ZBase) CloseLibrary(ZBase);
    #endif
    if (AsyncIOBase) CloseLibrary(AsyncIOBase);
    //if (SocketBase) CloseLibrary(SocketBase);
    if (MUIMasterBase) CloseLibrary(MUIMasterBase);
    if (CyberGfxBase) CloseLibrary(CyberGfxBase);
	if (GfxBase) CloseLibrary((struct Library *)GfxBase);
	if (LayersBase) CloseLibrary(LayersBase);
    return FALSE;
}

void gui_init(int argc, char** argv)
{

	LOG(("DEBUG: FUNCTION=%s", __FUNCTION__));
	font_init();
	LOG(("DEBUG: FUNCTION=%s", __FUNCTION__));
	#if defined(__MORPHOS__)
	NewGetTaskAttrsA(NULL, &process_priority, sizeof(ULONG), TASKINFOTYPE_PRI, NULL);
	#else
	process_priority = SysBase->ThisTask->tc_Node.ln_Pri;
	#endif
	LOG(("DEBUG: FUNCTION=%s", __FUNCTION__));
	atexit(cleanup);
	LOG(("DEBUG: FUNCTION=%s", __FUNCTION__));
	if (startup())
	{
		struct Locale *locale;
		TEXT lang[100];
		BPTR lock, file;
		ULONG i, found;

		options_read("PROGDIR:Resources/Options");
		LOG(("DEBUG: FUNCTION=%s", __FUNCTION__));
		verbose_log = option_verbose_log;

		if((!option_url_file) || (option_url_file[0] == '\0'))
			option_url_file = (char *)strdup("PROGDIR:Resources/URLs");

/*
		if((!option_cookie_jar) || (option_cookie_jar[0] == '\0'))
			option_cookie_jar = (char *)strdup("Resources/CookieJar");
*/

		if((!option_ca_bundle) || (option_ca_bundle[0] == '\0'))
			option_ca_bundle = (char *)strdup("PROGDIR:Resources/ca-bundle");

		if (lock = Lock("PROGDIR:Resources/LangNames", ACCESS_READ)) {
			UnLock(lock);
			messages_load("PROGDIR:Resources/LangNames");
		}
LOG(("DEBUG: FUNCTION=%s", __FUNCTION__));
#if 0
		locale = OpenLocale(NULL);
		found = FALSE;

		for (i = 0;i < 10; i++) {
			if (locale->loc_PrefLanguages[i] == NULL)
				continue;

			strcpy(lang, "PROGDIR:Resources/");
			strcat(lang, messages_get(locale->loc_PrefLanguages[i]));
			strcat(lang, "/messages");

			if (lock = Lock(lang, ACCESS_READ)) {
				UnLock(lock);
				found = TRUE;
				break;
			}
		}
#else	
	found = FALSE;
#endif		
LOG(("DEBUG: FUNCTION=%s", __FUNCTION__));
		if (!found)
			strcpy(lang, "PROGDIR:Resources/en/messages");
LOG(("DEBUG: FUNCTION=%s", __FUNCTION__));
		//CloseLocale(locale);
LOG(("DEBUG: FUNCTION=%s", __FUNCTION__));
		messages_load(lang); // check locale language and read appropriate file
LOG(("DEBUG: FUNCTION=%s", __FUNCTION__));
		default_stylesheet_url = "file:///Resources/default.css";
		adblock_stylesheet_url ="file:///Resources/adblock.css";
LOG(("DEBUG: FUNCTION=%s", __FUNCTION__));
		netsurf_setup();

#ifdef WITH_HUBBUB
		if (hubbub_initialise("Resources/Aliases", myrealloc, NULL) != HUBBUB_OK)
			die(messages_get("NoMemory"));
#endif
		css_screen_dpi = 90;
LOG(("DEBUG: FUNCTION=%s", __FUNCTION__));
		plot = muiplot;

		urldb_load(option_url_file);
		urldb_load_cookies(APPLICATION_COOKIES_FILE);

		mui_global_history_initialise();
		mui_cookies_initialise();
		save_complete_init();

		return;
	}

	exit(20);
}

void gui_init2(int argc, char** argv)
{
	LOG(("DEBUG: gui_init2 called"));

	LONG priority, got_window;

	priority = -1;
	got_window = 0;

	mui_fetch_file_register();
	LOG(("DEBUG: gui_init2: mui_fetch_file_register called"));
#ifdef MULTITHREADED
	NewSetTaskAttrsA(NULL, &priority, sizeof(ULONG), TASKINFOTYPE_PRI, NULL);
#endif

	if (argc) {
		enum
		{
			ARG_URL = 0,
			ARG_COUNT
		};

		STATIC CONST TEXT template[] = "URL/A";
		struct RDArgs *args;
		IPTR array[ARG_COUNT] = {0}; //arczi było {NULL}

		if (args = ReadArgs(template, array, NULL)) {
			if (array[ARG_URL]) {
				STRPTR url;

				url = (char *)DupStr((char *)array[ARG_URL]);
				LOG(("DEBUG: gui_init2: URL from command line: %s", url));
				if (url) {
					if (browser_window_create(url, 0, 0, true, false))
						got_window = 1;

					free(url);
				}
			}

			FreeArgs(args);
		}
	} else {
		extern struct WBStartup *_WBenchMsg;
		struct WBStartup *WBenchMsg;
		struct WBArg *wbarg;
		ULONG i;

		WBenchMsg = (struct WBStartup *)argv;

		for (i = 1, wbarg = WBenchMsg->sm_ArgList + 1; i < WBenchMsg->sm_NumArgs; i++, wbarg++) {
			if (wbarg->wa_Lock && *wbarg->wa_Name) {
				STRPTR path = GetNameFromLock(wbarg->wa_Lock);
				LOG(("DEBUG: gui_init2: Path from lock: %s", path));
				if (path) {
					ULONG length;
					STRPTR file;

					length = strlen(path) + strlen(wbarg->wa_Name) + 4;
					file = AllocTaskPooled(length);
					LOG(("DEBUG: gui_init2: Allocated file %s buffer of length %lu",file, length));
					if (file) {
						char *url;

						strcpy(file, path);
						AddPart(file, wbarg->wa_Name, length);

						url = path_to_url(file);
						LOG(("DEBUG: gui_init2: Converted path to URL: %s", url));
						if (url) {
							if (browser_window_create(url, 0, 0, true, false))
								got_window = 1;

							free(url);
						}

						FreeTaskPooled(file, length);
					}

					FreeVecTaskPooled(path);
				}
			}
		}
	}

    if (!got_window) {
        LOG(("DEBUG: No window from command line, creating default page"));
        methodstack_push_imm(application, 2, MM_Window_MenuAction, MNA_NEW_PAGE);
    }
	LOG(("DEBUG: gui_init2 completed"));
	    // Sprawdź czy okno aplikacji jest utworzone
 // Sprawdź czy okno aplikacji jest utworzone
    /*if (application) {
        LOG(("DEBUG: Application object exists: %p", application));
        
        // Ustaw aplikację jako nie-zikonizowaną
        set(application, MUIA_Application_Iconified, FALSE);
        
        // Sprawdź czy są jakieś okna
        APTR win = NULL;
        get(application, MUIA_Application_WindowList, &win);
        LOG(("DEBUG: Window list: %p", win));
        
        // DODAJ TO: Otwórz pierwsze okno
        if (win) {
            APTR *window_array = (APTR *)win;
            if (window_array[0]) {
                LOG(("DEBUG: Opening first window: %p", window_array[0]));
                set(window_array[0], MUIA_Window_Open, TRUE);
                
                // Sprawdź czy okno się otworzyło
                BOOL opened = FALSE;
                get(window_array[0], MUIA_Window_Open, &opened);
                LOG(("DEBUG: Window opened: %s", opened ? "YES" : "NO"));
            }
        }
    }*/
}

void gui_multitask(void)
{
	if (netsurf_check_events(TRUE, schedule_sig) & schedule_sig)
		mui_schedule_poll();
}

void gui_poll(bool active)
{
		//LOG(("DEBUG: FILE=%s, LINE=%d\n", __FILE__, __LINE__));
	if (active || browser_reformat_pending) {
		//LOG(("DEBUG: FILE=%s, LINE=%d\n", __FILE__, __LINE__));
		gui_multitask();
	}
	else
	{
		//LOG(("DEBUG: FILE=%s, LINE=%d\n", __FILE__, __LINE__));
		if (netsurf_check_events(FALSE, schedule_sig) & schedule_sig)
			mui_schedule_poll();
	
	}
}

void gui_quit(void)
{
}
struct gui_window *gui_create_browser_window(struct browser_window *bw,
		struct browser_window *clone, bool new_tab)
{
	struct gui_window *gw;

	gw = AllocMem(sizeof(*gw), MEMF_ANY);

	if (gw) {
		LOG(("DEBUG: gui_create_browser_window called"));
		gw->bw = bw;
		gw->pointertype = -1;
		gw->obj = (APTR)methodstack_push_sync(application, 2,
				MM_Application_AddBrowser, gw);

		if (gw->obj == NULL) {
			FreeMem(gw, sizeof(*gw));
			gw = NULL;
		} else {
			gw->BitMap = NULL;
			gw->Layer = NULL;
			gw->LayerInfo = NULL;
			gw->RastPort = NULL;  // Dodaj to jeśli nie ma
			
			LOG(("DEBUG: gui_window created successfully"));
			LOG(("DEBUG: gw=%p, gw->obj=%p", gw, gw->obj));
			
			ADDTAIL(&window_list, gw);
		}
	}

	return gw;
}
struct gui_window *gui_create_browser_window1(struct browser_window *bw,
		struct browser_window *clone, bool new_tab)
{
	struct gui_window *gw;

	gw = AllocMem(sizeof(*gw), MEMF_ANY);

	if (gw) {
		gw->bw = bw;
		gw->pointertype = -1;
		gw->obj = (APTR)methodstack_push_sync(application, 2,
				MM_Application_AddBrowser, gw);

		if (gw->obj == NULL) {
			FreeMem(gw, sizeof(*gw));
			gw = NULL;
		} else {
			gw->BitMap = NULL;
			gw->Layer = NULL;
			gw->LayerInfo = NULL;
			ADDTAIL(&window_list, gw);
		}
	}

	return gw;
}

void gui_window_destroy(struct gui_window *g)
{
	REMOVE(g);

	if (g->win && !netsurf_quit)
		methodstack_push_sync(g->win, 2, MM_Window_RemovePage, g->obj);

	if (g->Layer) {
		TT_DoneRastPort(g->RastPort);
		DeleteLayer(0, g->Layer);
	}

	if (g->LayerInfo)
		DisposeLayerInfo(g->LayerInfo);

	FreeBitMap(g->BitMap);
	FreeMem(g, sizeof(*g));
}

void gui_window_set_title(struct gui_window *g, const char *title)
{
	methodstack_push_sync(g->obj, 3, MUIM_Set, MA_Browser_Title, title);
}

static void gui_redraw_all(struct gui_window *g)
{

}
void gui_window_redraw(struct gui_window *g, int x0, int y0, int x1, int y1)
{
    struct content *c;

    c = g->bw->current_content;
    LOG(("DEBUG: FUNCTION=%s", __FUNCTION__));
    LOG(("DEBUG: Window dimensions check:"));
    
    if (g->obj) {
        LOG(("DEBUG: _mleft(g->obj)=%d", _mleft(g->obj)));
        LOG(("DEBUG: _mtop(g->obj)=%d", _mtop(g->obj)));  
        LOG(("DEBUG: _mwidth(g->obj)=%d", _mwidth(g->obj)));
        LOG(("DEBUG: _mheight(g->obj)=%d", _mheight(g->obj)));
        LOG(("DEBUG: muiRenderInfo(g->obj)=%p", muiRenderInfo(g->obj)));
    }
    c = g->bw->current_content;
    LOG(("DEBUG: FUNCTION=%s", __FUNCTION__));
    LOG(("DEBUG: g=%p, g->obj=%p", g, g->obj));
    LOG(("DEBUG: g->BitMap=%p, g->RastPort=%p", g->BitMap, g->RastPort));
    LOG(("DEBUG: _win(g->obj)=%p", _win(g->obj)));
    LOG(("DEBUG: c=%p", c));

    if (c)
    {
        LOG(("DEBUG: Content available, checking if window is ready"));
        
        // Jeśli okno nie jest gotowe, po prostu pomiń renderowanie
        if (!_win(g->obj)) {
            LOG(("WARNING: Window not ready, skipping redraw"));
            return;
        }
        
        struct RastPort *screen_rp = _rp(g->obj);
        LOG(("DEBUG: Screen RastPort: %p", screen_rp));
        
        if (screen_rp) {
            LOG(("DEBUG: Using direct screen rendering"));
            
            renderinfo.rp = screen_rp;
            
            g->redraw = 0;
            current_redraw_browser = g->bw;
            plot = muiplot;

            renderinfo.width = c->width;
            renderinfo.height = c->height;
//mui/gui.c:785: DEBUG: _mleft(g->obj)=16.
//mui/gui.c:786: DEBUG: _mtop(g->obj)=96.
//mui/gui.c:787: DEBUG: _mwidth(g->obj)=1222.
//mui/gui.c:788: DEBUG: _mheight(g->obj)=662.
            LOG(("DEBUG: About to call content_redraw"));
            // Sprawdź czy x0, y0, x1, y1 są w granicach okna
            if (x0 < 0) x0 = 0;
            if (y0 < 0) y0 = 0;
            if (x1 > c->width) x1 = c->width;
            if (y1 > c->height) y1 = c->height;
            if (x1 < x0) x1 = x0; // Zapewnij, że x1 >= x0
            if (y1 < y0) y1 = y0; // Zapewnij, że y1 >= y0
            LOG(("DEBUG: Calling content_redraw with x0=%d, y0=%d, x1=%d, y1=%d", x0, y0, x1, y1));
            content_redraw(c, _mleft(g->obj), _mtop(g->obj), _mwidth(g->obj), _mheight(g->obj), _mleft(g->obj), _mtop(g->obj), x1, y1, g->bw->scale, 0xfffff);
            LOG(("DEBUG: content_redraw completed"));

            current_redraw_browser = NULL;
			//methodstack_push_imm(g->obj, 1, MM_Browser_Redraw);
            LOG(("DEBUG: Direct screen rendering completed"));
        } else {
            LOG(("ERROR: No screen RastPort even though window exists"));
        }
        
        LOG(("DEBUG: gui_window_redraw completed successfully"));
    }
    else
    {
        LOG(("ERROR: No content available"));
    }
}
void gui_window_redraw11(struct gui_window *g, int x0, int y0, int x1, int y1)
{
    struct content *c;
    static int redraw_attempts = 0;

    c = g->bw->current_content;
    LOG(("DEBUG: FUNCTION=%s, attempt=%d", __FUNCTION__, redraw_attempts));
    LOG(("DEBUG: g=%p, g->obj=%p", g, g->obj));
    LOG(("DEBUG: g->BitMap=%p, g->RastPort=%p", g->BitMap, g->RastPort));
    LOG(("DEBUG: _win(g->obj)=%p", _win(g->obj)));
    LOG(("DEBUG: c=%p", c));

    if (c)
    {
        LOG(("DEBUG: Content available, checking if window is ready"));
        
        // Sprawdź czy okno jest gotowe
        if (!_win(g->obj)) {
            LOG(("WARNING: Window not ready, deferring redraw"));
            
            if (redraw_attempts < 50) {
                redraw_attempts++;
                LOG(("DEBUG: Deferring redraw attempt %d", redraw_attempts));
                
                // Opóźnij renderowanie o 500ms
               // DoMethod(_app(g->obj), MUIM_Application_PushMethod, g->obj, 
                //    1 | MUIV_PushMethod_Delay(25), MM_Browser_Redraw);
                return;
            } else {
                LOG(("ERROR: Giving up on redraw after %d attempts", redraw_attempts));
                return;
            }
        }
        
        // Reset counter when window is ready
        redraw_attempts = 0;
        
        // Okno jest gotowe, sprawdź screen RastPort
        struct RastPort *screen_rp = _rp(g->obj);
        LOG(("DEBUG: Screen RastPort: %p", screen_rp));
        
        if (screen_rp) {
            LOG(("DEBUG: Using direct screen rendering"));
            
            // Renderuj bezpośrednio na ekran
            renderinfo.rp = screen_rp;
            
            g->redraw = 0;
            current_redraw_browser = g->bw;
            plot = muiplot;

            renderinfo.width = c->width;
            renderinfo.height = c->height;

            LOG(("DEBUG: About to call content_redraw"));
            content_redraw(c, 0, 0, c->width, c->height, x0, y0, x1, y1, g->bw->scale, 0xfffff);
            LOG(("DEBUG: content_redraw completed"));

            current_redraw_browser = NULL;
            LOG(("DEBUG: Direct screen rendering completed"));
        } else {
            LOG(("ERROR: No screen RastPort even though window exists"));
        }
        
        methodstack_push(g->obj, 1, MM_Browser_Redraw);
        LOG(("DEBUG: gui_window_redraw completed successfully"));
    }
    else
    {
        LOG(("ERROR: No content available"));
    }
}
void gui_window_redraw10(struct gui_window *g, int x0, int y0, int x1, int y1)
{
    struct content *c;

    c = g->bw->current_content;
    LOG(("DEBUG: FUNCTION=%s", __FUNCTION__));
    LOG(("DEBUG: g=%p, g->obj=%p", g, g->obj));
    LOG(("DEBUG: g->BitMap=%p, g->RastPort=%p", g->BitMap, g->RastPort));
    LOG(("DEBUG: _win(g->obj)=%p", _win(g->obj)));
    LOG(("DEBUG: c=%p", c));

    if (c)
    {
        LOG(("DEBUG: Content available, checking RastPort"));
        
        // Sprawdź czy masz screen RastPort
        if (_win(g->obj)) {
            struct RastPort *screen_rp = _rp(g->obj);
            LOG(("DEBUG: Screen RastPort: %p", screen_rp));
            
            if (screen_rp) {
                LOG(("DEBUG: Using direct screen rendering"));
                
                // Renderuj bezpośrednio na ekran
                renderinfo.rp = screen_rp;
                
                g->redraw = 0;
                current_redraw_browser = g->bw;
                plot = muiplot;

                renderinfo.width = c->width;
                renderinfo.height = c->height;

                LOG(("DEBUG: About to call content_redraw"));
                content_redraw(c, 0, 0, c->width, c->height, x0, y0, x1, y1, g->bw->scale, 0xfffff);
                LOG(("DEBUG: content_redraw completed"));

                current_redraw_browser = NULL;
                LOG(("DEBUG: Direct screen rendering completed"));
            } else {
                LOG(("ERROR: No screen RastPort"));
            }
        } else {
            LOG(("ERROR: No window available"));
        }
        
        methodstack_push(g->obj, 1, MM_Browser_Redraw);
        LOG(("DEBUG: gui_window_redraw completed successfully"));
    }
    else
    {
        LOG(("ERROR: No content available"));
    }
}

void gui_window_redraw9(struct gui_window *g, int x0, int y0, int x1, int y1)
{
    struct content *c;

    c = g->bw->current_content;
    LOG(("DEBUG: FUNCTION=%s", __FUNCTION__));

    if (c && _win(g->obj))
    {
        LOG(("DEBUG: Rendering directly to screen"));
        
        // Renderuj bezpośrednio na ekran
        renderinfo.rp = _rp(g->obj);
        
        if (renderinfo.rp)
        {
            LOG(("DEBUG: Using screen RastPort: %p", renderinfo.rp));
            
            g->redraw = 0;
            current_redraw_browser = g->bw;
            plot = muiplot;

            renderinfo.width = c->width;
            renderinfo.height = c->height;

            LOG(("DEBUG: About to call content_redraw"));
            content_redraw(c, 0, 0, c->width, c->height, x0, y0, x1, y1, g->bw->scale, 0xfffff);
            LOG(("DEBUG: content_redraw completed"));

            current_redraw_browser = NULL;
            LOG(("DEBUG: gui_window_redraw completed successfully"));
        }
    }
}
void gui_window_redraw8(struct gui_window *g, int x0, int y0, int x1, int y1)
{
    struct content *c;

    c = g->bw->current_content;
    LOG(("DEBUG: FUNCTION=%s", __FUNCTION__));

    if (c)
    {
        LOG(("DEBUG: g->RastPort = %p", g->RastPort));
        
        if (g->RastPort)
        {
            LOG(("DEBUG: Using existing g->RastPort"));
            renderinfo.rp = g->RastPort;
            
            LOG(("DEBUG: Starting redraw process"));
            g->redraw = 0;
            current_redraw_browser = g->bw;
            plot = muiplot;

            renderinfo.width = c->width;
            renderinfo.height = c->height;

            LOG(("DEBUG: About to call content_redraw"));
            content_redraw(c, 0, 0, c->width, c->height, x0, y0, x1, y1, g->bw->scale, 0xfffff);
            LOG(("DEBUG: content_redraw completed"));

            current_redraw_browser = NULL;
            
            // DODAJ TO - bezpośrednie kopiowanie na ekran
            LOG(("DEBUG: Copying to screen"));
            if (g->BitMap && _win(g->obj)) {
                struct RastPort *screen_rp = _rp(g->obj);
                if (screen_rp) {
                    BltBitMapRastPort(g->BitMap, 0, 0, screen_rp, 
                        _mleft(g->obj), _mtop(g->obj), 
                        MIN(c->width, _mwidth(g->obj)), 
                        MIN(c->height, _mheight(g->obj)), 
                        0xc0);
                    LOG(("DEBUG: Copied to screen"));
                } else {
                    LOG(("ERROR: No screen RastPort"));
                }
            } else {
                LOG(("ERROR: No BitMap or window"));
            }
            
            methodstack_push(g->obj, 1, MM_Browser_Redraw);
            LOG(("DEBUG: gui_window_redraw completed successfully"));
        }
    }
}
void gui_window_redraw7(struct gui_window *g, int x0, int y0, int x1, int y1)
{
    struct content *c;

    c = g->bw->current_content;
    LOG(("DEBUG: FUNCTION=%s", __FUNCTION__));

    if (c)
    {
        LOG(("DEBUG: g->RastPort = %p", g->RastPort));
        
        if (g->RastPort)
        {
            LOG(("DEBUG: Using existing g->RastPort"));
            renderinfo.rp = g->RastPort;
        }
        else
        {
            LOG(("DEBUG: About to call Browser_GetBitMap with width=%d, height=%d", c->width, c->height));
            renderinfo.rp = (APTR)methodstack_push_sync(g->obj, 3, MM_Browser_GetBitMap, c->width, c->height);
            LOG(("DEBUG: Browser_GetBitMap returned rp=%p", renderinfo.rp));
        }

        if (renderinfo.rp)
        {
            LOG(("DEBUG: Starting redraw process"));
            g->redraw = 0;
            current_redraw_browser = g->bw;
            plot = muiplot;

            renderinfo.width = c->width;
            renderinfo.height = c->height;

            LOG(("DEBUG: About to call content_redraw"));
            content_redraw(c, 0, 0, c->width, c->height, x0, y0, x1, y1, g->bw->scale, 0xfffff);
            LOG(("DEBUG: content_redraw completed"));

            current_redraw_browser = NULL;
            methodstack_push(g->obj, 1, MM_Browser_Redraw);
            LOG(("DEBUG: gui_window_redraw completed successfully"));
        }
        else
        {
            LOG(("WARNING: No RastPort available - skipping redraw"));
        }
    }
}
void gui_window_redraw6(struct gui_window *g, int x0, int y0, int x1, int y1)
{
    struct content *c;

    c = g->bw->current_content;
    LOG(("DEBUG: FUNCTION=%s", __FUNCTION__));

    if (c)
    {
        LOG(("DEBUG: About to call Browser_GetBitMap with width=%d, height=%d", c->width, c->height));
        renderinfo.rp = (APTR)methodstack_push_sync(g->obj, 3, MM_Browser_GetBitMap, c->width, c->height);
        LOG(("DEBUG: Browser_GetBitMap returned rp=%p", renderinfo.rp));

        if (renderinfo.rp)
        {
            // Renderowanie jak wcześniej
            LOG(("DEBUG: Starting redraw process"));
            g->redraw = 0;
            current_redraw_browser = g->bw;
            plot = muiplot;

            renderinfo.width = c->width;
            renderinfo.height = c->height;

            content_redraw(c, 0, 0, c->width, c->height, x0, y0, x1, y1, g->bw->scale, 0xfffff);

            current_redraw_browser = NULL;
            methodstack_push(g->obj, 1, MM_Browser_Redraw);
            LOG(("DEBUG: gui_window_redraw completed successfully"));
        }
        else
        {
            LOG(("WARNING: Browser_GetBitMap returned NULL - window not ready"));
            
            // Spróbuj ponownie za chwilę
            static int retry_count = 0;
            if (retry_count < 10) {
                retry_count++;
                LOG(("DEBUG: Retrying redraw in 100ms (attempt %d)", retry_count));
                DoMethod(_app(g->obj), MUIM_Application_PushMethod, g->obj, 1 | MUIV_PushMethod_Delay(25), 
                    MM_Browser_Redraw);
            } else {
                LOG(("ERROR: Giving up on redraw after 10 attempts"));
                retry_count = 0;
            }
        }
    }
}
void gui_window_redraw5(struct gui_window *g, int x0, int y0, int x1, int y1)
{
    struct content *c;

    c = g->bw->current_content;
    LOG(("DEBUG: FUNCTION=%s", __FUNCTION__));
    LOG(("DEBUG: FUNCTION=%s, c=%p, g->RastPort=%p", __FUNCTION__, c, g->RastPort));

    if (c)
    {
        LOG(("DEBUG: About to call Browser_GetBitMap with width=%d, height=%d", c->width, c->height));
        renderinfo.rp = (APTR)methodstack_push_sync(g->obj, 3, MM_Browser_GetBitMap, c->width, c->height);
        LOG(("DEBUG: Browser_GetBitMap returned rp=%p", renderinfo.rp));

        if (renderinfo.rp)
        {
            LOG(("DEBUG: Starting redraw process"));
            g->redraw = 0;
            current_redraw_browser = g->bw;
            plot = muiplot;

            renderinfo.width = c->width;
            renderinfo.height = c->height;

            LOG(("DEBUG: Content dimensions: %dx%d", c->width, c->height));
            LOG(("DEBUG: About to call content_redraw"));
            content_redraw(c, 0, 0, c->width, c->height, x0, y0, x1, y1, g->bw->scale, 0xfffff);
            LOG(("DEBUG: content_redraw completed"));

            current_redraw_browser = NULL;
            methodstack_push(g->obj, 1, MM_Browser_Redraw);
            LOG(("DEBUG: gui_window_redraw completed successfully"));
        }
        else
        {
            LOG(("WARNING: Browser_GetBitMap returned NULL - window not ready, deferring redraw"));
            // Opóźnij renderowanie
            DoMethod(_app(g->obj), MUIM_Application_PushMethod, g->obj, 1, 
                MM_Browser_Redraw);
        }
    }
    else
    {
        LOG(("ERROR: No current content"));
    }
}
void gui_window_redraw4(struct gui_window *g, int x0, int y0, int x1, int y1)
{
    struct content *c;

    c = g->bw->current_content;
    LOG(("DEBUG: FUNCTION=%s", __FUNCTION__));
    LOG(("DEBUG: FUNCTION=%s, c=%p, g->RastPort=%p", __FUNCTION__, c, g->RastPort));

    if (c)
    {
		LOG(("DEBUG: About to call Browser_GetBitMap with width=%d, height=%d", c->width, c->height));
		renderinfo.rp = (APTR)methodstack_push_sync(g->obj, 3, MM_Browser_GetBitMap, c->width, c->height);
		LOG(("DEBUG: Browser_GetBitMap returned rp=%p", renderinfo.rp));

        if (renderinfo.rp)
        {
            LOG(("DEBUG: Starting redraw process"));
            g->redraw = 0;
            current_redraw_browser = g->bw;
            plot = muiplot;

            renderinfo.width = c->width;
            renderinfo.height = c->height;

            LOG(("DEBUG: Content dimensions: %dx%d", c->width, c->height));
            LOG(("DEBUG: Redraw area: (%d,%d) to (%d,%d)", x0, y0, x1, y1));
            LOG(("DEBUG: renderinfo.rp: %p", renderinfo.rp));

            LOG(("DEBUG: About to call content_redraw"));
            content_redraw(c, 0, 0, c->width, c->height, x0, y0, x1, y1, g->bw->scale, 0xfffff);
            LOG(("DEBUG: content_redraw completed"));

            current_redraw_browser = NULL;
            LOG(("DEBUG: About to call methodstack_push MM_Browser_Redraw"));
            methodstack_push(g->obj, 1, MM_Browser_Redraw);
            LOG(("DEBUG: gui_window_redraw completed successfully"));
        }
        else
        {
            LOG(("ERROR: Browser_GetBitMap returned NULL"));
        }
    }
    else
    {
        LOG(("ERROR: No current content"));
    }
}
void gui_window_redraw3(struct gui_window *g, int x0, int y0, int x1, int y1)
{
    struct content *c;

    c = g->bw->current_content;
    LOG(("DEBUG: FUNCTION=%s", __FUNCTION__));
    LOG(("DEBUG: FUNCTION=%s, c=%p, g->RastPort=%p", __FUNCTION__, c, g->RastPort));

    if (c && g->RastPort)
    {
        LOG(("DEBUG: Using existing RastPort: %p", g->RastPort));
        
        renderinfo.rp = g->RastPort;
        
        if (renderinfo.rp)
        {
            LOG(("DEBUG: Starting redraw process"));
            g->redraw = 0;
            current_redraw_browser = g->bw;
            plot = muiplot;

            renderinfo.width = c->width;
            renderinfo.height = c->height;

            // DODAJ TEN DEBUG:
            LOG(("DEBUG: Content dimensions: %dx%d", c->width, c->height));
            LOG(("DEBUG: Redraw area: (%d,%d) to (%d,%d)", x0, y0, x1, y1));
            LOG(("DEBUG: Browser scale: %f", g->bw->scale));
            LOG(("DEBUG: Plot function: %p", plot));
            LOG(("DEBUG: renderinfo.rp: %p", renderinfo.rp));
            LOG(("DEBUG: renderinfo.width: %d", renderinfo.width));
            LOG(("DEBUG: renderinfo.height: %d", renderinfo.height));

            LOG(("DEBUG: About to call content_redraw"));
            content_redraw(c, 0, 0, c->width, c->height, x0, y0, x1, y1, g->bw->scale, 0xfffff);
            LOG(("DEBUG: content_redraw completed"));

            current_redraw_browser = NULL;
            methodstack_push(g->obj, 1, MM_Browser_Redraw);
            LOG(("DEBUG: gui_window_redraw completed successfully"));
        }
    }
}

void gui_window_redraw2(struct gui_window *g, int x0, int y0, int x1, int y1)
{
    struct content *c;

    c = g->bw->current_content;
    LOG(("DEBUG: FUNCTION=%s", __FUNCTION__));
    LOG(("DEBUG: FUNCTION=%s, c=%p, g->RastPort=%p", __FUNCTION__, c, g->RastPort));

    if (c && g->RastPort)  // Sprawdź czy mamy RastPortcontent_redraw(gui_window_redraw
    {
        LOG(("DEBUG: Using existing RastPort: %p", g->RastPort));
        
        // Użyj istniejący RastPort zamiast próbować pobrać nowy
        renderinfo.rp = g->RastPort;
        
        if (renderinfo.rp)
        {
            LOG(("DEBUG: Starting redraw process"));
            g->redraw = 0;
            current_redraw_browser = g->bw;
            plot = muiplot;

            renderinfo.width = c->width;
            renderinfo.height = c->height;

            LOG(("DEBUG: About to call content_redraw"));
            content_redraw(c, 0, 0, c->width, c->height, x0, y0, x1, y1, g->bw->scale, 0xfffff);
            LOG(("DEBUG: content_redraw completed"));

            current_redraw_browser = NULL;
            LOG(("DEBUG: About to call methodstack_push MM_Browser_Redraw"));
            methodstack_push(g->obj, 1, MM_Browser_Redraw);
            LOG(("DEBUG: gui_window_redraw completed successfully"));
        }
        else
        {
            LOG(("ERROR: RastPort is NULL"));
        }
    }
    else
    {
        if (!c)
            LOG(("ERROR: No current content"));
        if (!g->RastPort)
            LOG(("ERROR: No RastPort in gui_window"));
    }
}

void gui_window_redraw1(struct gui_window *g, int x0, int y0, int x1, int y1)
{
    struct content *c;

    c = g->bw->current_content;
    LOG(("DEBUG: FUNCTION=%s", __FUNCTION__));
    LOG(("DEBUG: FUNCTION=%s, c=%p, g->RastPort=%p", __FUNCTION__, c, g->RastPort));

    if (c)
    {
        LOG(("DEBUG: About to call methodstack_push_sync MM_Browser_GetBitMap"));
        //renderinfo.rp = (APTR)methodstack_push_sync(g->obj, 3, MM_Browser_GetBitMap, c->width, c->height);
		renderinfo.rp = (APTR)DoMethodA(g->obj, (Msg)&(ULONG[]){MM_Browser_GetBitMap, c->width, c->height});
        LOG(("DEBUG: methodstack_push_sync returned rp=%p", renderinfo.rp));

        if (renderinfo.rp)  // ← POPRAWIONE
        {
            LOG(("DEBUG: Starting redraw process"));
            g->redraw = 0;
            current_redraw_browser = g->bw;
            plot = muiplot;

            renderinfo.width = c->width;
            renderinfo.height = c->height;

            LOG(("DEBUG: About to call content_redraw"));
            content_redraw(c, 0, 0, c->width, c->height, x0, y0, x1, y1, g->bw->scale, 0xfffff);
            LOG(("DEBUG: content_redraw completed"));

            current_redraw_browser = NULL;
            LOG(("DEBUG: About to call methodstack_push_imm MM_Browser_Redraw"));
            methodstack_push_imm(g->obj, 1, MM_Browser_Redraw);
            LOG(("DEBUG: gui_window_redraw completed successfully"));
        }
        else
        {
            LOG(("ERROR: MM_Browser_GetBitMap returned NULL"));
        }
    }
    else
    {
        LOG(("ERROR: No current content"));
    }
}
void gui_window_redraw_org(struct gui_window *g, int x0, int y0, int x1, int y1)
{
#if 0
	struct IBox bbox;

	methodstack_push_sync(g->obj, 3, OM_GET, MA_Browser_Box, &bbox);

	if (bbox.Left >= x0 && bbox.Top >= y0)
	{
		if (bbox.Left < x1 && bbox.Top < y1)
			methodstack_push_sync(g->obj, 1, MM_Browser_Redraw);
	}
#else
	struct content *c;

	c = g->bw->current_content;

	if (c)
	{
		renderinfo.rp = (APTR)methodstack_push_sync(g->obj, 3, MM_Browser_GetBitMap, c->width, c->height);

		if (g->RastPort)
		{
			g->redraw = 0;
			current_redraw_browser = g->bw;
			plot = muiplot;

			renderinfo.rp = g->RastPort;
			renderinfo.width = c->width;
			renderinfo.height = c->height;	// * RENDER_MULTIPLIER;
			//renderinfo.maxwidth = _screen(g->obj)->Width;
			//renderinfo.maxheight = _screen(g->obj)->Height;

			content_redraw(c, 0, 0, c->width, c->height, x0, y0, x1, y1, g->bw->scale, 0xfffff);

			current_redraw_browser = NULL;
			methodstack_push_imm(g->obj, 1, MM_Browser_Redraw);
		}
	}
#endif
}

void gui_window_redraw_window(struct gui_window *g)
{
#if 1
	struct content *c;

	c = g->bw->current_content;

	if (c)
		gui_window_redraw(g, 0, 0, c->width, c->height);
#else
	methodstack_push_sync(g->obj, 1, MM_Browser_Redraw);
#endif
}

void gui_window_update_box(struct gui_window *g, const union content_msg_data *data)
{
	struct content *c;

	c = g->bw->current_content;

	if (c)
	{
		renderinfo.rp = (APTR)methodstack_push_sync(g->obj, 3, MM_Browser_GetBitMap, c->width, c->height);

		if (g->RastPort)
		{
			if (g->redraw)
			{
				gui_window_redraw(g, 0, 0, c->width, c->height);
			}
			else
			{
				current_redraw_browser = g->bw;
				plot = muiplot;

				renderinfo.rp = g->RastPort;
				renderinfo.width = c->width;
				renderinfo.height = c->height;	// * RENDER_MULTIPLIER;
				renderinfo.maxwidth = _screen(g->obj)->Width; //<< was off
				renderinfo.maxheight = _screen(g->obj)->Height; //<< was off

				content_redraw(data->redraw.object,
					0, 0,
					data->redraw.width + data->redraw.x, data->redraw.height + data->redraw.y,

					data->redraw.x, data->redraw.y,
					data->redraw.x + data->redraw.width, data->redraw.y + data->redraw.height,

					g->bw->scale, 0xFFFFFF);

				current_redraw_browser = NULL;
				methodstack_push_imm(g->obj, 1, MM_Browser_Redraw);
			}
		}
	}
}

bool gui_window_get_scroll(struct gui_window *g, int *sx, int *sy)
{
	methodstack_push(g->obj, 3, OM_GET, MUIA_Virtgroup_Left, sx);
	methodstack_push_imm(g->obj, 3, OM_GET, MUIA_Virtgroup_Top, sy);
}

void gui_window_set_scroll(struct gui_window *g, int sx, int sy)
{
	IPTR tags[5];

	if (sx < 0)
		sx = 0;

	if (sy < 0)
		sy = 0;

	tags[0] = MUIA_Virtgroup_Left;
	tags[1] = sx;
	tags[2] = MUIA_Virtgroup_Top;
	tags[3] = sy;
	tags[4] = TAG_DONE;

	methodstack_push_sync(g->obj, 2, OM_SET, tags);
}

void gui_window_scroll_visible(struct gui_window *g, int x0, int y0, int x1, int y1)
{
	gui_window_set_scroll(g, x0, y0);
}

void gui_window_position_frame(struct gui_window *g, int x0, int y0, int x1, int y1)
{
}

void gui_window_get_dimensions(struct gui_window *g, int *width, int *height, bool scaled)
{
	struct IBox bbox;
	int w, h;

	methodstack_push_sync(g->obj, 3, OM_GET, MA_Browser_Box, &bbox);

	w = bbox.Width;
	h = bbox.Height;

#if 0
	if (scaled)
	{
		w /= g->bw->scale;
		h /= g->bw->scale;
	}
#else
	//#warning gui_window_get_dimensions(): scaled boolean not supported
#endif

	*width = w;
	*height = h;
}

void gui_window_update_extent(struct gui_window *g)
{
	//#warning disabled gui_window_update_extent()
    LOG(("DEBUG: gui_window_update_extent()) called\n"));
	//methodstack_push(g->obj, 3, MM_Browser_SetContentSize, g->bw->current_content->width, g->bw->current_content->height);
}

void gui_window_set_status(struct gui_window *g, const char *text)
{
	global_obj = g->obj;
	//LOG(("DEBUG: gui_window_set_status: %s\n", text));
	methodstack_push_sync(g->obj, 3, MUIM_Set, MA_Browser_StatusText, text);
}

void gui_window_set_pointer(struct gui_window *g, gui_pointer_shape shape)
{
	#if defined(__MORPHOS__)
	ULONG pointertype = POINTERTYPE_NORMAL;

	switch (shape) {
	case GUI_POINTER_DEFAULT:
		pointertype = POINTERTYPE_NORMAL;
		break;

	case GUI_POINTER_POINT:
		pointertype = POINTERTYPE_SELECTLINK;
		break;

	case GUI_POINTER_CARET:
	case GUI_POINTER_MENU:
	case GUI_POINTER_UP:
	case GUI_POINTER_DOWN:
	case GUI_POINTER_LEFT:
	case GUI_POINTER_RIGHT:
	case GUI_POINTER_RU:
	case GUI_POINTER_LD:
	case GUI_POINTER_LU:
	case GUI_POINTER_RD:
		break;

	case GUI_POINTER_CROSS:
		pointertype = POINTERTYPE_AIMING;
		break;

	case GUI_POINTER_MOVE:
		pointertype = POINTERTYPE_MOVE;
		break;

	case GUI_POINTER_WAIT:
		pointertype = POINTERTYPE_BUSY;
		break;

	case GUI_POINTER_HELP:
		pointertype = POINTERTYPE_HELP;
		break;

	case GUI_POINTER_NO_DROP:
	case GUI_POINTER_NOT_ALLOWED:
		pointertype = POINTERTYPE_NOTAVAILABLE;
		break;

	case GUI_POINTER_PROGRESS:
		pointertype = POINTERTYPE_WORKING;
		break;
	}

	if (g->pointertype != pointertype) {
		g->pointertype = pointertype;
		methodstack_push(g->obj, 3, MUIM_Set, MA_Browser_Pointer, pointertype);
	}
	#endif
}

void gui_window_hide_pointer(struct gui_window *g)
{
	if (g->pointertype != POINTERTYPE_INVISIBLE) {
		g->pointertype = POINTERTYPE_INVISIBLE;
		methodstack_push(g->obj, 3, MUIM_Set, MA_Browser_Pointer, POINTERTYPE_INVISIBLE);
	}
}

void gui_window_set_url(struct gui_window *g, const char *url)
{
	methodstack_push_sync(g->obj, 3, MUIM_Set, MA_Browser_URL, url);
}

void gui_window_start_throbber(struct gui_window *g)
{
	methodstack_push_imm(g->obj, 3, MUIM_Set, MA_Browser_Loading, FALSE);
}

void gui_window_stop_throbber(struct gui_window *g)
{
	methodstack_push_imm(g->obj, 3, MUIM_Set, MA_Browser_Loading, TRUE);
}

void gui_window_place_caret(struct gui_window *g, int x, int y, int height)
{
	gui_window_remove_caret(g);
}

void gui_window_remove_caret(struct gui_window *g)
{
}

/**
 * Called when the gui_window has new content.
 *
 * \param  g  the gui_window that has new content
 */

void gui_window_new_content(struct gui_window *g)
{
	methodstack_push(g->obj, 2, MM_Browser_SetContentType, g->bw->current_content->type);
}

bool gui_window_scroll_start(struct gui_window *g)
{
	return true;
}

bool gui_window_box_scroll_start(struct gui_window *g, int x0, int y0, int x1, int y1)
{
	return true;
}

bool gui_window_frame_resize_start(struct gui_window *g)
{
}

void gui_window_save_as_link(struct gui_window *g, struct content *c)
{
}

void gui_window_set_scale(struct gui_window *g, float scale)
{
}

struct gui_download_window *gui_download_window_create(const char *url,
		const char *mime_type, struct fetch *fetch,
		unsigned int total_size, struct gui_window *gui)
{
	struct gui_download_window *dw;

	dw = AllocMem(sizeof(*dw), MEMF_ANY);

	if (dw)
	{
		struct download *dl;
		ULONG ok;

		dl = (APTR)methodstack_push_sync(application, 2, MM_Application_Download, url);
		ok = 0;

		if (dl)
		{
			BPTR lock;

			dw->dl = dl;
			dl->size = total_size;

			lock = Lock(dl->path, ACCESS_READ);

			if (lock)
			{
				lock = CurrentDir(lock);

				dw->fh = OpenAsync(dl->filename, MODE_WRITE, 8192);

				if (dw->fh)
				{
					ADDTAIL(&download_list, dw);
					SetComment(dl->filename, url);
					ok = 1;
				}

				UnLock(CurrentDir(lock));
			}
		}

		if (!ok)
		{
			if (dl)
			{
				methodstack_push_sync(application, 2, MM_Application_DownloadError, dw->dl);
			}
			else
			{
				FreeMem(dw, sizeof(*dw));
			}

			dw = NULL;
		}
	}

	return dw;
}

void gui_download_window_data(struct gui_download_window *dw, const char *data, unsigned int size)
{
	WriteAsync(dw->fh, (APTR)data, size);

	dw->dl->done += size;

	methodstack_push(application, 1, MM_Application_DownloadUpdate);
}

void gui_download_window_error(struct gui_download_window *dw, const char *error_msg)
{
	methodstack_push_sync(application, 2, MM_Application_DownloadError, dw->dl);
}

void gui_download_window_done(struct gui_download_window *dw)
{
	methodstack_push_sync(application, 2, MM_Application_DownloadDone, dw->dl);
}

void gui_drag_save_object(gui_save_type type, struct content *c, struct gui_window *g)
{
}

void gui_create_form_select_menu(struct browser_window *bw, struct form_control *control)
{
}

void gui_launch_url(const char *url)
{
	#if defined(__MORPHOS__)
	if (!strncmp("mailto:", url, 7))
	{
		if (OpenURLBase == NULL)
			OpenURLBase = OpenLibrary("openurl.library", 6);

		if (OpenURLBase)
			URL_OpenA((STRPTR)url, NULL);
	}
	#endif
}

void gui_cert_verify(struct browser_window *bw, struct content *c, const struct ssl_cert_info *certs, unsigned long num)
{
}
