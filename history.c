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

#include "cellinfo.h"
#include "table.h"
#include "history.h"

extern MyTable table;

typedef struct History
{
    unsigned int row;
    unsigned int col;
    CellInfo* orig_info;
    struct History* next;
} History;

static History* undo_head = NULL;
static History* redo_head = NULL;

extern int is_undoable()
{
    return undo_head != NULL;
}

extern int is_redoable()
{
    return redo_head != NULL;
}

static void free_undo_history()
{
    if ( undo_head != NULL ) {
        register History* node = undo_head;
        register History* node_next;
        while ( node != NULL ) {
            node_next = node->next;
            if ( node->orig_info != NULL )
                free_cell_info( node->orig_info );
            free( node );
            node = node_next;
        }
        undo_head = NULL;
    }
}

extern void free_redo_history()
{
    if ( redo_head != NULL ) {
        register History* node = redo_head;
        register History* node_next;
        while ( node != NULL ) {
            node_next = node->next;
            if ( node->orig_info != NULL )
                free_cell_info( node->orig_info );
            free( node );
            node = node_next;
        }
        redo_head = NULL;
    }
}

extern void free_history()
{
    free_undo_history();
    free_redo_history();
}

extern void history_append( unsigned int row, unsigned int col )
{
    CellInfo* orig_info = get_cell_info( table, row, col );
    if ( undo_head == NULL ) {
        // initialize the undo_head
        undo_head = malloc( sizeof(History) );
        undo_head->row = row;
        undo_head->col = col;
        undo_head->orig_info = clone_cell_info( orig_info );
        undo_head->next = NULL;
    }
    else {
        // append one as the undo_head
        History* node = malloc( sizeof(History) );
        node->row = row;
        node->col = col;
        node->orig_info = clone_cell_info( orig_info );
        node->next = undo_head;
        undo_head = node;
    }
}

static void history_redo_append( unsigned int row, unsigned int col )
{
    CellInfo* orig_info = get_cell_info( table, row, col );
    if ( redo_head == NULL ) {
        // initialize the redo_head
        redo_head = malloc( sizeof(History) );
        redo_head->row = row;
        redo_head->col = col;
        redo_head->orig_info = clone_cell_info( orig_info );
        redo_head->next = NULL;
    }
    else {
        // append one as the redo_head
        History* node = malloc( sizeof(History) );
        node->row = row;
        node->col = col;
        node->orig_info = clone_cell_info( orig_info );
        node->next = redo_head;
        redo_head = node;
    }
}

extern void history_undo()
{
    if ( undo_head == NULL )
        return;
    History* undo_node = undo_head;
    // get the orig for redo history
    history_redo_append( undo_node->row, undo_node->col );

    // undo action
    insert_cell_info( table, undo_node->row, undo_node->col, undo_node->orig_info );
    undo_head = undo_node->next;
    free( undo_node );
    undo_node = NULL;
}

extern void history_redo()
{
    if ( redo_head == NULL )
        return;
    History* redo_node = redo_head;
    // get the orig for undo history
    history_append( redo_node->row, redo_node->col );

    // redo action
    insert_cell_info( table, redo_node->row, redo_node->col, redo_node->orig_info );
    redo_head = redo_node->next;
    free( redo_node );
    redo_node = NULL;
}
