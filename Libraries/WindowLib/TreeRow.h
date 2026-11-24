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
#ifndef __TREE_ROW_H__
#define __TREE_ROW_H__
#include "BaseLib/ArrayInt.h"
#include "StandardLib/Array.h"
#include "SupportLib/ImageCel.h"
#include "Container.h"
#include "IconComponent.h"
#include "Label.h"


#define TREE_NO_NODE	-1


//What shows one row of a tree: the chevron (a gap for a node without children), the icon if the node has one, and
//the name in a label, or with parts in fonts of their own a label for each part.  They are laid out from the left,
//the chevron as far in as the node is deep, each across the row's middle.
//
//A tree only has rows for the nodes in view (see CTreeContainer::LayoutRows), and gives each another node to show as
//it is scrolled: the row keeps its components, and only what differs from the node it showed is changed.  A row
//showing no node isn't drawn.
//
//A tree with columns (CTreeContainer::SetNumColumns) has a label for each of the node's cells too, each at its
//column's place in the row and cut off at its width, as the name is at the name column's (SetColumns).
class CTreeRow : public CContainer
{
CONSTRUCTABLE(CTreeRow);
DESTRUCTABLE(CTreeRow);
protected:
	Ptr<CIconComponent>	mpChevron;
	Ptr<CIconComponent>	mpIcon;
	CArray<CLabel>		maLabels;		//As many as any node shown has needed: the first muiNumLabels are the node's.
	size				muiNumLabels;
	CArray<CLabel>		maCells;		//As many as the tree has columns after the name.
	CArrayInt			maiCellLefts;	//Each cell's place in the row and its width.
	CArrayInt			maiCellWidths;
	int32				miNameWidth;	//0 when the name isn't cut off.
	int32				miNode;			//The node shown, TREE_NO_NODE for none.
	int32				miIndent;		//How far in the chevron is.
	int32				miChevronWidth;
	int32				miIconWidth;
	int32				miRowHeight;
	bool				mbChevron;		//The node has children.
	bool				mbIcon;

public:
	void			Init(Ptr<CWindow> pWindow, int32 iChevronWidth, int32 iIconWidth, int32 iRowHeight);
	void			Class(void) override;
	void			Free(void) override;

	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	void			SetNode(int32 iNode, int32 iIndent);
	int32			GetNode(void) const;
	void			SetChevron(Ptr<CImageCel> pCel);
	void			SetIcon(Ptr<CImageCel> pCel);
	void			SetLabel(size uiIndex, Ptr<CFont> pFont, const char* szText);
	void			SetNumLabels(size uiNumLabels);
	size			NumLabels(void) const;
	Ptr<CLabel>		GetLabel(size uiIndex);
	void			SetCell(size uiIndex, Ptr<CFont> pFont, const char* szText);
	Ptr<CLabel>		GetCell(size uiIndex);
	void			SetColumns(int32 iNameWidth, CArrayInt* paiLefts, CArrayInt* paiWidths);
	bool			HasChevron(void) const;
	bool			HasIcon(void) const;

	void			SetRequiredSize(void) override;
	void			Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;
	bool			IsDrawn(void) const override;
	bool			IsChildDrawn(Ptr<CComponent> pChild) override;

protected:
	void			SetLabelIn(CArray<CLabel>* paLabels, size uiIndex, Ptr<CFont> pFont, const char* szText);
	void			LayoutLabel(Ptr<CLabel> pLabel, SInt32Vec2 sPosition, int32 iHeight, int32 iMostWidth);
};


#endif // __TREE_ROW_H__

