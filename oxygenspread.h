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

#ifndef OXYGENSPREAD_H
#define OXYGENSPREAD_H

const char g_szClassName[] = "OxygenSpread";
HWND hwnd;
HWND hEdit;

// for table view
MyTable table;

// preserved for zero things
const int col_zero_width = 50;
const int row_zero_height = 25;

#endif // OXYGENSPREAD_H
