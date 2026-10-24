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
#ifndef __EVENT_XML_H__
#define __EVENT_XML_H__
#include "BaseLib/XMLWriter.h"
#include "BaseLib/MarkupTag.h"
#include "MicroprocessorLib/Event.h"


class CLogicPort;
class CIntegratedCircuit;
class CPortEvent;
class CIntegratedCircuitEvent;


//Saves and loads one kind of pending simulation event, beside the domain's event classes.  Every event is saved with
//its time and ID.  Load makes the event for its port or IC, which the event's Init adds it to; the caller adds it to
//the timeline.  Events are made in time and then ID order so each time's events run in the order they did.
class CEventXML
{
public:
	virtual const char*	GetEventClassName(void) =0;
	virtual const char*	GetXMLName(void) =0;
	virtual void		Save(CXMLWriter* pcWriter, CEvent* pcEvent);

	static	bool		LoadTimeAndID(CMarkupTag* pcTag, int64* piTime, int64* piID);

protected:
	virtual void		SaveAttributes(CXMLWriter* pcWriter, CEvent* pcEvent);
};


//Events on a logic port.
class CPortEventXML : public CEventXML
{
public:
	virtual CPortEvent*	Load(CMarkupTag* pcTag, CLogicPort* pcPort) =0;
};


//Events on an IC.
class CIntegratedCircuitEventXML : public CEventXML
{
public:
	virtual CIntegratedCircuitEvent*	Load(CMarkupTag* pcTag, CIntegratedCircuit* pcIntegratedCircuit) =0;
};


class CDriveEventXML : public CPortEventXML
{
public:
	const char*	GetEventClassName(void) override;
	const char*	GetXMLName(void) override;
	CPortEvent*	Load(CMarkupTag* pcTag, CLogicPort* pcPort) override;

protected:
	void		SaveAttributes(CXMLWriter* pcWriter, CEvent* pcEvent) override;
};


class CSlewEventXML : public CPortEventXML
{
public:
	const char*	GetEventClassName(void) override;
	const char*	GetXMLName(void) override;
	CPortEvent*	Load(CMarkupTag* pcTag, CLogicPort* pcPort) override;

protected:
	void		SaveAttributes(CXMLWriter* pcWriter, CEvent* pcEvent) override;
};


class CTransitionEventXML : public CPortEventXML
{
public:
	const char*	GetEventClassName(void) override;
	const char*	GetXMLName(void) override;
	CPortEvent*	Load(CMarkupTag* pcTag, CLogicPort* pcPort) override;

protected:
	void		SaveAttributes(CXMLWriter* pcWriter, CEvent* pcEvent) override;
};


class CTickEventXML : public CIntegratedCircuitEventXML
{
public:
	const char*					GetEventClassName(void) override;
	const char*					GetXMLName(void) override;
	CIntegratedCircuitEvent*	Load(CMarkupTag* pcTag, CIntegratedCircuit* pcIntegratedCircuit) override;
};


CPortEventXML*					FindPortEventXMLForEvent(CEvent* pcEvent);
CPortEventXML*					FindPortEventXMLForElement(const char* szXMLName);
CIntegratedCircuitEventXML*		FindIntegratedCircuitEventXMLForEvent(CEvent* pcEvent);
CIntegratedCircuitEventXML*		FindIntegratedCircuitEventXMLForElement(const char* szXMLName);


#endif // __EVENT_XML_H__
