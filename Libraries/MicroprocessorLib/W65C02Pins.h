#ifndef __W65C02_PINS_H__
#define __W65C02_PINS_H__
#include "W65CPins.h"


class CW65C02Pins : public CW65CPins
{
protected:
	CLogicSignal*		mpcSYNC;
	CLogicSignal*		mpcSOB;

public:
	void			Init(CLogicBus* pcAddress, CLogicBus* pcData, CLogicSignal* pcPHI2, CLogicSignal* pcRESB, CLogicSignal* pcRWB);
	void			SetW65C02Traces(CLogicSignal* pcSYNC, CLogicSignal* pcSOB);
	void			Kill(void);

	//Microprocessor side methods.
	void			WriteSYNC(CTimeline* pcTimeline, bool bValue);
	STraceValue			ReadSOB(CTimeline* pcTimeline);
};


#endif // __W65C02_PINS_H__

