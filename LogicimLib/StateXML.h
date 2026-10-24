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


class CIntegratedCircuit;


//Saves and loads what one kind of IC remembers (a clock's level, a counter's count), beside the domain's IC classes so
//the domain knows nothing of files.  Each names the IC class it handles (its ClassName) and the element it's saved as.
//Load sets the remembered values on an IC of that class; ICs that remember nothing have no helper.
class CStateXML
{
public:
	virtual const char*	GetIntegratedCircuitClassName(void) =0;
	virtual const char*	GetXMLName(void) =0;
	virtual void		Save(CXMLWriter* pcWriter, CIntegratedCircuit* pcIntegratedCircuit) =0;
	virtual bool		Load(CMarkupTag* pcTag, CIntegratedCircuit* pcIntegratedCircuit) =0;
};


class CClockOscillatorStateXML : public CStateXML
{
public:
	const char*	GetIntegratedCircuitClassName(void) override;
	const char*	GetXMLName(void) override;
	void		Save(CXMLWriter* pcWriter, CIntegratedCircuit* pcIntegratedCircuit) override;
	bool		Load(CMarkupTag* pcTag, CIntegratedCircuit* pcIntegratedCircuit) override;
};


class CConstantStateXML : public CStateXML
{
public:
	const char*	GetIntegratedCircuitClassName(void) override;
	const char*	GetXMLName(void) override;
	void		Save(CXMLWriter* pcWriter, CIntegratedCircuit* pcIntegratedCircuit) override;
	bool		Load(CMarkupTag* pcTag, CIntegratedCircuit* pcIntegratedCircuit) override;
};


class CCounterStateXML : public CStateXML
{
public:
	const char*	GetIntegratedCircuitClassName(void) override;
	const char*	GetXMLName(void) override;
	void		Save(CXMLWriter* pcWriter, CIntegratedCircuit* pcIntegratedCircuit) override;
	bool		Load(CMarkupTag* pcTag, CIntegratedCircuit* pcIntegratedCircuit) override;
};


class CDTypeFlipFlopStateXML : public CStateXML
{
public:
	const char*	GetIntegratedCircuitClassName(void) override;
	const char*	GetXMLName(void) override;
	void		Save(CXMLWriter* pcWriter, CIntegratedCircuit* pcIntegratedCircuit) override;
	bool		Load(CMarkupTag* pcTag, CIntegratedCircuit* pcIntegratedCircuit) override;
};


//Each input's samples are a <Samples> element of space separated minimum and maximum voltages.  The input and sample
//counts come from the oscilloscope's properties, so a saved state must match them.
class COscilloscopeStateXML : public CStateXML
{
public:
	const char*	GetIntegratedCircuitClassName(void) override;
	const char*	GetXMLName(void) override;
	void		Save(CXMLWriter* pcWriter, CIntegratedCircuit* pcIntegratedCircuit) override;
	bool		Load(CMarkupTag* pcTag, CIntegratedCircuit* pcIntegratedCircuit) override;
};


CStateXML*	FindStateXMLForIntegratedCircuit(CIntegratedCircuit* pcIntegratedCircuit);
CStateXML*	FindStateXMLForElement(const char* szXMLName);


#endif // __STATE_XML_H__
