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
#ifndef __NODE_EDITOR_H__
#define __NODE_EDITOR_H__
#include "StandardLib/Array.h"
#include "Container.h"
#include "NodeBox.h"


#define NODE_EDITOR_LINK_WIDTH		2.5		//Pixels.
#define NODE_EDITOR_LINK_REACH		40.0	//How far along a link's ends its curve's controls are at the least, so a link going back on itself loops out and round.
#define NODE_EDITOR_LINK_STEPS		24		//Straight strokes a link's curve is drawn as.
#define NODE_EDITOR_FRAME_MARGIN	30		//Round the nodes, when they're framed.


class CNodeEditor;


//A link: from a node's output to a node's input.  The nodes are the editor's, which holds them.
struct SNodeLink
{
	CNodeBox*	pcFrom;
	size		uiOutput;
	CNodeBox*	pcTo;
	size		uiInput;
};

typedef CArrayTemplate<SNodeLink>	CArrayNodeLink;


//What a node editor asks and tells whoever's nodes they are.  Each is called for what's done with the pointer and keys,
//not for what the editor's own functions are called to do.
class CNodeEditorListener
{
public:
	virtual bool	CanLink(CNodeBox* pcFrom, size uiOutput, CNodeBox* pcTo, size uiInput);  //True unless overridden.
	virtual void	Linked(CNodeBox* pcFrom, size uiOutput, CNodeBox* pcTo, size uiInput);
	virtual void	Unlinked(CNodeBox* pcFrom, size uiOutput, CNodeBox* pcTo, size uiInput);
	virtual void	NodesMoved(void);  //The selected nodes, once they're let go.
	virtual void	DeleteSelected(void);  //X or Delete: the editor removes nothing itself.
	virtual void	DuplicateSelected(void);  //Shift+D.
	virtual void	AddNode(int32 x, int32 y);  //Shift+A: x and y are the pointer, in the editor's space.
};


//What the pointer is dragging.
enum ENodeEditorDrag
{
	NED_None,
	NED_Nodes,	//The selected nodes.
	NED_Box,	//A box that selects the nodes it touches.
	NED_Link,	//A link, from the socket it was pressed on.
};


//Paints a node editor's back: its background, and its links over that, each a level cubic (VectorLevelCubic) in its
//output's colour; then the link being dragged and the box being selected with, if there's one.
class CNodeEditorDraw : public CCelCanvasDraw
{
CONSTRUCTABLE(CNodeEditorDraw);
DESTRUCTABLE(CNodeEditorDraw);
protected:
	CNodeEditor*	mpcEditor;  //Not held: it holds this.

public:
	void	Init(CNodeEditor* pcEditor);
	void	Class(void) override;

protected:
	void	Paint(CImageDraw* pcDraw, SInt32Vec2 sSize) override;
	void	PaintLink(CImageDraw* pcDraw, SInt32Vec2 sFrom, SInt32Vec2 sTo, ARGB32 sColour);
};


//A node editor, as Blender's: nodes (CNodeBox) laid where each says it is, joined by links from outputs to inputs, drawn
//behind them.  It knows nothing of what the nodes stand for: whoever does adds them and their links, and is asked and
//told of what's done to them through a CNodeEditorListener.
//
//With the pointer, the left button:
//  on a socket          drags a link from it.  Let go over a socket of the other kind on another node, the link is made if
//                       the listener allows it, in place of any the input had.  An input that has a link is taken hold of
//                       by that link's end: let go over nothing, the link is removed.
//  on a node            selects it (with Shift, as well as the others, or not if it was), and drags the selected nodes.
//  on the background    selects nothing, and drags a box: the nodes it touches are selected.
//The middle button's drag pans, and Home frames all the nodes.  X or Delete, Shift+D and Shift+A ask the listener to
//delete, to duplicate and to add.
//
//An output feeds any number of inputs; an input takes one link.  A node's settings are its own components, which take
//the pointer themselves.
class CNodeEditor : public CContainer
{
CONSTRUCTABLE(CNodeEditor);
DESTRUCTABLE(CNodeEditor);
protected:
	Ptr<CWindowTheme>		mpTheme;
	Ptr<CNodeEditorDraw>	mpDraw;
	Ptr<CCanvas>			mpBack;
	CArray<CNodeBox>		mapNodes;
	CArrayNodeLink			masLinks;
	CNodeEditorListener*	mpcListener;  //Not held.  NULL for none.
	SInt32Vec2				msPan;  //Where the editor's space's origin is, from its top left.
	ENodeEditorDrag			meDrag;
	SInt32Vec2				msPressed;  //Where the drag started, in the editor.
	SInt32Vec2				msPointer;  //Where the pointer was last, in the editor.
	bool					mbDragged;  //The pointer has moved since it was pressed.
	CNodeBox*				mpcLinkNode;  //The socket a link is dragged from.
	bool					mbLinkInput;
	size					muiLinkSocket;
	bool					mbPanning;  //The middle button is down on it.
	bool					mbFrame;  //The nodes are framed when it's next laid out: it was asked to before it had a size.

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme);
	void				Class(void) override;
	void				Free(void) override;

	void				Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;
	void				SetRequiredSize(void) override;

	void				SetListener(CNodeEditorListener* pcListener);
	Ptr<CWindowTheme>	GetTheme(void) const;

	void				AddNode(Ptr<CNodeBox> pNode);
	bool				RemoveNode(Ptr<CNodeBox> pNode);
	void				RemoveAllNodes(void);
	size				NumNodes(void);
	Ptr<CNodeBox>		GetNode(size uiIndex);
	Ptr<CNodeBox>		FindNodeAt(int32 x, int32 y);

	bool				Link(CNodeBox* pcFrom, size uiOutput, CNodeBox* pcTo, size uiInput);
	bool				Unlink(CNodeBox* pcTo, size uiInput);
	size				NumLinks(void) const;
	SNodeLink*			GetLink(size uiIndex);
	SNodeLink*			FindLinkTo(CNodeBox* pcTo, size uiInput);

	void				Select(Ptr<CNodeBox> pNode, bool bAdd);
	void				SelectNone(void);
	size				NumSelected(void);

	SInt32Vec2			GetPan(void) const;
	void				SetPan(int32 x, int32 y);
	void				FrameAll(void);

	ENodeEditorDrag		GetDrag(void) const;
	bool				GetDraggedLink(SInt32Vec2* psFrom, SInt32Vec2* psTo, ARGB32* psColour);
	bool				GetDraggedBox(CInt32Rect* pcDest);

	void				ActivateComponent(int x, int y);
	void				ExecuteComponent(int x, int y);
	void				ContDragSelect(int x, int y);
	void				StopDragSelect(int x, int y);
	void				MovementAction(SComponentInputEvent* psEvent);
	void				PanPressAction(SComponentInputEvent* psEvent);
	void				PanReleaseAction(SComponentInputEvent* psEvent);
	void				HomeAction(SComponentInputEvent* psEvent);
	void				DeleteAction(SComponentInputEvent* psEvent);
	void				DuplicateAction(SComponentInputEvent* psEvent);
	void				AddAction(SComponentInputEvent* psEvent);

protected:
	void				Changed(void);
	void				DropLink(int x, int y);
	void				Frame(SInt32Vec2 sSize);
	void				SelectBox(void);
	void				RemoveLinksOf(CNodeBox* pcNode);
	bool				IsShift(void);
};


#endif // __NODE_EDITOR_H__

