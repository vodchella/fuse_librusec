//
// Created by twister on 10.03.16.
//

#include <sqlite3.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "str.h"
#include "../main.h"


sqlite3* db;


int
sqlile_operation_callback(void *d, int argc, char **argv, char **azColName)
{
    (void) azColName;

    struct sqlile_callback_data* data = (struct sqlile_callback_data *)d;
    switch (data->operation) {
        case SQLITE_OP_FILL_BUFFER:
            if (argc == 1) {
                if (argv[0]) {
                    data->count++;
                    data->filler( data->buf, argv[0], NULL, 0 );
                }
            }
            break;
        case SQLITE_OP_RETURN_ONE_VALUE:
            if (argc == 1) {
                if (argv[0]) {
                    string_copy_to_buffer( data->result, argv[0] );
                }
            }
            break;
        default:
            break;
    }
    return 0;
}


unsigned long
sqlile_operation(char *sql, struct sqlile_callback_data *data)
{
    int rc;
    rc = sqlite3_exec( db, sql, sqlile_operation_callback, data, NULL );
    if (rc == SQLITE_OK) {
        return data->count;
    } else {
        return 0;
    }
}


char*
sqlite_get_column_by_subdir(int subdir)
{
    switch (subdir) {
        case AUTHORS_SUBDIR:
            return "author_prefix";
        case BOOKS_SUBDIR:
            return "name_prefix";
        default:
            return NULL;
    }
}


unsigned long
sqlite_get_prefixes(struct sqlile_callback_data* data, int subdir)
{
    char* column = sqlite_get_column_by_subdir( subdir );
    if (column) {
        char sql_buf[255];
        memset( &sql_buf, 0, 255 );
        string_format( (char *)SQL_PREFIXES, sql_buf, sizeof(sql_buf), column, column );
        data->operation = SQLITE_OP_FILL_BUFFER;
        return sqlile_operation( sql_buf, data );
    }
    return 0;
}


bool
sqlite_value_exists(char* column, char* value)
{
    struct sqlile_callback_data data;
    memset(&data, 0, sizeof(struct sqlile_callback_data));
    data.operation = SQLITE_OP_RETURN_ONE_VALUE;

    char sql_buf[512];
    memset( &sql_buf, 0, 512 );
    string_format( (char *)SQL_VALUE_EXISTS, sql_buf, sizeof(sql_buf), column, value );

    sqlile_operation( sql_buf, &data );
    return strcmp( data.result, "0" ) != 0;
}


bool
sqlite_prefix_exists(char *prefix, int subdir)
{
    char* column = sqlite_get_column_by_subdir( subdir );
    return sqlite_value_exists( column, prefix );
}


bool
sqlite_author_exists(char* author)
{
    char* column = "author";
    return sqlite_value_exists( column, author );
}


bool
sqlite_book_exists(char* book)
{
    char* column = "name";
    return sqlite_value_exists( column, book );
}


unsigned long
sqlite_get_authors(struct sqlile_callback_data* data, char* prefix)
{
    char sql_buf[MAX_BUFFER_LENGTH];
    memset( &sql_buf, 0, MAX_BUFFER_LENGTH );
    string_format( (char *)SQL_AUTHORS, sql_buf, sizeof(sql_buf), prefix );
    data->operation = SQLITE_OP_FILL_BUFFER;
    return sqlile_operation( sql_buf, data );
}


unsigned long
sqlite_get_books(struct sqlile_callback_data* data, int subdir, char* param)
{
    char* column = NULL;
    switch (subdir) {
        case AUTHORS_SUBDIR:
            column = "author";
            break;
        case BOOKS_SUBDIR:
            column = "name_prefix";
            break;
        default:
            return 0;
    }

    if (column) {
        char sql_buf[MAX_BUFFER_LENGTH];
        memset( &sql_buf, 0, MAX_BUFFER_LENGTH );
        string_format( (char *)SQL_BOOKS, sql_buf, sizeof(sql_buf), column, param );
        data->operation = SQLITE_OP_FILL_BUFFER;
        return sqlile_operation( sql_buf, data );
    }

    return 0;
}


int sqlite_open_database()
{
    int result;
    result = sqlite3_open_v2( "/home/twister/Dropbox/fuse_librusec/collection.sqlite3", &db, SQLITE_OPEN_READONLY, NULL );
    if (result) {
        fprintf( stderr, "Can't open database: %s\n", sqlite3_errmsg(db) );
    }
    return result;
}