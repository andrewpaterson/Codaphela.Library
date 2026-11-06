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


//What a key, chord or pointer button means to a component.  CFocus gives each one to the input actions (IAF_Command) of
//the focussed component and its parents.
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
	CIC_Undo,
	CIC_StepUndo,
	CIC_Redo,
	CIC_StepRedo,
	CIC_FlipHorizontally,  //Control+Shift+F, then H.
	CIC_FlipVertically,  //Control+Shift+F, then V.

	//Files
	CIC_Save,  //Control+S.
	CIC_SaveAll,  //Control+Shift+S.

	//Differences
	CIC_NextDifference,  //F7.
	CIC_PreviousDifference,  //Shift+F7.

	//Code
	CIC_GoToDefinition,  //F12.
	CIC_FindReferences,  //Shift+F12.
	CIC_Build,  //Control+Shift+B.
	CIC_GoToClass,  //Control+N, as IntelliJ's.
	CIC_GoToFile,  //Control+Shift+N.
	CIC_GoToSymbol,  //Control+Alt+Shift+N.
	CIC_ShowProjectView,  //Alt+1, as IntelliJ's Project tool window: the active file selected in it.
	CIC_Rename,  //Shift+F6, as IntelliJ's Rename.

	//Debugging, with Visual Studio's keys.
	CIC_StartDebugging,  //F5: start, or continue.
	CIC_StopDebugging,  //Shift+F5.
	CIC_RestartDebugging,  //Control+Shift+F5.
	CIC_BreakAll,  //Control+Pause (Control+Break).
	CIC_ToggleBreakpoint,  //F9.
	CIC_StepOver,  //F10.
	CIC_StepInto,  //F11.
	CIC_StepOut,  //Shift+F11.

	//Components
	CIC_ActivateComponent,
	CIC_ExecuteComponent,
	CIC_ContextMenu,  //The right button: a click, so the component under the pointer is focussed first.
	CIC_ContextMenuKey,  //The Menu key or Shift+F10: for the focussed component.
	CIC_CloseDocument,  //Control+F4: the active document closes.
	CIC_HideToolWindow,  //Shift+Escape: the active tool window is hidden.
	CIC_StartDragScroll,
	CIC_ContDragScroll,  //The pointer moved during a drag; only CFocus gives these, to the drag component.
	CIC_StopDragScroll,
	CIC_StartDragSelect,
	CIC_ContDragSelect,
	CIC_StopDragSelect,
	CIC_Hover,
	CIC_WheelUp,  //The wheel goes to the component under the pointer, not the focussed one.
	CIC_WheelDown,
	CIC_WheelLeft,
	CIC_WheelRight,
};


//The command a source gives when it is pressed, or when it is released; a device keeps them apart.
class CInputSourceDesc;
class CComponentInputCommand
{
public:
	EComponentInputCommand		meCommand;
	CInputSourceDesc*			mpcSource;

	void Init(EComponentInputCommand eCommand, CInputSourceDesc* pcSource);
	void Kill(void);
};


typedef CArrayTemplate<CComponentInputCommand>	CArrayComponentInputCommand;


#endif // __COMPONENT_INPUT_COMMAND_H__

