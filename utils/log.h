/*
 * Copyright 2003 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2004 John Tytgat <joty@netsurf-browser.org>
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

#ifndef _NETSURF_LOG_H_
#define _NETSURF_LOG_H_


#include <exec/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <clib/debug_protos.h>

#include "desktop/netsurf.h"
//#undef NDEBUG
#define NDEBUG
#define MAX_LOGFMT 256
// pomocnicza funkcja do RawDoFmt()
static VOID my_kputc(UBYTE c, APTR unused)
{
    char s[2] = { c, '\0' };
    kprintf("%s", s);
}

static void logprintf(const char *fmt, ...)
{
    char buf[256];
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);  // przygotuj format do bufora
    va_end(args);

    kprintf("%s", buf);  // wypisz gotowy bufor
}


#ifdef NDEBUG
#  define LOG(x) ((void) 0)
#else
#define LOG(x) \
    do { \
        if (verbose_log) { \
            kprintf("%s:%ld: ", __FILE__, (LONG)__LINE__); \
            logprintf x; \
            kprintf("\n"); \
        } \
    } while (0)
#endif


#endif
