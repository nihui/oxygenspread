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

#include "table.h"
#include "view.h"

extern double ratio;
extern int row_zero_height;
extern int col_zero_width;

typedef struct View
{
    short splitted_rows;
    short splitted_cols;
    ViewRect* rects[2][2];
} View;

static View* main_view = NULL;
static ViewRect* current_view_rect = NULL;

extern void init_view()
{
    if ( main_view == NULL ) {
        main_view = malloc( sizeof(View) );
        memset( main_view, 0, sizeof(View) );
        main_view->splitted_rows = 1;
        main_view->splitted_cols = 1;
        main_view->rects[0][0] = malloc( sizeof(ViewRect) );
        memset( main_view->rects[0][0], 0, sizeof(ViewRect) );
        main_view->rects[0][0]->row_head = 1;
        main_view->rects[0][0]->col_head = 1;
    }
    current_view_rect = main_view->rects[0][0];
}

extern void free_view()
{
    if ( main_view != NULL ) {
        int i = 0;
        for ( ; i<2; ++i ) {
            int j = 0;
            for ( ; j<2; ++j ) {
                if ( main_view->rects[i][j] != NULL )
                    free( main_view->rects[i][j] );
            }
        }
        free( main_view );
        main_view = NULL;
        current_view_rect = NULL;
    }
}

extern void set_current_view_from_point( int x, int y )
{
    current_view_rect = get_view_rect_from_point( x, y );
}

extern ViewRect* current_view()
{
    return current_view_rect;
}

extern ViewRect* get_view_rect( int view_row, int view_col )
{
    if ( view_row < 0
        || view_col < 0
        || view_row > main_view->splitted_rows
        || view_col > main_view->splitted_cols )
        return NULL;
    return main_view->rects[ view_row ][ view_col ];
}

extern ViewRect* get_view_rect_from_point( int x, int y )
{
    int i = 0;
    int j = 0;
    // get the view row i
    while ( y > main_view->rects[i][0]->edge[ VIEW_BOTTOM ] )
        ++i;
    // get the view col j
    while ( x > main_view->rects[0][j]->edge[ VIEW_RIGHT ] )
        ++j;
    return main_view->rects[i][j];
}

extern ViewRect* get_view_rect_LT()
{
    return main_view->rects[0][0];
}

extern ViewRect* get_view_rect_RB()
{
    return main_view->rects[ main_view->splitted_rows-1 ][ main_view->splitted_cols-1 ];
}

static int row_at( const ViewRect* rect, register int y )
{
    register int i = rect->row_head;
    register int drawed = rect->edge[ VIEW_TOP ] + row_zero_height;
    while ( drawed < y ) {
        drawed += row_height_of(i)*ratio;
        ++i;
        if ( i == rect->row_tail )
            return i;
    }
    return i-1;
}

static int col_at( const ViewRect* rect, register int x )
{
    register int i = rect->col_head;
    register int drawed = rect->edge[ VIEW_LEFT ] + col_zero_width;
    while ( drawed < x ) {
        drawed += col_width_of(i)*ratio;
        ++i;
        if ( i == rect->col_tail )
            return i;
    }
    return i-1;
}


extern void get_row_and_col_from_point( const ViewRect* rect, int x, int y, int* row, int* col )
{
    *row = row_at( rect, y );
    *col = col_at( rect, x );
}

extern void get_absolute_cell_pos( const ViewRect* rect, int row, int col, int* x, int* y )
{
    int x_pos = rect->edge[ VIEW_LEFT ] + col_zero_width;
    int y_pos = rect->edge[ VIEW_TOP ] + row_zero_height;
    int i;
    if ( col < rect->col_head || col >= rect->col_tail )
        x_pos = -1;
    else {
        for ( i=rect->col_head; i<col; ++i )
            x_pos += col_width_of( i )*ratio;
    }
    if ( row < rect->row_head || row >= rect->row_tail )
        y_pos = -1;
    else {
        for ( i=rect->row_head; i<row; ++i )
            y_pos += row_height_of( i )*ratio;
    }
    *x = x_pos;
    *y = y_pos;
}

extern void update_row_tail_of( ViewRect* rect )
{
    register int i = rect->row_head;
    register int drawed = row_zero_height;
    int draw_limit = rect->edge[VIEW_BOTTOM] - rect->edge[VIEW_TOP];
    while ( drawed < draw_limit )
        drawed += row_height_of(i++)*ratio;
    rect->row_tail = i;
}

extern void update_col_tail_of( ViewRect* rect )
{
    register int i = rect->col_head;
    register int drawed = col_zero_width;
    int draw_limit = rect->edge[VIEW_RIGHT] - rect->edge[VIEW_LEFT];
    while ( drawed < draw_limit )
        drawed += col_width_of(i++)*ratio;
    rect->col_tail = i;
}

