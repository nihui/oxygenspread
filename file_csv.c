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

#include "file_csv.h"

extern int import_csv( FILE* fp, MyTable t )
{
#define TEXT_BUF_LEN 10
    char c;
    int row = 1;
    int col = 1;

    int buf_len_count = 1;
    char* buf = malloc( TEXT_BUF_LEN * buf_len_count * sizeof( char ) );
    memset( buf, '\0', TEXT_BUF_LEN * buf_len_count * sizeof( char ) );
    int text_index = 0;

    // if an unclosed double_quote has been met before
    int double_quote_before = 0;
    // if the unclosed double_quote is just before the current double_quote
    int double_quote_just_before = 0;

    while (( c = fgetc( fp ) ) != EOF ) {
        switch ( c ) {
            case ',':
            case '\n':
                if ( double_quote_before == 0 ) {
                    if ( buf[0] != '\0' ) {
                        CellInfo* info = create_cell_info();
                        info->text = malloc(( strlen( buf ) + 1 ) * sizeof( char ) );
                        memcpy( info->text, buf, strlen( buf ) + 1 );
                        insert_cell_info( t, row, col, info );
                    }
                    memset( buf, '\0', TEXT_BUF_LEN * buf_len_count * sizeof( char ) );
                    text_index = 0;
                    if ( c == ',' )
                        ++col;
                    else if ( c == '\n' ) {
                        ++row;
                        col = 1;
                    }
                    double_quote_just_before = 0;
                }
                else {
                    // text after an unclosed double_quote
                    buf[text_index] = c;
                    ++text_index;
                    double_quote_just_before = 0;
                }
                break;
            case '"':
                if ( double_quote_just_before == 1 ) {
                    // we meet the double_quote twice after an unclosed double_quote
                    buf[text_index] = c;
                    ++text_index;
                    double_quote_just_before = 0;
                    double_quote_before = 1;
                }
                else if ( double_quote_before == 1 ) {
                    // may be a closed double_quote
                    double_quote_before = 0;
                    // may also be a double_quote_just_before char
                    double_quote_just_before = 1;
                }
                else {
                    // that should be an open double_quote
                    double_quote_before = 1;
                    // open double_quote couldn't also be a double_quote_just_before char
                    double_quote_just_before = 0;
                }
                break;
            default:
                buf[text_index] = c;
                ++text_index;
                double_quote_just_before = 0;
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
#undef TEXT_BUF_LEN
    rewind( fp );
    return 0;
}

extern int export_csv( FILE* fp, const MyTable t )
{
    unsigned int prev_row = 1;
    unsigned int prev_col = 1;
    register Row* r = t;
    register Col* c;
    while ( r != NULL ) {
        c = r->col_head;
        while ( c != NULL ) {
            if ( c->data != NULL && c->data->text != NULL ) {
                printf( "%d:%d  %s\n", r->row, c->col, c->data->text );
                if ( r->row != prev_row ) {
                    prev_col = 1;
                }
                while ( r->row != prev_row ) {
                    fprintf( fp, "\n" );
                    ++prev_row;
                }
                if ( c->col != 1 ) {
                    fprintf( fp, "," );
                    ++prev_col;
                }
                while ( c->col != prev_col ) {
                    fprintf( fp, "," );
                    ++prev_col;
                }
                // ensure all the texts are surrounded by double_quotes
                fprintf( fp, "\"" );
                char* t = c->data->text;
                while ( *t != '\0' ) {
                    if ( *t == '"' ) {
                        // additional double_quote for double_quote in text
                        fprintf( fp, "\"" );
                    }
                    fprintf( fp, "%c", *t );
                    ++t;
                }
                fprintf( fp, "\"" );
            }
            c = c->next;
        }
        r = r->next;
    }
    // extra linefeed before eof makes import_csv() happy
    fprintf( fp, "\n" );
}
/*
int main( int argc, char** argv )
{
    FILE* fp = fopen( "test.csv", "r" );
    if ( fp == NULL )
        return -1;
    // parse
    MyTable t = create_table();
    import_csv( fp, t );

    fclose( fp );

    // out
    FILE* out = fopen( "testout.csv", "w" );
    if ( out != NULL ) {
        export_csv( out, t );
        fclose( out );
    }

    // free
    free_table( t );

    return 0;
}
*/
