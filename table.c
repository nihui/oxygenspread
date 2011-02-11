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
#include "layout.h"

#include "table.h"

static Row* find_prev_row( Row* row_head, register int row )
{
    register Row* r = row_head;
    while ( r->next != NULL && r->next->row < row )
        r = r->next;
    return r;
}

static Col* find_prev_col( Col* col_head, register int col )
{
    register Col* c = col_head;
    while ( c->next != NULL && c->next->col < col )
        c = c->next;
    return c;
}

extern Row* create_table()
{
    // create a 1x1 table
    Col* col_head = malloc( sizeof(Col) );
    col_head->col = 1;
    col_head->data = NULL;
    col_head->next = NULL;
    Row* row_head = malloc( sizeof(Row) );
    row_head->row = 1;
    row_head->col_head = col_head;
    row_head->next = NULL;
    // initialize row heights and col widths
    init_layout();
    return row_head;
}

extern void free_table( Row* row_head )
{
    // free table resource
    register Row* r = row_head;
    register Row* r_next;
    register Col* c;
    register Col* c_next;
    while ( r != NULL ) {
        c = r->col_head;
        while ( c != NULL ) {
            if ( c->data != NULL ) {
                free_cell_info( c->data );
                c->data = NULL;
            }
            c_next = c->next;
            free( c );
            c = c_next;
        }
        r_next = r->next;
        free( r );
        r = r_next;
    }
    r = NULL;
    // row heights and col widths
    free_layout();
}

extern CellInfo* get_cell_info( Row* row_head, int row, int col )
{
    // get info at row col, NULL if not exists
    // find the row first
    Row* r = ( row == 1 )? row_head: find_prev_row( row_head, row )->next;
    if ( r == NULL )
        return NULL;
    if ( r->row != row )
        return NULL;

    // well, we get the row ;)
    // find the col second then
    Col* c = ( col == 1 )? r->col_head: find_prev_col( r->col_head, col )->next;
    if ( c == NULL )
        return NULL;
    if ( c->col != col )
        return NULL;

    return c->data;
}

extern CellInfo* iterate_cell_info( Row* row_head, int* row, int* col )
{
    static Row* r = NULL;
    static Col* c = NULL;
    if ( r == NULL && c == NULL ) {
        r = row_head;
        c = row_head->col_head;
    }
    CellInfo* info = c->data;
    *row = r->row;
    *col = c->col;

    // roll the next
    // preserve the first one
    if ( r != NULL || c != NULL )
        c = c->next;
    if ( c == NULL ) {
        r = r->next;
        if ( r == NULL ) {
            // meet the table end here
            c = NULL;
            return NULL;
        }
        c = r->col_head;
    }

    // always try to return a valid info unless meeting the table end
    if ( info == NULL )
        return iterate_cell_info( row_head, row, col );
    return info;
}

