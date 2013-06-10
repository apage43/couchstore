/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 *     Copyright 2010, 2011 Couchbase, Inc.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

/**
 * This file contains the static part of the configure script. Please add
 * all platform specific conditional code to this file.
 */
#ifndef COUCHSTORE_CONFIG_STATIC_H
#define COUCHSTORE_CONFIG_STATIC_H 1

/* Large File Support */
#define _LARGE_FILE 1
#ifndef _FILE_OFFSET_BITS
#  define _FILE_OFFSET_BITS 64
#elif (_FILE_OFFSET_BITS != 64)
#error "bad things"
#endif
#define _LARGEFILE_SOURCE 1
#ifndef O_LARGEFILE
# define O_LARGEFILE 0
#endif

#include <sys/types.h>
#include <inttypes.h>

#ifdef WIN32
#include <windows.h>
#define WINDOWS
#else
#include <netinet/in.h>
#endif

#ifdef __APPLE__
   /* autoconf things OS X has fdatasync but it doesn't */
#define fdatasync(FD) fsync(FD)
/* On Darwin, use built-in functions for 64-bit byte-swap: */
#include <libkern/OSByteOrder.h>
#endif /* __APPLE__ */

#ifdef __GLIBC__
#include <byteswap.h>
#endif /* __GLIBC__ */

#ifndef __sun
#define ntohll(a) couchstore_byteswap64(a)
#define htonll(a) couchstore_byteswap64(a)
#endif

#ifdef linux
#undef ntohs
#undef ntohl
#undef htons
#undef htonl
#endif

#endif
