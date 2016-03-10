//
// Created by twister on 10.03.16.
//

#ifndef FUSE_LIBRUSEC_SQLITE_H
#define FUSE_LIBRUSEC_SQLITE_H

#include <stdbool.h>

bool sqlite_prefix_exists(char *prefix, int subdir);
bool sqlite_author_exists(char* author);
bool sqlite_book_exists(char* book);
unsigned long sqlite_get_prefixes(struct sqlile_callback_data* data, int subdir);
unsigned long sqlite_get_authors(struct sqlile_callback_data* data, char* prefix);
unsigned long sqlite_get_books(struct sqlile_callback_data* data, int subdir, char* param);
int sqlite_open_database();

#endif //FUSE_LIBRUSEC_SQLITE_H
