#ifndef COUCHSTORE_UTIL_H
#define COUCHSTORE_UTIL_H

#include <string.h>
#include <signal.h>
#include <libcouchstore/couch_db.h>

#include "internal.h"
#include "fatbuf.h"

#ifndef DEBUG
#define error_pass(C) if((errcode = (C)) < 0) { goto cleanup; }
#else
#include <stdio.h>
#define error_pass(C) if((errcode = (C)) < 0) { \
                            fprintf(stderr, "Couchstore error `%s' at %s:%d\r\n", \
                            couchstore_strerror(errcode), __FILE__, __LINE__); goto cleanup; }
#endif
#define error_unless(C, E) if(!(C)) { error_pass(E); }
#define error_nonzero(C, E) if((C) != 0) { error_pass(E); }

#ifdef __cplusplus
extern "C" {
#endif

#define atom_check(B, A) do_atom_check(B, A, sizeof(A) - 1)

    static inline int do_atom_check(char *buf, const char *atomname, int len)
    {
        //quick atom check for < 255 in len
        if (buf[0] != 100) {
            return 0;
        }
        if (buf[1] != 0) {
            return 0;
        }
        if (buf[2] != len) {
            return 0;
        }
        return !strncmp(atomname, buf + 3, len);
    }


#ifdef __cplusplus
}
#endif

#endif
