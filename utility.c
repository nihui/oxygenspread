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
#include <string.h>

#include <windows.h>
#include <commctrl.h>

#include "resource.h"
#include "utility.h"
#include "style.h"
#include "table.h"
#include "view.h"

// mingw32 doesn't have these define
#ifndef CFM_BACKCOLOR
#define CFM_BACKCOLOR 0x04000000
#endif
#ifndef CFM_WEIGHT
#define CFM_WEIGHT 0x00400000
#endif


extern HWND hwnd;
extern HWND hEdit;
extern RECT rcClient;

extern unsigned int col_zero_width;
extern unsigned int row_zero_height;

extern double ratio;

void add_button( HWND hwndToolBar, int nImage, int nCommand )
{
    TBBUTTON button;

    ZeroMemory( &button, sizeof(button) );
    button.iBitmap = nImage;
    button.idCommand = nCommand;
    button.fsState = TBSTATE_ENABLED;
    button.fsStyle = TBSTYLE_BUTTON;
    button.dwData = 0;
    button.iString = -1;
    SendMessageW( hwndToolBar, TB_ADDBUTTONS, 1, (LPARAM)&button );
}

void add_separator( HWND hwndToolBar )
{
    TBBUTTON button;
    
    ZeroMemory( &button, sizeof(button) );
    button.iBitmap = -1;
    button.idCommand = 0;
    button.fsState = 0;
    button.fsStyle = TBSTYLE_SEP;
    button.dwData = 0;
    button.iString = -1;
    SendMessageW( hwndToolBar, TB_ADDBUTTONS, 1, (LPARAM)&button );
}

void update_table_view()
{
    // get client rect
    GetClientRect( hwnd, &rcClient );
    rcClient.top += 28;
    rcClient.bottom -= 42;
    rcClient.right -= 20;
    InvalidateRect( hwnd, &rcClient, 1 );
    UpdateWindow( hwnd );
}


void ShowOpenError(DWORD Code)
{
    LPWSTR Message;
    switch(Code) {
        case ERROR_ACCESS_DENIED:
            Message = MAKEINTRESOURCEW(STRING_OPEN_ACCESS_DENIED);
            break;
        default:
            Message = MAKEINTRESOURCEW(STRING_OPEN_FAILED);
    }
    MessageBoxW( hwnd, Message, L"wszAppTitle", MB_ICONEXCLAMATION | MB_OK);
}

void ShowWriteError(DWORD Code)
{
    LPWSTR Message;
    switch(Code) {
        case ERROR_ACCESS_DENIED:
            Message = MAKEINTRESOURCEW(STRING_WRITE_ACCESS_DENIED);
            break;
        default:
            Message = MAKEINTRESOURCEW(STRING_WRITE_FAILED);
    }
    MessageBoxW( hwnd, Message, L"wszAppTitle", MB_ICONEXCLAMATION | MB_OK);
}

BOOL prompt_save_changes(void)
{/*
    if(!wszFileName[0])
    {
        GETTEXTLENGTHEX gt;
        gt.flags = GTL_NUMCHARS;
        gt.codepage = 1200;
        if(!SendMessageW(hEditorWnd, EM_GETTEXTLENGTHEX, (WPARAM)&gt, 0))
            return TRUE;
    }

    if(!SendMessageW(hEditorWnd, EM_GETMODIFY, 0, 0))
    {
        return TRUE;
    } else
    {
        LPWSTR displayFileName;
        WCHAR *text;
        int ret;

        if(!wszFileName[0])
            displayFileName = wszDefaultFileName;
        else
            displayFileName = file_basename(wszFileName);

        text = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                         (lstrlenW(displayFileName)+lstrlenW(wszSaveChanges))*sizeof(WCHAR));

        if(!text)
            return FALSE;

        wsprintfW(text, wszSaveChanges, displayFileName);

        ret = MessageBoxW(hMainWnd, text, wszAppTitle, MB_YESNOCANCEL | MB_ICONEXCLAMATION);

        HeapFree(GetProcessHeap(), 0, text);

        switch(ret)
        {
            case IDNO:
                return TRUE;

            case IDYES:
                if(wszFileName[0])
                    DoSaveFile(wszFileName, fileFormat);
                else
                    DialogSaveFile();
                return TRUE;

            default:
                return FALSE;
        }
    }*/
    return TRUE;
}

