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

#ifndef VIEW_H
#define VIEW_H

typedef enum view_edge_t
{
    VIEW_LEFT    = 0,
    VIEW_TOP     = 1,
    VIEW_RIGHT   = 2,
    VIEW_BOTTOM  = 3
} view_edge_t;

typedef struct ViewRect
{
    int edge[4];
    int row_head;
    int row_tail;
    int col_head;
    int col_tail;
} ViewRect;

extern void init_view();
extern void free_view();
extern void set_current_view_from_point( int x, int y );
extern ViewRect* current_view();
extern ViewRect* get_view_rect( int view_row, int view_col );
extern ViewRect* get_view_rect_from_point( int x, int y );
extern ViewRect* get_view_rect_LT();
extern ViewRect* get_view_rect_RB();
extern void get_row_and_col_from_point( const ViewRect* rect, int x, int y, int* row, int* col );
extern void get_absolute_cell_pos( const ViewRect* rect, int row, int col, int* x, int* y );
// extern void get_relative_rect_pos( int view_row, int view_col, int* left, int* top, int* right, int* bottom );
extern void update_row_tail_of( ViewRect* rect );
extern void update_col_tail_of( ViewRect* rect );
extern void update_row_tails();
extern void update_col_tails();
extern void update_view_rect( int view_row, int view_col, int left, int top, int right, int bottom );
extern void update_view_area( int left, int top, int right, int bottom );
extern int split_view( int x, int y );
extern void restore_view();

#endif // VIEW_H