extern void insert_cell_info( Row* row_head, int row, int col, CellInfo* info )
{
    fprintf( stderr, " insert at (%d %d)\n", row, col );
    // insert info at row col, overwrite the original info if exists
    // find the row first
    Row* r = find_prev_row( row_head, row );
    Row* r_next = ( row == 1 )? row_head: r->next;

    if ( r_next == NULL ) {
        // over the max current row size
        Col* new_col_head = malloc( sizeof(Col) );
        new_col_head->col = 1;
        if ( col == 1 ) {
            new_col_head->data = info;
            new_col_head->next = NULL;
        }
        else {
            Col* new_col = malloc( sizeof(Col) );
            new_col->col = col;
            new_col->data = info;
            new_col->next = NULL;
            new_col_head->data = NULL;
            new_col_head->next = new_col;
        }
        Row* new_row = malloc( sizeof(Row) );
        new_row->row = row;
        new_row->col_head = new_col_head;
        new_row->next = NULL;
        r->next = new_row;
    }
    else if ( r_next->row == row ) {
        // well, we get the row ;)
        // find the col second then
        Col* c = find_prev_col( r_next->col_head, col );
        Col* c_next = ( col == 1 )? r_next->col_head: c->next;

        if ( c_next == NULL ) {
            // over the max current col size of row
            if ( col == 1 )
                c->data = info;
            else {
                Col* new_col = malloc( sizeof(Col) );
                new_col->col = col;
                new_col->data = info;
                new_col->next = NULL;
                c->next = new_col;
            }
        }
        else if ( c_next->col == col ) {
            if ( c_next->data != NULL ) {
                // free the original info
                free_cell_info( c_next->data );
            }
            c_next->data = info;
        }
        else /* c_next->col > col */ {
            // insert a new col after it
            Col* new_col = malloc( sizeof(Col) );
            new_col->col = col;
            new_col->data = info;
            new_col->next = c->next;
            c->next = new_col;
        }
    }
    else /* r_next->row > row */ {
        // hmm, insert a new row after it
        Col* new_col_head = malloc( sizeof(Col) );
        new_col_head->col = 1;
        if ( col == 1 ) {
            new_col_head->data = info;
            new_col_head->next = NULL;
        }
        else {
            Col* new_col = malloc( sizeof(Col) );
            new_col->col = col;
            new_col->data = info;
            new_col->next = NULL;
            new_col_head->data = NULL;
            new_col_head->next = new_col;
        }
        Row* new_row = malloc( sizeof(Row) );
        new_row->row = row;
        new_row->col_head = new_col_head;
        new_row->next = r->next;
        r->next = new_row;
    }
}

extern void delete_cell_info( Row* row_head, int row, int col )
{
    fprintf( stderr, " delete at (%d %d)\n", row, col );
    // delete info at row col(preserve the struct), do nothing if not exists
    CellInfo* info = get_cell_info( row_head, row, col );
    if ( info != NULL && info->text != NULL ) {
        // free the info text
        free( info->text );
        info->text = NULL;
    }
}

extern void modify_cell_info( Row* row_head, int row, int col, void(*function)(CellInfo*) )
{
    fprintf( stderr, " modify at (%d %d)\n", row, col );
    // traverse info at row col, do nothing if not exists
    CellInfo* info = get_cell_info( row_head, row, col );
    if ( info != NULL ) {
        // do the function
        function( info );
    }
}

extern void traverse_cell_info( Row* row_head, void(*function)(CellInfo*) )
{
    register Row* r = row_head;
    register Col* c;
    while ( r != NULL ) {
        c = r->col_head;
        while ( c != NULL ) {
            if ( c->data != NULL )
                function( c->data );
            c = c->next;
        }
        r = r->next;
    }
}

static int get_line_count( const char* str )
{
    int count = 1;
    register const char* c = str;
    while ( *c != '\0' ) {
        if ( *c == '\n' )
            ++count;
        ++c;
    }
    return count;
}

extern void auto_adapt_layout( const MyTable t )
{
    // auto adapt height/width
    register Row* r = t;
    register Col* c;
    while ( r != NULL ) {
        c = r->col_head;
        while ( c != NULL ) {
            if ( c->data != NULL ) {
                if ( c->data->text != NULL ) {
                    int len = strlen( c->data->text );
                    int width = len * 8;
                    int linecount = get_line_count( c->data->text );
                    int height = linecount * c->data->style.text_size * 2;
                    // cell at ( r->row, c->col ) is width x height
                    if ( col_width_of( c->col ) < width )
                        set_col_width( c->col, width );
                    if ( row_height_of( r->row ) < height )
                        set_row_height( r->row, height );
                }
            }
            c = c->next;
        }
        r = r->next;
    }
}


extern int cell_width_of( MyTable t, int row, int col )
{
    CellInfo* info = get_cell_info( t, row, col );
    int width = col_width_of( col );
    if ( info != NULL ) {
        int i;
        for ( i=1; i<=info->combine_right; ++i ) {
            width += col_width_of( col+i );
        }
    }
    return width;
}

extern int cell_height_of( MyTable t, int row, int col )
{
    CellInfo* info = get_cell_info( t, row, col );
    int height = row_height_of( row );
    if ( info != NULL ) {
        int i;
        for ( i=1; i<=info->combine_bottom; ++i ) {
            height += row_height_of( row+i );
        }
    }
    return height;
}

