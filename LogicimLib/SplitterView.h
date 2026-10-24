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
#ifndef __SPLITTER_VIEW_H__
#define __SPLITTER_VIEW_H__
#include "BaseLib/ArrayInt.h"
#include "ComponentView.h"


#define SPLITTER_FONT_SIZE			7.0
#define SPLITTER_BUS_LINE_WIDTH		4.0


//Which way a splitter's ends run from its bus, as Logicim's SplitterAppearance.
enum ESplitterAppearance
{
	SA_Cannot,
	SA_LeftHanded,
	SA_RightHanded,
};


//Splits a bus into ends, as Logicim's SplitterView.  The bus comes in at its start, a grid unit up and iEndOffset
//along from the position, onto a bar; the iFanOut ends leave the bar downwards iGridSpacing apart (to the left, or
//to the right when right handed) and stop two grid units down.  Bit b of the bus goes to end aiSplitIndices[b], in
//order with the end's other bits; each end is labelled with its bits.  An end with no bits has no port.
//
//The splitter joins each bit of the bus to its bit of an end onto one trace (see GetBitJoins); its domain component
//only holds the ports.  (Logicim paired the bus's bits with the ends' ports by name, which only matches this when the
//split indices are in order.)
class CSplitterView : public CComponentView
{
CONSTRUCTABLE(CSplitterView);
protected:
	int32					miBitWidth;
	int32					miFanOut;
	int32					miGridSpacing;
	ESplitterAppearance		meAppearance;
	int32					miEndOffset;
	CArrayInt				maiSplitIndices;
	CArrayInt				maiEndPortViews;	//By fan: the end's port view, or -1 with no bits.

public:
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName, int32 iBitWidth, int32 iFanOut, int32 iGridSpacing, ESplitterAppearance eAppearance, int32 iEndOffset, int* aiSplitIndices);
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName, int32 iBitWidth, int32 iFanOut, int32 iGridSpacing, ESplitterAppearance eAppearance, int32 iEndOffset);
	virtual void					Kill(void);

	virtual const char*				GetType(void);
	virtual CCircuitComponent*		CreateComponent(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation, CFamilyVoltageConfigurationStore* pcFamilies);
	virtual CComponentStateXML*		GetComponentStateXML(void);
	virtual void					GetBitJoins(CArrayPortBitJoin* pasJoins);
	virtual const char*				GetXMLName(void);
	virtual bool					LoadXML(CMarkupTag* pcTag);

			int32					GetBitWidth(void);
			int32					GetFanOut(void);
			int32					GetSplitIndex(int32 iBit);
			CPortView*				GetStartPortView(void);
			CPortView*				GetEndPortView(int32 iFan);

	static	void					CreateSplitIndices(int* aiSplitIndices, int32 iBitWidth, int32 iFanOut);
	static	void					AppendBitsText(CChars* psz, CArrayInt* paiBits);

protected:
	virtual	void					SaveXMLAttributes(CXMLWriter* pcWriter);
			SFloat64Vec2			GetFanPosition(int32 iFan, float64 y);
			SFloat64Vec2			GetStartPosition(void);
			SFloat64Vec2			GetBarPosition(void);
			void					GetFanBits(CArrayInt* paiBits, int32 iFan);
			void					GetPortNames(CArrayChars* paszStartNames, CArrayChars* paszEndNames);
};


const char*			SplitterAppearanceToString(ESplitterAppearance eAppearance);
ESplitterAppearance	StringToSplitterAppearance(const char* szAppearance);


#endif // __SPLITTER_VIEW_H__
