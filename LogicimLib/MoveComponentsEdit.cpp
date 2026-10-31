/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LogicimLib

Codaphela LogicimLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LogicimLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LogicimLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "CircuitEditor.h"
#include "MoveComponentsEdit.h"


//////////////////////////////////////////////////////////////////////////
//
// One component, left unselected when done.
//
//////////////////////////////////////////////////////////////////////////
void CMoveComponentsEdit::Init(CComponentView* pcComponent, bool bNew, bool bConnected)
{
	InitMove(bNew, false, bConnected);
	AddComponent(pcComponent);
}


//////////////////////////////////////////////////////////////////////////
//
// Views selected when done.
//
//////////////////////////////////////////////////////////////////////////
void CMoveComponentsEdit::Init(CArrayComponentViewPtr* papcComponents, CArrayTraceViewPtr* papcTraces, bool bNew, bool bConnected)
{
	size			i;

	InitMove(bNew, true, bConnected);
	for (i = 0; i < papcComponents->NumElements(); i++)
	{
		AddComponent(papcComponents->GetPtr(i));
	}
	for (i = 0; i < papcTraces->NumElements(); i++)
	{
		AddTrace(papcTraces->GetPtr(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMoveComponentsEdit::InitMove(bool bNew, bool bSelect, bool bConnected)
{
	masComponents.Init();
	masTraces.Init();
	mbNew = bNew;
	mbSelect = bSelect;
	mbConnected = bConnected;
	msLastDiff.Init(0, 0);
	miLastRightRotations = 0;
	mbLastMirrored = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMoveComponentsEdit::Kill(void)
{
	masTraces.Kill();
	masComponents.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMoveComponentsEdit::AddComponent(CComponentView* pcComponent)
{
	SMovedComponent*	psComponent;

	psComponent = masComponents.Add();
	psComponent->pcComponent = pcComponent;
	psComponent->sPosition = pcComponent->GetPosition();
	psComponent->eRotation = pcComponent->GetRotation();
	psComponent->bMirrored = pcComponent->IsMirrored();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMoveComponentsEdit::AddTrace(CTraceView* pcTrace)
{
	SMovedTrace*	psTrace;

	psTrace = masTraces.Add();
	psTrace->pcTrace = pcTrace;
	psTrace->sStart = pcTrace->GetStart();
	psTrace->sEnd = pcTrace->GetEnd();
}


//////////////////////////////////////////////////////////////////////////
//
// The views being moved aren't selected while they move, and the leads of
// moved components are found (see CCircuitEditor::StartMoveViews).  Wires
// that only hang off them move with them.
//
//////////////////////////////////////////////////////////////////////////
void CMoveComponentsEdit::Start(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit)
{
	CArrayComponentViewPtr	apcComponents;
	CArrayTraceViewPtr		apcTraces;
	CArrayTraceViewPtr		apcFollowing;
	size					i;

	apcComponents.Init();
	apcTraces.Init();
	apcFollowing.Init();
	GetViews(&apcComponents, &apcTraces);
	pcEdit->GetEditor()->StartMoveViews(&apcComponents, &apcTraces, mbNew, mbConnected, &apcFollowing);
	for (i = 0; i < apcFollowing.NumElements(); i++)
	{
		AddTrace(apcFollowing.GetPtr(i));
	}
	apcFollowing.Kill();
	apcTraces.Kill();
	apcComponents.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStatefulEdit* CMoveComponentsEdit::Move(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit)
{
	Follow(pcEdit);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStatefulEdit* CMoveComponentsEdit::Transform(CCircuitEdit* pcEdit)
{
	Follow(pcEdit);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
// Moves the views to where the edit has taken them, if that's changed,
// and tells the editor they've moved.
//
//////////////////////////////////////////////////////////////////////////
void CMoveComponentsEdit::Follow(CCircuitEdit* pcEdit)
{
	if ((pcEdit->GetDiff() == msLastDiff) && (pcEdit->GetRightRotations() == miLastRightRotations) && (pcEdit->IsMirrored() == mbLastMirrored))
	{
		return;
	}
	msLastDiff = pcEdit->GetDiff();
	miLastRightRotations = pcEdit->GetRightRotations();
	mbLastMirrored = pcEdit->IsMirrored();
	MoveViews(mbLastMirrored, miLastRightRotations, pcEdit->GetStart(), msLastDiff);
	pcEdit->GetEditor()->MovedViews();
}


//////////////////////////////////////////////////////////////////////////
//
// New components are made in the simulations, and the simulations are
// rewired if anything moved.
//
//////////////////////////////////////////////////////////////////////////
void CMoveComponentsEdit::Done(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit)
{
	CArrayComponentViewPtr	apcComponents;
	CArrayTraceViewPtr		apcTraces;

	apcComponents.Init();
	apcTraces.Init();
	GetViews(&apcComponents, &apcTraces);
	pcEdit->GetEditor()->DoneMoveViews(&apcComponents, &apcTraces, mbSelect, mbNew, pcEdit->HasDiff());
	apcTraces.Kill();
	apcComponents.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Moved views go back where they were; placed views are taken out.
//
//////////////////////////////////////////////////////////////////////////
void CMoveComponentsEdit::Discard(CCircuitEdit* pcEdit)
{
	CArrayComponentViewPtr	apcComponents;
	CArrayTraceViewPtr		apcTraces;
	SInt32Vec2				sNoDiff;

	apcComponents.Init();
	apcTraces.Init();
	GetViews(&apcComponents, &apcTraces);
	if (mbNew)
	{
		pcEdit->GetEditor()->DeleteViews(&apcComponents, &apcTraces);
	}
	else
	{
		sNoDiff.Init(0, 0);
		MoveViews(false, 0, pcEdit->GetStart(), sNoDiff);
		pcEdit->GetEditor()->DoneMoveViews(&apcComponents, &apcTraces, mbSelect, false, false);
	}
	apcTraces.Kill();
	apcComponents.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMoveComponentsEdit::CanTransformComponents(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Each view from where it started, mirrored top to bottom (if bMirror)
// and turned right iRightRotations times about sStart, and moved by sDiff.
//
//////////////////////////////////////////////////////////////////////////
void CMoveComponentsEdit::MoveViews(bool bMirror, int iRightRotations, SInt32Vec2 sStart, SInt32Vec2 sDiff)
{
	size				i;
	SMovedComponent*	psComponent;
	SMovedTrace*		psTrace;
	SInt32Vec2			sPosition;
	ERotation			eRotation;
	bool				bMirrored;

	for (i = 0; i < masComponents.NumElements(); i++)
	{
		psComponent = masComponents.Get(i);
		sPosition = MoveAbout(psComponent->sPosition, bMirror, iRightRotations, sStart, sDiff);
		psComponent->pcComponent->SetPosition(sPosition.x, sPosition.y);
		eRotation = psComponent->eRotation;
		bMirrored = psComponent->bMirrored;
		TransformOrientation(&eRotation, &bMirrored, bMirror, iRightRotations);
		psComponent->pcComponent->SetRotation(eRotation);
		psComponent->pcComponent->SetMirrored(bMirrored);
	}
	for (i = 0; i < masTraces.NumElements(); i++)
	{
		psTrace = masTraces.Get(i);
		psTrace->pcTrace->SetEnds(MoveAbout(psTrace->sStart, bMirror, iRightRotations, sStart, sDiff), MoveAbout(psTrace->sEnd, bMirror, iRightRotations, sStart, sDiff));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMoveComponentsEdit::GetViews(CArrayComponentViewPtr* papcComponents, CArrayTraceViewPtr* papcTraces)
{
	size	i;

	for (i = 0; i < masComponents.NumElements(); i++)
	{
		papcComponents->Add(masComponents.Get(i)->pcComponent);
	}
	for (i = 0; i < masTraces.NumElements(); i++)
	{
		papcTraces->Add(masTraces.Get(i)->pcTrace);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The grid position mirrored top to bottom (if bMirror) and turned right
// (clockwise) iRightRotations times about sStart, and then moved by sDiff.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 MoveAbout(SInt32Vec2 sPosition, bool bMirror, int iRightRotations, SInt32Vec2 sStart, SInt32Vec2 sDiff)
{
	SInt32Vec2	sMoved;

	sMoved.Init(sPosition.x - sStart.x, sPosition.y - sStart.y);
	TransformPoint(RotateRight(ROT_East, iRightRotations), bMirror, &sMoved);
	sMoved.Init(sMoved.x + sStart.x + sDiff.x, sMoved.y + sStart.y + sDiff.y);
	return sMoved;
}

