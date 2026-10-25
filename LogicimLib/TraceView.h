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
#ifndef __TRACE_VIEW_H__
#define __TRACE_VIEW_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/Int32Vec2.h"
#include "BaseLib/XMLWriter.h"
#include "BaseLib/MarkupTag.h"
#include "MicroprocessorLib/Trace.h"
#include "CircuitPainter.h"


typedef CArrayTemplatePtr<CTrace>	CArrayTraceViewTracePtr;


class CViewPath;
class CCircuitSimulation;


//A wire's traces in one circuit simulation of one view path.
struct SViewPathTraces
{
	CViewPath*					pcViewPath;
	CCircuitSimulation*			pcCircuitSimulation;
	CArrayTraceViewTracePtr		apcTraces;
};


typedef CArrayTemplate<SViewPathTraces>	CArrayViewPathTraces;


//A straight piece of wire between two grid points, as Logicim's TraceView.  It carries one trace per bit of the
//ports it joins and is coloured by their average voltage.  A junction dot is drawn on an end where it meets a port or
//where three or more wires meet.  Its traces are kept for each view path and circuit simulation its circuit is
//simulated in, as Logicim's.
class CTraceView : public CConstructable
{
CONSTRUCTABLE(CTraceView);
protected:
	SInt32Vec2					msStart;
	SInt32Vec2					msEnd;
	CArrayViewPathTraces		masTraces;
	bool						mbStartJunction;
	bool						mbEndJunction;

public:
			void				Init(int32 x0, int32 y0, int32 x1, int32 y1);
	virtual void				Kill(void);

			bool				LoadXML(CMarkupTag* pcTag);
			void				SaveXML(CXMLWriter* pcWriter);

			void				Paint(CCircuitPainter* pcPainter);

			bool				Contains(SInt32Vec2 sPoint);
			bool				IsEnd(SInt32Vec2 sPoint);
			bool				Touches(CTraceView* pcOther);

			void						SetTraces(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation, CArrayTraceViewTracePtr* papcTraces);
			void						ClearTraces(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation);
			CArrayTraceViewTracePtr*	GetTraces(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation);
			size						NumTraces(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation);
			CTrace*						GetTrace(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation, size uiIndex);
			void				SetJunctions(bool bStart, bool bEnd);
			bool				IsStartJunction(void);
			bool				IsEndJunction(void);
			SInt32Vec2			GetStart(void);
			SInt32Vec2			GetEnd(void);
			void				SetEnds(SInt32Vec2 sStart, SInt32Vec2 sEnd);

protected:
			ARGB32				GetColour(CCircuitPainter* pcPainter);
			SViewPathTraces*	FindTraces(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation, size* puiIndex);
};


typedef CArrayTemplatePtr<CTraceView>	CArrayTraceViewPtr;


#endif // __TRACE_VIEW_H__
