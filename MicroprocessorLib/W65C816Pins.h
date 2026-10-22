#ifndef __W65C816_PINS_H__
#define __W65C816_PINS_H__
#include "W65CPins.h"


class CW65C816Pins : public CW65CPins
{
protected:
	CW65MetaTrace*		mpcVPA;
	CW65MetaTrace*		mpcVDA;
	CW65MetaTrace*		mpcMX;
	CW65MetaTrace*		mpcE;

public:
	void			Init(CW65MetaBus16* pcAddress, CW65MetaBus8* pcData, CW65MetaTrace* pcPHI2, CW65MetaTrace* pcRESB, CW65MetaTrace* pcRWB);
	void			SetImportantTraces(CW65MetaTrace* pcVPA, CW65MetaTrace* pcVDA, CW65MetaTrace* pcVPB);
	void			SetOtherTraces(CW65MetaTrace* pcMLB, CW65MetaTrace* pcMX, CW65MetaTrace* pcBE, CW65MetaTrace* pcE, CW65MetaTrace* pcRDY);
	void			Kill(void);

	//Microprocessor side methods.
	void			WriteMX(CW65Timeline* pcTimeline, bool bValue);
	void			WriteVDA(CW65Timeline* pcTimeline, bool bValue);
	void			WriteVPA(CW65Timeline* pcTimeline, bool bValue);
	void			WriteE(CW65Timeline* pcTimeline, bool bValue);
	SW65TraceValue		ReadMX(CW65Timeline* pcTimeline);
};


#endif // __W65C816_PINS_H__

