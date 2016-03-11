//
// Created by twister on 11.03.16.
//


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "main.h"
#include "utils/list.h"


struct list_head opened_files;


void
fcache_init()
{
    INIT_LIST_HEAD( &opened_files );
}


void
fcache_append(struct path_info* pi)
{
    list_add_tail( &pi->list, &opened_files );
}


void
fcache_append_with_copy(struct path_info* pi)
{
    struct path_info* tmp;
    tmp = (struct path_info*)malloc( sizeof(struct path_info) );
    memcpy( tmp, pi, sizeof(struct path_info) );
    fcache_append( tmp );
}


struct path_info*
fcache_find_by_path(const char* path)
{
    struct path_info* iter;
    struct path_info* result = NULL;
    list_for_each_entry( iter, &opened_files, list ) {
        if (!strcmp( path, iter->path )) {
            result = iter;
            break;
        }
    }
    return result;
}


void
fcache_unlink_and_free(struct path_info* pi)
{
    list_del( &pi->list );
    free( pi );
}