void set_caption( const char* filename )
{
    const char sep[] = " - ";
    const char untitiled_name[] = "[Untitiled]";
    const char appname[] = "Oxygen Spread";
    char* caption;
    int length = 0;

    if ( filename == NULL ) {
        caption = malloc( strlen(untitiled_name) + 3 + strlen(appname) + 1 );
        memcpy( caption, untitiled_name, strlen(untitiled_name)*sizeof(char) );
        length += strlen(untitiled_name);
    }
    else {
        caption = malloc( strlen(filename) + 3 + strlen(appname) + 1 );
        memcpy( caption, filename, strlen(filename)*sizeof(char) );
        length += strlen(filename);
    }

    memcpy( caption + length, sep, 3 );
    length += 3;
    memcpy( caption + length, appname, (strlen(appname)+1/*'\0'*/)*sizeof(char) );

    SetWindowText( hwnd, caption );
    free( caption );
}

void get_editctrl_charformat( CHARFORMAT2* fmt )
{
    ZeroMemory( fmt, sizeof(CHARFORMAT2) );
    fmt->cbSize = sizeof(CHARFORMAT2);
    fmt->dwMask = CFM_COLOR | CFM_BACKCOLOR | CFM_SIZE | CFM_FACE
                | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT | CFM_WEIGHT;
    SendMessage( hEdit, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)fmt );
}

void set_editctrl_charformat( CHARFORMAT2* fmt )
{
    fmt->cbSize = sizeof(CHARFORMAT2);
    fmt->dwMask = CFM_COLOR | CFM_BACKCOLOR | CFM_SIZE | CFM_FACE
                | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT | CFM_WEIGHT;
    // send message to ensure all the texts in edit control have the same style, selection and rests
    SendMessage( hEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)fmt );
    SendMessage( hEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)fmt );
}

int compare_style_charformat( const Style* style, const CHARFORMAT2* fmt )
{
    // text color
    if ( style->text_rgb != fmt->crTextColor )
        return 1;
    // text background color
    if ( style->bg_rgb != fmt->crBackColor )
        return 1;
    // text size
    if ( style->text_size * 20 != fmt->yHeight )
        return 1;
    // text font face
    if ( strcmp( style->text_font, fmt->szFaceName ) != 0 )
        return 1;
    // text style effect
    if ( get_text_style_status( style, TEXT_BOLD ) == 0
        && fmt->dwEffects == (fmt->dwEffects|CFE_BOLD) )
        return 1;
    if ( get_text_style_status( style, TEXT_ITALIC ) == 0
        && fmt->dwEffects == (fmt->dwEffects|CFE_ITALIC) )
        return 1;
    if ( get_text_style_status( style, TEXT_UNDERLINE ) == 0
        && fmt->dwEffects == (fmt->dwEffects|CFE_UNDERLINE) )
        return 1;
    if ( get_text_style_status( style, TEXT_STRIKEOUT ) == 0
        && fmt->dwEffects == (fmt->dwEffects|CFE_STRIKEOUT) )
        return 1;
    return 0;
}

void charformat2style( Style* style, const CHARFORMAT2* fmt )
{
    // text color
    style->text_rgb = fmt->crTextColor;
    // text background color
    style->bg_rgb = fmt->crBackColor;
    // text size
    style->text_size = fmt->yHeight / 20;
    // text font face
    strcpy(style->text_font, fmt->szFaceName);
    // text style effect
    if ( fmt->dwEffects == (fmt->dwEffects|CFE_BOLD) )
        apply_text_style( style, TEXT_BOLD );
    if ( fmt->dwEffects == (fmt->dwEffects|CFE_ITALIC) )
        apply_text_style( style, TEXT_ITALIC );
    if ( fmt->dwEffects == (fmt->dwEffects|CFE_UNDERLINE) )
        apply_text_style( style, TEXT_UNDERLINE );
    if ( fmt->dwEffects == (fmt->dwEffects|CFE_STRIKEOUT) )
        apply_text_style( style, TEXT_STRIKEOUT );
}

