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
#include "BaseLib/ArrayChars.h"
#include "SupportLib/ImageCel.h"
#include "ListContainer.h"
#include "TreeRow.h"


#define TREE_NODE_FONT	((size)-1)


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
	size	uiFirstRange;	//In masRanges, if uiNumRanges isn't 0.
	size	uiNumRanges;
	int32	iWidth;			//What its row needs across, -1 until it is worked out (GetNodeWidth).
	bool	bExpanded;
};


//A part of a node's text in a font of its own (AddNodeRange): its start in the text, its length and its font's index.
struct STreeNodeRange
{
	size	uiStart;
	size	uiLength;
	size	uiFont;
};


//A piece of a node's name, as NextNodePiece walks them: a range, or the text between two.
struct STreeNodePiece
{
	size	uiStart;		//In the node's text.
	size	uiLength;
	size	uiFont;			//The range's font's index, TREE_NODE_FONT for the node's own.
	size	uiNextRange;	//The node's range after the piece.

	void	Init(void);
};


class CTreeContainer;
typedef CComponentValueEvent<CTreeContainer, int32>	CTreeNodeEvent;  //A node, by its index.


typedef CArrayTemplate<STreeNode>			CArrayTreeNode;
typedef CArrayTemplate<STreeNodeRange>	CArrayTreeNodeRange;


//A tree, as Solution Explorer's: nodes with a chevron that opens and closes them when they have children, an icon and
//a name, each indented by its depth.  A node's name is in the theme's font unless it is given its own (SetNodeFont),
//like a file's in its version control status's colour.  Parts of a name can each have a font of their own
//(AddNodeRange), as a found line's has its line number dim and the text found bold.  The rows of the list are the nodes that can be seen: those at the top and those
//in open nodes.  A double click (or enter) on a node with children opens or closes it; on one without, the listeners
//are told it was activated.  A press on the chevron opens or closes its node.  Whoever wants to know that a node was
//opened (to find its children only then, say) hears the expanded event; ExpandAll doesn't call it.  A tree whose
//nodes with children are things to open in themselves (a class with its derived classes under it) can leave the
//opening and closing to the chevron and the left and right keys (SetActivationToggles(false)).
//
//The rows are all one height, so where each is is worked out, not laid out: row i is i rows down.  The tree only has
//components (CTreeRow) for the rows in view, as many as the scroll pane it is in shows, and as it is scrolled each is
//given the node that came into view (LayoutRows).  A tree of a hundred thousand open nodes has as few components as
//one of a hundred.  How wide the tree is comes from its nodes' texts and fonts, kept for each node (GetNodeWidth).
//
//A tree can have columns after its nodes' names (SetNumColumns, before any node is added), as a tree table's Value and
//Type: each node has a cell in each, text in a font of its own if it is given one (SetCellFont).  Where the columns
//are is its owner's (SetColumnBounds); the name is cut off at the first's left, and the tree is as wide as its columns.
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
	CArrayTreeNodeRange	masRanges;		//The nodes' ranges, each node's together in the order of its text.
	CArray<CFont>		maRangeFonts;	//The ranges' fonts, each once.
	CArrayInt			maiRowNodes;	//The node of each row.
	CArrayInt			maiTopNodes;	//The nodes at the top, in order.
	size				muiNumColumns;	//After the name.
	CArrayChars			maszCells;		//Each node's cells, node by node.
	CArray<CFont>		maCellFonts;	//The same, NULL for the theme's font.
	CArrayInt			maiColumnLefts;	//Each column's left and width, from a row's left.
	CArrayInt			maiColumnWidths;
	int32				miNameWidth;	//Where the name is cut off, 0 when it isn't (there are no columns).
	CArray<CTreeRow>	maRows;			//The rows in view's components: row i's is the one at i modulo how many there are.
	SInt32Vec2			msRowsOrigin;	//Where the first row was laid out, in window client space.
	int32				miRowsWidth;	//What the widest row needs across.
	int32				miIndent;		//How far each depth is in.
	int32				miRowHeight;
	bool				mbRowsDirty;	//Nodes changed since the rows were made.
	bool				mbRowsBound;	//The rows' components show their nodes as they are now.
	bool				mbLaidOut;
	CTreeNodeEvent		mcExpanded;		//Called when a node is opened (SetExpanded, a chevron's press), with the node.
	bool				mbActivationToggles;	//A double click or enter on a node with children opens or closes it.

