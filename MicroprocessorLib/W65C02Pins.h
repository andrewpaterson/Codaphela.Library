#ifndef __W65C02_PINS_H__
#define __W65C02_PINS_H__
#include "W65CPins.h"


class CW65C02Pins : public CW65CPins
{
protected:
	CMetaTrace*			mpcSYNC;
	CMetaTrace*			mpcSOB;

public:
	void			Init(CMetaBus16* pcAddress, CMetaBus8* pcData, CMetaTrace* pcPHI2, CMetaTrace* pcRESB, CMetaTrace* pcRWB);
	void			SetW65C02Traces(CMetaTrace* pcSYNC, CMetaTrace* pcSOB);
	void			Kill(void);

	//Microprocessor side methods.
	void			WriteSYNC(CTimeline* pcTimeline, bool bValue);
	STraceValue			ReadSOB(CTimeline* pcTimeline);
};


#endif // __W65C02_PINS_H__

