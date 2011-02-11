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

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <windows.h>
#include <richedit.h>
#include <commctrl.h>

#include "resource.h"
#include "file_csv.h"
#include "oxygenspread.h"
#include "mymenu.h"
#include "utility.h"
#include "history.h"
#include "stream.h"
#include "view.h"

// mingw32 doesn't have these define
#ifndef CFM_BACKCOLOR
#define CFM_BACKCOLOR 0x04000000
#endif
#ifndef CFM_WEIGHT
#define CFM_WEIGHT 0x00400000
#endif

//     TOOLINFO toolInfo = { 0 };

RECT rcClient; // 客户区矩形

int showgrid = 1;
int editmode = 0;
double ratio = 1.0;


static BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch ( Message ) {
        case WM_INITDIALOG:
            return TRUE;
        case WM_COMMAND:
            switch ( LOWORD(wParam) ) {
                case IDOK:
                    EndDialog( hwnd, IDOK );
                    break;
                case IDCANCEL:
                    EndDialog( hwnd, IDCANCEL );
                    break;
            }
            break;
        default:
            return FALSE;
    }
    return TRUE;
}

static LRESULT OnCreate( HWND hwnd )
{
    // Toolbar
    HWND hTool = CreateWindowEx( 0, TOOLBARCLASSNAME, NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0,
                                 hwnd, ( HMENU )IDC_MAIN_TOOL, GetModuleHandle( NULL ), NULL );
    SendMessage( hTool, TB_BUTTONSTRUCTSIZE, ( WPARAM )sizeof( TBBUTTON ), 0 );

    TBADDBITMAP tbab;
    tbab.hInst = HINST_COMMCTRL;
    tbab.nID = IDB_STD_SMALL_COLOR;
    int nStdBitmaps = SendMessage( hTool, TB_ADDBITMAP, 0, ( LPARAM ) & tbab );
    add_button( hTool, nStdBitmaps + STD_FILENEW, ID_FILE_NEW );
    add_button( hTool, nStdBitmaps + STD_FILEOPEN, ID_FILE_OPEN );
    add_button( hTool, nStdBitmaps + STD_FILESAVE, ID_FILE_SAVE );
    add_separator( hTool );
    add_button( hTool, nStdBitmaps + STD_UNDO, ID_EDIT_UNDO );
    add_button( hTool, nStdBitmaps + STD_REDOW, ID_EDIT_REDO );
    add_separator( hTool );
    add_button( hTool, nStdBitmaps + STD_CUT, ID_EDIT_CUT );
    add_button( hTool, nStdBitmaps + STD_COPY, ID_EDIT_COPY );
    add_button( hTool, nStdBitmaps + STD_PASTE, ID_EDIT_PASTE );

    // statusbar
    HWND hStatus = CreateWindowEx( 0, STATUSCLASSNAME, NULL,
                                   WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0,
                                   hwnd, ( HMENU )IDC_MAIN_STATUS, GetModuleHandle( NULL ), NULL );
    int statwidths[] = {300, -1};
    SendMessage( hStatus, SB_SETPARTS, sizeof( statwidths ) / sizeof( int ), ( LPARAM )statwidths );
    SendMessage( hStatus, SB_SETTEXT, 0, ( LPARAM )"Hi there :)" );

    // scroll bar
    HWND hScroll = CreateWindowEx( 0, "SCROLLBAR", NULL, WS_CHILD | WS_VISIBLE,
                                   0, 0, 0, 20,
                                   hwnd, ( HMENU )IDC_MAIN_SCROLL, GetModuleHandle( NULL ), NULL );
    SetScrollRange( hScroll, SB_CTL, 1, 50, 0 );
//     // Create the tooltip.TODO:
//     HWND hwndTip = CreateWindowEx(0, TOOLTIPS_CLASS, NULL,
//                               WS_POPUP |TTS_ALWAYSTIP | TTS_BALLOON,
//                               CW_USEDEFAULT, CW_USEDEFAULT,
//                               CW_USEDEFAULT, CW_USEDEFAULT,
//                               hwnd, /*(HMENU)IDC_MAIN_SCROLL_TOOLTIP*/NULL,
//                               GetModuleHandle( NULL ), NULL);
//     // Associate the tooltip with the tool.
//     toolInfo.cbSize = sizeof(toolInfo);
//     toolInfo.hwnd = hwnd;
//     toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
//     toolInfo.uId = (UINT_PTR)hScroll;
//     toolInfo.lpszText = "Tooltip:)";
//     SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
    // scroll bar V
    HWND hScrollV = CreateWindowEx( 0, "SCROLLBAR", NULL, WS_CHILD | WS_VISIBLE | SBS_VERT,
                                    0, 0, 20, 0,
                                    hwnd, ( HMENU )IDC_MAIN_SCROLLV, GetModuleHandle( NULL ), NULL );
    SetScrollRange( hScrollV, SB_CTL, 1, INT_MAX - 200, 0 );
    // edit
    LoadLibrary( TEXT( "riched20.dll" ) );
    hEdit = CreateWindowEx( WS_EX_CLIENTEDGE, RICHEDIT_CLASS, NULL,
                            WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_BORDER/*|ES_SELECTIONBAR|ES_MULTILINE|ES_AUTOVSCROLL
                            |ES_WANTRETURN|WS_VSCROLL|ES_NOHIDESEL|WS_HSCROLL*/,
                            CW_USEDEFAULT, CW_USEDEFAULT, 98, 23,
                            hwnd, ( HMENU )IDC_MAIN_EDIT, GetModuleHandle( NULL ), NULL );
    // set it to a proper position
    SetWindowPos( hEdit, NULL, 1 , 28 + 1, 0, 0, SWP_NOZORDER );
    ShowWindow( hEdit, SW_HIDE );

    // a new table
    table = create_table();
    init_view();
    update_view_rect( 0, 0, 0, 0, 640, 480 );

    return 0;
}