public:
	void			Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme);
	void			Class(void) override;
	void			Free(void) override;

	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	int32			AddNode(int32 iParent, const char* szText, Ptr<CImageCel> pIcon = NULL, int64 iData = 0);
	void			RemoveAllNodes(void);
	int32			NumNodes(void) const;
	char*			GetNodeText(int32 iNode, CChars* pszDest);
	int64			GetNodeData(int32 iNode);
	void			SetNodeFont(int32 iNode, Ptr<CFont> pFont);
	Ptr<CFont>		GetNodeFont(int32 iNode);
	Ptr<CImageCel>	GetNodeIcon(int32 iNode);
	bool			AddNodeRange(int32 iNode, size uiStart, size uiLength, Ptr<CFont> pFont);
	size			NumNodeRanges(int32 iNode);
	bool			GetNodeRange(int32 iNode, size uiIndex, size* puiStart, size* puiLength);
	Ptr<CFont>		GetNodeRangeFont(int32 iNode, size uiIndex);
	int32			GetNodeParent(int32 iNode);
	int32			GetNodeDepth(int32 iNode);
	int32			GetNameLeft(int32 iNode);
	int32			GetFirstChild(int32 iNode);
	int32			GetNextSibling(int32 iNode);
	bool			HasChildren(int32 iNode);
	int32			FindNode(int32 iParent, const char* szText);

	bool			SetNumColumns(size uiNumColumns);
	size			NumColumns(void) const;
	void			SetColumnBounds(int32 iNameWidth, CArrayInt* paiLefts, CArrayInt* paiWidths);
	void			SetCellText(int32 iNode, size uiColumn, const char* szText);
	const char*		GetCellText(int32 iNode, size uiColumn);
	void			SetCellFont(int32 iNode, size uiColumn, Ptr<CFont> pFont);
	Ptr<CFont>		GetCellFont(int32 iNode, size uiColumn);

	void			SetExpanded(int32 iNode, bool bExpanded);
	bool			IsExpanded(int32 iNode);
	void			Toggle(int32 iNode);
	void			ExpandAll(void);
	void			CollapseAll(void);
	CTreeNodeEvent*	GetExpandedEvent(void);
	void			SetActivationToggles(bool bToggles);
	bool			IsActivationToggles(void) const;

	bool			SelectNode(int32 iNode);
	int32			GetSelectedNode(void);
	int32			GetRowNode(int32 iRow);
	int32			FindNodeRow(int32 iNode);
	void			UpdateRows(void);

	SInt32Vec2		GetDesiredSize(void) override;
	void			SetRequiredSize(void) override;
	void			Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;
	bool			GetRowBounds(int32 iRow, int32* piTop, int32* piHeight) override;
	bool			GetRowText(int32 iRow, CChars* pszDest) override;
	size			NumRowComponents(void);
	Ptr<CTreeRow>	GetRowComponent(int32 iRow);
	void			ActivateComponent(int x, int y) override;
	void			Left(void);
	void			Right(void);
	bool			IsOverChevron(int32 iRow, int x);

protected:
	void			RowActivated(int32 iRow) override;
	int32			FindRowAtY(int32 y) override;
	void			AddVisibleRows(int32 iNode);
	bool			IsNodeVisible(int32 iNode);
	bool			NextNodePiece(STreeNode* psNode, STreeNodePiece* psPiece);
	Ptr<CFont>		GetPieceFont(int32 iNode, STreeNodePiece* psPiece);
	int32			GetNodeWidth(int32 iNode);
	void			GetRowsInView(SInt32Vec2 sOrigin, SInt32Vec2 sArea, int32* piFirst, int32* piNum);
	void			LayoutRows(SInt32Vec2 sOrigin, SInt32Vec2 sArea);
	void			BindRow(Ptr<CTreeRow> pRow, int32 iNode);
	void			CellChanged(int32 iNode);
};


#endif // __TREE_CONTAINER_H__

