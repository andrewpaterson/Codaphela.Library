#ifndef __W65C02_PINS_H__
#define __W65C02_PINS_H__
#include "W65CPins.h"


class CW65C02Pins : public CW65CPins
{
protected:
	CW65MetaTrace*		mpcSYNC;
	CW65MetaTrace*		mpcSOB;

public:
	void			Init(CW65MetaBus16* pcAddress, CW65MetaBus8* pcData, CW65MetaTrace* pcPHI2, CW65MetaTrace* pcRESB, CW65MetaTrace* pcRWB);
	void			SetW65C02Traces(CW65MetaTrace* pcSYNC, CW65MetaTrace* pcSOB);
	void			Kill(void);

	//Microprocessor side methods.
	void			WriteSYNC(CW65Timeline* pcTimeline, bool bValue);
	SW65TraceValue		ReadSOB(CW65Timeline* pcTimeline);
};


#endif // __W65C02_PINS_H__

