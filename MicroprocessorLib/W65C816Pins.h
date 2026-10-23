#ifndef __W65C816_PINS_H__
#define __W65C816_PINS_H__
#include "W65CPins.h"


class CW65C816Pins : public CW65CPins
{
protected:
	CLogicSignal*		mpcVPA;
	CLogicSignal*		mpcVDA;
	CLogicSignal*		mpcMX;
	CLogicSignal*		mpcE;

public:
	void			Init(CLogicBus* pcAddress, CLogicBus* pcData, CLogicSignal* pcPHI2, CLogicSignal* pcRESB, CLogicSignal* pcRWB);
	void			SetImportantTraces(CLogicSignal* pcVPA, CLogicSignal* pcVDA, CLogicSignal* pcVPB);
	void			SetOtherTraces(CLogicSignal* pcMLB, CLogicSignal* pcMX, CLogicSignal* pcBE, CLogicSignal* pcE, CLogicSignal* pcRDY);
	void			Kill(void);

	//Microprocessor side methods.
	void			WriteMX(CTimeline* pcTimeline, bool bValue);
	void			WriteVDA(CTimeline* pcTimeline, bool bValue);
	void			WriteVPA(CTimeline* pcTimeline, bool bValue);
	void			WriteE(CTimeline* pcTimeline, bool bValue);
	STraceValue			ReadMX(CTimeline* pcTimeline);
};


#endif // __W65C816_PINS_H__

