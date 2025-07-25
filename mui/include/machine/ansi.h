/*      $NetBSD: ansi.h,v 1.5 1994/10/26 07:50:40 cgd Exp $     */

/*-
 * Copyright (c) 1990, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 *      @(#)ansi.h      8.2 (Berkeley) 1/4/94
 */
  
#include <_G_config.h>

#ifndef _ANSI_H_
#define _ANSI_H_

#define __need___valist
#include <stdarg.h>

/*
 * Types which are fundamental to the implementation and may appear in
 * more than one standard header are defined here.  Standard headers
 * then use:
 *      #ifdef  _BSD_SIZE_T_
 *      typedef _BSD_SIZE_T_ size_t;
 *      #undef  _BSD_SIZE_T_
 *      #endif
 */
#define _BSD_CLOCK_T_   _G_clock_t              /* clock() */
#define _BSD_PTRDIFF_T_ _G_ptrdiff_t            /* ptr1 - ptr2 */
#define _BSD_SIZE_T_    _G_size_t               /* sizeof() */
#define _BSD_SSIZE_T_   _G_ssize_t              /* byte count or error */
#define _BSD_TIME_T_    _G_time_t               /* time() */
#define _BSD_VA_LIST_   __gnuc_va_list          /* va_list */

/*
 * Runes (wchar_t) is declared to be an ``int'' instead of the more natural
 * ``unsigned long'' or ``long''.  Two things are happening here.  It is not
 * unsigned so that EOF (-1) can be naturally assigned to it and used.  Also,
 * it looks like 10646 will be a 31 bit standard.  This means that if your
 * ints cannot hold 32 bits, you will be in trouble.  The reason an int was
 * chosen over a long is that the is*() and to*() routines take ints (says
 * ANSI C), but they use _RUNE_T_ instead of int.  By changing it here, you
 * lose a bit of ANSI conformance, but your programs will still work.
 *    
 * Note that _WCHAR_T_ and _RUNE_T_ must be of the same type.  When wchar_t
 * and rune_t are typedef'd, _WCHAR_T_ will be undef'd, but _RUNE_T remains
 * defined for ctype.h.
 */
#define _BSD_WCHAR_T_   _G_wchar_t              /* wchar_t */
#define _BSD_RUNE_T_    _G_wchar_t              /* rune_t */

#endif  /* _ANSI_H_ */
