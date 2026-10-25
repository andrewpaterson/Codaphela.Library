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
#ifndef __LOGIC_GATE_VIEW_H__
#define __LOGIC_GATE_VIEW_H__
#include "BaseLib/ArrayInt.h"
#include "IntegratedCircuitView.h"


//Logicim's OR shape: the arcs are parts of ellipses twice the gate's half width across and three grid units high.
#define OR_FILL_OFFSET			0.25
#define OR_ARC_UPPER_START		264
#define OR_ARC_SIDE_LENGTH		66
#define OR_ARC_LOWER_START		30
#define OR_ARC_BACK_START		-29
#define OR_ARC_BACK_LENGTH		58
#define OR_ARC_FILL_OFFSET		6


//The properties a gate is saved with.  The strings point into the tag they were read from.
struct SLogicGateXML
{
	int32		x;
	int32		y;
	ERotation	eRotation;
	char*		szName;
	char*		szFamily;
	bool		bExplicitPowerPorts;
	int32		iInputCount;
	int32		iInputWidth;
};


//The gates and buffers, as Logicim's BaseGateView and LogicGateView.  A gate has its inputs on the bottom (grid y 1,
//or further back) spread either side of its position, skipping 0 for an even count, and its output two grid units up.
//Each input is iInputWidth ports wide; the IC has iInputCount * iInputWidth inputs named "Input n".
class CLogicGateView : public CIntegratedCircuitView
{
CONSTRUCTABLE(CLogicGateView);
protected:
	int		miInputCount;
	int		miInputWidth;
	bool	mbNegated;

public:
			void	Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int iInputCount, int iInputWidth, bool bNegated);

			int		GetInputCount(void);
	virtual void	GetProperties(CComponentProperties* pcProperties, SComponentPropertiesContext* psContext);
			int		GetInputWidth(void);
			bool	IsNegated(void);

	static	void	CalculatePortOffsets(CArrayInt* paiOffsets, int iInputCount);
	static	float64	CalculateWidth(int iInputCount);
	static	void	GetPortNames(CArrayChars* paszNames, const char* szName, int iPortNumber, int iInputWidth, int iInputCount);

	static	bool	LoadLogicGateXML(CMarkupTag* pcTag, SLogicGateXML* psGate);

protected:
	virtual	void	SaveXMLAttributes(CXMLWriter* pcWriter);
			void	CreateGatePortViews(int iInputOffset);
	virtual	bool	IncludeExtraBounds(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum, bool bHasBounds);
};


#endif // __LOGIC_GATE_VIEW_H__
