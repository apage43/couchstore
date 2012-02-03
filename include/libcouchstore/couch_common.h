#ifndef COUCH_COMMON_H
#define COUCH_COMMON_H
#include <unistd.h>
#include <stdint.h>

#define COUCH_BLOCK_SIZE 4096
#define COUCH_DISK_VERSION 8
#define COUCH_SNAPPY_THRESHOLD 64

typedef struct _sized_buf {
    char* buf;
    size_t size;
} sized_buf;

typedef struct _nodepointer {
    sized_buf key;
    uint64_t pointer;
    sized_buf reduce_value;
    uint64_t subtreesize;
} node_pointer;

typedef struct _doc {
    sized_buf id;
    sized_buf json;
    sized_buf binary;
} Doc;

typedef struct _docinfo {
    sized_buf id;
    sized_buf meta;
    uint64_t deleted;
    uint64_t seq;
    uint64_t rev;
    uint64_t bp;
    size_t size;
} DocInfo;

typedef struct _local_doc {
    sized_buf id;
    sized_buf json;
    int deleted;
} LocalDoc;

typedef struct _db_header
{
    uint64_t disk_version;
    uint64_t update_seq;
    node_pointer* by_id_root;
    node_pointer* by_seq_root;
    node_pointer* local_docs_root;
    uint64_t purge_seq;
    sized_buf *purged_docs;
} db_header;

typedef struct _db {
    int fd;
    uint64_t file_pos;
    db_header header;
} Db;

/* File ops

//Read a chunk from file, remove block prefixes, and decompress.
//Don't forget to free when done with the returned value.
//(If it returns -1 it will not have set ret_ptr, no need to free.) */
int pread_bin(int fd, off_t pos, char **ret_ptr);

int pread_header(int fd, off_t pos, char **ret_ptr);

ssize_t total_read_len(off_t blockoffset, ssize_t finallen);

int db_write_header(Db* db, sized_buf* buf);
int db_write_buf(Db* db, sized_buf* buf, off_t *pos);

/* Errors */

#define ERROR_OPEN_FILE -1
#define ERROR_PARSE_TERM -2
#define ERROR_ALLOC_FAIL -3
#define ERROR_READ -4
#define DOC_NOT_FOUND -5
#define ERROR_NO_HEADER -6
#define ERROR_WRITE -7
#define ERROR_HEADER_VERSION -8
#define ERROR_CHECKSUM_FAIL -9

#endif
