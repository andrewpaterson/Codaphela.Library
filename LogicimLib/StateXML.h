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
#ifndef __STATE_XML_H__
#define __STATE_XML_H__
#include "BaseLib/XMLWriter.h"
#include "BaseLib/MarkupTag.h"
#include "MicroprocessorLib/State.h"


//Saves and loads one kind of integrated circuit state.  The helpers sit beside the domain's CState classes so the
//domain knows nothing of files: each names the state class it handles (its ClassName) and the element it is saved as.
//Load makes a new state from an element; the caller gives it to its IC.
class CStateXML
{
public:
	virtual const char*	GetStateClassName(void) =0;
	virtual const char*	GetXMLName(void) =0;
	virtual void		Save(CXMLWriter* pcWriter, CState* pcState) =0;
	virtual CState*		Load(CMarkupTag* pcTag) =0;
};


class CClockOscillatorStateXML : public CStateXML
{
public:
	const char*	GetStateClassName(void) override;
	const char*	GetXMLName(void) override;
	void		Save(CXMLWriter* pcWriter, CState* pcState) override;
	CState*		Load(CMarkupTag* pcTag) override;
};


class CConstantStateXML : public CStateXML
{
public:
	const char*	GetStateClassName(void) override;
	const char*	GetXMLName(void) override;
	void		Save(CXMLWriter* pcWriter, CState* pcState) override;
	CState*		Load(CMarkupTag* pcTag) override;
};


class CCounterStateXML : public CStateXML
{
public:
	const char*	GetStateClassName(void) override;
	const char*	GetXMLName(void) override;
	void		Save(CXMLWriter* pcWriter, CState* pcState) override;
	CState*		Load(CMarkupTag* pcTag) override;
};


class CDTypeFlipFlopStateXML : public CStateXML
{
public:
	const char*	GetStateClassName(void) override;
	const char*	GetXMLName(void) override;
	void		Save(CXMLWriter* pcWriter, CState* pcState) override;
	CState*		Load(CMarkupTag* pcTag) override;
};


//Each input's samples are a <Samples> element of space separated minimum and maximum voltages.
class COscilloscopeStateXML : public CStateXML
{
public:
	const char*	GetStateClassName(void) override;
	const char*	GetXMLName(void) override;
	void		Save(CXMLWriter* pcWriter, CState* pcState) override;
	CState*		Load(CMarkupTag* pcTag) override;
};


CStateXML*	FindStateXMLForState(CState* pcState);
CStateXML*	FindStateXMLForElement(const char* szXMLName);


#endif // __STATE_XML_H__
