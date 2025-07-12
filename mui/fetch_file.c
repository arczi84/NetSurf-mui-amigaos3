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

/** \file
 * Fetching of data from a file (implementation).
 */

#include <malloc.h>
#include <string.h>
#include <time.h>

#include <proto/asyncio.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/utility.h>

#include "content/content.h"
#include "content/fetch.h"
#include "mui/fetch.h"
#include "mui/mui.h"
#include "mui/utils.h"
#include "utils/log.h"
#include "utils/url.h"
#include "utils/messages.h"

STATIC struct MinList file_fetcher_list = { (APTR)&file_fetcher_list.mlh_Tail, NULL, (APTR)&file_fetcher_list };
STATIC UBYTE fetch_buffer[1024];

/**
 * Finalise a MUI fetcher
 */
void mui_fetch_file_finalise(const char *scheme)
{
	mui_fetch_finalise(&file_fetcher_list);
}

/**
 * Start fetching data for the given URL.
 *
 * The function returns immediately. The fetch may be queued for later
 * processing.
 *
 * A pointer to an opaque struct fetch_info is returned, which can be passed to
 * fetch_abort() to abort the fetch at any time. Returns 0 if memory is
 * exhausted (or some other fatal error occurred).
 *
 * The caller must supply a callback function which is called when anything
 * interesting happens. The callback function is first called with msg
 * FETCH_TYPE, with the Content-Type header in data, then one or more times
 * with FETCH_DATA with some data for the url, and finally with
 * FETCH_FINISHED. Alternatively, FETCH_ERROR indicates an error occurred:
 * data contains an error message. FETCH_REDIRECT may replace the FETCH_TYPE,
 * FETCH_DATA, FETCH_FINISHED sequence if the server sends a replacement URL.
 *
 * Some private data can be passed as the last parameter to fetch_start, and
 * callbacks will contain this.
 */
static void *mui_fetch_file_setup(struct fetch *parent_fetch, const char *url, bool only_2xx, const char *post_urlenc, struct form_successful_control *post_multipart, const char **headers)
{
	struct fetch_info *fetch;

	fetch = mui_fetch_setup(parent_fetch, url, only_2xx, post_urlenc, post_multipart, headers, &file_fetcher_list);

	if (fetch) {
		LONG ok = 0;

		fetch->path = url_to_path(url);

		if (fetch->path) {
			// Don't open file immediately - do it in poll function
			LOG(("fetch %p path %s", fetch, fetch->path));
			
			// Initialize cache data like in ami version
			fetch->cachedata.req_time = time(NULL);
			fetch->cachedata.res_time = time(NULL);
			fetch->cachedata.date = 0;
			fetch->cachedata.expires = 0;
			fetch->cachedata.age = INVALID_AGE;
			fetch->cachedata.max_age = 0;
			fetch->cachedata.no_cache = true;
			fetch->cachedata.etag = NULL;
			fetch->cachedata.last_modified = 0;
			
			ok = 1;
		}

		if (!ok) {
			mui_fetch_free(fetch);
			fetch = NULL;
		}
	}

	return fetch;
}

/**
 * Do some work on current fetches.
 *
 * Must be called regularly to make progress on fetches.
 */
static void mui_fetch_file_poll(const char *scheme_ignored)
{
	struct fetch_info *next, *fetch;

	ITERATELISTSAFE(fetch, next, &file_fetcher_list) {
		if (fetch->locked) continue;

		if (!fetch->aborted) {
			if (fetch->fh) {
				// File is already open, read data
				ULONG len;

				len = ReadAsync(fetch->fh, fetch_buffer, sizeof(fetch_buffer));

				LOG(("fetch %p read %ld", fetch, len));

				mui_fetch_send_callback(FETCH_DATA, fetch, fetch_buffer, len);

				if (len < sizeof(fetch_buffer) && !fetch->aborted) {
					mui_fetch_send_callback(FETCH_FINISHED, fetch, &fetch->cachedata, 0);
					fetch->aborted = TRUE;
				}
			} else {
				// Open file for the first time
				fetch->fh = OpenAsync(fetch->path, MODE_READ, sizeof(fetch_buffer));

				if (fetch->fh) {
					BPTR lock;
					LOG(("fetch %p path %s", fetch, fetch->path));
					lock = Lock(fetch->path, ACCESS_READ);

					if (lock) {
						D_S(struct FileInfoBlock, fib)

						#if defined(__MORPHOS__)
						if (DOSBase->dl_lib.lib_Version >= 51) {
							if (Examine64(lock, fib, NULL))
								fetch->len = fib->fib_Size64;
						}
						else
						#endif
						{
							if (Examine(lock, fib))
								fetch->len = fib->fib_Size;
							else
								LOG(("Examine failed on %s", fetch->path));
						}

						UnLock(lock);
					}
					else
						LOG(("Unable to lock file %s", fetch->path));

					fetch_set_http_code(fetch->fetch_handle, 200);
					fetch->mimetype = fetch_mimetype(fetch->path);
					LOG(("mimetype %s len %ld", fetch->mimetype, fetch->len));

					mui_fetch_send_callback(FETCH_TYPE, fetch, fetch->mimetype, fetch->len);
				} else {
					STRPTR errorstring;
					ULONG length;

					// Improved error message like in ami version
					if (messages_get("FileError")) {
						length = strlen(messages_get("FileError")) + strlen(fetch->path) + 2;
						errorstring = AllocTaskPooled(length);

						if (errorstring) {
							NewRawDoFmt("%s %s", NULL, errorstring, messages_get("FileError"), fetch->path);
						}
					} else {
						// Fallback to original message
						length = sizeof("Could not open file\n%s") + strlen(fetch->path);
						errorstring = AllocTaskPooled(length);

						if (errorstring) {
							NewRawDoFmt("Could not open file\n%s", NULL, errorstring, fetch->path);
						}
					}

					if (errorstring) {
						fetch_set_http_code(fetch->fetch_handle, 404);
						mui_fetch_send_callback(FETCH_ERROR, fetch, errorstring, 0);
						FreeTaskPooled(errorstring, length);
					}
					fetch->aborted = true;
				}
			}
		}

		if (fetch->aborted) {
			fetch_remove_from_queues(fetch->fetch_handle);
			fetch_free(fetch->fetch_handle);
		}
	}
}

/**
 * Initialise the fetcher.
 *
 * Must be called once before any other function.
 */
void mui_fetch_file_register(void)
{
	if (!fetch_add_fetcher("file",
				       mui_fetch_initialise,
				       mui_fetch_file_setup,
				       mui_fetch_start,
				       mui_fetch_abort,
				       mui_fetch_free,
				       mui_fetch_file_poll,
				       mui_fetch_file_finalise)) {
			LOG(("Unable to register MUI fetcher for file:"));
	}
}