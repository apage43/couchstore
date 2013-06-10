/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
#include "config.h"
#include <libcouchstore/couch_common.h>

uint64_t couchstore_byteswap64(uint64_t val)
{
#ifdef __sun
    return ntohll(val);
#elif defined(__APPLE__)
    return OSSwapBigToHostInt64(val);
#elif defined(__GLIBC__)
    return bswap_64(val);
#else
    size_t ii;
    uint64_t ret = 0;
    for (ii = 0; ii < sizeof(uint64_t); ii++) {
        ret <<= 8;
        ret |= val & 0xff;
        val >>= 8;
    }
    return ret;
#endif
}
