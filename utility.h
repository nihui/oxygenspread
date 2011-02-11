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

#ifndef UTILITY_H
#define UTILITY_H

#include <richedit.h>

#include "style.h"

void add_button( HWND hwndToolBar, int nImage, int nCommand );
void add_separator( HWND hwndToolBar );

void update_table_view();

void ShowOpenError(DWORD Code);
void ShowWriteError(DWORD Code);
BOOL prompt_save_changes(void);
void set_caption( const char* filename );
void get_editctrl_charformat( CHARFORMAT2* fmt );
void set_editctrl_charformat( CHARFORMAT2* fmt );
int compare_style_charformat( const Style* style, const CHARFORMAT2* fmt );
void charformat2style( Style* style, const CHARFORMAT2* fmt );
void style2charformat( CHARFORMAT2* fmt, const Style* style );
void style2logfont( LOGFONT* logfont, const Style* style, HDC hdc );
void draw_vertical_line( HDC hdc, int x, int top, int bottom );
void draw_horizontal_line( HDC hdc, int y, int left, int right );
void draw_rect_border( HDC hdc, int left, int top, int right, int bottom );
void update_selection_rect();
void draw_selection_rect_border( HDC hdc );

#endif // UTILITY_H
