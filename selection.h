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

#ifndef SELETION_H
#define SELETION_H

#include "table.h"
#include "cellinfo.h"

extern void selection_start( int row, int col );
extern void selection_end( int row, int col );
extern void get_selection( int* start_row, int* start_col, int* end_row, int* end_col );
extern CellInfo* iterate_selection( MyTable table, int* row, int* col );

#endif // SELECTION_H
