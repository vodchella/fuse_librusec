//
// Created by twister on 10.03.16.
//

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "../main.h"


void
string_format(char *fmt, char *buf, size_t buf_size, ...)
{
    va_list vl;
    va_start( vl, buf_size );
    vsnprintf( buf, buf_size, fmt, vl );
    va_end( vl );
}


void
string_copy_to_buffer(char *dest, char *src)
{
    if (dest && src) {
        memset( dest, 0, MAX_BUFFER_LENGTH );
        size_t l = strlen( src );
        if (l > MAX_BUFFER_LENGTH) l = MAX_BUFFER_LENGTH - 1;
        strncpy( dest, src, l );
    }
}