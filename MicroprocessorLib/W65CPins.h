#ifndef __W65C_PINS_H__
#define __W65C_PINS_H__
#include "BaseLib/PrimitiveTypes.h"
#include "W65TraceValue.h"
#include "W65MetaBus8.h"
#include "W65MetaBus16.h"
#include "W65MetaTrace.h"


//The pins the W65C02S and W65C816S have in common.  Processor specific pins are added by subclasses.
class CW65Timeline;
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
	void			WriteRWB(CW65Timeline* pcTimeline, bool bValue);
	void			WriteMLB(CW65Timeline* pcTimeline, bool bValue);
	void			WriteVPB(CW65Timeline* pcTimeline, bool bValue);
	void			WriteRdy(CW65Timeline* pcTimeline, bool bValue);
	void			WriteAddress(CW65Timeline* pcTimeline, uint16 uiAddress);
	void			WriteData(CW65Timeline* pcTimeline, uint8 uiData);
	uint8			ReadData(CW65Timeline* pcTimeline);
	uint16			ReadAddress(CW65Timeline* pcTimeline);
	SW65TraceValue		ReadRES(CW65Timeline* pcTimeline);
	SW65TraceValue		ReadPhi2(CW65Timeline* pcTimeline);
	SW65TraceValue		ReadNMI(CW65Timeline* pcTimeline);
	SW65TraceValue		ReadIRQ(CW65Timeline* pcTimeline);
	SW65TraceValue		ReadAbort(CW65Timeline* pcTimeline);
	SW65TraceValue		ReadBE(CW65Timeline* pcTimeline);
};


#endif // __W65C_PINS_H__

