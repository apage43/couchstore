/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
#include "config.h"
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "internal.h"

#undef LOG_IO
#ifdef LOG_IO
#include <stdio.h>
#endif

static inline int handle_to_fd(couch_file_handle handle)
{
    return (int)(intptr_t)handle;
}

static inline couch_file_handle fd_to_handle(int fd)
{
    return (couch_file_handle)(intptr_t)fd;
}

static ssize_t couch_pread(couch_file_handle handle, void *buf, size_t nbyte, off_t offset)
{
#ifdef LOG_IO
    fprintf(stderr, "PREAD  %8llx -- %8llx  (%6.1f kbytes)\n", offset, offset+nbyte, nbyte/1024.0);
#endif
    int fd = handle_to_fd(handle);
    ssize_t rv;
    do {
        rv = pread(fd, buf, nbyte, offset);
    } while (rv == -1 && errno == EINTR);

    return rv;
}

static ssize_t couch_pwrite(couch_file_handle handle, const void *buf, size_t nbyte, off_t offset)
{
#ifdef LOG_IO
    fprintf(stderr, "PWRITE %8llx -- %8llx  (%6.1f kbytes)\n", offset, offset+nbyte, nbyte/1024.0);
#endif
    int fd = handle_to_fd(handle);
    ssize_t rv;
    do {
        rv = pwrite(fd, buf, nbyte, offset);
    } while (rv == -1 && errno == EINTR);

    return rv;
}

static couchstore_error_t couch_open(couch_file_handle* handle, const char *path, int oflag)
{
    int fd;
    do {
        fd = open(path, oflag | O_LARGEFILE, 0666);
    } while (fd == -1 && errno == EINTR);

    if (fd == -1) {
        if (errno == ENOENT) {
            return COUCHSTORE_ERROR_NO_SUCH_FILE;
        } else {
            return COUCHSTORE_ERROR_OPEN_FILE;
        }
    }
    // Tell the caller about the new handle (file descriptor)
    *handle = fd_to_handle(fd);
    return COUCHSTORE_SUCCESS;
}

static void couch_close(couch_file_handle handle)
{
    int fd = handle_to_fd(handle);
    int rv;

    if (fd != -1) {
        do {
            assert(fd >= 3);
            rv = close(fd);
        } while (rv == -1 && errno == EINTR);
    }
}

static off_t couch_goto_eof(couch_file_handle handle)
{
    int fd = handle_to_fd(handle);
    return lseek(fd, 0, SEEK_END);
}


static couchstore_error_t couch_sync(couch_file_handle handle)
{
    int fd = handle_to_fd(handle);
    int rv;
    do {
        rv = fsync(fd);
    } while (rv == -1 && errno == EINTR);

    if (rv == -1) {
        return COUCHSTORE_ERROR_WRITE;
    }

    return COUCHSTORE_SUCCESS;
}

static int couchstore_advice_to_posix(couchstore_io_advice_t advice)
{
    switch(advice) {
#ifdef POSIX_FADV_NORMAL
        case COUCHSTORE_IOADV_NORMAL:
            return POSIX_FADV_NORMAL;
        case COUCHSTORE_IOADV_DONTNEED:
            return POSIX_FADV_DONTNEED;
        case COUCHSTORE_IOADV_SEQUENTIAL:
            return POSIX_FADV_SEQUENTIAL;
#else
        default:
            return 0;
#endif
    }
}

static couchstore_error_t couch_advise(couch_file_handle handle, off_t offset, off_t len,
                                       couchstore_io_advice_t advice)
{
#ifdef POSIX_FADV_NORMAL
    posix_fadvise(handle_to_fd(handle), offset, len, couchstore_advice_to_posix(advice));
#endif
    return COUCHSTORE_SUCCESS;
}

static couch_file_handle couch_constructor(void)
{
    // We don't have a file descriptor till couch_open runs, so return an invalid value for now.
    return fd_to_handle(-1);
}

static void couch_destructor(couch_file_handle handle)
{
    // nothing to do here
    (void)handle;
}

static const couch_file_ops default_file_ops = {
    (uint64_t)2,
    couch_constructor,
    couch_open,
    couch_close,
    couch_pread,
    couch_pwrite,
    couch_goto_eof,
    couch_sync,
    couch_advise,
    couch_destructor
};

const couch_file_ops *couch_get_default_file_ops(void)
{
    return &default_file_ops;
}
