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
#include <stdlib.h>
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "MicroprocessorLib/CircuitSimulation.h"
#include "MicroprocessorLib/Circuit.h"
#include "MicroprocessorLib/IntegratedCircuit.h"
#include "MicroprocessorLib/IntegratedCircuitEvent.h"
#include "MicroprocessorLib/PortEvent.h"
#include "MicroprocessorLib/Timeline.h"
#include "ComponentStateXML.h"
#include "SimulationStateXML.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SaveSimulationXML(CXMLWriter* pcWriter, CCircuitView* pcCircuit, CCircuitSimulation* pcSimulation)
{
	STimelineData			sTimeline;
	size					i;
	CComponentView*			pcView;
	CCircuitComponent*		pcComponent;
	CComponentStateXML*		pcStateXML;

	pcSimulation->GetTimeline()->Save(&sTimeline);
	pcWriter->Begin("Simulation");
	pcWriter->Attribute("name", pcSimulation->GetName());
	pcWriter->Attribute("time", sTimeline.iTime);
	pcWriter->Attribute("previousEventTime", sTimeline.iPreviousEventTime);
	pcWriter->Attribute("eventTime", sTimeline.iEventTime);
	for (i = 0; i < pcCircuit->NumComponents(); i++)
	{
		pcView = pcCircuit->GetComponent(i);
		pcComponent = pcView->GetComponent();
		pcStateXML = pcView->GetComponentStateXML();
		if ((pcComponent != NULL) && (pcStateXML != NULL))
		{
			pcWriter->Begin("Component");
			pcWriter->Attribute("index", (int64)i);
			pcStateXML->Save(pcWriter, pcComponent);
			pcWriter->End();
		}
	}
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
// Earlier times first; at a time, the event made first first.
//
//////////////////////////////////////////////////////////////////////////
static int ComparePendingEventXML(const void* pv1, const void* pv2)
{
	SPendingEventXML*	ps1;
	SPendingEventXML*	ps2;

	ps1 = (SPendingEventXML*)pv1;
	ps2 = (SPendingEventXML*)pv2;
	if (ps1->iTime != ps2->iTime)
	{
		return (ps1->iTime < ps2->iTime) ? -1 : 1;
	}
	if (ps1->iID != ps2->iID)
	{
		return (ps1->iID < ps2->iID) ? -1 : 1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Makes the events in order and puts them on the timeline.  Every event
// must be after the restored time.
//
//////////////////////////////////////////////////////////////////////////
static bool MakePendingEventsXML(CArrayPendingEventXML* pasEvents, CTimeline* pcTimeline)
{
	size				i;
	SPendingEventXML*	psPending;
	CEvent*				pcEvent;

	pasEvents->Sort(ComparePendingEventXML);
	for (i = 0; i < pasEvents->NumElements(); i++)
	{
		psPending = pasEvents->Get(i);
		if (psPending->iTime <= pcTimeline->GetTime())
		{
			return gcLogger.Error2(__METHOD__, " Event [", psPending->pcTag->GetName(), "] at [", LongToString(psPending->iTime), "] isn't after the simulation's time.", NULL);
		}
		if (psPending->pcPortEventXML != NULL)
		{
			pcEvent = psPending->pcPortEventXML->Load(psPending->pcTag, psPending->pcPort);
		}
		else
		{
			pcEvent = psPending->pcIntegratedCircuitEventXML->Load(psPending->pcTag, psPending->pcIntegratedCircuit);
		}
		if (pcEvent == NULL)
		{
			return false;
		}
		if (!pcTimeline->AddEvent(pcEvent))
		{
			return gcLogger.Error2(__METHOD__, " Could not add event [", psPending->pcTag->GetName(), "] to the timeline.", NULL);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// ICs the file says nothing about keep what they were made with.
//
//////////////////////////////////////////////////////////////////////////
bool LoadSimulationXML(CMarkupTag* pcSimulationTag, CCircuitView* pcCircuit, CCircuitSimulation* pcSimulation)
{
	STimelineData			sTimeline;
	STagIterator			sIter;
	CMarkupTag*				pcTag;
	int64					iIndex;
	CComponentView*			pcView;
	CComponentStateXML*		pcStateXML;
	CArrayPendingEventXML	asEvents;
	bool					bResult;

	if (!pcSimulationTag->Is("Simulation"))
	{
		return gcLogger.Error2(__METHOD__, " Expected a [Simulation] element, not [", pcSimulationTag->GetName(), "].", NULL);
	}
	if (!pcSimulationTag->GetInt64Attribute("time", &sTimeline.iTime) || !pcSimulationTag->GetInt64Attribute("previousEventTime", &sTimeline.iPreviousEventTime) || !pcSimulationTag->GetInt64Attribute("eventTime", &sTimeline.iEventTime))
	{
		return gcLogger.Error2(__METHOD__, " [Simulation] needs whole number time, previousEventTime and eventTime.", NULL);
	}
	pcSimulation->GetTimeline()->Load(&sTimeline);

	asEvents.Init();
	bResult = true;
	pcTag = pcSimulationTag->GetTag(&sIter);
	while ((pcTag != NULL) && bResult)
	{
		if (!pcTag->Is("Component") || !pcTag->GetInt64Attribute("index", &iIndex) || (iIndex < 0) || ((size)iIndex >= pcCircuit->NumComponents()))
		{
			bResult = gcLogger.Error2(__METHOD__, " Expected a [Component] with the index of one of the circuit's components, not [", pcTag->GetName(), "].", NULL);
			break;
		}
		pcView = pcCircuit->GetComponent((size)iIndex);
		pcStateXML = pcView->GetComponentStateXML();
		if ((pcView->GetComponent() == NULL) || (pcStateXML == NULL))
		{
			bResult = gcLogger.Error2(__METHOD__, " Component [", pcView->GetName(), "] has no simulation state to load.", NULL);
			break;
		}
		bResult = pcStateXML->Load(pcTag, pcView->GetComponent(), &asEvents);
		pcTag = pcSimulationTag->GetNextTag(&sIter);
	}

	if (bResult)
	{
		bResult = MakePendingEventsXML(&asEvents, pcSimulation->GetTimeline());
	}
	asEvents.Kill();
	return bResult;
}

