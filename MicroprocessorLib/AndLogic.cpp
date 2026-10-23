/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

Codaphela MicroprocessorLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MicroprocessorLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MicroprocessorLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "AndLogic.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAndLogic::InputTransition(CTimeline* pcTimeline, CArrayLogicPortPtr* papcInputs, CLogicPort* pcOutput)
{
	size		i;
	size		uiLows;
	size		uiHighs;
	size		uiUndriven;
	STraceValue	sValue;

	uiLows = 0;
	uiHighs = 0;
	uiUndriven = 0;
	for (i = 0; i < papcInputs->NumElements(); i++)
	{
		sValue = papcInputs->GetPtr(i)->Read(pcTimeline);
		if (sValue.IsLow())
		{
			uiLows++;
		}
		else if (sValue.IsHigh())
		{
			uiHighs++;
		}
		else if (sValue.eValue == TV_Undriven)
		{
			uiUndriven++;
		}
	}

	if (uiLows > 0)
	{
		pcOutput->WriteBool(pcTimeline, TransformOutput(false));
	}
	else if (uiUndriven > 0)
	{
		pcOutput->WriteImpedance(pcTimeline);
	}
	else if (uiHighs > 0)
	{
		pcOutput->WriteBool(pcTimeline, TransformOutput(true));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAndLogic::TransformOutput(bool bValue)
{
	return bValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNandLogic::TransformOutput(bool bValue)
{
	return !bValue;
}