void style2charformat( CHARFORMAT2* fmt, const Style* style )
{
    ZeroMemory( fmt, sizeof(CHARFORMAT2) );
    fmt->cbSize = sizeof(CHARFORMAT2);
    fmt->dwMask =  CFM_COLOR | CFM_BACKCOLOR | CFM_SIZE | CFM_FACE;
    if ( style == NULL ) {
        fmt->crTextColor = 0x00000000;
        fmt->crBackColor = 0x00ffffff;
        fmt->yHeight = 12 * 20;
        return;
    }
    // text color
    fmt->crTextColor = style->text_rgb;
    // text background color
    fmt->crBackColor = style->bg_rgb;
    // text size
    fmt->yHeight = style->text_size * 20;
    // text font face
    strcpy( fmt->szFaceName, style->text_font );
    // text style effect
    if ( get_text_style_status( style, TEXT_BOLD ) ) {
        fmt->dwMask |= CFM_BOLD;
        fmt->dwEffects |= CFE_BOLD;
        fmt->dwMask |= CFM_WEIGHT;
        fmt->wWeight = FW_BOLD;
    }
    if ( get_text_style_status( style, TEXT_ITALIC ) ) {
        fmt->dwMask |= CFM_ITALIC;
        fmt->dwEffects |= CFE_ITALIC;
    }
    if ( get_text_style_status( style, TEXT_UNDERLINE ) ) {
        fmt->dwMask |= CFM_UNDERLINE;
        fmt->dwEffects |= CFE_UNDERLINE;
    }
    if ( get_text_style_status( style, TEXT_STRIKEOUT ) ) {
        fmt->dwMask |= CFM_STRIKEOUT;
        fmt->dwEffects |= CFE_STRIKEOUT;
    }
}

void style2logfont( LOGFONT* logfont, const Style* style, HDC hdc )
{
    ZeroMemory( logfont, sizeof(LOGFONT) );
    // text size
    logfont->lfHeight = -MulDiv( style->text_size, GetDeviceCaps( hdc, LOGPIXELSY ), 72 ) * ratio;
    // text font face
    strcpy( logfont->lfFaceName, style->text_font );
    // text style effect
    logfont->lfWeight = (get_text_style_status( style, TEXT_BOLD ) != 0)? FW_BOLD: FW_NORMAL;
    logfont->lfItalic = (get_text_style_status( style, TEXT_ITALIC ) != 0)? TRUE: FALSE;
    logfont->lfUnderline = (get_text_style_status( style, TEXT_UNDERLINE ) != 0)? TRUE: FALSE;
    logfont->lfStrikeOut = (get_text_style_status( style, TEXT_STRIKEOUT ) != 0)? TRUE: FALSE;

    logfont->lfEscapement = 0;
    logfont->lfOutPrecision = OUT_TT_PRECIS;
    logfont->lfClipPrecision = CLIP_DEFAULT_PRECIS;
}

void draw_vertical_line( HDC hdc, int x, int top, int bottom )
{
    MoveToEx( hdc, x, top, NULL );
    LineTo( hdc, x, bottom );
}

void draw_horizontal_line( HDC hdc, int y, int left, int right )
{
    MoveToEx( hdc, left, y, NULL );
    LineTo( hdc, right, y );
}

void draw_rect_border( HDC hdc, int left, int top, int right, int bottom )
{
    MoveToEx( hdc, left, top, NULL );
    LineTo( hdc, right, top );
    LineTo( hdc, right, bottom );
    LineTo( hdc, left, bottom );
    LineTo( hdc, left, top );
}

static RECT rcSelectedCell;

void update_selection_rect()
{
    int start_row,start_col,end_row,end_col;
    get_selection( &start_row, &start_col, &end_row, &end_col );
    int start_x = 0;
    int start_y = 0;
    int end_x = 0;
    int end_y = 0;
    get_absolute_cell_pos( current_view(), start_row, start_col, &start_x, &start_y );
    get_absolute_cell_pos( current_view(), end_row, end_col, &end_x, &end_y );
    // refresh
    if ( start_x == -1 )
        rcSelectedCell.left = current_view()->edge[VIEW_LEFT] + col_zero_width;
    else
        rcSelectedCell.left = start_x + 1;
    if ( start_y == -1 )
        rcSelectedCell.top = current_view()->edge[VIEW_TOP] + row_zero_height;
    else
        rcSelectedCell.top = start_y + 1;
    if ( end_x == -1 )
        rcSelectedCell.right = rcSelectedCell.left;
    else {
        extern MyTable table;
        rcSelectedCell.right = end_x + cell_width_of( table, end_row, end_col ) * ratio;
    }
    if ( end_y == -1 )
        rcSelectedCell.bottom = rcSelectedCell.top;
    else {
        extern MyTable table;
        rcSelectedCell.bottom = end_y + cell_height_of( table, end_row, end_col ) * ratio;
    }
}

void draw_selection_rect_border( HDC hdc )
{
    // draw the selected cell rectangle
    HPEN hPen = CreatePen( PS_SOLID, 2, RGB( 255, 25, 5 ) );
    HPEN hPenold = SelectObject( hdc, hPen );
    draw_rect_border( hdc, rcSelectedCell.left, rcSelectedCell.top, rcSelectedCell.right, rcSelectedCell.bottom );
    DeleteObject( hPenold );
    DeleteObject( hPen );
}
