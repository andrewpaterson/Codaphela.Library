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
#ifndef __TREE_CONTAINER_H__
#define __TREE_CONTAINER_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayInt.h"
#include "SupportLib/ImageCel.h"
#include "ListContainer.h"


#define TREE_NO_NODE	-1


//A node of a tree.  Its children are linked from iFirstChild through their iNextSibling, in the order they were added.
struct STreeNode
{
	int32	iParent;		//TREE_NO_NODE for a node at the top.
	int32	iFirstChild;
	int32	iLastChild;
	int32	iNextSibling;
	int32	iDepth;			//0 at the top.
	int64	iData;			//The user's, to find what the node is of.
	size	uiTextStart;	//In mszTexts.
	size	uiTextLength;
	bool	bExpanded;
};


typedef CArrayTemplate<STreeNode>	CArrayTreeNode;


//A tree, as Solution Explorer's: nodes with a chevron that opens and closes them when they have children, an icon and
//a name, each indented by its depth.  A node's name is in the theme's font unless it is given its own (SetNodeFont),
//like a file's in its version control status's colour.  The rows of the list are the nodes that can be seen: those at the top and those
//in open nodes.  A double click (or enter) on a node with children opens or closes it; on one without, the listeners
//are told it was activated.  A press on the chevron opens or closes its node.
class CTreeContainer : public CListContainer
{
CONSTRUCTABLE(CTreeContainer);
DESTRUCTABLE(CTreeContainer);
protected:
	Ptr<CWindowTheme>	mpTheme;
	CArrayTreeNode		masNodes;
	CChars				mszTexts;
	CArray<CImageCel>	maIcons;		//One for each node, NULL for none.
	CArray<CFont>		maFonts;		//One for each node, NULL for the theme's.
	CArrayInt			maiRowNodes;	//The node of each row.
	CArrayInt			maiTopNodes;	//The nodes at the top, in order.
	Ptr<CImageCel>		mpCollapsedChevron;
	Ptr<CImageCel>		mpExpandedChevron;
	int32				miIndent;		//How far each depth is in.
	int32				miRowHeight;
	bool				mbRowsDirty;	//Nodes changed since the rows were made.

public:
	void			Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme);
	void			Class(void) override;
	void 			Free(void) override;

	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	int32			AddNode(int32 iParent, const char* szText, Ptr<CImageCel> pIcon = NULL, int64 iData = 0);
	void			RemoveAllNodes(void);
	int32			NumNodes(void);
	char*			GetNodeText(int32 iNode, CChars* pszDest);
	int64			GetNodeData(int32 iNode);
	void			SetNodeFont(int32 iNode, Ptr<CFont> pFont);
	Ptr<CFont>		GetNodeFont(int32 iNode);
	int32			GetNodeParent(int32 iNode);
	int32			GetNodeDepth(int32 iNode);
	int32			GetFirstChild(int32 iNode);
	int32			GetNextSibling(int32 iNode);
	bool			HasChildren(int32 iNode);
	int32			FindNode(int32 iParent, const char* szText);

	void			SetExpanded(int32 iNode, bool bExpanded);
	bool			IsExpanded(int32 iNode);
	void			Toggle(int32 iNode);
	void			ExpandAll(void);
	void			CollapseAll(void);

	bool			SelectNode(int32 iNode);
	int32			GetSelectedNode(void);
	int32			GetRowNode(int32 iRow);
	int32			FindNodeRow(int32 iNode);
	void			UpdateRows(void);

	SInt32Vec2		GetDesiredSize(void) override;
	void			Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;
	void			ActivateComponent(int x, int y) override;
	void			Left(void);
	void			Right(void);
	bool			IsOverChevron(int32 iRow, int x);

protected:
	void			RowActivated(int32 iRow) override;
	void			AddVisibleRows(int32 iNode);
	void			AddNodeRow(int32 iNode);
	bool			IsNodeVisible(int32 iNode);
};


#endif // __TREE_CONTAINER_H__

