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

#include "file_kiss.h"

extern int import_kiss( FILE* fp, MyTable t )
{
#define TEXT_BUF_LEN 10
    char c;
    int line_sep = 0;
    // temp buf
    CellInfo* info_buf = create_cell_info();
    int row = 0;
    int col = 0;

    int buf_len_count = 1;
    char* buf = malloc( TEXT_BUF_LEN * buf_len_count * sizeof( char ) );
    memset( buf, '\0', TEXT_BUF_LEN * buf_len_count * sizeof( char ) );
    int text_index = 0;

    // if the unclosed backslash is just before the current backslash
    int backslash_just_before = 0;

    while (( c = fgetc( fp ) ) != EOF ) {
        switch ( c ) {
            case ';':
                if ( line_sep < 2 ) {
                    if ( buf[0] != '\0' ) {
                        int index = 0;
                        switch ( line_sep ) {
                            case 0: {
                                /// row/col/r_s/c_s
                                enum { ROW=0, COL=1, COMBINE_RIGHT=2, COMBINE_BOTTOM=3 };
                                char* pch;
                                pch = strtok( buf, "," );
                                while ( pch != NULL ) {
                                    switch ( index ) {
                                        case ROW:
                                            row = atoi( pch );
                                            break;
                                        case COL:
                                            col = atoi( pch );
                                            break;
                                        case COMBINE_RIGHT:
                                            info_buf->combine_right = atoi( pch );
                                            break;
                                        case COMBINE_BOTTOM:
                                            info_buf->combine_bottom = atoi( pch );
                                            break;
                                        default:
                                            // throw away the rest
                                            break;
                                    }
                                    pch = strtok( NULL, "," );
                                    ++index;
                                }
                                break;
                            }
                            case 1: {
                                /// style
                                enum { TEXT_RGB=0, TEXT_FONT=1, TEXT_SIZE=2, TEXT_TYPE=3, TEXT_ALIGN=4, BG_RGB=5 };
                                char* pch;
                                pch = strtok( buf, "," );
                                while ( pch != NULL ) {
                                    switch ( index ) {
                                        case TEXT_RGB:
                                            sscanf( pch, "%x", &(info_buf->style.text_rgb) );
                                            break;
                                        case TEXT_FONT:
                                            strncpy( info_buf->style.text_font, pch, 40 );
                                            break;
                                        case TEXT_SIZE:
                                            info_buf->style.text_size = atoi( pch );
                                            break;
                                        case TEXT_TYPE:
                                            info_buf->style.text_type = atoi( pch );
                                            break;
                                        case TEXT_ALIGN:
                                            info_buf->style.text_align = atoi( pch );
                                            break;
                                        case BG_RGB:
                                            sscanf( pch, "%x", &(info_buf->style.bg_rgb) );
                                            break;
                                        default:
                                            // throw away the rest
                                            break;
                                    }
                                    pch = strtok( NULL, "," );
                                    ++index;
                                }
                                break;
                            }
                            default:
                                // should not go here
                                break;
                        }
                    }
                    ++line_sep;
                    memset( buf, '\0', TEXT_BUF_LEN * buf_len_count * sizeof( char ) );
                    text_index = 0;
                }
                else {
                    buf[text_index] = c;
                    ++text_index;
                }
                backslash_just_before = 0;
                break;
            case '\n':
                if ( backslash_just_before == 0 ) {
                    info_buf->text = malloc(( strlen( buf ) + 1 ) * sizeof( char ) );
                    memcpy( info_buf->text, buf, strlen( buf ) + 1 );
                    insert_cell_info( t, row, col, info_buf );
                    // we have referred to the info_buf, so create a new one
                    info_buf = create_cell_info();
                    memset( buf, '\0', TEXT_BUF_LEN * buf_len_count * sizeof( char ) );
                    text_index = 0;
                    line_sep = 0;
                }
                else {
                    // store '\n'
                    buf[text_index] = c;
                    ++text_index;
                    backslash_just_before = 0;
                }
                break;
            case '\\':
                if ( backslash_just_before == 1 ) {
                    // we meet the backslash twice after an unclosed backslash
                    // store '\\'
                    buf[text_index] = c;
                    ++text_index;
                    backslash_just_before = 0;
                }
                else {
                    // may also be a backslash_just_before char
                    backslash_just_before = 1;
                }
                break;
            default:
                buf[text_index] = c;
                ++text_index;
                backslash_just_before = 0;
                break;
        }
        // resize text buf to contain more char
        if ( text_index == TEXT_BUF_LEN * buf_len_count - 1 ) {
            char* buf_old = buf;
            ++buf_len_count;
            buf = malloc( TEXT_BUF_LEN * buf_len_count * sizeof( char ) );
            memset( buf, '\0', TEXT_BUF_LEN * buf_len_count * sizeof( char ) );
            memcpy( buf, buf_old, TEXT_BUF_LEN * ( buf_len_count - 1 ) );
            free( buf_old );
        }
    }
    free( buf );
//     free( info_buf->text );// info_buf->text is initialized as NULL
    free( info_buf );
#undef TEXT_BUF_LEN
    rewind( fp );
    return 0;
}

extern int export_kiss( FILE* fp, const MyTable t )
{
    unsigned int prev_row = 1;
    unsigned int prev_col = 1;
    register Row* r = t;
    register Col* c;
    while ( r != NULL ) {
        c = r->col_head;
        while ( c != NULL ) {
            if ( c->data != NULL ) {
                printf( "%d:%d  %s\n", r->row, c->col, c->data->text );
                fprintf( fp, "%d,%d", r->row, c->col );
                if ( c->data->combine_right != 0 || c->data->combine_bottom != 0 )
                    fprintf( fp, ",%d,%d", c->data->combine_right, c->data->combine_bottom );
                fprintf( fp, ";" );
                // NOTE: style out
                fprintf( fp, "%x", c->data->style.text_rgb );
                fprintf( fp, ",%s", c->data->style.text_font );
                fprintf( fp, ",%u", c->data->style.text_size );
                fprintf( fp, ",%u", c->data->style.text_type );
                fprintf( fp, ",%u", c->data->style.text_align );
                fprintf( fp, ",%x", c->data->style.bg_rgb );
                fprintf( fp, ";" );
                if ( c->data->text != NULL ) {
                    char* t = c->data->text;
                    while ( *t != '\0' ) {
                        if ( *t == '\\' || *t == '\n' ) {
                            // additional backslash for backslash or linefeed in text
                            // NOTE: should we treat '\r' in windows?
                            fprintf( fp, "\\" );
                        }
                        fprintf( fp, "%c", *t );
                        ++t;
                    }
                }
                fprintf( fp, "\n" );
            }
            c = c->next;
        }
        r = r->next;
    }
}
/*
int main( int argc, char** argv )
{
    FILE* fp = fopen( "test.kiss", "r" );
    if ( fp == NULL )
        return -1;
    // parse
    MyTable t = create_table();
    import_kiss( fp, t );
    
    fclose( fp );
    
    // out
    FILE* out = fopen( "testout.kiss", "w" );
    if ( out != NULL ) {
        export_kiss( out, t );
        fclose( out );
    }
    
    // free
    free_table( t );
    
    return 0;
}
*/
