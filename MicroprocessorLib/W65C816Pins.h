#ifndef __W65C816_PINS_H__
#define __W65C816_PINS_H__
#include "W65CPins.h"


class CW65C816Pins : public CW65CPins
{
protected:
	CMetaTrace*		mpcVPA;
	CMetaTrace*		mpcVDA;
	CMetaTrace*		mpcMX;
	CMetaTrace*		mpcE;

public:
	void			Init(CMetaBus16* pcAddress, CMetaBus8* pcData, CMetaTrace* pcPHI2, CMetaTrace* pcRESB, CMetaTrace* pcRWB);
	void			SetImportantTraces(CMetaTrace* pcVPA, CMetaTrace* pcVDA, CMetaTrace* pcVPB);
	void			SetOtherTraces(CMetaTrace* pcMLB, CMetaTrace* pcMX, CMetaTrace* pcBE, CMetaTrace* pcE, CMetaTrace* pcRDY);
	void			Kill(void);

	//Microprocessor side methods.
	void			WriteMX(CTimeline* pcTimeline, bool bValue);
	void			WriteVDA(CTimeline* pcTimeline, bool bValue);
	void			WriteVPA(CTimeline* pcTimeline, bool bValue);
	void			WriteE(CTimeline* pcTimeline, bool bValue);
	STraceValue		ReadMX(CTimeline* pcTimeline);
};


#endif // __W65C816_PINS_H__

