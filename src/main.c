/**
 * Created by twister on 09.03.2016
 *
 * sudo apt-get install libfuse-dev libsqlite3-dev
 *
 * mkdir mnt
 * ./fuse_librusec mnt
 * fusermount -u mnt
 */


#define FUSE_USE_VERSION 30


#include <fuse.h>
#include <string.h>
#include <errno.h>
#include "main.h"
#include "utils/sqlite.h"
#include "utils/str.h"


int
get_path_type(const char* path, struct path_info* pi)
{
    int result = 0;
    if ( (strcmp( path, ROOT_PATH ) == 0) ||
         (strcmp( path, AUTHORS_PATH ) == 0) ||
         (strcmp( path, BOOKS_PATH ) == 0)) {
        result = PATH_TO_DIRECTORY;
    } else {
        result = PATH_TO_DIRECTORY;

        int subdir = 0;
        char* save_ptr;
        size_t path_len = strlen( path );
        size_t buf_size = path_len + 1;
        char buf[buf_size];
        memset( &buf, 0, buf_size );
        strncpy( (char*)&buf, path, path_len );

        int level = 0;
        char* pch = strtok_r( buf, PATH_DELIMITER, &save_ptr );
        while (pch != NULL) {
            if (level == 0 && !subdir) {
                if (!strcmp( pch, AUTHORS_DIR )) {
                    subdir = AUTHORS_SUBDIR;
                } else if (!strcmp( pch, BOOKS_DIR )) {
                    subdir = BOOKS_SUBDIR;
                } else {
                    return 0;
                }
            } else if (level == 1) {
                if (!sqlite_prefix_exists( pch, subdir )) {
                    return 0;
                }
            } else if (level == 2) {
                if (subdir == AUTHORS_SUBDIR) {
                    if (!sqlite_author_exists( pch )) {
                        return 0;
                    }
                } else if (subdir == BOOKS_SUBDIR) {
                    if (sqlite_book_exists( pch )) {
                        result = PATH_TO_FILE;
                    }
                }
            } else if (level == 3) {
                if (subdir == AUTHORS_SUBDIR) {
                    if (sqlite_book_exists( pch )) {
                        result = PATH_TO_FILE;
                    }
                }
            }

            if (pi) {
                string_copy_to_buffer( pi->target, pch );
                pi->level = level;
                pi->subdir = subdir;
            }

            pch = strtok_r( NULL, PATH_DELIMITER, &save_ptr );
            level++;
        }
    }
    return result;
}


static int
librusec_getattr(const char *path, struct stat *stbuf)
{
    int res = 0;
    struct path_info pi;

    memset( stbuf, 0, sizeof(struct stat) );
    memset( &pi, 0, sizeof(struct path_info) );
    int path_type = get_path_type( path, &pi );
    if (path_type == PATH_TO_DIRECTORY) {
        stbuf->st_mode = S_IFDIR | 0555;
        stbuf->st_nlink = 2;
    } else if (path_type == PATH_TO_FILE) {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = 0;
    } else {
        res = -ENOENT;
    }

    return res;
}


static int
librusec_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                 off_t offset, struct fuse_file_info *fi)
{
    (void) offset;
    (void) fi;

    struct sqlile_callback_data data;
    memset(&data, 0, sizeof(struct sqlile_callback_data));
    data.buf = buf;
    data.filler = filler;

    if (strcmp( path, ROOT_PATH ) == 0) {
        filler( buf, AUTHORS_PATH + 1, NULL, 0 );
        filler( buf, BOOKS_PATH + 1, NULL, 0 );
    } else if (strcmp( path, AUTHORS_PATH ) == 0) {
        sqlite_get_prefixes( &data, AUTHORS_SUBDIR );
    } else if (strcmp( path, BOOKS_PATH ) == 0) {
        sqlite_get_prefixes( &data, BOOKS_SUBDIR );
    } else {
        struct path_info pi;
        memset( &pi, 0, sizeof(struct path_info) );
        int path_type = get_path_type( path, &pi );
        if (path_type == PATH_TO_DIRECTORY) {
            if (pi.subdir == AUTHORS_SUBDIR) {
                if (pi.level == 1) {
                    sqlite_get_authors( &data, pi.target );
                } else if (pi.level == 2) {
                    sqlite_get_books( &data, pi.subdir, pi.target );
                } else {
                    return -ENOENT;
                }
            } else if (pi.subdir == BOOKS_SUBDIR) {
                if (pi.level == 1) {
                    sqlite_get_books( &data, pi.subdir, pi.target );
                } else {
                    return -ENOENT;
                }
            } else {
                return -ENOENT;
            }
        } else {
            return -ENOENT;
        }
    }

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    return 0;
}


static int
librusec_open(const char *path, struct fuse_file_info *fi)
{
    return -EACCES;
}


static int
librusec_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
    return -EACCES;
}


static struct fuse_operations librusec_oper = {
    .getattr	= librusec_getattr,
    .readdir	= librusec_readdir,
    .open		= librusec_open,
    .read		= librusec_read,
};


int
main(int argc, char* argv[])
{
    int rc = sqlite_open_database();
    if (rc) {
        return rc;
    }
    return fuse_main( argc, argv, &librusec_oper, NULL );
}