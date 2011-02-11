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

#ifndef TABLE_H
#define TABLE_H

#include "cellinfo.h"

typedef struct Col
{
    int col;
    CellInfo* data;
    struct Col* next;
} Col;

typedef struct Row
{
    int row;
    Col* col_head;
    struct Row* next;
} Row;

typedef Row* MyTable;

extern MyTable create_table();
extern void free_table( MyTable t );

extern CellInfo* get_cell_info( MyTable t, int row, int col );
extern CellInfo* iterate_cell_info( MyTable t, int* row, int* col );
extern void insert_cell_info( MyTable t, int row, int col, CellInfo* info );
extern void delete_cell_info( MyTable t, int row, int col );
extern void modify_cell_info( MyTable t, int row, int col, void(*function)(CellInfo*) );
extern void traverse_cell_info( MyTable t, void(*function)(CellInfo*) );

extern void auto_adapt_layout( const MyTable t );

extern int cell_width_of( MyTable t, int row, int col );
extern int cell_height_of( MyTable t, int row, int col );

#endif // TABLE_H