extern void update_row_tails()
{
    int i = 0;
    for ( ; i<main_view->splitted_rows; ++i ) {
        int j = 0;
        for ( ; j<main_view->splitted_cols; ++j ) {
            update_row_tail_of( main_view->rects[i][j] );
        }
    }
}

extern void update_col_tails()
{
    int i = 0;
    for ( ; i<main_view->splitted_rows; ++i ) {
        int j = 0;
        for ( ; j<main_view->splitted_cols; ++j ) {
            update_col_tail_of( main_view->rects[i][j] );
        }
    }
}

extern void update_view_rect( int view_row, int view_col, int left, int top, int right, int bottom )
{
    if ( view_row < 0
        || view_col < 0
        || view_row > main_view->splitted_rows
        || view_col > main_view->splitted_cols )
        return;
    ViewRect* rect = main_view->rects[ view_row ][ view_col ];
    rect->edge[ VIEW_LEFT   ] = left;
    rect->edge[ VIEW_TOP    ] = top;
    rect->edge[ VIEW_RIGHT  ] = right;
    rect->edge[ VIEW_BOTTOM ] = bottom;
//     rect->row_head = row_at( top );
//     rect->col_head = col_at( left );
    update_row_tail_of( rect );
    update_col_tail_of( rect );
}

extern void update_view_area( int left, int top, int right, int bottom )
{
    // resize the right most view cols
    int i = 0;
    for ( ; i<main_view->splitted_rows; ++i ) {
        ViewRect* rect = main_view->rects[i][ main_view->splitted_cols-1 ];
        rect->edge[ VIEW_RIGHT ] = right;
        update_col_tail_of( rect );
    }

    // resize the bottom most view rows
    int j = 0;
    for ( ; j<main_view->splitted_cols; ++j ) {
        ViewRect* rect = main_view->rects[ main_view->splitted_rows-1 ][j];
        rect->edge[ VIEW_BOTTOM ] = bottom;
        update_row_tail_of( rect );
    }
}

static const int get_view_rect_left( register int x )
{
    register int i = 0;
    while ( main_view->rects[0][i]->edge[ VIEW_RIGHT ] < x )
        ++i;
    return main_view->rects[0][i]->edge[ VIEW_LEFT ];
}

static const int get_view_rect_right( register int x )
{
    register int i = 0;
    while ( main_view->rects[0][i]->edge[ VIEW_RIGHT ] < x )
        ++i;
    return main_view->rects[0][i]->edge[ VIEW_RIGHT ];
}

static const int get_view_rect_top( register int y )
{
    register int i = 0;
    while ( main_view->rects[i][0]->edge[ VIEW_BOTTOM ] < y )
        ++i;
    return main_view->rects[i][0]->edge[ VIEW_TOP ];
}

static const int get_view_rect_bottom( register int y )
{
    register int i = 0;
    while ( main_view->rects[i][0]->edge[ VIEW_BOTTOM ] < y )
        ++i;
    return main_view->rects[i][0]->edge[ VIEW_BOTTOM ];
}

static int split_view_vertically( int x )
{
    if ( main_view->rects[0][1] != NULL )
        return -1;

    // get left edge and right edge for newly splitted rects
    int splitted_rect_left = x;
    int splitted_rect_right = get_view_rect_right( x );
    int splitted_rect_top;
    int splitted_rect_bottom;

    // set up the new view cols
    int i = 0;
    for ( ; i<main_view->splitted_rows; ++i ) {
        splitted_rect_top = main_view->rects[i][0]->edge[ VIEW_TOP ];
        splitted_rect_bottom = main_view->rects[i][0]->edge[ VIEW_BOTTOM ];
        ViewRect* rect = malloc( sizeof(ViewRect) );
        rect->edge[ VIEW_LEFT   ] = splitted_rect_left;
        rect->edge[ VIEW_TOP    ] = splitted_rect_top;
        rect->edge[ VIEW_RIGHT  ] = splitted_rect_right;
        rect->edge[ VIEW_BOTTOM ] = splitted_rect_bottom;
        rect->row_head = main_view->rects[i][0]->row_head;
        rect->col_head = col_at( main_view->rects[0][0], splitted_rect_left );// TODO: replace [0][0] with the base rect
        update_row_tail_of( rect );
        update_col_tail_of( rect );
        main_view->rects[i][1] = rect;
    }

    // shrink the orig views
    i = 0;
    for ( ; i<main_view->splitted_rows; ++i ) {
        ViewRect* rect = main_view->rects[i][0];
        rect->edge[ VIEW_RIGHT  ] = splitted_rect_left;
        update_row_tail_of( rect );
        update_col_tail_of( rect );
    }

    ++main_view->splitted_cols;
}

