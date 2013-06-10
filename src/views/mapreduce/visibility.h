/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
#ifndef _MAPREDUCE_VISIBILITY_H
#define _MAPREDUCE_VISIBILITY_H

#if defined(LIBMAPREDUCE_INTERNAL)

#ifdef __SUNPRO_C
#define LIBMAPREDUCE_API __global
#elif defined(__GNUC__)
#define LIBMAPREDUCE_API __attribute__ ((visibility("default")))
#elif defined(_MSC_VER)
#define LIBMAPREDUCE_API extern __declspec(dllexport)
#else
#define LIBMAPREDUCE_API
#endif

#else

#ifdef _MSC_VER
#define LIBMAPREDUCE_API extern __declspec(dllimport)
#else
#define LIBMAPREDUCE_API
#endif

#endif

#endif
