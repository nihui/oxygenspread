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

#include <stdlib.h>
#include <string.h>

#include "layout.h"

#define DEFAULT_ROW_HEIGHT 25
#define DEFAULT_COL_WIDTH 100

#define RESIZED_ROW_HEIGHTS_BUFS 8
#define RESIZED_COL_WIDTHS_BUFS 8

typedef struct HW
{
    int hw_id;
    int size;
} HW;

static HW* resized_row_heights;
static HW* resized_col_widths;
static int resized_rows_buf_count;
static int resized_cols_buf_count;
static int resized_row_count;
static int resized_col_count;

extern void init_layout()
{
    resized_rows_buf_count = 1;
    resized_cols_buf_count = 1;
    resized_row_heights = malloc( resized_rows_buf_count * RESIZED_ROW_HEIGHTS_BUFS * sizeof(HW) );
    resized_col_widths = malloc( resized_cols_buf_count * RESIZED_COL_WIDTHS_BUFS * sizeof(HW) );
    resized_row_count = 0;
    resized_col_count = 0;
}

extern void free_layout()
{
    free( resized_row_heights );
    resized_row_heights = NULL;
    free( resized_col_widths );
    resized_col_widths = NULL;
    resized_rows_buf_count = 1;
    resized_cols_buf_count = 1;
}

extern int row_height_of( int row )
{
    int i = 0;
    for ( ; i < resized_row_count; ++i ) {
        if ( resized_row_heights[i].hw_id == row )
            return resized_row_heights[i].size;
    }
    return DEFAULT_ROW_HEIGHT;
}

extern int col_width_of( int col )
{
    int i = 0;
    for ( ; i < resized_col_count; ++i ) {
        if ( resized_col_widths[i].hw_id == col )
            return resized_col_widths[i].size;
    }
    return DEFAULT_COL_WIDTH;
}

extern void set_row_height( int row, int height )
{
    int i = 0;
    for ( ; i < resized_row_count; ++i ) {
        if ( resized_row_heights[i].hw_id == row ) {
            resized_row_heights[i].size = height;
            return;
        }
    }
    ++resized_row_count;
    if ( resized_row_count >= resized_rows_buf_count * RESIZED_ROW_HEIGHTS_BUFS ) {
        // resize buf size
        ++resized_rows_buf_count;
        HW* old = resized_row_heights;
        resized_row_heights = malloc( resized_rows_buf_count * RESIZED_ROW_HEIGHTS_BUFS * sizeof(HW) );
        memcpy( resized_row_heights, old, RESIZED_ROW_HEIGHTS_BUFS * ( resized_rows_buf_count - 1 ) );
        free( old );
    }
    resized_row_heights[resized_row_count-1].hw_id = row;
    resized_row_heights[resized_row_count-1].size = height;
}

extern void set_col_width( int col, int width )
{
    int i = 0;
    for ( ; i < resized_col_count; ++i ) {
        if ( resized_col_widths[i].hw_id == col ) {
            resized_col_widths[i].size = width;
            return;
        }
    }
    ++resized_col_count;
    if ( resized_col_count >= resized_cols_buf_count * RESIZED_COL_WIDTHS_BUFS ) {
        // resize buf size
        ++resized_cols_buf_count;
        HW* old = resized_col_widths;
        resized_col_widths = malloc( resized_cols_buf_count * RESIZED_COL_WIDTHS_BUFS * sizeof(HW) );
        memcpy( resized_col_widths, old, RESIZED_COL_WIDTHS_BUFS * ( resized_cols_buf_count - 1 ) );
        free( old );
    }
    resized_col_widths[resized_col_count-1].hw_id = col;
    resized_col_widths[resized_col_count-1].size = width;
}
