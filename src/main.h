//
// Created by twister on 10.03.2016
//

#ifndef FUSE_LIBRUSEC_MAIN_H
#define FUSE_LIBRUSEC_MAIN_H

#include <fuse.h>

#define PATH_TO_DIRECTORY 1
#define PATH_TO_FILE      2
#define AUTHORS_SUBDIR    1
#define BOOKS_SUBDIR      2

#define  PATH_DELIMITER       "/"
#define  SQL_PREFIXES         "select distinct %s from books order by %s"
#define  SQL_VALUE_EXISTS     "select count(*) from books where %s = '%s'"
#define  SQL_GET_VALUE        "select max(%s) from books where %s = '%s'"
#define  SQL_AUTHORS          "select distinct author from books where author_prefix = '%s' order by author"
#define  SQL_BOOKS            "select distinct name from books where %s = '%s' order by name"
#define  ROOT_PATH            "/"
#define  AUTHORS_PATH         "/authors"
#define  AUTHORS_DIR          "authors"
#define  BOOKS_PATH           "/books"
#define  BOOKS_DIR            "books"

#define SQLITE_OP_FILL_BUFFER       1
#define SQLITE_OP_RETURN_ONE_VALUE  2

#define MAX_BUFFER_LENGTH  512

struct sqlile_callback_data
{
    int              operation;
    void*            buf;
    fuse_fill_dir_t  filler;
    unsigned long    count;
    char             result[MAX_BUFFER_LENGTH];
};

struct path_info
{
    int     subdir;
    int     level;
    size_t  file_length;
    char    target[MAX_BUFFER_LENGTH];
};

#endif //FUSE_LIBRUSEC_MAIN_H
