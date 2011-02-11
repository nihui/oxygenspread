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

#include "style.h"

extern void clear_style( Style* style )
{
    style->text_rgb = 0x00000000;
    memset( style->text_font, '\0', 40 * sizeof(char) );
    char default_font[] = "Arial";
    strcpy( style->text_font, default_font );
    style->text_size = 12;
    style->text_type = TEXT_NONE;
    style->text_align = ALIGN_LEFT;
    style->bg_rgb = 0x00ffffff;
}

extern int get_text_style_status( const Style* style, text_style_t type )
{
    return style->text_type == ( style->text_type | type );
}

extern void apply_text_style( Style* style, text_style_t type )
{
    style->text_type |= type;
}

extern void remove_text_style( Style* style, text_style_t type )
{
    style->text_type &= ~type;
}

extern void set_text_align( Style* style, text_align_t align )
{
    style->text_align = align;
}
