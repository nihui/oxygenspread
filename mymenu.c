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

#include <windows.h>
#include <richedit.h>

#include "utility.h"
#include "table.h"
#include "history.h"
#include "selection.h"
#include "view.h"
#include "stream.h"

#include "resource.h"

#include "mymenu.h"

extern const int col_zero_width;
extern const int row_zero_height;

extern HWND hwnd;
extern HWND hEdit;
extern MyTable table;

extern int showgrid;
extern int editmode;
extern double ratio;

static int unsaved = 1;
static char szFileName[MAX_PATH];// opened file name

void MYMENU_FileNew()
{
    free_table( table );
    free_history();
    // a new table
    table = create_table();
    set_caption( NULL );
    restore_view();
    update_row_tails();
    update_col_tails();
    update_selection_rect();
    update_table_view( hwnd );
    unsaved = 1;
}

void MYMENU_FileOpen()
{
    OPENFILENAMEA ofn;
    ZeroMemory( &ofn, sizeof( ofn ) );
    
    ofn.lStructSize = sizeof( ofn ); // SEE NOTE BELOW
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "CSV Files (*.csv)\0*.csv\0Kiss Files (*.kiss)\0*.kiss\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "csv";
    
    if ( GetOpenFileName( &ofn ) ) {
        if ( prompt_save_changes() ) {
            FILE* fp = fopen( szFileName, "r" );
            if ( fp == NULL )
                return;
            free_table( table );
            free_history();
            // parse
            table = create_table();
            if ( strncmp( ofn.lpstrFile + ofn.nFileExtension, "csv", 3 ) == 0 )
                import_csv( fp, table );
            else if ( strncmp( ofn.lpstrFile + ofn.nFileExtension, "kiss", 4 ) == 0 )
                import_kiss( fp, table );
            fclose( fp );
            set_caption( szFileName );
            auto_adapt_layout( table );
            update_selection_rect();
            update_table_view( hwnd );
            unsaved = 0;
        }
    }
}

void MYMENU_FileSave()
{
    if ( unsaved == 1 )
        MYMENU_FileSaveAs();
    else {
        FILE* out = fopen( szFileName, "w" );
        if ( out != NULL ) {
            if ( strncmp( szFileName + strlen(szFileName) - 3, "csv", 3 ) == 0 )
                export_csv( out, table );
            else if ( strncmp( szFileName + strlen(szFileName) - 4, "kiss", 4 ) == 0 )
                export_kiss( out, table );
            fclose( out );
        }
    }
}

void MYMENU_FileSaveAs()
{
    OPENFILENAMEA ofn;
    ZeroMemory( &ofn, sizeof( ofn ) );
    
    ofn.lStructSize = sizeof( ofn ); // SEE NOTE BELOW
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "CSV Files (*.csv)\0*.csv\0Kiss Files (*.kiss)\0*.kiss\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "csv";
    
    if ( GetSaveFileName( &ofn ) ) {
        // out
        FILE* out = fopen( szFileName, "w" );
        if ( out != NULL ) {
            if ( strncmp( ofn.lpstrFile + ofn.nFileExtension, "csv", 3 ) == 0 )
                export_csv( out, table );
            else if ( strncmp( ofn.lpstrFile + ofn.nFileExtension, "kiss", 4 ) == 0 )
                export_kiss( out, table );
            fclose( out );
            set_caption( szFileName );
            unsaved = 0;
        }
    }
}

void MYMENU_FileClose()
{
    MYMENU_FileNew();
}

void MYMENU_FileExit()
{
    PostMessage( hwnd, WM_CLOSE, 0, 0 );
}

void MYMENU_EditUndo()
{
    if ( editmode == 1 ) {
        SendMessageW( hEdit, EM_UNDO, 0, 0 );
    }
    else {
        history_undo();
        update_selection_rect();
        update_table_view( hwnd );
    }
}

