/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela InputLib

Codaphela InputLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela InputLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela InputLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "InputChordCriteria.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void KillActionCriteria(UInputChordCriteria* puCriteria)
{
	switch (puCriteria->eType.eAction)
	{
	case BAA_Unknown:
		break;
	case BAA_Active:
		puCriteria->cActive.Kill();
		break;
	case BAA_Inactive:
		puCriteria->cInactive.Kill();
		break;
	case BAA_Group:
		puCriteria->cGroup.Kill();
		break;
	case BAA_Ordered:
		puCriteria->cOrdered.Kill();
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToStringActionCriteria(UInputChordCriteria* puCriteria, CChars* psz)
{
	switch (puCriteria->eType.eAction)
	{
	case BAA_Unknown:
		psz->Append("BAA_Unknown\n");
		break;
	case BAA_Active:
		puCriteria->cActive.ToString(psz);
		break;
	case BAA_Inactive:
		puCriteria->cInactive.ToString(psz);
		break;
	case BAA_Group:
		puCriteria->cGroup.ToString(psz);
		break;
	case BAA_Ordered:
		puCriteria->cOrdered.ToString(psz);
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CActionInputChordCriteria::SetEvaluator(CInputSourceEvaluator* pcEvaluator)
{
	mpcEvaluator = pcEvaluator;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CActiveInputChordCriteria::Init(CInputSourceEvaluator* pcEvaluator)
{
	mpcEvaluator = pcEvaluator;
	eAction = BAA_Active;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CActiveInputChordCriteria::Kill(void)
{
	eAction = BAA_Unknown;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CActiveInputChordCriteria::ToArray(CArrayActionInputChordCriteria* pcDest)
{
	CActionInputChordCriteria*	psCriteria;

	pcDest->FakeSetUsedElements(Size());
	psCriteria = pcDest->Get(0);
	psCriteria->mpcEvaluator = mpcEvaluator;
	psCriteria->eAction = eAction;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CActiveInputChordCriteria::Size(void)
{
	return 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CActiveInputChordCriteria::ToString(CChars* psz)
{
	psz->Append(" --- CActiveInputChordCriteria ---\n");
	mpcEvaluator->ToString(psz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInactiveInputChordCriteria::Init(CInputSourceEvaluator* pcEvaluator)
{
	mpcEvaluator = pcEvaluator;
	eAction = BAA_Inactive;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInactiveInputChordCriteria::Kill(void)
{
	eAction = BAA_Unknown;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInactiveInputChordCriteria::ToArray(CArrayActionInputChordCriteria* pcDest)
{
	CActionInputChordCriteria*	psCriteria;

	pcDest->FakeSetUsedElements(Size());
	psCriteria = pcDest->Get(0);
	psCriteria->mpcEvaluator = mpcEvaluator;
	psCriteria->eAction = eAction;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CInactiveInputChordCriteria::Size(void)
{
	return 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInactiveInputChordCriteria::ToString(CChars* psz)
{
	psz->Append(" --- CInactiveInputChordCriteria ---\n");
	mpcEvaluator->ToString(psz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGroupInputChordCriteria::Init(void)
{
	mausBasicActionCriteria.Init(sizeof(UInputChordCriteria), 1);
	eAction = BAA_Group;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGroupInputChordCriteria::Kill(void)
{
	int						i;
	UInputChordCriteria*	puCriteria;

	for (i = 0; i < Size(); i++)
	{
		puCriteria = (UInputChordCriteria*)mausBasicActionCriteria.Get(i);
		KillActionCriteria(puCriteria);
	}
	mausBasicActionCriteria.Kill();
	eAction = BAA_Unknown;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGroupInputChordCriteria::ToArray(CArrayActionInputChordCriteria* pcDest)
{
	CActionInputChordCriteria*	psCriteria;
	int							i;
	UInputChordCriteria*		puCriteria;
	CInputSourceEvaluator*		pcEvaluator;

	pcDest->FakeSetUsedElements(Size());
	for (i = 0; i < Size(); i++)
	{
		puCriteria = (UInputChordCriteria*)mausBasicActionCriteria.Get(i);
		switch (puCriteria->eType.eAction)
		{
		case BAA_Active:
			pcEvaluator = puCriteria->cActive.mpcEvaluator;
			break;
		case BAA_Inactive:
			pcEvaluator = puCriteria->cInactive.mpcEvaluator;
			break;
		default:
			pcEvaluator = NULL;
		}

		if (pcEvaluator)
		{
			psCriteria = pcDest->Get(i);

			psCriteria->mpcEvaluator = pcEvaluator;
			psCriteria->eAction = puCriteria->eType.eAction;
		}
		else
		{
			pcDest->FakeSetUsedElements(0);
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CGroupInputChordCriteria::Size(void)
{
	return mausBasicActionCriteria.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGroupInputChordCriteria::ToString(CChars* psz)
{
	int							i;
	UInputChordCriteria*		puCriteria;

	psz->Append(" --- CGroupInputChordCriteria ---\n");

	for (i = 0; i < Size(); i++)
	{
		puCriteria = (UInputChordCriteria*)mausBasicActionCriteria.Get(i);
		switch (puCriteria->eType.eAction)
		{
		case BAA_Active:
			puCriteria->cActive.ToString(psz);
			break;
		case BAA_Inactive:
			puCriteria->cInactive.ToString(psz);
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CActiveInputChordCriteria* CGroupInputChordCriteria::AddActiveAction(CInputSourceEvaluator* pcAction)
{
	UInputChordCriteria*	puCriteria;

	puCriteria = (UInputChordCriteria*)mausBasicActionCriteria.Add();
	puCriteria->cActive.Init(pcAction);
	return &puCriteria->cActive;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInactiveInputChordCriteria* CGroupInputChordCriteria::AddInactiveAction(CInputSourceEvaluator* pcAction)
{
	UInputChordCriteria*	puCriteria;

	puCriteria = (UInputChordCriteria*)mausBasicActionCriteria.Add();
	puCriteria->cInactive.Init(pcAction);
	return &puCriteria->cInactive;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COrderedInputChordCriteria::Init(void)
{
	mausBasicActionCriteria.Init(sizeof(UInputChordCriteria), 1);
	eAction = BAA_Ordered;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COrderedInputChordCriteria::Kill(void)
{
	((CGroupInputChordCriteria*)this)->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int COrderedInputChordCriteria::Size(void)
{
	return Size(mausBasicActionCriteria.NumElements()-1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int COrderedInputChordCriteria::Size(int iLastIndexInclusive)
{
	int							i;
	UInputChordCriteria*		puCriteria;
	int							iSize;

	if (iLastIndexInclusive >= mausBasicActionCriteria.NumElements())
	{
		iLastIndexInclusive = mausBasicActionCriteria.NumElements()-1;
	}

	iSize = 0;
	for (i = 0; i <= iLastIndexInclusive; i++)
	{
		puCriteria = (UInputChordCriteria*)mausBasicActionCriteria.Get(i);
		switch (puCriteria->eType.eAction)
		{
		case BAA_Active:
			iSize += puCriteria->cActive.Size();
			break;
		case BAA_Inactive:
			iSize += puCriteria->cInactive.Size();
			break;
		case BAA_Group:
			iSize += puCriteria->cGroup.Size();
			break;
		default:
			return 0;
		}
	}
	return iSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COrderedInputChordCriteria::ToString(CChars* psz)
{
	int							i;
	UInputChordCriteria*		puCriteria;

	psz->Append(" --- COrderedInputChordCriteria ---\n");

	for (i = 0; i < mausBasicActionCriteria.NumElements(); i++)
	{
		puCriteria = (UInputChordCriteria*)mausBasicActionCriteria.Get(i);
		switch (puCriteria->eType.eAction)
		{
		case BAA_Active:
			puCriteria->cActive.ToString(psz);
			break;
		case BAA_Inactive:
			puCriteria->cInactive.ToString(psz);
			break;
		case BAA_Group:
			puCriteria->cGroup.ToString(psz);
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CActiveInputChordCriteria* COrderedInputChordCriteria::AddActiveAction(CInputSourceEvaluator* pcAction)
{
	return ((CGroupInputChordCriteria*)this)->AddActiveAction(pcAction);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInactiveInputChordCriteria* COrderedInputChordCriteria::AddInactiveAction(CInputSourceEvaluator* pcAction)
{
	return ((CGroupInputChordCriteria*)this)->AddInactiveAction(pcAction);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGroupInputChordCriteria* COrderedInputChordCriteria::AddGroupAction(void)
{
	UInputChordCriteria*	puCriteria;

	puCriteria = (UInputChordCriteria*)mausBasicActionCriteria.Add();
	puCriteria->cGroup.Init();
	return &puCriteria->cGroup;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COrderedInputChordCriteria::RemoveLastAction(void)
{
	mausBasicActionCriteria.RemoveTail();
}

