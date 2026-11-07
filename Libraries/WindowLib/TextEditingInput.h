/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#ifndef __TEXT_EDITING_INPUT_H__
#define __TEXT_EDITING_INPUT_H__
#include "BaseLib/TextEditing.h"
#include "ComponentInputEvent.h"


class CComponent;


//The input actions that edit a text (CTextArea, CTextField): the keys, given as commands, go to the text's
//CTextEditing; the pointer puts the caret and selects; cut, copy and paste use the window's clipboard.  A single line
//text has no up and down, pages, tabs, line feeds or line commands.  The pointer's actions can be turned off (a text
//that can't be selected), so its presses go on to the parent.
class CTextEditingInput
{
protected:
	CComponent*		mpcComponent;  //Not held: the text area or field the input reaches.
	CTextEditing*	mpcEditing;  //Not held.
	bool			mbDragging;

public:
	void	Init(CComponent* pcComponent, CTextEditing* pcEditing, bool bMultiLine);
	void	Kill(void);

	void	SetPointerEnabled(bool bEnabled);
	void	FocusLost(void);

	void	PrintableAction(SComponentInputEvent* psEvent);
	void	SpaceAction(SComponentInputEvent* psEvent);
	void	TabAction(SComponentInputEvent* psEvent);
	void	UndoAction(void);
	void	RedoAction(void);
	void	CutToClipboard(void);
	void	CopyToClipboard(void);
	void	PasteFromClipboard(void);

	void	ActivateComponent(int x, int y);
	void	ExecuteComponent(int x, int y);
	void	ContextMenuClick(SComponentInputEvent* psEvent);
	void	StartDragSelect(int x, int y);
	void	ContDragSelect(int x, int y);
	void	StopDragSelect(int x, int y);
};


#endif // __TEXT_EDITING_INPUT_H__