void MYMENU_EditRedo()
{
    if ( editmode == 1 ) {
        SendMessageW( hEdit, EM_REDO, 0, 0 );
    }
    else {
        history_redo();
        update_selection_rect();
        update_table_view( hwnd );
    }
}

void MYMENU_EditCut()
{
    if ( editmode == 1 ) {
        SendMessageW( hEdit, WM_CUT, 0, 0 );
    }
    else {
        MYMENU_EditCopy();
        MYMENU_EditDelete();
    }
}

void MYMENU_EditCopy()
{
    if ( editmode == 1 ) {
        SendMessageW( hEdit, WM_COPY, 0, 0 );
    }
    else {
        clear_stream();
        selection2stream( table );
    }
}

void MYMENU_EditPaste()
{
    if ( editmode == 1 ) {
        SendMessageW( hEdit, WM_PASTE, 0, 0 );
    }
    else {
        // get the cell where to paste on
        int start_row = 0;
        int start_col = 0;
        int end_row, end_col;
        get_selection( &start_row, &start_col, &end_row, &end_col );
        // let us extract cells from stream
        int row = 0;
        int col = 0;
        CellInfo* info = iterate_stream_cell( &row, &col );
        // base row and col, used to form relative rows and cols
        int base_row = row;
        int base_col = col;
        while ( info != NULL ) {
            // history
            history_append( start_row + row - base_row, start_col + col - base_col );
            free_redo_history();
            insert_cell_info( table, start_row + row - base_row, start_col + col - base_col, info );
            info = iterate_stream_cell( &row, &col );
        }
        auto_adapt_layout( table );
        update_selection_rect();
        update_table_view();
    }
}

void MYMENU_EditDelete()
{
    if ( editmode == 1 ) {
        SendMessageW( hEdit, WM_CLEAR, 0, 0 );
    }
    else {
        int row, col;
        CellInfo* info = iterate_selection( table, &row, &col );
        while ( info != NULL ) {
            // history
            history_append( row, col );
            free_redo_history();
            delete_cell_info( table, row, col );
            info = iterate_selection( table, &row, &col );
        }
        auto_adapt_layout( table );
        update_selection_rect();
        update_table_view();
    }
}

void MYMENU_EditSelectAll()
{
    SendMessageW( hEdit, EM_SETSEL, 0, -1 );
}

void MYMENU_EditTimeDate()
{
    int MAX_STRING_LEN = 100;
    SYSTEMTIME   st;
    WCHAR        szDate[MAX_STRING_LEN];
    static const WCHAR spaceW[] = { ' ',0 };
    GetLocalTime(&st);
    GetTimeFormatW(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, NULL, szDate, MAX_STRING_LEN);
    SendMessageW(hEdit, EM_REPLACESEL, TRUE, (LPARAM)szDate);
    SendMessageW(hEdit, EM_REPLACESEL, TRUE, (LPARAM)spaceW);
    GetDateFormatW(LOCALE_USER_DEFAULT, 0, &st, NULL, szDate, MAX_STRING_LEN);
    SendMessageW(hEdit, EM_REPLACESEL, TRUE, (LPARAM)szDate);
}

void MYMENU_ViewShowGrid()
{
    showgrid = (showgrid == 1)? 0 : 1;
    update_table_view();
}

void MYMENU_ViewSplitView()
{
    static int splitview = 0;
    if ( splitview == 0 ) {
        // get the cell where to split
        int start_row = 0;
        int start_col = 0;
        int end_row, end_col;
        get_selection( &start_row, &start_col, &end_row, &end_col );
        // get x/y pos
        int xx = current_view()->edge[ VIEW_LEFT ] + col_zero_width;
        int yy = current_view()->edge[ VIEW_TOP ] + row_zero_height;
        int p;
        for ( p = current_view()->col_head; p < start_col; ++p )
            xx += col_width_of( p ) * ratio;
        for ( p = current_view()->row_head; p < start_row; ++p )
            yy += row_height_of( p ) * ratio;

        // NOTE: always shift a bit to let the user select the last row/col
        if ( xx != current_view()->edge[ VIEW_LEFT ] + col_zero_width ) {
            split_view( xx + 1, -1 );
            splitview = 1;
        }
        if ( yy != current_view()->edge[ VIEW_TOP ] + row_zero_height ) {
            split_view( -1, yy + 1 );
            splitview = 1;
        }
    }
    else {
        restore_view();
        splitview = 0;
    }
    update_selection_rect();
    update_table_view();
}