// Step 4: the Window Procedure
LRESULT CALLBACK WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    static int dragrow = 0;
    static int dragcol = 0;
    static int dragedline = 0;

    switch ( msg ) {
        case WM_CREATE: {
            OnCreate( hwnd );
            showgrid = 1;
            break;
        }
        case WM_COMMAND: {
            switch ( LOWORD( wParam ) ) {
                case ID_FILE_NEW:
                    MYMENU_FileNew();break;
                case ID_FILE_CLOSE:
                    MYMENU_FileClose();break;
                case ID_FILE_OPEN:
                    MYMENU_FileOpen();break;
                case ID_FILE_SAVE:
                    MYMENU_FileSave();break;
                case ID_FILE_SAVEAS:
                    MYMENU_FileSaveAs();break;
                case ID_FILE_EXIT:
                    MYMENU_FileExit();break;
                case ID_EDIT_UNDO:
                    MYMENU_EditUndo();break;
                case ID_EDIT_REDO:
                    MYMENU_EditRedo();break;
                case ID_EDIT_CUT:
                    MYMENU_EditCut();break;
                case ID_EDIT_COPY:
                    MYMENU_EditCopy();break;
                case ID_EDIT_PASTE:
                    MYMENU_EditPaste();break;
                case ID_EDIT_DELETE:
                    MYMENU_EditDelete();break;
                case ID_EDIT_SELECTALL:
                    MYMENU_EditSelectAll();break;
                case ID_EDIT_TIMEDATE:
                    MYMENU_EditTimeDate();break;
                case ID_VIEW_SHOWGRID:
                    MYMENU_ViewShowGrid();break;
                case ID_VIEW_SPLITVIEW:
                    MYMENU_ViewSplitView();break;
                case ID_VIEW_P60:
                    ratio = 0.6;update_col_tails();update_row_tails();update_selection_rect();update_table_view();
                    break;
                case ID_VIEW_P100:
                    ratio = 1.0;update_col_tails();update_row_tails();update_selection_rect();update_table_view();
                    break;
                case ID_VIEW_P200:
                    ratio = 2.0;update_col_tails();update_row_tails();update_selection_rect();update_table_view();
                    break;
                case ID_FORMAT_FONT:
                    MYMENU_FormatFont();break;
                case ID_FORMAT_BGCOLOR:
                    MYMENU_FormatBGColor();break;
                case ID_FORMAT_COMBINECELLS:
                    MYMENU_FormatCombineCells();break;
                case ID_FORMAT_SPLITCELLS:
                    MYMENU_FormatSplitCells();break;
                case ID_ALIGN_LEFT:
                    MYMENU_FormatAlignLeft();break;
                case ID_ALIGN_RIGHT:
                    MYMENU_FormatAlignRight();break;
                case ID_ALIGN_CENTER:
                    MYMENU_FormatAlignCenter();break;
                case ID_FORMAT_CLEARFORMAT:
                    MYMENU_FormatClearStyle();break;
                case ID_HELP_ABOUT:
                    MYMENU_HelpAbout();break;
            }
            break;
        }
        case WM_SIZE: {
            if ( wParam == SIZE_MINIMIZED )
                break;

            RECT rcTool;
            RECT rcStatus;
            RECT rcScroll;
            RECT rcScrollV;

            // Size toolbar and get height
            HWND hTool = GetDlgItem( hwnd, IDC_MAIN_TOOL );
            SendMessage( hTool, TB_AUTOSIZE, 0, 0 );

            GetWindowRect( hTool, &rcTool );
            int iToolHeight = rcTool.bottom - rcTool.top;

            // Size status bar and get height
            HWND hStatus = GetDlgItem( hwnd, IDC_MAIN_STATUS );
            SendMessage( hStatus, WM_SIZE, 0, 0 );

            GetWindowRect( hStatus, &rcStatus );
            int iStatusHeight = rcStatus.bottom - rcStatus.top;

            // Size scrollbar and get height
            HWND hScroll = GetDlgItem( hwnd, IDC_MAIN_SCROLL );
//             SendMessage( hScroll, WM_SIZE, 0, 0 );
            GetWindowRect( hScroll, &rcScroll );
            int iScrollHeight = rcScroll.bottom - rcScroll.top;

            HWND hScrollV = GetDlgItem( hwnd, IDC_MAIN_SCROLLV );
//             SendMessage( hScroll, WM_SIZE, 0, 0 );
            GetWindowRect( hScrollV, &rcScrollV );
            int iScrollVWidth = rcScrollV.right - rcScrollV.left;

            // Calculate remaining height and size edit
            GetClientRect( hwnd, &rcClient );

            SetWindowPos( hScroll, NULL, 0, rcClient.bottom - iStatusHeight - iScrollHeight, rcClient.right - iScrollVWidth, iScrollHeight, SWP_NOZORDER );
            SetWindowPos( hScrollV, NULL, rcClient.right - iScrollVWidth, iToolHeight, iScrollVWidth, rcClient.bottom - iToolHeight - iStatusHeight - iScrollHeight, SWP_NOZORDER );

            // change the tail row/col
            update_view_area( 0, 0, rcClient.right, rcClient.bottom );
//             update_table_view();
            InvalidateRect( hwnd, NULL, 1 );
            UpdateWindow( hwnd );
            break;
        }
        case WM_LBUTTONDOWN: {
            static int editcol = 0;
            static int editrow = 0;
            int x = LOWORD( lParam );
            int y = HIWORD( lParam ) - 28;
            ///NOTE: set the current view
            set_current_view_from_point( x, y );
            ViewRect* rcView = current_view();
            // for drag
            int p;
            if ( y > rcView->edge[ VIEW_TOP ]
                    && y < rcView->edge[ VIEW_TOP ] + row_zero_height ) {
                int distance = rcView->edge[ VIEW_LEFT ] + col_zero_width;
                for ( p = rcView->col_head; p < rcView->col_tail; ++p ) {
                    if ( x >= distance - 1
                            && x <= distance + 1 ) {
                        SetCursor( LoadCursor( NULL, IDC_SIZEWE ) );
                        if ( dragcol == 0 ) {
                            dragedline = p;
                            dragcol = 1;
                        }
                        break;
                    }
                    distance += col_width_of( p ) * ratio;
                }
            }
            if ( x > rcView->edge[ VIEW_LEFT ]
                    && x < rcView->edge[ VIEW_LEFT ] + col_zero_width ) {
                int distance = rcView->edge[ VIEW_TOP ] + row_zero_height;
                for ( p = rcView->row_head; p < rcView->row_tail; ++p ) {
                    if ( y >= distance - 1
                            && y <= distance + 1 ) {
                        SetCursor( LoadCursor( NULL, IDC_SIZENS ) );
                        if ( dragrow == 0 ) {
                            dragedline = p;
                            dragrow = 1;
                        }
                        break;
                    }
                    distance += row_height_of( p ) * ratio;
                }
            }

            // get proper edit position
            int row = 0;
            int col = 0;
            get_row_and_col_from_point( rcView, x, y, &row, &col );

            // we do not want to select the overboarded cell
            if ( col < rcView->col_tail - 1
                    && col >= rcView->col_head
                    && row < rcView->row_tail - 1
                    && row >= rcView->row_head ) {
                selection_start( row, col );
                update_selection_rect();
                update_table_view();

                // update status bar info
                char status[50];
                sprintf( status, "( %d , %d )%d,%d,%d,%d", col, row, rcView->row_head, rcView->col_head, rcView->row_tail, rcView->col_tail );
                HWND hStatus = GetDlgItem( hwnd, IDC_MAIN_STATUS );
                SendMessage( hStatus, SB_SETTEXT, 0, ( LPARAM )status );

                HWND hEdit = GetDlgItem( hwnd, IDC_MAIN_EDIT );
                // to see if we want to enter edit mode
                if ( col == editcol && row == editrow && editmode == 0 ) {
                    int x = 0;
                    int y = 0;
                    get_absolute_cell_pos( rcView, row, col, &x, &y );
                    SetWindowPos( hEdit, NULL, x + 1 , y + 1 + 28, cell_width_of( table, row, col )*ratio - 1, cell_height_of( table, row, col )*ratio - 1, SWP_NOZORDER );
                    ShowWindow( hEdit, SW_SHOW );
                    SetFocus( hEdit );
                    editmode = 1;
                }
                else if ( editmode == 1 ) {
                    editmode = 0;
                    // get edit text
                    int len = GetWindowTextLength( GetDlgItem( hwnd, IDC_MAIN_EDIT ) );
                    CellInfo* info = get_cell_info( table, editrow, editcol );
                    if ( len > 0 ) {
                        // Now we allocate, and get the string into our buffer
                        int i;
                        char* buf = malloc(( len + 1 ) * sizeof( char ) );
                        GetDlgItemText( hwnd, IDC_MAIN_EDIT, buf, len + 1 );
                        // do insertion/updating only if the cell text dose not exist or not equal to the edit text
                        CHARFORMAT2 fmt;
                        get_editctrl_charformat( &fmt );
                        if ( info == NULL )
                            info = create_cell_info();
                        if ( info->text == NULL
                                || memcmp( info->text, buf, len ) != 0
                                || compare_style_charformat( &( info->style ), &fmt ) != 0 ) {
                            CellInfo* new_info = clone_cell_info( info );
                            new_info->text = buf;
                            // history
                            history_append( editrow, editcol );
                            free_redo_history();
                            // style stuff
                            charformat2style( &( new_info->style ), &fmt );
                            insert_cell_info( table, editrow, editcol, new_info );
                            update_table_view();
                        }
                    }
                    else if ( len == 0 ) {
                        // do deletion only if the cell is not empty currently
                        if ( info != NULL && info->text != NULL ) {
                            // history
                            history_append( editrow, editcol );
                            free_redo_history();
                            if ( info->combine_right != 0 || info->combine_bottom != 0 ) {
                                free( info->text );
                                info->text = NULL;
                            }
                            else {
                                // preseve the cell info of combination, but clear texts
                                delete_cell_info( table, editrow, editcol );
                            }
                            update_table_view();
                        }
                    }
                    ShowWindow( hEdit, SW_HIDE );
                }
                editcol = col;
                editrow = row;

                // set text
                CellInfo* info = get_cell_info( table, row, col );
                CHARFORMAT2 fmt;
                if ( info != NULL && info->text != NULL ) {
                    SetDlgItemText( hwnd, IDC_MAIN_EDIT, info->text );
                    // set text style in edit control TODO: for empty text apply style as well ?
                    style2charformat( &fmt, &( info->style ) );
                }
                else {
                    SetDlgItemText( hwnd, IDC_MAIN_EDIT, "" );
                    // set text style in edit control TODO: for empty text apply style as well ?
                    style2charformat( &fmt, NULL );
                }
                set_editctrl_charformat( &fmt );

            }
            break;
        }
        case WM_LBUTTONUP: {
            int x = LOWORD( lParam );
            int y = HIWORD( lParam ) - 28;

            ViewRect* rcView = current_view();
            int row = 0;
            int col = 0;
            get_row_and_col_from_point( rcView, x, y, &row, &col );
            // we do not want to select the overboarded cell
            if ( col < rcView->col_tail - 1
                && col >= rcView->col_head
                && row < rcView->row_tail - 1
                && row >= rcView->row_head ) {
                int start_row, start_col, end_row, end_col;
                get_selection( &start_row, &start_col, &end_row, &end_col );

                if (( row != end_row && row != start_row )
                        || ( col != end_col && col != start_col ) ) {
                    selection_end( row, col );
                    update_selection_rect();
                    update_table_view();
                }
            }

            if ( dragrow == 1 ) {
                dragrow = 0;
            }
            if ( dragcol == 1 ) {
                dragcol = 0;
            }
            break;
        }
        case WM_MOUSEMOVE: {
            // TODO: point change when on the lines
            int x = LOWORD( lParam );
            int y = HIWORD( lParam ) - 28;

            ViewRect* rcView = current_view();
            int distance = rcView->edge[ VIEW_LEFT ] + col_zero_width;
            int p = 0;
            for ( p = rcView->col_head; p < rcView->col_tail; ++p ) {
                if ( x >= distance - 1
                        && x <= distance + 1
                        && y > rcView->edge[ VIEW_TOP ]
                        && y < rcView->edge[ VIEW_TOP ] + row_zero_height ) {
                    if ( GetCursor() != ( HCURSOR )IDC_SIZEWE && dragcol == 0 )
                        SetCursor( LoadCursor( NULL, IDC_SIZEWE ) );
                    break;
                }
                distance += col_width_of( p ) * ratio;
            }
            distance = rcView->edge[ VIEW_TOP ] + row_zero_height;
            for ( p = rcView->row_head; p < rcView->row_tail; ++p ) {
                if ( y >= distance - 1
                        && y <= distance + 1
                        && x > rcView->edge[ VIEW_LEFT ]
                        && x < rcView->edge[ VIEW_LEFT ] + col_zero_width ) {
                    if ( GetCursor() != ( HCURSOR )IDC_SIZENS && dragrow == 0 )
                        SetCursor( LoadCursor( NULL, IDC_SIZENS ) );
                    break;
                }
                distance += row_height_of( p ) * ratio;
            }

            if ( wParam == MK_LBUTTON && dragcol == 1 && y < rcView->edge[ VIEW_TOP ] + row_zero_height ) {
                int dragedcol_width = rcView->edge[ VIEW_LEFT ] + col_zero_width;
                for ( p = rcView->col_head; p < dragedline; ++p ) {
                    dragedcol_width += col_width_of( p ) * ratio;
                }
//                 fprintf( stderr, "%d, %d\n", dragedline, x - dragedcol_width );

                // do not drag it too much to be over the prev line
                int w = col_width_of( dragedline - 1 ) + x - dragedcol_width;
                if ( w > 2 ) {
                    set_col_width( dragedline - 1, w );
                }
                // change the tail col
                update_col_tails();
                update_selection_rect();
                update_table_view();
            }
            else if ( wParam == MK_LBUTTON && dragrow == 1 && x < rcView->edge[ VIEW_LEFT ] + col_zero_width ) {
                int dragedrow_height = rcView->edge[ VIEW_TOP ] + row_zero_height;
                for ( p = rcView->row_head; p < dragedline; ++p ) {
                    dragedrow_height += row_height_of( p ) * ratio;
                }
//                 fprintf( stderr, "%d, %d\n", dragedline, y - dragedrow_height );

                // do not drag it too much to be over the prev line
                int h = row_height_of( dragedline - 1 ) + y - dragedrow_height;
                if ( h > 2 ) {
                    set_row_height( dragedline - 1, h );
                }
                // change the tail row
                update_row_tails();
                update_selection_rect();
                update_table_view();
            }
            else if ( wParam == MK_LBUTTON ) {
                int row = 0;
                int col = 0;
                get_row_and_col_from_point( rcView, x, y, &row, &col );
                // we do not want to select the overboarded cell
                if ( col < rcView->col_tail - 1
                    && col >= rcView->col_head
                    && row < rcView->row_tail - 1
                    && row >= rcView->row_head ) {
                    int start_row, start_col, end_row, end_col;
                    get_selection( &start_row, &start_col, &end_row, &end_col );
                    if (( row != end_row && row != start_row )
                            || ( col != end_col && col != start_col ) ) {

                        selection_end( row, col );
                        update_selection_rect();
                        update_table_view();
                    }
                }
            }
            break;
        }
        case WM_HSCROLL: {
//             HWND hScroll = GetDlgItem( hwnd, IDC_MAIN_SCROLL );
            HWND hScroll = ( HWND )lParam;
            SCROLLINFO si;
            ZeroMemory( &si, sizeof( si ) );
            si.cbSize = sizeof( si );
            si.fMask = SIF_ALL;
            GetScrollInfo( hScroll, SB_CTL, &si );
            int CurPos = si.nPos;

            switch ( LOWORD( wParam ) ) {
                case SB_LEFT:
                    si.nPos = si.nMin;
                    break;
                case SB_LINELEFT:
                    si.nPos--;
                    break;
                case SB_PAGELEFT:
                    si.nPos -= 10;
                    break;
                case SB_THUMBTRACK:
                    si.nPos = si.nTrackPos;
                    break;
                case SB_PAGERIGHT:
                    si.nPos += 10;
                    break;
                case SB_LINERIGHT:
                    si.nPos++;
                    break;
                case SB_RIGHT:
                    si.nPos = si.nMax;
                    break;
                case SB_ENDSCROLL:
                    si.nMax = si.nPos + si.nPos / ( INT_MAX / 1024 ) * 128 + 128;
                    if ( si.nPos > INT_MAX - 1000 ) {
                        si.nMax = INT_MAX - ( current_view()->col_tail - current_view()->col_head );
                    }
                    si.fMask = SIF_RANGE;
                    SetScrollInfo( hScroll, SB_CTL, &si, 0 );
                    break;
                default:
                    break;
            }

            si.fMask = SIF_POS;
            SetScrollInfo( hScroll, SB_CTL, &si, TRUE );
            GetScrollInfo( hScroll, SB_CTL, &si );
            if ( si.nPos != CurPos ) {
                current_view()->col_head = si.nPos;
                update_col_tail_of( current_view() );
                update_selection_rect();
                update_table_view( current_view() );
            }
            // TODO: update tooltip
//             char strPosition[10] = "";
//             sprintf( strPosition, "%d", CurPos );
//             SetDlgItemText(hwnd, IDC_MAIN_SCROLL, strPosition);
//             HWND Tooltip = GetDlgItem( hwnd, IDC_MAIN_SCROLL_TOOLTIP );
//             toolInfo.lpszText = strPosition;
//             SendMessage( hScroll, TTM_UPDATETIPTEXT, 0, (LPARAM)&toolInfo );
            break;
        }
        case WM_VSCROLL: {
//             HWND hScroll = GetDlgItem( hwnd, IDC_MAIN_SCROLL );
            HWND hScrollV = ( HWND )lParam;
            SCROLLINFO si;
            ZeroMemory( &si, sizeof( si ) );
            si.cbSize = sizeof( si );
            si.fMask = SIF_ALL;
            GetScrollInfo( hScrollV, SB_CTL, &si );
            int CurPos = si.nPos;

            switch ( LOWORD( wParam ) ) {
                case SB_TOP:
                    si.nPos = si.nMin;
                    break;
                case SB_LINEUP:
                    si.nPos--;
                    break;
                case SB_PAGEUP:
                    si.nPos -= 10;
                    break;
                case SB_THUMBTRACK:
                    si.nPos = si.nTrackPos;
                    break;
                case SB_PAGEDOWN:
                    si.nPos += 10;
                    break;
                case SB_LINEDOWN:
                    si.nPos++;
                    break;
                case SB_BOTTOM:
                    si.nPos = si.nMax;
                    break;
                case SB_ENDSCROLL:
                    si.nMax = si.nPos + si.nPos / ( INT_MAX / 1024 ) * 128 + 128;
                    if ( si.nPos > INT_MAX - 1000 ) {
                        si.nMax = INT_MAX - ( current_view()->row_tail - current_view()->row_head );
                    }
                    si.fMask = SIF_RANGE;
                    SetScrollInfo( hScrollV, SB_CTL, &si, 0 );
                    break;
                default:
                    break;
            }

            si.fMask = SIF_POS;
            SetScrollInfo( hScrollV, SB_CTL, &si, TRUE );
            GetScrollInfo( hScrollV, SB_CTL, &si );
            if ( si.nPos != CurPos ) {
                current_view()->row_head = si.nPos;
                update_row_tail_of( current_view() );
                update_selection_rect();
                update_table_view( current_view() );
            }
            // TODO: update tooltip
//             char strPosition[10] = "";
//             sprintf( strPosition, "%d", CurPos );
//             SetDlgItemText(hwnd, IDC_MAIN_SCROLLV, strPosition);
            break;
        }
        case WM_ERASEBKGND:
            return TRUE;
        case WM_PAINT: {
            HDC hdcMem;  // 内存设备描述表
            HBITMAP hBitmap; // 缓冲位图
            HGDIOBJ hOldObject; // 内存设备描述表原位图
            // draw table
            PAINTSTRUCT Ps;
            HDC hDC = BeginPaint( hwnd, &Ps );

            GetClientRect( hwnd, &rcClient );

            // apply for memory descriptor and bitmap cache
            GetClientRect( hwnd, &rcClient );
            hdcMem = CreateCompatibleDC( hDC );
            hBitmap = CreateCompatibleBitmap( hDC, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top - 20 - 28 );
            hOldObject = SelectObject( hdcMem, hBitmap );
            // WM_ERASEBKGND
            // 使用的Brush 同窗口类注册时使用的Brush
            FillRect( hdcMem, &rcClient, ( HBRUSH ) GetStockObject( WHITE_BRUSH ) );

            // let's go
            {
                ViewRect* rcView;
                int m = 0;
                for ( ; m < 2; ++m ) {
                    int n = 0;
                    for ( ; n < 2; ++n ) {
                        if (( rcView = get_view_rect( m, n ) ) != NULL ) {
                            int edge_left = rcView->edge[ VIEW_LEFT ];
                            int edge_top = rcView->edge[ VIEW_TOP ];
                            int edge_right = rcView->edge[ VIEW_RIGHT ];
                            int edge_bottom = rcView->edge[ VIEW_BOTTOM ];
                            // draw a gray rect for zero row and zero col
                            RECT RcZeroRow = { edge_left, edge_top, edge_right, edge_top + row_zero_height };
                            FillRect( hdcMem, &RcZeroRow, GetStockObject( LTGRAY_BRUSH ) );
                            RECT RcZeroCol = { edge_left, edge_top, edge_left + col_zero_width, edge_bottom };
                            FillRect( hdcMem, &RcZeroCol, GetStockObject( LTGRAY_BRUSH ) );

                            // draw row number and col number, table lines
                            {
                                // reset font
                                LOGFONT LogFont;
                                HFONT font;
                                memset( &LogFont, 0, sizeof( LOGFONT ) );
                                LogFont.lfStrikeOut = FALSE;
                                LogFont.lfUnderline = FALSE;
                                LogFont.lfHeight = -MulDiv( 12, GetDeviceCaps( hdcMem, LOGPIXELSY ), 72 ) * ratio;//   字体大小
                                LogFont.lfWeight = FW_NORMAL;
                                LogFont.lfItalic = FALSE;
                                font = CreateFontIndirect( &LogFont );
                                HFONT fontold = SelectObject( hdcMem, font );
                                int i;
                                char buf[5];
                                // vertical
                                int x = edge_left + col_zero_width;
                                for ( i = rcView->col_head; i < rcView->col_tail; ++i ) {
                                    itoa( i, buf, 10 );
                                    TextOut( hdcMem, x + 1, edge_top + 1, buf, strlen( buf ) );
                                    if ( showgrid )
                                        draw_vertical_line( hdcMem, x, edge_top, edge_bottom );
                                    x += col_width_of( i ) * ratio;
                                }
                                // horizontal
                                int y = edge_top + row_zero_height;
                                for ( i = rcView->row_head; i < rcView->row_tail; ++i ) {
                                    itoa( i, buf, 10 );
                                    TextOut( hdcMem, edge_left + 1, y + 1, buf, strlen( buf ) );
                                    if ( showgrid )
                                        draw_horizontal_line( hdcMem, y, edge_left, edge_right );
                                    y += row_height_of( i ) * ratio;
                                }
                                DeleteObject( fontold );
                                DeleteObject( font );
                            }

                            // draw texts!
                            register Row* r = table;
                            register Col* c;
                            while ( r != NULL ) {
                                c = r->col_head;
                                while ( c != NULL ) {
                                    if ( c->data != NULL ) {
                                        if ( c->col >= rcView->col_head
                                                && c->col < rcView->col_tail
                                                && r->row >= rcView->row_head
                                                && r->row < rcView->row_tail ) {
                                            int x = 0;
                                            int y = 0;
                                            get_absolute_cell_pos( rcView, r->row, c->col, &x, &y );
                                            RECT textRC = { x + 1, y + 1,
                                                            x + cell_width_of( table, r->row, c->col ) * ratio,
                                                            y + cell_height_of( table, r->row, c->col ) * ratio };
                                            // fill the cell rect background color
                                            HBRUSH brush = CreateSolidBrush( 0x00ffffff );
                                            FillRect( hdcMem, &textRC, brush );
                                            DeleteObject( brush );

                                            if ( c->data->text != NULL ) {
                                                Style* style = &( c->data->style );
                                                SetTextColor( hdcMem, style->text_rgb );
                                                SetBkColor( hdcMem, style->bg_rgb );
                                                // set current font for text
                                                LOGFONT LogFont;
                                                style2logfont( &LogFont, style, hdcMem );
                                                HFONT font = CreateFontIndirect( &LogFont );
                                                HFONT oldfont = SelectObject( hdcMem, font );
                                                // clip rect, not over draw the view
                                                RECT clipRC = textRC;
                                                if ( clipRC.right > edge_right )
                                                    clipRC.right = edge_right;
                                                if ( clipRC.bottom > edge_bottom )
                                                    clipRC.bottom = edge_bottom;
                                                HRGN cliprng = CreateRectRgnIndirect( &clipRC );
                                                SelectClipRgn( hdcMem, cliprng );
                                                // align format
                                                unsigned int format = 0;
                                                switch ( style->text_align ) {
                                                    case ALIGN_LEFT:
                                                        format = DT_LEFT;
                                                        break;
                                                    case ALIGN_RIGHT:
                                                        format = DT_RIGHT;
                                                        break;
                                                    case ALIGN_CENTER:
                                                        format = DT_CENTER;
                                                        break;
                                                    default:
                                                        format = DT_LEFT;
                                                        break;
                                                }
                                                // draw !
                                                DrawText( hdcMem, c->data->text, strlen( c->data->text ), &textRC, format );
                                                DeleteObject( cliprng );
                                                SelectClipRgn( hdcMem, NULL );
                                                DeleteObject( oldfont );
                                                DeleteObject( font );
                                                // reset the colors
                                                SetTextColor( hdcMem, 0x00000000 );
                                                SetBkColor( hdcMem, 0x00ffffff );
                                                ///
                                            }
                                        }
                                    }
                                    c = c->next;
                                }
                                r = r->next;
                            }
                        }
                    }
                }
            }

            HPEN hPen, hPenold;
            // draw the selected view rectangle
            hPen = CreatePen( PS_SOLID, 2, RGB( 185, 20, 205 ) );
            hPenold = SelectObject( hdcMem, hPen );
            draw_rect_border( hdcMem, current_view()->edge[ VIEW_LEFT ], current_view()->edge[ VIEW_TOP ], current_view()->edge[ VIEW_RIGHT ], current_view()->edge[ VIEW_BOTTOM ] );
            DeleteObject( hPenold );
            DeleteObject( hPen );

            // draw the selected cell rectangle
            draw_selection_rect_border( hdcMem );

            // draw a gray rect at the bottom corner for scrollbars
            RECT Rcl = {rcClient.right - 20, rcClient.bottom - 20 - 20 - 30, rcClient.right, rcClient.bottom};
            FillRect( hdcMem, &Rcl, GetStockObject( LTGRAY_BRUSH ) );

            // hdcMem cache to hDC
            BitBlt( hDC, rcClient.left, rcClient.top + 28, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top - 20 - 28, hdcMem, 0, 0, SRCCOPY );
            // free hdcmem and cache bitmap
            HGDIOBJ mmmmm = SelectObject( hdcMem, hOldObject );
            DeleteObject( mmmmm );
            DeleteObject( hBitmap );
            DeleteDC( hdcMem );
            ReleaseDC( hwnd, hdcMem );
            ReleaseDC( hwnd, hDC );

            EndPaint( hwnd, &Ps );
            break;
        }
        case WM_CLOSE: {
            DestroyWindow( hwnd );
            break;
        }
        case WM_DESTROY: {
            PostQuitMessage( 0 );
            break;
        }
        default:
            return DefWindowProc( hwnd, msg, wParam, lParam );
    }
    return 0;
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine, int nCmdShow )
{
    WNDCLASSEX wc;
    MSG Msg;

    //Step 1: Registering the Window Class
    wc.cbSize        = sizeof( WNDCLASSEX );
    wc.style         = 0;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_MYICON ) );
    wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wc.lpszMenuName  = MAKEINTRESOURCE( IDR_MYMENU );
    wc.lpszClassName = g_szClassName;
    wc.hIconSm       = ( HICON )LoadImage( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_MYICON ), IMAGE_ICON, 16, 16, 0 );

    if ( !RegisterClassEx( &wc ) ) {
        MessageBox( NULL, "Window Registration Failed!", "Error!",
                    MB_ICONEXCLAMATION | MB_OK );
        return 0;
    }

    // Step 2: Creating the Window
    hwnd = CreateWindowEx(
               WS_EX_CLIENTEDGE,
               g_szClassName,
               "Oxygen Spread",
               WS_OVERLAPPEDWINDOW,
               CW_USEDEFAULT, CW_USEDEFAULT, 640, 480,
               NULL, NULL, hInstance, NULL );

    if ( hwnd == NULL ) {
        MessageBox( NULL, "Window Creation Failed!", "Error!",
                    MB_ICONEXCLAMATION | MB_OK );
        return 0;
    }

    set_caption( NULL );
    ShowWindow( hwnd, nCmdShow );
    UpdateWindow( hwnd );

    HACCEL hAccel = LoadAcceleratorsW( hInstance, MAKEINTRESOURCEW( ID_ACCEL ) );
    // Step 3: The Message Loop
    while ( GetMessage( &Msg, NULL, 0, 0 ) > 0 ) {
        if ( !TranslateAcceleratorW( hwnd, hAccel, &Msg ) ) {
            TranslateMessage( &Msg );
            DispatchMessage( &Msg );
        }
    }

    free_table( table );
    free_history();
    clear_stream();
    free_view();

    return Msg.wParam;
}
