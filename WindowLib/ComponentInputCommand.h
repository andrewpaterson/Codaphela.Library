/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __COMPONENT_INPUT_COMMAND_H__
#define __COMPONENT_INPUT_COMMAND_H__
#include "BaseLib/ArrayTemplate.h"


enum EComponentInputCommand
{
	CIC_Ignored,

	//Navigation
	CIC_Up,
	CIC_Down,
	CIC_Left,
	CIC_Right,
	CIC_WordLeft,
	CIC_WordRight,
	CIC_Home,
	CIC_End,
	CIC_DocumentHome,
	CIC_DocumentEnd,
	CIC_PageUp,
	CIC_PageDown,
	CIC_LineUp,
	CIC_LineDown,

	//Selection
	CIC_SelectUp,
	CIC_SelectDown,
	CIC_SelectLeft,
	CIC_SelectRight,
	CIC_SelectWordLeft,
	CIC_SelectWordRight,
	CIC_SelectHome,
	CIC_SelectEnd,
	CIC_SelectDocumentHome,
	CIC_SelectDocumentEnd,
	CIC_SelectPageUp,
	CIC_SelectPageDown,
	CIC_SelectWholeWord,
	CIC_SelectAll,
	CIC_ClearSelection,

	//Modification
	CIC_Space,
	CIC_Tab,
	CIC_BackTab,
	CIC_Enter,
	CIC_HomeEnter,
	CIC_EndEnter,

	//Modification
	CIC_Delete,
	CIC_Backspace,
	CIC_Cut,
	CIC_Copy,
	CIC_Paste,
	CIC_CutLine,
	CIC_Duplicate,
	CIC_DeleteWordRemainingLeft,
	CIC_DeleteWordRemainingRight,
	CIC_DeleteLine,

	//Components
	CIC_ActivateComponent,
	CIC_ExecuteComponent,
	CIC_ContextMenu,
	CIC_StartDragScroll,
	CIC_StopDragScroll,
	CIC_StartDragSelect,
	CIC_StopDragSelect,
	CIC_Hover,
};


class CInputSourceDesc;
class CComponentInputCommand
{
public:
	EComponentInputCommand		eCommand;
	CInputSourceDesc*			pcSource;
	BOOL						bPressed;
	BOOL						bControl;
	BOOL						bAlt;
	BOOL						bShift;

	void Init(EComponentInputCommand eCommand, CInputSourceDesc* pcSource, BOOL bControl, BOOL bAlt, BOOL bShift, BOOL bPressed);
	void Kill(void);
};


typedef CArrayTemplate<CComponentInputCommand>	CArrayComponentInputCommand;



#endif // __COMPONENT_INPUT_COMMAND_H__

