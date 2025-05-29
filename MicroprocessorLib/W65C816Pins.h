#ifndef __W65C816_PINS_H__
#define __W65C816_PINS_H__
#include "BaseLib/PrimitiveTypes.h"
#include "TraceValue.h"
#include "MetaBus8.h"
#include "MetaBus16.h"
#include "MetaTrace.h"


class CTimeline;
class CW65C816Pins
{
protected:
	CMetaBus16*		mpcAddress;
	CMetaBus8*		mpcData;
	CMetaTrace*		mpcVPB;
	CMetaTrace*		mpcRDY;
	CMetaTrace*		mpcABORTB;
	CMetaTrace*		mpcIRQB;
	CMetaTrace*		mpcNMIB;
	CMetaTrace*		mpcMLB;
	CMetaTrace*		mpcVPA;
	CMetaTrace*		mpcVDA;
	CMetaTrace*		mpcRESB;
	CMetaTrace*		mpcMX;
	CMetaTrace*		mpcPHI2;
	CMetaTrace*		mpcBE;
	CMetaTrace*		mpcE;
	CMetaTrace*		mpcRWB;

public:
	void			Init(CMetaBus16* pcAddress, CMetaBus8* pcData, CMetaTrace* pcPHI2, CMetaTrace* pcRESB, CMetaTrace* pcRWB);
	void			SetImportantTraces(CMetaTrace* pcVPA, CMetaTrace* pcVDA, CMetaTrace* pcVPB);
	void			SetInterruptTraces(CMetaTrace* pcABORTB, CMetaTrace* pcIRQB, CMetaTrace* pcNMIB);
	void			SetOtherTraces(CMetaTrace* pcMLB, CMetaTrace* pcMX, CMetaTrace* pcBE, CMetaTrace* pcE, CMetaTrace* pcRDY);
	void			Kill(void);

	//Microprocessor side methods.
	void			WriteRWB(CTimeline* pcTimeline, bool bValue);
	void			WriteMX(CTimeline* pcTimeline, bool bValue);
	void			WriteVDA(CTimeline* pcTimeline, bool bValue);
	void			WriteVPA(CTimeline* pcTimeline, bool bValue);
	void			WriteMLB(CTimeline* pcTimeline, bool bValue);
	void			WriteVPB(CTimeline* pcTimeline, bool bValue);
	void			WriteE(CTimeline* pcTimeline, bool bValue);
	void			WriteRdy(CTimeline* pcTimeline, bool bValue);
	void			WriteAddress(CTimeline* pcTimeline, uint16 uiAddress);
	void			WriteData(CTimeline* pcTimeline, uint8 uiData);

	uint8			ReadData(CTimeline* pcTimeline);
	uint16			ReadAddress(CTimeline* pcTimeline);
	STraceValue		ReadRES(CTimeline* pcTimeline);
	STraceValue		ReadPhi2(CTimeline* pcTimeline);
	STraceValue		ReadNMI(CTimeline* pcTimeline);
	STraceValue		ReadIRQ(CTimeline* pcTimeline);
	STraceValue		ReadAbort(CTimeline* pcTimeline);
	STraceValue		ReadBE(CTimeline* pcTimeline);
};


#endif // __W65C816_PINS_H__

