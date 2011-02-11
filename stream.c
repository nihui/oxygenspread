/*
    This file is part of OxygenSpread project
    Copyright (C) 2009 Ni Hui <shuizhuyuanluo@126.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cellinfo.h"
#include "selection.h"
#include "table.h"

#include "stream.h"

///
/// linear cellinfo stream implementation
/// 'T' flag + cellinfo memory + TEXT(if exists)'\0' + row + col
/// 'T' flag indicates that the stream hasn't ended
/// '\0' END flag ends the whole stream
///
///    +-------------------------------------------------------------------+
///    | char** stream_current                                             |
///    +-----+---------------+--------------+----+----+--         --+------+
///    |     |CellInfo mem   |              |    |    |             |      |
///    | 'T' |               |              | row| col|     ...     | '\0' |
///    | flag|char* text-+----> TEXT ('\0') |    |    |             |  END |
///    +-----+-----------|---+--------------+----+----+--         --+------+
///                      +----> NULL ( empty text )
///

static char* stream_head = NULL;
static char* stream_current = NULL;

static int cell_info_storage_size( const CellInfo* info )
{
    int size = 1 + sizeof(CellInfo);// 'T' flag
    if ( info->text != NULL )
        size += (strlen(info->text)+1);// text length and '\0'
    else
        size += 1;// zero text length and '\0'
    return size+2*sizeof(int);// extra two int for relative row and col
}

static void append_cell( char** stream, const CellInfo* info, int row, int col )
{
    **stream = 'T';
    *stream += 1;
    // write cellinfo struct data
    memcpy( *stream, info, sizeof(CellInfo) );
    *stream += sizeof(CellInfo);
    int len = 0;
    // write cellinfo text data
    if ( info->text != NULL ) {
        len = strlen(info->text);
        memcpy( *stream, info->text, len+1 );
    }
    else {
        printf( "get preserved cell at %d %d\n", row, col );
        **stream = '\0';
    }
    *stream += (len+1);
    // write cellinfo row and col data
    *(int*)*stream = row;
    *stream += sizeof(int);
    *(int*)*stream = col;
    *stream += sizeof(int);
    printf( "append cell to stream at %d %d\n", row, col );
}

extern void selection2stream( MyTable table )
{
    // ensure that mem not leaks
    clear_stream();
    // go through to get the stream mem size
    int row, col;
    CellInfo* info = iterate_selection( table, &row, &col );
    int memsize = 0;
    while ( info != NULL ) {
        memsize += cell_info_storage_size( info );
        info = iterate_selection( table, &row, &col );
    }
    stream_head = malloc( (memsize+1) * sizeof(char) );// +1 for END flag

    // go through to copy selection cells to stream mem
    stream_current = stream_head;
    info = iterate_selection( table, &row, &col );
    while ( info != NULL ) {
        append_cell( &stream_current, info, row, col );
        info = iterate_selection( table, &row, &col );
    }
    *stream_current = '\0';// END flag
    stream_current = stream_head;// rewind
}

extern CellInfo* iterate_stream_cell( int* row, int* col )
{
    if ( *stream_current == '\0' ) {
        stream_current = stream_head;// rewind
        return NULL;
    }
    stream_current += 1;// skip 'T' flag
    // not the end, generate one
    CellInfo* info = create_cell_info();
    // cellinfo struct data
    memcpy( info, stream_current, sizeof(CellInfo) );
    stream_current += sizeof(CellInfo);
    int len = strlen( stream_current );
    // cellinfo text data
    if ( len > 0 ) {
        info->text = malloc( (len+1) * sizeof(char) );
        memcpy( info->text, stream_current, len+1 );
    }
    else {
        len = 0;
        info->text = NULL;
        printf( " iter cell preserved\n" );
    }
    stream_current += (len+1);
    // cellinfo row and col data
    *row = *(int*)stream_current;
    stream_current += sizeof(int);
    *col = *(int*)stream_current;
    stream_current += sizeof(int);
    return info;
}

extern void clear_stream()
{
    if ( stream_head != NULL ) {
        free( stream_head );
        stream_head = NULL;
    }
    stream_current = stream_head;
}
