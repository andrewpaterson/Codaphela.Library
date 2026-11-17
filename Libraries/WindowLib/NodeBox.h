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
#ifndef __NODE_BOX_H__
#define __NODE_BOX_H__
#include "BaseLib/ArrayTemplate.h"
#include "Canvas.h"
#include "CelCanvasDraw.h"
#include "FillContainer.h"
#include "GridContainer.h"
#include "Label.h"
#include "WindowTheme.h"


#define NODE_BOX_SOCKET_RADIUS	5		//Of a socket's dot, which sits on the box's edge.
#define NODE_BOX_CORNER_RADIUS	5		//Of the box's corners.
#define NODE_BOX_WIDTH			190		//Unless a desired width is set.
#define NODE_BOX_PADDING		4		//Between the box's edge and its rows, and between a row and the next.


class CNodeBox;


//A socket of a node box: where a link ends.
struct SNodeBoxSocket
{
	bool			bInput;
	ARGB32			sColour;
	int32			iRow;  //Of the box's grid.
	CComponent*		pcName;  //The row's label.  The box's grid holds it, or the box while a value is shown in its place.
	CComponent*		pcValue;  //An input's value, shown in place of the name while nothing is linked to it: NULL for none.
	bool			bLinked;
};

typedef CArrayTemplate<SNodeBoxSocket>	CArrayNodeBoxSocket;


//Paints a node box's back: its body with rounded corners and a border (the accent colour when it's selected), the header
//behind its title, and its sockets' dots on its edges, inputs on the left and outputs on the right.
class CNodeBoxDraw : public CCelCanvasDraw
{
CONSTRUCTABLE(CNodeBoxDraw);
DESTRUCTABLE(CNodeBoxDraw);
protected:
	CNodeBox*	mpcBox;  //Not held: it holds this.

public:
	void	Init(CNodeBox* pcBox);
	void	Class(void) override;

protected:
	void	Paint(CImageDraw* pcDraw, SInt32Vec2 sSize) override;
};


//A node of a node editor (CNodeEditor), as Blender's nodes are: a box with its title in a header and rows under it, added
//top to bottom.  Blender's order is its outputs, then its settings, then its inputs.
//
//An output's row is its name on the right, by its socket on the box's right edge; an input's is its name on the left, by
//its socket on the left edge.  An input may have a value (AddInput's component, a number field say): it's shown in place
//of the name while nothing is linked to the input (SetInputLinked, which the editor calls).  A setting's row is any
//component, as wide as the box.
//
//The box is as tall as its rows need.  Where it is in the editor (SetNodePosition) is the editor's to lay out.  What it
//stands for is whoever made it's to say (SetData).
class CNodeBox : public CFillContainer
{
CONSTRUCTABLE(CNodeBox);
DESTRUCTABLE(CNodeBox);
protected:
	Ptr<CWindowTheme>		mpTheme;
	Ptr<CNodeBoxDraw>		mpDraw;
	Ptr<CCanvas>			mpBack;
	Ptr<CGridContainer>		mpRows;
	Ptr<CLabel>				mpTitle;
	CArray<CComponent>		mapShelved;  //The names and values of inputs that aren't in the grid just now, kept while the other shows.
	CArrayNodeBoxSocket		masSockets;
	ARGB32					msHeader;
	int32					miNumRows;
	SInt32Vec2				msNodePosition;  //In the editor's space, its top left.
	bool					mbSelected;
	int64					miData;

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, const char* szTitle, ARGB32 sHeader);
	void				Class(void) override;
	void				Free(void) override;

	size				AddOutput(const char* szName, ARGB32 sColour);
	size				AddInput(const char* szName, ARGB32 sColour, Ptr<CComponent> pValue = NULL);
	void				AddRow(Ptr<CComponent> pComponent);

	size				NumSockets(bool bInput);
	SNodeBoxSocket*		GetSocket(bool bInput, size uiIndex);
	SInt32Vec2			GetSocketPosition(bool bInput, size uiIndex);
	bool				FindSocketAt(int32 x, int32 y, bool* pbInput, size* puiIndex);
	void				SetInputLinked(size uiIndex, bool bLinked);
	bool				IsInputLinked(size uiIndex);

	char*				GetTitle(void);
	ARGB32				GetHeaderColour(void);
	int32				GetHeaderHeight(void);
	Ptr<CWindowTheme>	GetTheme(void);

	void				SetNodePosition(int32 x, int32 y);
	SInt32Vec2			GetNodePosition(void);
	void				SetSelected(bool bSelected);
	bool				IsSelected(void);
	void				SetData(int64 iData);
	int64				GetData(void);

protected:
	size				AddSocket(bool bInput, const char* szName, ARGB32 sColour, Ptr<CComponent> pValue);
	void				ShowInput(SNodeBoxSocket* psSocket);
	void				Changed(void);
};


#endif // __NODE_BOX_H__

