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
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/FileUtil.h"
#include "ComponentFactory.h"
#include "TextField.h"
#include "Toolbar.h"
#include "Block.h"
#include "Widget.h"
#include "FillContainer.h"
#include "Text.h"
#include "Window.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentFactory::Init(CWorld* pcWorld, CUnknowns* pcUnknowns, CInput* pcInput, CFontList* pcFontList)
{
	CFileUtil	cFileUtil;
	CChars		sz;
	Ptr<CFont>	pFont;

	macComponents.Init();

	mpcWorld = pcWorld;
	mpcUnknowns = pcUnknowns;
	mpcFontList = pcFontList;
	mpcViewport = NULL;

	mpcState0 = CreateState(0);
	mpcState1 = CreateState(1);

	pFont = mpcFontList->CreateFromSystem("MS Sans Serif", 6, 7, FW_NORMAL);
	mpcSansSerif = CreateTextParameters(pFont);
	pFont = mpcFontList->CreateFromSystem("Fixedsys", 0, 0, FW_DONTCARE);
	mpcFixedSys = CreateTextParameters(pFont);
	mpcDefaultFont = mpcFixedSys;

	sz.Init("BlueBorderNormal.bmp");
	cFileUtil.FullPath(&sz);
	mpcButtonBorderNormal = CreateBorderParameters(sz.Text(), 8, 8 ,8);
	mpcButtonBlockNormal = CreateBlockParameters(115, 151, 255, 255);
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentFactory::Kill(void)
{
	int				i;
	CComponent*		pcComponent;

	for (;;)
	{
		i = macComponents.NumElements();
		if (i == 0)
		{
			break;
		}
		pcComponent = (CComponent*)macComponents.Get(i-1);
		if (pcComponent)
		{
			pcComponent->SetParent(NULL);
			pcComponent->Kill();
		}
	}

	macComponents.Kill();

	mpcWorld->ReleaseGraphicsState(mpcState0);
	mpcWorld->ReleaseGraphicsState(mpcState1);

	//mpcButtonBorderNormal->Kill();
	//mpcButtonBlockNormal->Kill();

	mpcSansSerif->Kill();
	mpcFixedSys->Kill();

	mpcWorld = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CComponentFactory::PrivateAllocateComponent(int iType)
{
	return macComponents.Add(iType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CComponentFactory::RemoveComponent(CComponent* pcComponent)
{
	//Caution,  this just removes the component.  It does NOT release it. 
	return macComponents.Remove(pcComponent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTextParameters* CComponentFactory::CreateTextParameters(Ptr<CFont> pFont)
{
	CTextParameters*	pcTextParameters;

	pcTextParameters = mpcUnknowns->Add<CTextParameters>();
	if (pcTextParameters == NULL)
	{
		return NULL;
	}

	pcTextParameters->Init(pFont, mpcState1, mpcWorld);
	return pcTextParameters;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CText* CComponentFactory::CreateText(void)
{
	CText* pcText;

	pcText = mpcUnknowns->Add<CText>();
	pcText->Init(mpcViewport);
	return pcText;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBorderParameters* CComponentFactory::CreateBorderParameters(char* szTexture, int iCornerWidth, int iConerHeight, int iEdgeWidth)
{
	CBorderParameters*	pcBorderParameters;

	pcBorderParameters = mpcUnknowns->Add<CBorderParameters>();
	if (pcBorderParameters == NULL)
	{
		return NULL;
	}

	pcBorderParameters->Init(BORDER_EDGES_CORNERS, BET_Stretch, mpcState1, mpcWorld, szTexture, iCornerWidth, iConerHeight, iEdgeWidth);
	return pcBorderParameters;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBorder* CComponentFactory::CreateBorder(CBorderParameters* pcParameters)
{
	CBorder*	pcBorder;

	if (pcParameters == NULL)
	{
		pcParameters = mpcButtonBorderNormal;
	}

	pcBorder = mpcUnknowns->Add<CBorder>();
	pcBorder->Init(mpcViewport);
	pcBorder->SetBorderParameters(pcParameters);
	return pcBorder;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsState* CComponentFactory::CreateState(int iNumTextures)
{
	CGraphicsState*	mpcState;

	//All alpha values and no z buffer reads/writes.
	mpcState = mpcWorld->CreateGraphicsState();
	mpcState->SetDefaults(iNumTextures, D3DTEXF_POINT);
	return mpcState;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBlockParameters* CComponentFactory::CreateBlockParameters(int r, int g, int b, int a)
{
	CBlockParameters*	pcBlockParameters;

	pcBlockParameters = mpcUnknowns->Add<CBlockParameters>();
	if (pcBlockParameters == NULL)
	{
		return NULL;
	}

	pcBlockParameters->Init(mpcState0, mpcWorld, r, g, b, a);
	return pcBlockParameters;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBlock* CComponentFactory::CreateBlock(CBlockParameters* pcParameters)
{
	CBlock*		pcBlock;

	if (pcParameters == NULL)
	{
		pcParameters = mpcButtonBlockNormal;
	}

	pcBlock = mpcUnknowns->Add<CBlock>();
	pcBlock->Init(mpcViewport, pcParameters);
	return pcBlock;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTextField* CComponentFactory::CreateTextField(char* szString, BOOL bDefaultBorder, BOOL bDefaultBackground)
{
	CTextField*		pcTextField;

	pcTextField = mpcUnknowns->Add<CTextField>();
	pcTextField->Init(mpcViewport, szString, bDefaultBorder, bDefaultBackground);
	return pcTextField;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CToolbar* CComponentFactory::CreateToolbar(void)
{
	CToolbar*	pcToolbar;

	pcToolbar = mpcUnknowns->Add<CToolbar>();
	pcToolbar->Init(mpcViewport);
	return pcToolbar;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CViewport* CComponentFactory::CreateViewport(CInput* pcInput, CRectangle* pcClientRect)
{
	CViewport*			pcViewport;

	pcViewport = mpcUnknowns->Add<CViewport>();
	pcViewport->Init(mpcWorld, pcInput, this, pcClientRect);
	pcViewport->SetActualSize();
	mpcViewport = pcViewport;
	return pcViewport;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CWidget* CComponentFactory::CreateWidget(BOOL bDefaultBorder, BOOL bDefaultBackground)
{
	CWidget*	pcWidget;

    pcWidget = mpcUnknowns->Add<CWidget>();
	pcWidget->Init(mpcViewport, bDefaultBorder, bDefaultBackground);
	return pcWidget;	
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFillContainer* CComponentFactory::CreateFillContainer(void)
{
	CFillContainer*	pcFillContainer;

    pcFillContainer = mpcUnknowns->Add<CFillContainer>();
	pcFillContainer->Init(mpcViewport);
	return pcFillContainer;	
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTextParameters* CComponentFactory::GetDefaultFont(void)
{
	return mpcDefaultFont;
}

