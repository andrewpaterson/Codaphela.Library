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
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/Logger.h"
#include "BaseLib/PointerFunctions.h"
#include "SubcircuitInstance.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SSubcircuitNamedPin::Init(const char* szName)
{
	szPinName.Init(szName);
	apcTracePorts.Init();
}


//////////////////////////////////////////////////////////////////////////
//
// The ports belong to the instance.
//
//////////////////////////////////////////////////////////////////////////
void SSubcircuitNamedPin::Kill(void)
{
	apcTracePorts.Kill();
	szPinName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSubcircuitInstance::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName)
{
	PreInit();

	CPassive::Init(pcContainingSubcircuitSimulation, szName);
	mapsNamedPins.Init();
	mpcSubcircuitInstanceSimulation = NULL;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSubcircuitInstance::Class(void)
{
	CPassive::Class();

	U_Data(CArraySubcircuitNamedPinPtr, mapsNamedPins);
	U_Pointer(mpcSubcircuitInstanceSimulation);
}


//////////////////////////////////////////////////////////////////////////
//
// The ports are freed by CPortHolder.
//
//////////////////////////////////////////////////////////////////////////
void CSubcircuitInstance::Free(void)
{
	size					i;
	SSubcircuitNamedPin*	psNamedPin;

	for (i = 0; i < mapsNamedPins.NumElements(); i++)
	{
		psNamedPin = mapsNamedPins.GetPtr(i);
		SafeKill(psNamedPin);
	}
	mapsNamedPins.Kill();
	mpcSubcircuitInstanceSimulation = NULL;
	CPassive::Free();
}


//////////////////////////////////////////////////////////////////////////
//
// Adds uiPortCount trace ports, "Pin 0" up for a pin named "Pin", for
// the pin named szPinName.  Logicim's caller makes the ports and passes
// them in.  NULL if the pin already has ports.
//
//////////////////////////////////////////////////////////////////////////
CArrayTracePortPtr* CSubcircuitInstance::AddTracePorts(const char* szPinName, size uiPortCount)
{
	SSubcircuitNamedPin*	psNamedPin;
	size					i;
	CTracePort*				pcPort;
	CChars					szPortName;

	if (FindNamedPin(szPinName) != NULL)
	{
		gcLogger.Error2(__METHOD__, " Ports for pin named [", szPinName, "] have already been added.", NULL);
		return NULL;
	}

	psNamedPin = NewMalloc<SSubcircuitNamedPin>();
	psNamedPin->Init(szPinName);
	mapsNamedPins.Add(psNamedPin);
	for (i = 0; i < uiPortCount; i++)
	{
		szPortName.Init(szPinName);
		szPortName.Append(' ');
		szPortName.Append(i);
		pcPort = NewMalloc<CTracePort>();
		pcPort->Init(szPortName.Text(), this);
		psNamedPin->apcTracePorts.Add(pcPort);
		szPortName.Kill();
	}
	return &psNamedPin->apcTracePorts;
}


//////////////////////////////////////////////////////////////////////////
//
// Logs an error and returns NULL if there is no pin named szPinName.
//
//////////////////////////////////////////////////////////////////////////
CArrayTracePortPtr* CSubcircuitInstance::GetTracePorts(const char* szPinName)
{
	SSubcircuitNamedPin*	psNamedPin;

	psNamedPin = FindNamedPin(szPinName);
	if (psNamedPin != NULL)
	{
		return &psNamedPin->apcTracePorts;
	}
	gcLogger.Error2(__METHOD__, " Cannot get ports for pin named [", szPinName, "].", NULL);
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SSubcircuitNamedPin* CSubcircuitInstance::FindNamedPin(const char* szPinName)
{
	size					i;
	SSubcircuitNamedPin*	psNamedPin;

	for (i = 0; i < mapsNamedPins.NumElements(); i++)
	{
		psNamedPin = mapsNamedPins.GetPtr(i);
		if (psNamedPin->szPinName.Equals(szPinName))
		{
			return psNamedPin;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// NULL past the end.
//
//////////////////////////////////////////////////////////////////////////
char* CSubcircuitInstance::GetPinName(size uiIndex)
{
	SSubcircuitNamedPin*	psNamedPin;

	psNamedPin = mapsNamedPins.SafeGetPtr(uiIndex);
	if (psNamedPin != NULL)
	{
		return psNamedPin->szPinName.Text();
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CSubcircuitInstance::GetType(void) { return "Subcircuit instance"; }
size CSubcircuitInstance::NumNamedPins(void) { return mapsNamedPins.NumElements(); }
CSubcircuitInstanceSimulation* CSubcircuitInstance::GetSubcircuitInstanceSimulation(void) { return mpcSubcircuitInstanceSimulation; }
void CSubcircuitInstance::SetSubcircuitInstanceSimulation(CSubcircuitInstanceSimulation* pcSubcircuitInstanceSimulation) { mpcSubcircuitInstanceSimulation = pcSubcircuitInstanceSimulation; }

