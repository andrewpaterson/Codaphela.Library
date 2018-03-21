/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __COMPONENT_FACTORY_H__
#define __COMPONENT_FACTORY_H__
#include "BaseLib/ArrayVariable.h"
#include "SupportLib/FontList.h"
#include "BorderParameters.h"
#include "BlockParameters.h"
#include "Viewport.h"


class CWorld;
class CTextField;
class CToolbar;
class CButton;
class CBlock;
class CWidget;
class CFillContainer;
class CText;
class CWindow;
class CInput;


class CComponentFactory
{
public:
	CWorld*					mpcWorld;
	CFontList*				mpcFontList;
	CViewport*				mpcViewport;
	CComponentInput			mcInput;
	CArrayVariable			macComponents;

	CTextParameters*		mpcSansSerif;
	CTextParameters*		mpcFixedSys;
	CBorderParameters*		mpcButtonBorderNormal;
	CBlockParameters*		mpcButtonBlockNormal;

	CTextParameters*		mpcDefaultFont;
	CUnknowns*				mpcUnknowns;

	CGraphicsState*			mpcState0;
	CGraphicsState*			mpcState1;

	void 				Init(CWorld* pcWorld, CUnknowns* pcUnknowns, CInput* pcInput, CFontList* pcFontList);
	void 				Kill(void);

	void*				PrivateAllocateComponent(int iType);
	BOOL				RemoveComponent(CComponent* pcComponent);
	CComponent*			GetComponent(int iIndex);

	CGraphicsState*		CreateState(int iNumTextures);

	CTextParameters*	CreateTextParameters(Ptr<CFont> pFont);
	CText*				CreateText(void);
	CTextField*			CreateTextField(char* szString, BOOL bDefaultBorder = TRUE, BOOL bDefaultBackground = TRUE);

	CBorderParameters*	CreateBorderParameters(char* szTexture, int iCornerWidth, int iConerHeight, int iEdgeWidth);
	CBorder*			CreateBorder(CBorderParameters* pcParameters = NULL);

	CBlockParameters*	CreateBlockParameters(int r, int g, int b, int a);
	CBlock*				CreateBlock(CBlockParameters* pcParameters = NULL);

	CToolbar*			CreateToolbar(void);
	CViewport*			CreateViewport(CInput* pcInput, CRectangle* pcClientRect);
	CWidget*			CreateWidget(BOOL bDefaultBorder = TRUE, BOOL bDefaultBackground = TRUE);
	CFillContainer*		CreateFillContainer(void);

	CTextParameters*	GetDefaultFont(void);
};


#endif // __COMPONENT_FACTORY_H__

