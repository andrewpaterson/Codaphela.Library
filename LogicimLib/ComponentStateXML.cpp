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
#include "BaseLib/Logger.h"
#include "BaseLib/StringHelper.h"
#include "MicroprocessorLib/IntegratedCircuit.h"
#include "MicroprocessorLib/IntegratedCircuitEvent.h"
#include "MicroprocessorLib/LogicPort.h"
#include "MicroprocessorLib/PortEvent.h"
#include "MicroprocessorLib/PortOutput.h"
#include "MicroprocessorLib/Pins.h"
#include "ComponentStateXML.h"


CIntegratedCircuitStateXML	gcIntegratedCircuitStateXML;
CPassiveStateXML			gcPassiveStateXML;


//////////////////////////////////////////////////////////////////////////
//
// An IC without a GetStateXMLName remembers nothing beyond its ports and
// events.
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuitStateXML::Save(CXMLWriter* pcWriter, CCircuitComponent* pcComponent)
{
	CIntegratedCircuit*				pcIntegratedCircuit;
	const char*						szStateName;
	size							i;
	CEvent*							pcEvent;
	CIntegratedCircuitEventXML*		pcEventXML;
	CPort*							pcPort;

	pcIntegratedCircuit = (CIntegratedCircuit*)pcComponent;
	szStateName = pcIntegratedCircuit->GetStateXMLName();
	if (szStateName != NULL)
	{
		pcWriter->Begin(szStateName);
		pcIntegratedCircuit->SaveXML(pcWriter);
		pcWriter->End();
	}

	for (i = 0; i < pcIntegratedCircuit->NumEvents(); i++)
	{
		pcEvent = pcIntegratedCircuit->GetEvent(i);
		pcEventXML = FindIntegratedCircuitEventXMLForEvent(pcEvent);
		if (pcEventXML != NULL)
		{
			pcEventXML->Save(pcWriter, pcEvent);
		}
		else
		{
			gcLogger.Error2(__METHOD__, " Cannot save event [", pcEvent->ClassName(), "] of [", pcComponent->GetName(), "].", NULL);
		}
	}

	for (i = 0; i < pcIntegratedCircuit->NumPorts(); i++)
	{
		pcPort = pcIntegratedCircuit->GetPort(i);
		if (pcPort->IsLogicPort())
		{
			SavePort(pcWriter, (CLogicPort*)pcPort);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Ports that have never driven and have nothing pending aren't written.
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuitStateXML::SavePort(CXMLWriter* pcWriter, CLogicPort* pcPort)
{
	size				i;
	CPortEvent*			pcEvent;
	CPortEventXML*		pcEventXML;

	if (pcPort->GetOutput()->IsNone() && (pcPort->NumEvents() == 0))
	{
		return;
	}

	pcWriter->Begin("Port");
	pcWriter->Attribute("name", pcPort->GetName());
	SaveOutput(pcWriter, pcPort->GetOutput());
	for (i = 0; i < pcPort->NumEvents(); i++)
	{
		pcEvent = pcPort->GetEvent(i);
		pcEventXML = FindPortEventXMLForEvent(pcEvent);
		if (pcEventXML != NULL)
		{
			pcEventXML->Save(pcWriter, pcEvent);
		}
		else
		{
			gcLogger.Error2(__METHOD__, " Cannot save event [", pcEvent->ClassName(), "] of port [", pcPort->GetName(), "].", NULL);
		}
	}
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
// The last output the port gave its trace.
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuitStateXML::SaveOutput(CXMLWriter* pcWriter, CPortOutput* pcOutput)
{
	if (pcOutput->IsDrive())
	{
		pcWriter->Begin("Output");
		pcWriter->Attribute("type", "Drive");
		pcWriter->Attribute("time", pcOutput->GetTime());
		pcWriter->Attribute("voltage", pcOutput->GetEndVoltage());
		pcWriter->End();
	}
	else if (pcOutput->IsSlew())
	{
		pcWriter->Begin("Output");
		pcWriter->Attribute("type", "Slew");
		pcWriter->Attribute("time", pcOutput->GetTime());
		pcWriter->Attribute("startVoltage", pcOutput->GetStartVoltage());
		pcWriter->Attribute("endVoltage", pcOutput->GetEndVoltage());
		pcWriter->Attribute("slewTime", pcOutput->GetSlewTime());
		pcWriter->End();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// An IC keeps what it remembered when it was made unless the file has its
// state.
//
//////////////////////////////////////////////////////////////////////////
bool CIntegratedCircuitStateXML::Load(CMarkupTag* pcComponentTag, CCircuitComponent* pcComponent, CArrayPendingEventXML* pasEvents)
{
	CIntegratedCircuit*				pcIntegratedCircuit;
	STagIterator					sIter;
	CMarkupTag*						pcTag;
	const char*						szStateName;
	CIntegratedCircuitEventXML*		pcEventXML;
	SPendingEventXML*				psPending;
	bool							bStateLoaded;

	pcIntegratedCircuit = (CIntegratedCircuit*)pcComponent;
	szStateName = pcIntegratedCircuit->GetStateXMLName();
	bStateLoaded = false;
	pcTag = pcComponentTag->GetTag(&sIter);
	while (pcTag != NULL)
	{
		if (pcTag->Is("Port"))
		{
			if (!LoadPort(pcTag, pcIntegratedCircuit, pasEvents))
			{
				return false;
			}
		}
		else
		{
			pcEventXML = FindIntegratedCircuitEventXMLForElement(pcTag->GetName());
			if ((szStateName != NULL) && pcTag->Is(szStateName))
			{
				if (bStateLoaded)
				{
					return gcLogger.Error2(__METHOD__, " [", pcComponent->GetName(), "] has more than one state.", NULL);
				}
				if (!pcIntegratedCircuit->LoadXML(pcTag))
				{
					return false;
				}
				bStateLoaded = true;
			}
			else if (pcEventXML != NULL)
			{
				psPending = pasEvents->Add();
				psPending->pcTag = pcTag;
				psPending->pcPortEventXML = NULL;
				psPending->pcPort = NULL;
				psPending->pcIntegratedCircuitEventXML = pcEventXML;
				psPending->pcIntegratedCircuit = pcIntegratedCircuit;
				if (!CEventXML::LoadTimeAndID(pcTag, &psPending->iTime, &psPending->iID))
				{
					return false;
				}
			}
			else
			{
				return gcLogger.Error2(__METHOD__, " Unknown simulation state [", pcTag->GetName(), "] on [", pcComponent->GetName(), "].", NULL);
			}
		}
		pcTag = pcComponentTag->GetNextTag(&sIter);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIntegratedCircuitStateXML::LoadPort(CMarkupTag* pcPortTag, CIntegratedCircuit* pcIntegratedCircuit, CArrayPendingEventXML* pasEvents)
{
	char*				szName;
	CPort*				pcPort;
	CLogicPort*			pcLogicPort;
	STagIterator		sIter;
	CMarkupTag*			pcTag;
	CPortEventXML*		pcEventXML;
	SPendingEventXML*	psPending;

	szName = pcPortTag->GetStringAttribute("name");
	pcPort = (szName != NULL) ? pcIntegratedCircuit->GetPort(szName) : NULL;
	if ((pcPort == NULL) || !pcPort->IsLogicPort())
	{
		return gcLogger.Error2(__METHOD__, " [", pcIntegratedCircuit->GetName(), "] has no logic port [", (szName != NULL) ? szName : "", "].", NULL);
	}
	pcLogicPort = (CLogicPort*)pcPort;

	pcTag = pcPortTag->GetTag(&sIter);
	while (pcTag != NULL)
	{
		if (pcTag->Is("Output"))
		{
			if (!LoadOutput(pcTag, pcLogicPort->GetOutput()))
			{
				return false;
			}
		}
		else
		{
			pcEventXML = FindPortEventXMLForElement(pcTag->GetName());
			if (pcEventXML == NULL)
			{
				return gcLogger.Error2(__METHOD__, " Unknown port state [", pcTag->GetName(), "] on port [", szName, "].", NULL);
			}
			psPending = pasEvents->Add();
			psPending->pcTag = pcTag;
			psPending->pcPortEventXML = pcEventXML;
			psPending->pcPort = pcLogicPort;
			psPending->pcIntegratedCircuitEventXML = NULL;
			psPending->pcIntegratedCircuit = NULL;
			if (!CEventXML::LoadTimeAndID(pcTag, &psPending->iTime, &psPending->iID))
			{
				return false;
			}
		}
		pcTag = pcPortTag->GetNextTag(&sIter);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIntegratedCircuitStateXML::LoadOutput(CMarkupTag* pcOutputTag, CPortOutput* pcOutput)
{
	char*		szType;
	int64		iTime;
	float32		fVoltage;
	float32		fStartVoltage;
	float32		fEndVoltage;
	int64		iSlewTime;

	szType = pcOutputTag->GetStringAttribute("type");
	if (!pcOutputTag->GetInt64Attribute("time", &iTime) || (szType == NULL))
	{
		return gcLogger.Error2(__METHOD__, " [Output] needs a type and a whole number time.", NULL);
	}

	if (StringCompare(szType, "Drive") == 0)
	{
		if (!pcOutputTag->GetFloat32Attribute("voltage", &fVoltage))
		{
			return gcLogger.Error2(__METHOD__, " A Drive [Output] needs a voltage.", NULL);
		}
		pcOutput->InitDrive(iTime, fVoltage);
		return true;
	}
	if (StringCompare(szType, "Slew") == 0)
	{
		if (!pcOutputTag->GetFloat32Attribute("startVoltage", &fStartVoltage) || !pcOutputTag->GetFloat32Attribute("endVoltage", &fEndVoltage) || !pcOutputTag->GetInt64Attribute("slewTime", &iSlewTime))
		{
			return gcLogger.Error2(__METHOD__, " A Slew [Output] needs a startVoltage, endVoltage and whole number slewTime.", NULL);
		}
		pcOutput->InitSlew(iTime, fStartVoltage, fEndVoltage, iSlewTime);
		return true;
	}
	return gcLogger.Error2(__METHOD__, " Unknown [Output] type [", szType, "].", NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPassiveStateXML::Save(CXMLWriter* pcWriter, CCircuitComponent* pcComponent)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPassiveStateXML::Load(CMarkupTag* pcComponentTag, CCircuitComponent* pcComponent, CArrayPendingEventXML* pasEvents)
{
	if (pcComponentTag->GetTag((STagIterator*)NULL) != NULL)
	{
		return gcLogger.Error2(__METHOD__, " [", pcComponent->GetName(), "] keeps no simulation state.", NULL);
	}
	return true;
}

