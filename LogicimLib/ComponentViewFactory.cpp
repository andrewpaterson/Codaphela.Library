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
#include "BaseLib/StringHelper.h"
#include "AndGateView.h"
#include "OrGateView.h"
#include "BufferView.h"
#include "Logic74View.h"
#include "LogicBlockView.h"
#include "PowerSourceView.h"
#include "ValueView.h"
#include "FlipFlopView.h"
#include "CounterView.h"
#include "ClockView.h"
#include "LabelView.h"
#include "TunnelView.h"
#include "W65C816View.h"
#include "65C64View.h"
#include "OscilloscopeView.h"
#include "SplitterView.h"
#include "SubcircuitInstanceView.h"
#include "ComponentViewFactory.h"


typedef CComponentView* (*NewComponentViewFunction)(void);


struct SComponentViewType
{
	const char*					szXMLName;
	NewComponentViewFunction	fNew;
};


//////////////////////////////////////////////////////////////////////////
//
// An allocated view of the class, not yet initialised: LoadXML or
// InitFromProperties does that.
//
//////////////////////////////////////////////////////////////////////////
template<class M>
CComponentView* NewComponentViewOfClass(void)
{
	return NewMalloc<M>();
}


//Every view a circuit file can hold, by the element name its GetXMLName gives.
SComponentViewType gasComponentViewTypes[] =
{
	{ "AndGate", NewComponentViewOfClass<CAndGateView> },
	{ "NandGate", NewComponentViewOfClass<CNandGateView> },
	{ "OrGate", NewComponentViewOfClass<COrGateView> },
	{ "NorGate", NewComponentViewOfClass<CNorGateView> },
	{ "XorGate", NewComponentViewOfClass<CXorGateView> },
	{ "XnorGate", NewComponentViewOfClass<CXnorGateView> },
	{ "Buffer", NewComponentViewOfClass<CBufferView> },
	{ "Inverter", NewComponentViewOfClass<CInverterView> },
	{ "Logic74Package", NewComponentViewOfClass<CLogic74PackageView> },
	{ "Logic74Unit", NewComponentViewOfClass<CLogic74UnitView> },
	{ "Register", NewComponentViewOfClass<CRegisterView> },
	{ "Latch", NewComponentViewOfClass<CLatchView> },
	{ "BusBuffer", NewComponentViewOfClass<CBusBufferView> },
	{ "BusTransceiver", NewComponentViewOfClass<CBusTransceiverView> },
	{ "Multiplexer", NewComponentViewOfClass<CMultiplexerView> },
	{ "Decoder", NewComponentViewOfClass<CDecoderView> },
	{ "Adder", NewComponentViewOfClass<CAdderView> },
	{ "Comparator", NewComponentViewOfClass<CComparatorView> },
	{ "Ground", NewComponentViewOfClass<CGroundView> },
	{ "Constant", NewComponentViewOfClass<CConstantView> },
	{ "Probe", NewComponentViewOfClass<CProbeView> },
	{ "DTypeFlipFlop", NewComponentViewOfClass<CDTypeFlipFlopView> },
	{ "Counter", NewComponentViewOfClass<CCounterView> },
	{ "Clock", NewComponentViewOfClass<CClockView> },
	{ "PositivePower", NewComponentViewOfClass<CPositivePowerView> },
	{ "Pin", NewComponentViewOfClass<CPinView> },
	{ "W65C816", NewComponentViewOfClass<CW65C816View> },
	{ "MPU65C64", NewComponentViewOfClass<C65C64View> },
	{ "Oscilloscope", NewComponentViewOfClass<COscilloscopeView> },
	{ "Splitter", NewComponentViewOfClass<CSplitterView> },
	{ "Tunnel", NewComponentViewOfClass<CTunnelView> },
	{ "Label", NewComponentViewOfClass<CLabelView> },
	{ "Subcircuit", NewComponentViewOfClass<CSubcircuitInstanceView> },
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponentView* NewComponentView(const char* szXMLName)
{
	size	i;

	for (i = 0; i < NumComponentViewTypes(); i++)
	{
		if (StringCompare(gasComponentViewTypes[i].szXMLName, szXMLName) == 0)
		{
			return gasComponentViewTypes[i].fNew();
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size NumComponentViewTypes(void)
{
	return sizeof(gasComponentViewTypes) / sizeof(SComponentViewType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* GetComponentViewXMLName(size uiIndex)
{
	if (uiIndex >= NumComponentViewTypes())
	{
		return NULL;
	}
	return gasComponentViewTypes[uiIndex].szXMLName;
}

