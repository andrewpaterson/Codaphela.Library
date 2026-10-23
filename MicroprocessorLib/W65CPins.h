#ifndef __W65C_PINS_H__
#define __W65C_PINS_H__
#include "BaseLib/PrimitiveTypes.h"
#include "TraceValue.h"
#include "W65MetaBus8.h"
#include "W65MetaBus16.h"
#include "W65MetaTrace.h"


//The pins the W65C02S and W65C816S have in common.  Processor specific pins are added by subclasses.
class CTimeline;
class CW65CPins
{
protected:
	CW65MetaBus16*		mpcAddress;
	CW65MetaBus8*		mpcData;
	CW65MetaTrace*		mpcVPB;
	CW65MetaTrace*		mpcRDY;
	CW65MetaTrace*		mpcABORTB;
	CW65MetaTrace*		mpcIRQB;
	CW65MetaTrace*		mpcNMIB;
	CW65MetaTrace*		mpcMLB;
	CW65MetaTrace*		mpcRESB;
	CW65MetaTrace*		mpcPHI2;
	CW65MetaTrace*		mpcBE;
	CW65MetaTrace*		mpcRWB;

public:
	void			Init(CW65MetaBus16* pcAddress, CW65MetaBus8* pcData, CW65MetaTrace* pcPHI2, CW65MetaTrace* pcRESB, CW65MetaTrace* pcRWB);
	void			SetInterruptTraces(CW65MetaTrace* pcABORTB, CW65MetaTrace* pcIRQB, CW65MetaTrace* pcNMIB);
	void			SetVectorPullTrace(CW65MetaTrace* pcVPB);
	void			SetControlTraces(CW65MetaTrace* pcMLB, CW65MetaTrace* pcBE, CW65MetaTrace* pcRDY);
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

