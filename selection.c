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
#include <stddef.h>

#include "table.h"
#include "cellinfo.h"
#include "style.h"

#include "selection.h"

static int start_row = 0;
static int start_col = 0;

static int LT_row = 0;
static int LT_col = 0;
static int RB_row = 0;
static int RB_col = 0;

extern void selection_start( int row, int col )
{
    start_row = row;
    start_col = col;
    LT_row = row;
    LT_col = col;
    RB_row = row;
    RB_col = col;
}

extern void selection_end( int row, int col )
{
    // make sure the end cell is the later
    if ( row < start_row )
        LT_row = row;
    else
        RB_row = row;
    if ( col < start_col )
        LT_col = col;
    else
        RB_col = col;
}

extern void get_selection( int* start_row, int* start_col, int* end_row, int* end_col )
{
    *start_row = LT_row;
    *start_col = LT_col;
    *end_row = RB_row;
    *end_col = RB_col;
}

extern CellInfo* iterate_selection( MyTable table, int* row, int* col )
{
    static int r = 0;
    static int c = 0;
    if ( r == 0 && c == 0 ) {
        r = LT_row;
        c = LT_col-1;// for the first one
    }

    // roll the next
    ++c;
    if ( c > RB_col ) {
        ++r;
        c = LT_col;
    }
    if ( r > RB_row ) {
        // meet the table selection here
        r = 0;
        c = 0;
        return NULL;
    }

    CellInfo* info = get_cell_info( table, r, c );

    // always try to return a valid info unless meeting the selection end
    if ( info == NULL )
        return iterate_selection( table, row, col );
    *row = r;
    *col = c;
    return info;
}
