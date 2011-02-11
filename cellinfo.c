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

#include "style.h"

#include "cellinfo.h"

extern CellInfo* create_cell_info()
{
    CellInfo* info = malloc( sizeof(CellInfo) );
    info->text = NULL;
    clear_style( &(info->style) );
    info->combine_right = 0;
    info->combine_bottom = 0;
    return info;
}

extern CellInfo* clone_cell_info( const CellInfo* info )
{
    if ( info == NULL )
        return NULL;
    CellInfo* new_info = malloc( sizeof(CellInfo) );
    if ( info->text == NULL )
        new_info->text = NULL;
    else {
        new_info->text = malloc( ( strlen( info->text ) + 1 ) * sizeof(char) );
        strncpy( new_info->text, info->text, ( strlen( info->text ) + 1 ) );
    }
    new_info->style = info->style;
    new_info->combine_right = info->combine_right;
    new_info->combine_bottom = info->combine_bottom;
    return new_info;
}

extern void free_cell_info( CellInfo* info )
{
    free( info->text );
    free( info );
}
