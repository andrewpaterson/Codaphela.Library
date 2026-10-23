#ifndef __W65C_PINS_H__
#define __W65C_PINS_H__
#include "BaseLib/PrimitiveTypes.h"
#include "TraceValue.h"
#include "LogicSignal.h"
#include "LogicBus.h"


//The pins the W65C02S and W65C816S have in common.  Processor specific pins are added by subclasses.
class CTimeline;
class CW65CPins
{
protected:
	CLogicBus*			mpcAddress;
	CLogicBus*			mpcData;
	CLogicSignal*		mpcVPB;
	CLogicSignal*		mpcRDY;
	CLogicSignal*		mpcABORTB;
	CLogicSignal*		mpcIRQB;
	CLogicSignal*		mpcNMIB;
	CLogicSignal*		mpcMLB;
	CLogicSignal*		mpcRESB;
	CLogicSignal*		mpcPHI2;
	CLogicSignal*		mpcBE;
	CLogicSignal*		mpcRWB;

public:
	void			Init(CLogicBus* pcAddress, CLogicBus* pcData, CLogicSignal* pcPHI2, CLogicSignal* pcRESB, CLogicSignal* pcRWB);
	void			SetInterruptTraces(CLogicSignal* pcABORTB, CLogicSignal* pcIRQB, CLogicSignal* pcNMIB);
	void			SetVectorPullTrace(CLogicSignal* pcVPB);
	void			SetControlTraces(CLogicSignal* pcMLB, CLogicSignal* pcBE, CLogicSignal* pcRDY);
	void			Kill(void);

	//Microprocessor side methods.
	void			WriteRWB(CTimeline* pcTimeline, bool bValue);
	void			WriteMLB(CTimeline* pcTimeline, bool bValue);
	void			WriteVPB(CTimeline* pcTimeline, bool bValue);
	void			WriteRdy(CTimeline* pcTimeline, bool bValue);
	void			WriteAddress(CTimeline* pcTimeline, uint16 uiAddress);
	void			WriteData(CTimeline* pcTimeline, uint8 uiData);
	uint8			ReadData(CTimeline* pcTimeline);
	uint16			ReadAddress(CTimeline* pcTimeline);
	STraceValue			ReadRES(CTimeline* pcTimeline);
	STraceValue			ReadPhi2(CTimeline* pcTimeline);
	STraceValue			ReadNMI(CTimeline* pcTimeline);
	STraceValue			ReadIRQ(CTimeline* pcTimeline);
	STraceValue			ReadAbort(CTimeline* pcTimeline);
	STraceValue			ReadBE(CTimeline* pcTimeline);
};


#endif // __W65C_PINS_H__

