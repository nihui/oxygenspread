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

#ifndef MYMENU_H
#define MYMENU_H

void MYMENU_FileNew();
void MYMENU_FileOpen();
void MYMENU_FileSave();
void MYMENU_FileSaveAs();
void MYMENU_FileClose();
void MYMENU_FileExit();

void MYMENU_EditUndo();
void MYMENU_EditRedo();
void MYMENU_EditCut();
void MYMENU_EditCopy();
void MYMENU_EditPaste();
void MYMENU_EditDelete();
void MYMENU_EditSelectAll();
void MYMENU_EditTimeDate();

void MYMENU_ViewShowGrid();
void MYMENU_ViewSplitView();

void MYMENU_FormatFont();
void MYMENU_FormatBGColor();
void MYMENU_FormatCombineCells();
void MYMENU_FormatSplitCells();
void MYMENU_FormatAlignLeft();
void MYMENU_FormatAlignRight();
void MYMENU_FormatAlignCenter();
void MYMENU_FormatClearStyle();

void MYMENU_HelpAbout();

#endif // MYMENU_H