void MYMENU_FormatFont()
{
    CHOOSEFONT cf;
    LOGFONT lf;
    CHARFORMAT2 fmt;
    HDC hDC = GetDC( hwnd );

    ZeroMemory(&cf, sizeof(cf));
    cf.lStructSize = sizeof(cf);
    cf.hwndOwner = hwnd;
    cf.lpLogFont = &lf;
    cf.Flags = CF_SCREENFONTS | CF_NOSCRIPTSEL | CF_INITTOLOGFONTSTRUCT | CF_EFFECTS;

    ZeroMemory(&fmt, sizeof(fmt));
    fmt.cbSize = sizeof(fmt);

    SendMessage( hEdit, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&fmt );
    lstrcpy(cf.lpLogFont->lfFaceName, fmt.szFaceName);
    cf.lpLogFont->lfItalic = (fmt.dwEffects & CFE_ITALIC) ? TRUE : FALSE;
    cf.lpLogFont->lfWeight = (fmt.dwEffects & CFE_BOLD) ? FW_BOLD : FW_NORMAL;
    cf.lpLogFont->lfUnderline = (fmt.dwEffects & CFE_UNDERLINE) ? TRUE : FALSE;
    cf.lpLogFont->lfStrikeOut = (fmt.dwEffects & CFE_STRIKEOUT) ? TRUE : FALSE;
    cf.lpLogFont->lfHeight = -MulDiv(fmt.yHeight / 20, GetDeviceCaps(hDC, LOGPIXELSY), 72);
    cf.rgbColors = fmt.crTextColor;
    unsigned int bg_rgb = fmt.crBackColor;

    ReleaseDC( hwnd, hDC );

    if(ChooseFont(&cf))
    {
        ZeroMemory(&fmt, sizeof(fmt));
        fmt.cbSize = sizeof(fmt);
        fmt.dwMask = CFM_FACE | CFM_BOLD | CFM_ITALIC | CFM_SIZE | CFM_UNDERLINE | CFM_STRIKEOUT | CFM_COLOR;
        fmt.yHeight = cf.iPointSize * 2;

        lstrcpy( fmt.szFaceName, cf.lpLogFont->lfFaceName );
        if(cf.nFontType & BOLD_FONTTYPE)
            fmt.dwEffects |= CFE_BOLD;
        if(cf.nFontType & ITALIC_FONTTYPE)
            fmt.dwEffects |= CFE_ITALIC;
        if(cf.lpLogFont->lfUnderline == TRUE)
            fmt.dwEffects |= CFE_UNDERLINE;
        if(cf.lpLogFont->lfStrikeOut == TRUE)
            fmt.dwEffects |= CFE_STRIKEOUT;

        fmt.crTextColor = cf.rgbColors;
        fmt.crBackColor = bg_rgb;

        if ( editmode == 1 ) {
            SendMessage( hEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&fmt );
        }
        else {
            int row, col;
            CellInfo* info = iterate_selection( table, &row, &col );
            while ( info != NULL ) {
                // history
                history_append( row, col );
                free_redo_history();
                charformat2style( &(info->style), &fmt );
                info = iterate_selection( table, &row, &col );
            }
            auto_adapt_layout( table );
            update_selection_rect();
            update_table_view();
        }
    }
}

