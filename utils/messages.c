/*
 * Copyright 2004 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2006 Rob Kendrick <rjek@rjek.com>
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
 * Localised message support (implementation).
 *
 * Native language messages are loaded from a file and stored hashed by key for
 * fast access.
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <zlib.h>
#include "utils/log.h"
#include "utils/messages.h"
#include "utils/utils.h"
#include "utils/hashtable.h"

/** We store the messages in a fixed-size hash table. */
#define HASH_SIZE 101

/** The hash table used to store the standard Messages file for the old API */
static struct hash_table *messages_hash = NULL;

/**
 * Read keys and values from messages file.
 *
 * \param  path  pathname of messages file
 * \param  ctx   struct hash_table to merge with, or NULL for a new one.
 * \return struct hash_table containing the context or NULL in case of error.
 */
#if 1
struct hash_table *
messages_load_ctx(const char *path, struct hash_table *ctx)
{
    char s[400];
    gzFile fp;

    assert(path != NULL);

    ctx = (ctx != NULL) ? ctx : hash_create(HASH_SIZE);
    if (ctx == NULL) {
        LOG(("Unable to create hash table for messages file %s", path));
        return NULL;
    }

    fp = gzopen(path, "r");
    if (!fp) {
        snprintf(s, sizeof s,
                 "Unable to open messages file \"%.100s\": %s",
                 path, strerror(errno));
        s[sizeof s - 1] = 0;
        LOG(("%s", s));
        hash_destroy(ctx);
        return NULL;
    }

    while (gzgets(fp, s, sizeof s)) {
        char *colon, *value;

        if (s[0] == '\0' || s[0] == '#')
            continue;

        s[strlen(s) - 1] = '\0';  /* strip trailing '\n' */
        colon = strchr(s, ':');
        if (!colon)
            continue;
        *colon = '\0';            /* terminate key */
        value = colon + 1;

        if (!hash_add(ctx, s, value)) {
            LOG(("Unable to add %s:%s to hash table of %s",
                 s, value, path));
            gzclose(fp);
            hash_destroy(ctx);
            return NULL;
        }
    }

    gzclose(fp);
    return ctx;
}

#else
struct hash_table *messages_load_ctx(const char *path, struct hash_table *ctx)
{
	char s[400];
	gzFile *fp;

	assert(path != NULL);

	ctx = (ctx != NULL) ? ctx : hash_create(HASH_SIZE);

	if (ctx == NULL) {
		LOG(("Unable to create hash table for messages file %s", path));
		return NULL;
	}

	fp = gzopen(path, "r");
	if (!fp) {
		snprintf(s, sizeof s, "Unable to open messages file "
				"\"%.100s\": %s", path, strerror(errno));
		s[sizeof s - 1] = 0;
		LOG(("%s", s));
		hash_destroy(ctx);
		return NULL;
	}

	while (gzgets(fp, s, sizeof s)) {
		char *colon, *value;

		if (s[0] == 0 || s[0] == '#')
			continue;

		s[strlen(s) - 1] = 0;  /* remove \n at end */
		colon = strchr(s, ':');
		if (!colon)
			continue;
		*colon = 0;  /* terminate key */
		value = colon + 1;

		if (hash_add(ctx, s, value) == false) {
			LOG(("Unable to add %s:%s to hash table of %s",
				s, value, path));
			gzclose(fp);
			hash_destroy(ctx);
			return NULL;
		}
	}

	gzclose(fp);

	return ctx;
}
#endif
/**
 * Read keys and values from messages file into the standard Messages hash.
 *
 * \param  path  pathname of messages file
 *
 * The messages are merged with any previously loaded messages. Any keys which
 * are present already are replaced with the new value.
 *
 * Exits through die() in case of error.
 */

void messages_load(const char *path)
{
	struct hash_table *m;
	char s[400];

	assert(path != NULL);

	m = messages_load_ctx(path, messages_hash);
	if (m == NULL) {
		LOG(("Unable to open Messages file '%s'.  Possible reason: %s",
				path, strerror(errno)));
		snprintf(s, sizeof s,
				"Unable to open Messages file '%s'.", path);
		die(s);
	}

	messages_hash = m;
}

/**
 * Fast lookup of a message by key.
 *
 * \param  key  key of message
 * \param  ctx  context of messages file to look up in
 * \return value of message, or key if not found
 */

const char *messages_get_ctx(const char *key, struct hash_table *ctx)
{
	const char *r;

	assert(key != NULL);

	/* If we're called with no context, it's nicer to return the
	 * key rather than explode - this allows attempts to get messages
	 * before messages_hash is set up to fail gracefully, for example */
	if (ctx == NULL)
		return key;

	r = hash_get(ctx, key);

	return r ? r : key;
}

/**
 * Fast lookup of a message by key from the standard Messages hash.
 *
 * \param  key  key of message
 * \return value of message, or key if not found
 */

const char *messages_get(const char *key)
{
	return messages_get_ctx(key, messages_hash);
}
