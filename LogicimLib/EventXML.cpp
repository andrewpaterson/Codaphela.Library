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
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/Logger.h"
#include "BaseLib/StringHelper.h"
#include "MicroprocessorLib/DriveEvent.h"
#include "MicroprocessorLib/SlewEvent.h"
#include "MicroprocessorLib/TransitionEvent.h"
#include "MicroprocessorLib/TickEvent.h"
#include "EventXML.h"


CDriveEventXML		gcDriveEventXML;
CSlewEventXML		gcSlewEventXML;
CTransitionEventXML	gcTransitionEventXML;
CTickEventXML		gcTickEventXML;

CPortEventXML* gapcPortEventXMLs[] =
{
	&gcDriveEventXML,
	&gcSlewEventXML,
	&gcTransitionEventXML,
};

CIntegratedCircuitEventXML* gapcIntegratedCircuitEventXMLs[] =
{
	&gcTickEventXML,
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPortEventXML* FindPortEventXMLForEvent(CEvent* pcEvent)
{
	size	i;

	for (i = 0; i < sizeof(gapcPortEventXMLs) / sizeof(CPortEventXML*); i++)
	{
		if (StringCompare(gapcPortEventXMLs[i]->GetEventClassName(), pcEvent->ClassName()) == 0)
		{
			return gapcPortEventXMLs[i];
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPortEventXML* FindPortEventXMLForElement(const char* szXMLName)
{
	size	i;

	for (i = 0; i < sizeof(gapcPortEventXMLs) / sizeof(CPortEventXML*); i++)
	{
		if (StringCompare(gapcPortEventXMLs[i]->GetXMLName(), szXMLName) == 0)
		{
			return gapcPortEventXMLs[i];
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIntegratedCircuitEventXML* FindIntegratedCircuitEventXMLForEvent(CEvent* pcEvent)
{
	size	i;

	for (i = 0; i < sizeof(gapcIntegratedCircuitEventXMLs) / sizeof(CIntegratedCircuitEventXML*); i++)
	{
		if (StringCompare(gapcIntegratedCircuitEventXMLs[i]->GetEventClassName(), pcEvent->ClassName()) == 0)
		{
			return gapcIntegratedCircuitEventXMLs[i];
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIntegratedCircuitEventXML* FindIntegratedCircuitEventXMLForElement(const char* szXMLName)
{
	size	i;

	for (i = 0; i < sizeof(gapcIntegratedCircuitEventXMLs) / sizeof(CIntegratedCircuitEventXML*); i++)
	{
		if (StringCompare(gapcIntegratedCircuitEventXMLs[i]->GetXMLName(), szXMLName) == 0)
		{
			return gapcIntegratedCircuitEventXMLs[i];
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEventXML::Save(CXMLWriter* pcWriter, CEvent* pcEvent)
{
	pcWriter->Begin(GetXMLName());
	SaveAttributes(pcWriter, pcEvent);
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEventXML::SaveAttributes(CXMLWriter* pcWriter, CEvent* pcEvent)
{
	pcWriter->Attribute("time", pcEvent->GetTime());
	pcWriter->Attribute("id", pcEvent->GetID());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEventXML::LoadTimeAndID(CMarkupTag* pcTag, int64* piTime, int64* piID)
{
	if (!pcTag->GetInt64Attribute("time", piTime) || !pcTag->GetInt64Attribute("id", piID))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a whole number time and id.", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CDriveEventXML::GetEventClassName(void) { return "CDriveEvent"; }
const char* CDriveEventXML::GetXMLName(void) { return "DriveEvent"; }


//////////////////////////////////////////////////////////////////////////
//
// A voltage of nan stops driving.
//
//////////////////////////////////////////////////////////////////////////
void CDriveEventXML::SaveAttributes(CXMLWriter* pcWriter, CEvent* pcEvent)
{
	CEventXML::SaveAttributes(pcWriter, pcEvent);
	pcWriter->Attribute("voltage", ((CDriveEvent*)pcEvent)->GetVoltage(pcEvent->GetTime()));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPortEvent* CDriveEventXML::Load(CMarkupTag* pcTag, CLogicPort* pcPort)
{
	CDriveEvent*	pcEvent;
	int64			iTime;
	int64			iID;
	float32			fVoltage;

	if (!LoadTimeAndID(pcTag, &iTime, &iID))
	{
		return NULL;
	}
	if (!pcTag->GetFloat32Attribute("voltage", &fVoltage))
	{
		gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a voltage.", NULL);
		return NULL;
	}
	pcEvent = NewMalloc<CDriveEvent>();
	pcEvent->Init(pcPort, iTime, iID, fVoltage);
	return pcEvent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CSlewEventXML::GetEventClassName(void) { return "CSlewEvent"; }
const char* CSlewEventXML::GetXMLName(void) { return "SlewEvent"; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSlewEventXML::SaveAttributes(CXMLWriter* pcWriter, CEvent* pcEvent)
{
	CSlewEvent*	pcSlew;

	pcSlew = (CSlewEvent*)pcEvent;
	CEventXML::SaveAttributes(pcWriter, pcEvent);
	pcWriter->Attribute("startVoltage", pcSlew->GetStartVoltage());
	pcWriter->Attribute("endVoltage", pcSlew->GetEndVoltage());
	pcWriter->Attribute("slewTime", pcSlew->GetSlewTime());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPortEvent* CSlewEventXML::Load(CMarkupTag* pcTag, CLogicPort* pcPort)
{
	CSlewEvent*	pcEvent;
	int64		iTime;
	int64		iID;
	float32		fStartVoltage;
	float32		fEndVoltage;
	int64		iSlewTime;

	if (!LoadTimeAndID(pcTag, &iTime, &iID))
	{
		return NULL;
	}
	if (!pcTag->GetFloat32Attribute("startVoltage", &fStartVoltage) || !pcTag->GetFloat32Attribute("endVoltage", &fEndVoltage) || !pcTag->GetInt64Attribute("slewTime", &iSlewTime))
	{
		gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a startVoltage, endVoltage and whole number slewTime.", NULL);
		return NULL;
	}
	pcEvent = NewMalloc<CSlewEvent>();
	pcEvent->Init(pcPort, iTime, iID, fStartVoltage, fEndVoltage, iSlewTime);
	return pcEvent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CTransitionEventXML::GetEventClassName(void) { return "CTransitionEvent"; }
const char* CTransitionEventXML::GetXMLName(void) { return "TransitionEvent"; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransitionEventXML::SaveAttributes(CXMLWriter* pcWriter, CEvent* pcEvent)
{
	CEventXML::SaveAttributes(pcWriter, pcEvent);
	pcWriter->Attribute("voltage", ((CTransitionEvent*)pcEvent)->GetVoltage());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPortEvent* CTransitionEventXML::Load(CMarkupTag* pcTag, CLogicPort* pcPort)
{
	CTransitionEvent*	pcEvent;
	int64				iTime;
	int64				iID;
	float32				fVoltage;

	if (!LoadTimeAndID(pcTag, &iTime, &iID))
	{
		return NULL;
	}
	if (!pcTag->GetFloat32Attribute("voltage", &fVoltage))
	{
		gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a voltage.", NULL);
		return NULL;
	}
	pcEvent = NewMalloc<CTransitionEvent>();
	pcEvent->Init(pcPort, iTime, iID, fVoltage);
	return pcEvent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CTickEventXML::GetEventClassName(void) { return "CTickEvent"; }
const char* CTickEventXML::GetXMLName(void) { return "TickEvent"; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIntegratedCircuitEvent* CTickEventXML::Load(CMarkupTag* pcTag, CIntegratedCircuit* pcIntegratedCircuit)
{
	CTickEvent*	pcEvent;
	int64		iTime;
	int64		iID;

	if (!LoadTimeAndID(pcTag, &iTime, &iID))
	{
		return NULL;
	}
	pcEvent = NewMalloc<CTickEvent>();
	pcEvent->Init(iTime, iID, pcIntegratedCircuit);
	return pcEvent;
}