static int split_view_horizontally( int y )
{
    if ( main_view->rects[1][0] != NULL )
        return -1;
    
    // get top edge and bottom edge for newly splitted rects
    int splitted_rect_left;
    int splitted_rect_right;
    int splitted_rect_top = y;
    int splitted_rect_bottom = get_view_rect_bottom( y );

    // set up the new view rows
    int j = 0;
    for ( ; j<main_view->splitted_cols; ++j ) {
        splitted_rect_left = main_view->rects[0][j]->edge[ VIEW_LEFT ];
        splitted_rect_right = main_view->rects[0][j]->edge[ VIEW_RIGHT ];
        ViewRect* rect = malloc( sizeof(ViewRect) );
        rect->edge[ VIEW_LEFT   ] = splitted_rect_left;
        rect->edge[ VIEW_TOP    ] = splitted_rect_top;
        rect->edge[ VIEW_RIGHT  ] = splitted_rect_right;
        rect->edge[ VIEW_BOTTOM ] = splitted_rect_bottom;
        rect->row_head = row_at( main_view->rects[0][0], splitted_rect_top );// TODO: replace [0][0] with the base rect
        rect->col_head = main_view->rects[0][j]->col_head;
        update_row_tail_of( rect );
        update_col_tail_of( rect );
        main_view->rects[1][j] = rect;
    }

    // shrink the orig views
    j = 0;
    for ( ; j<main_view->splitted_cols; ++j ) {
        ViewRect* rect = main_view->rects[0][j];
        rect->edge[ VIEW_BOTTOM  ] = splitted_rect_top;
        update_row_tail_of( rect );
        update_col_tail_of( rect );
    }

    ++main_view->splitted_rows;
}

extern int split_view( int x, int y )
{
    // range check
    if ( x > 0 && x < get_view_rect_RB()->edge[ VIEW_RIGHT ] )
        split_view_vertically( x );
    if ( y > 0 && y < get_view_rect_RB()->edge[ VIEW_BOTTOM ] )
        split_view_horizontally( y );
}

extern void restore_view()
{
    // get the rect at left top corner
    ViewRect* ltrect = main_view->rects[0][0];
    int left = ltrect->edge[VIEW_LEFT];
    int top = ltrect->edge[VIEW_TOP];
    // get the rect at right bottom corner
    ViewRect* rbrect = main_view->rects[ main_view->splitted_rows-1 ][ main_view->splitted_cols-1 ];
    int right = rbrect->edge[VIEW_RIGHT];
    int bottom = rbrect->edge[VIEW_BOTTOM];

    // free the splitted views
    if ( main_view->rects[0][1] ) {
        free( main_view->rects[0][1] );
        main_view->rects[0][1] = NULL;
    }
    if ( main_view->rects[1][0] ) {
        free( main_view->rects[1][0] );
        main_view->rects[1][0] = NULL;
    }
    if ( main_view->rects[1][1] ) {
        free( main_view->rects[1][1] );
        main_view->rects[1][1] = NULL;
    }

    main_view->splitted_rows = 1;
    main_view->splitted_cols = 1;

    // resize the left top view to full window client size
    update_view_rect( 0, 0, left, top, right, bottom );
    current_view_rect = main_view->rects[0][0];
}
/*
void p()
{
    printf( "PRINT\n" );
    int i = 0;
    for ( ; i<2; ++i ) {
        int j = 0;
        for ( ; j<2; ++j ) {
            if ( main_view->rects[i][j] != NULL ) {
                int l = main_view->rects[i][j]->edge[0];
                int t = main_view->rects[i][j]->edge[1];
                int r = main_view->rects[i][j]->edge[2];
                int b = main_view->rects[i][j]->edge[3];
                printf( "[ %d %d ] %d %d %d %d\n", i, j, l, t, r, b );
                int row_head = main_view->rects[i][j]->row_head;
                int row_tail = main_view->rects[i][j]->row_tail;
                int col_head = main_view->rects[i][j]->col_head;
                int col_tail = main_view->rects[i][j]->col_tail;
                printf( "     [ %d %d %d %d ]\n", row_head, row_tail, col_head, col_tail );
            }
        }
    }
}
*/
/*
int main( int argc, char** argv )
{
    init_view();
    p();
    update_view_rect( 0, 0, 0, 0, 200, 150 );// [0,0]
    p();
    split_view( 40, 60 );
    p();
    restore_view();
    p();
    free_view();
    return 0;
}
*/