void MYMENU_FormatBGColor()
{
    CHOOSECOLOR cc;
    static COLORREF acrCustClr[16];// array of custom colors
    static DWORD rgbCurrent;// initial color selection

    ZeroMemory(&cc, sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = hwnd;
    cc.lpCustColors = (LPDWORD) acrCustClr;
    cc.rgbResult = rgbCurrent;
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;

    if ( ChooseColor(&cc) ) {
        int row, col;
        CellInfo* info = iterate_selection( table, &row, &col );
        while ( info != NULL ) {
            // history
            history_append( row, col );
            free_redo_history();
            info->style.bg_rgb = cc.rgbResult;
            info = iterate_selection( table, &row, &col );
        }
        update_table_view();
    }
}

void MYMENU_FormatCombineCells()
{
    // get the start/end info
    int start_row, start_col, end_row, end_col;
    get_selection( &start_row, &start_col, &end_row, &end_col );
    // get the left top cell
    CellInfo* info = get_cell_info( table, start_row, start_col );
    CellInfo* infocopy;
    if ( info == NULL ) {
        // empty cell, create a new one
        infocopy = create_cell_info();
        infocopy->combine_right = end_col - start_col;
        infocopy->combine_bottom = end_row - start_row;
    }
    else {
        // create a new cell copy
        infocopy = clone_cell_info( info );
        // modify the exist one
        infocopy->combine_right = end_col - start_col;
        infocopy->combine_bottom = end_row - start_row;
    }
    // history
    history_append( start_row, start_col );
    free_redo_history();
    // update
    insert_cell_info( table, start_row, start_col, infocopy );
    update_table_view();
}

void MYMENU_FormatSplitCells()
{
    int row, col;
    CellInfo* info = iterate_selection( table, &row, &col );
    while ( info != NULL ) {
        if ( info->combine_right != 0 || info->combine_bottom != 0 ) {
            // history
            history_append( row, col );
            free_redo_history();
            info->combine_right = 0;
            info->combine_bottom = 0;
        }
        info = iterate_selection( table, &row, &col );
    }
    update_table_view();
}

void MYMENU_FormatAlignLeft()
{
    int row, col;
    CellInfo* info = iterate_selection( table, &row, &col );
    while ( info != NULL ) {
        // history
        history_append( row, col );
        free_redo_history();
        info->style.text_align = ALIGN_LEFT;
        info = iterate_selection( table, &row, &col );
    }
    update_table_view();
}

void MYMENU_FormatAlignRight()
{
    int row, col;
    CellInfo* info = iterate_selection( table, &row, &col );
    while ( info != NULL ) {
        // history
        history_append( row, col );
        free_redo_history();
        info->style.text_align = ALIGN_RIGHT;
        info = iterate_selection( table, &row, &col );
    }
    update_table_view();
}

void MYMENU_FormatAlignCenter()
{
    int row, col;
    CellInfo* info = iterate_selection( table, &row, &col );
    while ( info != NULL ) {
        // history
        history_append( row, col );
        free_redo_history();
        info->style.text_align = ALIGN_CENTER;
        info = iterate_selection( table, &row, &col );
    }
    update_table_view();
}

void MYMENU_FormatClearStyle()
{
    int row, col;
    CellInfo* info = iterate_selection( table, &row, &col );
    while ( info != NULL ) {
        // history
        history_append( row, col );
        free_redo_history();
        clear_style( &(info->style) );
        info = iterate_selection( table, &row, &col );
    }
    auto_adapt_layout( table );
    update_selection_rect();
    update_table_view();
}

void MYMENU_HelpAbout()
{
    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr( hwnd, GWLP_HINSTANCE );
    HICON icon = LoadImageW( hInstance, MAKEINTRESOURCEW(IDI_MYICON), IMAGE_ICON, 48, 48, LR_SHARED );
    static const WCHAR apptitle[] = {'O','x','y','g','e','n',' ','S','p','r','e','a','d','\0'};
    static const WCHAR extratext[] = L"(C) 2009 Ni Hui";
    ShellAboutW( hwnd, apptitle, extratext, icon );
}
