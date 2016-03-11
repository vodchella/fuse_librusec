/**
 * Created by twister on 09.03.2016
 *
 * sudo apt-get install libfuse-dev libsqlite3-dev libzip4 libzip-dev
 *
 * mkdir mnt
 * ./fuse_librusec mnt
 * fusermount -u mnt
 */


#define FUSE_USE_VERSION 30


#include <fuse.h>
#include <zip.h>
#include <string.h>
#include <errno.h>
#include "main.h"
#include "utils/sqlite.h"
#include "utils/str.h"


int
get_path_type(const char* path, struct path_info* pi)
{
    // TODO Реализовать кэширование обработанных путей

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
                if (result == PATH_TO_FILE) {
                    pi->file_length = sqlite_get_book_file_length( pi->target );
                }
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
        stbuf->st_size = pi.file_length;
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
    int path_type = get_path_type( path, NULL );
    if (path_type == PATH_TO_FILE) {
        if ((fi->flags & 3) != O_RDONLY) {
            return -EROFS;
        }
        return 0;
    }
    return -ENOENT;
}


static int
librusec_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
    (void)fi;

    struct path_info pi;
    memset( &pi, 0, sizeof(struct path_info) );
    int path_type = get_path_type( path, &pi );
    if (path_type == PATH_TO_FILE) {
        zip_t* f = NULL;
        struct zip_stat file_info;
        f = zip_open( "/home/twister/Dropbox/fuse_librusec/some_file.zip", ZIP_RDONLY, NULL );
        if (f) {
            char file_name[MAX_BUFFER_LENGTH];
            memset( file_name, 0, MAX_BUFFER_LENGTH );
            sqlite_get_book_file_name( pi.target, file_name );
            if (file_name) {
                int rs = zip_stat( f, file_name, 0, &file_info );
                if (!rs) {
                    size_t len = file_info.size;
                    if (offset < len) {
                        if (offset + size > len) {
                            size = len - offset;
                        }
                        zip_file_t *zf = zip_fopen_index( f, file_info.index, 0 );
                        if (zf) {
                            if (offset) {
                                char buffer[1024 * 1024];
                                zip_int64_t r;
                                size_t bytes_read = 0;
                                memset( buffer, 0, sizeof(buffer) );

                                // Seek to offset
                                size_t remains = (size_t) offset; // offset always positive
                                r = zip_fread( zf, buffer, (remains > sizeof(buffer) ? sizeof(buffer) : remains) );
                                while (bytes_read < offset) {
                                    printf("%s", buffer);
                                    bytes_read += r;
                                    remains -= r;
                                    memset( buffer, 0, (size_t)r );
                                    r = zip_fread( zf, buffer, (remains > sizeof(buffer) ? sizeof(buffer) : remains) );
                                };
                            }

                            // Read data
                            memset( buf, 0, size );
                            zip_fread( zf, buf, size );

                            zip_fclose( zf);
                        }
                    } else {
                        size = 0;
                    }
                }
            }
            zip_close( f );
            return (int)size; // TODO Разобраться, почему результат int, а возвращать надо size_t
        }
    }
    return 0;
}


static int
librusec_write(const char *path, const char *buf, size_t size,
               off_t offset, struct fuse_file_info *fi)
{
    (void)path;
    (void)buf;
    (void)size;
    (void)offset;
    (void)fi;

    return -EROFS;
}


static struct fuse_operations librusec_oper = {
    .getattr    = librusec_getattr,
    .readdir    = librusec_readdir,
    .open       = librusec_open,
    .read       = librusec_read,
    .write      = librusec_write,
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