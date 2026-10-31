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
#ifndef __LOGIC_BLOCK_VIEW_H__
#define __LOGIC_BLOCK_VIEW_H__
#include "BaseLib/ArrayTemplate.h"
#include "IntegratedCircuitView.h"


#define LOGIC_BLOCK_FONT_SIZE		8.0
#define LOGIC_BLOCK_NAME_LENGTH		16
#define LOGIC_BLOCK_MAX_BITS		32
#define LOGIC_BLOCK_MAX_GROUPS		4


//The generic chips drawn as a block of labelled pins.
enum ELogicBlockKind
{
	LBK_Register,
	LBK_Latch,
	LBK_BusBuffer,
	LBK_BusTransceiver,
	LBK_Multiplexer,
	LBK_Decoder,
	LBK_Adder,
	LBK_Comparator,
	LBK_Counter,
	LBK_FlipFlops,
	LBK_NumKinds
};


//What sizes a block's chip: iBits is the bits of each group (a multiplexer's select bits, a decoder's address bits)
//and iGroups the groups (a multiplexer's channels).  A bus buffer has iEnables active low enables a group; a decoder
//iEnables in all, the first iLowEnables active low.  bOneBased labels an adder's bits from 1, as a 74LVC283's are.
struct SLogicBlockParams
{
	int		iBits;
	int		iGroups;
	int		iEnables;
	int		iLowEnables;
	bool	bInverting;
	bool	bChannelEnables;
	bool	bDisabledHighImpedance;
	bool	bSynchronousLoad;
	bool	bOneBased;
};


//One of a block's pins: its label, the name of its chip's port, which side and row it's on, and whether it's
//inverted (a bubble) or a clock (a notch).
struct SLogicBlockPin
{
	char	szLabel[LOGIC_BLOCK_NAME_LENGTH];
	char	szPortName[LOGIC_BLOCK_NAME_LENGTH];
	bool	bRight;
	int		iRow;
	bool	bInverted;
	bool	bClock;
};


typedef CArrayTemplate<SLogicBlockPin>	CArrayLogicBlockPin;


//A generic chip drawn as a box of labelled pins, inputs and controls down its left and outputs down its right a grid
//unit apart, and its title at the top: a register, latch, bus buffer, bus transceiver, multiplexer, decoder, adder or
//equality comparator of any size (see SLogicBlockParams).  Each kind is saved as its own element ("Register") and is
//its own class below, so a view knows what it is before it's loaded.  The real 74 series parts made from them (see
//CLogic74PackageView) are drawn the same way with their part's labels.
class CLogicBlockView : public CIntegratedCircuitView
{
CONSTRUCTABLE(CLogicBlockView);
protected:
	ELogicBlockKind		meKind;
	SLogicBlockParams	msParams;

public:
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, ELogicBlockKind eKind, SLogicBlockParams* psParams);

	virtual const char*				GetType(void);
	virtual const char*				GetXMLName(void);
	virtual void					GetProperties(CComponentProperties* pcProperties, SComponentPropertiesContext* psContext);
	virtual bool					InitFromProperties(int32 x, int32 y, ERotation eRotation, CComponentProperties* pcProperties, SComponentPropertiesContext* psContext);
	virtual bool					LoadXML(CMarkupTag* pcTag);
	virtual CIntegratedCircuit*		CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily);
	virtual ELogicBlockKind			GetBlockKind(void);

			ELogicBlockKind			GetKind(void);
			SLogicBlockParams*		GetParams(void);
			CPortView*				GetPortViewByLabel(const char* szLabel, bool bInverted);

	static	const char*				GetKindXMLName(ELogicBlockKind eKind);
	static	const char*				GetKindTitle(ELogicBlockKind eKind);
	static	bool					FindKind(const char* szXMLName, ELogicBlockKind* peKind);
	static	void					GetDefaultParams(ELogicBlockKind eKind, SLogicBlockParams* psParams);
	static	void					ClampParams(ELogicBlockKind eKind, SLogicBlockParams* psParams);
	static	void					GetPins(ELogicBlockKind eKind, SLogicBlockParams* psParams, CArrayLogicBlockPin* pasPins);
	static	CIntegratedCircuit*		CreateChip(ELogicBlockKind eKind, SLogicBlockParams* psParams, CSubcircuitSimulation* pcSimulation, const char* szName, CFamilyVoltageConfiguration* pcFamily);

protected:
	virtual	void					SaveXMLAttributes(CXMLWriter* pcWriter);
			void					CreateBlock(const char* szTitle, const char* szSubtitle);
};


//A logic block of one kind.
#define LOGIC_BLOCK_KIND_VIEW(Class, Kind)																	class Class : public CLogicBlockView																			{																												CONSTRUCTABLE(Class);																							public:																												virtual ELogicBlockKind			GetBlockKind(void) { return Kind; }										};


LOGIC_BLOCK_KIND_VIEW(CRegisterView, LBK_Register)
LOGIC_BLOCK_KIND_VIEW(CLatchView, LBK_Latch)
LOGIC_BLOCK_KIND_VIEW(CBusBufferView, LBK_BusBuffer)
LOGIC_BLOCK_KIND_VIEW(CBusTransceiverView, LBK_BusTransceiver)
LOGIC_BLOCK_KIND_VIEW(CMultiplexerView, LBK_Multiplexer)
LOGIC_BLOCK_KIND_VIEW(CDecoderView, LBK_Decoder)
LOGIC_BLOCK_KIND_VIEW(CAdderView, LBK_Adder)
LOGIC_BLOCK_KIND_VIEW(CComparatorView, LBK_Comparator)


#endif // __LOGIC_BLOCK_VIEW_H__

