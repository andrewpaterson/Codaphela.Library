#ifndef __W65C_PINS_H__
#define __W65C_PINS_H__
#include "BaseLib/PrimitiveTypes.h"
#include "TraceValue.h"
#include "MetaBus8.h"
#include "MetaBus16.h"
#include "MetaTrace.h"


//The pins the W65C02S and W65C816S have in common.  Processor specific pins are added by subclasses.
class CTimeline;
class CW65CPins
{
protected:
	CMetaBus16*			mpcAddress;
	CMetaBus8*			mpcData;
	CMetaTrace*			mpcVPB;
	CMetaTrace*			mpcRDY;
	CMetaTrace*			mpcABORTB;
	CMetaTrace*			mpcIRQB;
	CMetaTrace*			mpcNMIB;
	CMetaTrace*			mpcMLB;
	CMetaTrace*			mpcRESB;
	CMetaTrace*			mpcPHI2;
	CMetaTrace*			mpcBE;
	CMetaTrace*			mpcRWB;

public:
	void			Init(CMetaBus16* pcAddress, CMetaBus8* pcData, CMetaTrace* pcPHI2, CMetaTrace* pcRESB, CMetaTrace* pcRWB);
	void			SetInterruptTraces(CMetaTrace* pcABORTB, CMetaTrace* pcIRQB, CMetaTrace* pcNMIB);
	void			SetVectorPullTrace(CMetaTrace* pcVPB);
	void			SetControlTraces(CMetaTrace* pcMLB, CMetaTrace* pcBE, CMetaTrace* pcRDY);
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

