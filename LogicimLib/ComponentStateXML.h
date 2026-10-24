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
#ifndef __COMPONENT_STATE_XML_H__
#define __COMPONENT_STATE_XML_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/XMLWriter.h"
#include "BaseLib/MarkupTag.h"
#include "EventXML.h"


class CCircuitComponent;
class CIntegratedCircuit;
class CLogicPort;
class CPortOutput;


//An event read from a file but not made yet.  Events are made once all are read, in time and then ID order.
struct SPendingEventXML
{
	int64						iTime;
	int64						iID;
	CMarkupTag*					pcTag;
	CPortEventXML*				pcPortEventXML;
	CLogicPort*					pcPort;
	CIntegratedCircuitEventXML*	pcIntegratedCircuitEventXML;
	CIntegratedCircuit*			pcIntegratedCircuit;
};


typedef CArrayTemplate<SPendingEventXML>	CArrayPendingEventXML;


//Saves and loads the simulation state of one kind of circuit component, beside the domain's component classes; its
//view says which helper it needs.  Save writes the children of the component's <Component> element; Load reads them
//back, adding the events it finds to papsEvents for the simulation to make.
class CComponentStateXML
{
public:
	virtual void	Save(CXMLWriter* pcWriter, CCircuitComponent* pcComponent) =0;
	virtual bool	Load(CMarkupTag* pcComponentTag, CCircuitComponent* pcComponent, CArrayPendingEventXML* pasEvents) =0;
};


//An IC's state (by its CStateXML), its own events, and each logic port's output and events in a <Port name="...">.
class CIntegratedCircuitStateXML : public CComponentStateXML
{
public:
	void	Save(CXMLWriter* pcWriter, CCircuitComponent* pcComponent) override;
	bool	Load(CMarkupTag* pcComponentTag, CCircuitComponent* pcComponent, CArrayPendingEventXML* pasEvents) override;

protected:
	void	SavePort(CXMLWriter* pcWriter, CLogicPort* pcPort);
	bool	LoadPort(CMarkupTag* pcPortTag, CIntegratedCircuit* pcIntegratedCircuit, CArrayPendingEventXML* pasEvents);
	void	SaveOutput(CXMLWriter* pcWriter, CPortOutput* pcOutput);
	bool	LoadOutput(CMarkupTag* pcOutputTag, CPortOutput* pcOutput);
};


//Passive components (supplies, pins, splitters) keep no state of their own.
class CPassiveStateXML : public CComponentStateXML
{
public:
	void	Save(CXMLWriter* pcWriter, CCircuitComponent* pcComponent) override;
	bool	Load(CMarkupTag* pcComponentTag, CCircuitComponent* pcComponent, CArrayPendingEventXML* pasEvents) override;
};


extern CIntegratedCircuitStateXML	gcIntegratedCircuitStateXML;
extern CPassiveStateXML				gcPassiveStateXML;


#endif // __COMPONENT_STATE_XML_H__
