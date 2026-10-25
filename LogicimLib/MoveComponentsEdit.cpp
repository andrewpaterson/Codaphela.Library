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
void CMoveComponentsEdit::Init(CComponentView* pcComponent, bool bNew)
{
	masComponents.Init();
	masTraces.Init();
	mbNew = bNew;
	mbSelect = false;
	AddComponent(pcComponent);
}


//////////////////////////////////////////////////////////////////////////
//
// Views selected when done.
//
//////////////////////////////////////////////////////////////////////////
void CMoveComponentsEdit::Init(CArrayComponentViewPtr* papcComponents, CArrayTraceViewPtr* papcTraces, bool bNew)
{
	size			i;
	CTraceView*		pcTrace;
	SMovedTrace*	psTrace;

	masComponents.Init();
	masTraces.Init();
	mbNew = bNew;
	mbSelect = true;
	for (i = 0; i < papcComponents->NumElements(); i++)
	{
		AddComponent(papcComponents->GetPtr(i));
	}
	for (i = 0; i < papcTraces->NumElements(); i++)
	{
		pcTrace = papcTraces->GetPtr(i);
		psTrace = masTraces.Add();
		psTrace->pcTrace = pcTrace;
		psTrace->sStart = pcTrace->GetStart();
		psTrace->sEnd = pcTrace->GetEnd();
	}
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
}


//////////////////////////////////////////////////////////////////////////
//
// The views being moved aren't selected while they move.
//
//////////////////////////////////////////////////////////////////////////
void CMoveComponentsEdit::Start(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit)
{
	pcEdit->GetEditor()->StartMoveViews();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStatefulEdit* CMoveComponentsEdit::Move(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit)
{
	MoveViews(pcEdit->GetRightRotations(), pcEdit->GetStart(), pcEdit->GetDiff());
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStatefulEdit* CMoveComponentsEdit::Rotate(bool bRight, CCircuitEdit* pcEdit)
{
	MoveViews(pcEdit->GetRightRotations(), pcEdit->GetStart(), pcEdit->GetDiff());
	return this;
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
		MoveViews(0, pcEdit->GetStart(), sNoDiff);
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
// Each view from where it started, turned right iRightRotations times
// about sStart and moved by sDiff.
//
//////////////////////////////////////////////////////////////////////////
void CMoveComponentsEdit::MoveViews(int iRightRotations, SInt32Vec2 sStart, SInt32Vec2 sDiff)
{
	size				i;
	SMovedComponent*	psComponent;
	SMovedTrace*		psTrace;
	SInt32Vec2			sPosition;

	for (i = 0; i < masComponents.NumElements(); i++)
	{
		psComponent = masComponents.Get(i);
		sPosition = MoveAbout(psComponent->sPosition, iRightRotations, sStart, sDiff);
		psComponent->pcComponent->SetPosition(sPosition.x, sPosition.y);
		psComponent->pcComponent->SetRotation(RotateRight(psComponent->eRotation, iRightRotations));
	}
	for (i = 0; i < masTraces.NumElements(); i++)
	{
		psTrace = masTraces.Get(i);
		psTrace->pcTrace->SetEnds(MoveAbout(psTrace->sStart, iRightRotations, sStart, sDiff), MoveAbout(psTrace->sEnd, iRightRotations, sStart, sDiff));
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
// The grid position turned right (clockwise) iRightRotations times about
// sStart and then moved by sDiff.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 MoveAbout(SInt32Vec2 sPosition, int iRightRotations, SInt32Vec2 sStart, SInt32Vec2 sDiff)
{
	SInt32Vec2	sMoved;

	sMoved.Init(sPosition.x - sStart.x, sPosition.y - sStart.y);
	RotatePoint(RotateRight(ROT_East, iRightRotations), &sMoved);
	sMoved.Init(sMoved.x + sStart.x + sDiff.x, sMoved.y + sStart.y + sDiff.y);
	return sMoved;
}

