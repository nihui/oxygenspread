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

#ifndef STYLE_H
#define STYLE_H

typedef enum
{
    TEXT_NONE       = 0x00000000,
    TEXT_BOLD       = 0x00000001,
    TEXT_ITALIC     = 0x00000010,
    TEXT_UNDERLINE  = 0x00000100,
    TEXT_STRIKEOUT  = 0x00001000
} text_style_t;

typedef enum
{
    ALIGN_LEFT      = 0,
    ALIGN_RIGHT     = 1,
    ALIGN_CENTER    = 2
} text_align_t;

typedef struct Style
{
    unsigned int text_rgb;
    // TODO: Font Type;
    char text_font[40];
    unsigned int text_size;
    text_style_t text_type;
    text_align_t text_align;
    unsigned int bg_rgb;
} Style;

extern void clear_style( Style* style );
extern int get_text_style_status( const Style* style, text_style_t type );
extern void apply_text_style( Style* style, text_style_t type );
extern void remove_text_style( Style* style, text_style_t type );

#endif // STYLE_H
