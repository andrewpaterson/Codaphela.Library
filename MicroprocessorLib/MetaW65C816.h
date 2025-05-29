#ifndef __META_W65C816_H__
#define __META_W65C816_H__
#include "W65C816.h"
#include "Timeline.h"


class CMetaW65C816
{
protected:
	CW65C816		mcMPU;
	CW65C816Pins	mcPins;
	CTimeline		mcTimeline;
	CMetaBus16 		mcAddress;
	CMetaBus8 		mcData;
	CMetaTrace 		mcVPB;
	CMetaTrace 		mcRDY;
	CMetaTrace 		mcABORTB;
	CMetaTrace 		mcIRQB;
	CMetaTrace 		mcNMIB;
	CMetaTrace 		mcMLB;
	CMetaTrace 		mcVPA;
	CMetaTrace 		mcVDA;
	CMetaTrace 		mcRESB;
	CMetaTrace 		mcMX;
	CMetaTrace 		mcPHI2;
	CMetaTrace 		mcBE;
	CMetaTrace 		mcE;
	CMetaTrace 		mcRWB;

public:
	void			Init(void);
	void			Kill(void);

	CW65C816*		GetMPU(void);
	CMetaBus16*		GetAddress(void);
	CMetaBus8* 		GetData(void);
	CMetaTrace* 	GetVPB(void);
	CMetaTrace* 	GetRDY(void);
	CMetaTrace* 	GetABORTB(void);
	CMetaTrace* 	GetIRQB(void);
	CMetaTrace* 	GetNMIB(void);
	CMetaTrace* 	GetMLB(void);
	CMetaTrace* 	GetVPA(void);
	CMetaTrace* 	GetVDA(void);
	CMetaTrace* 	GetRESB(void);
	CMetaTrace* 	GetMX(void);
	CMetaTrace* 	GetPHI2(void);
	CMetaTrace* 	GetBE(void);
	CMetaTrace* 	GetE(void);
	CMetaTrace* 	GetRWB(void);

	void			TickInstruction(void);

	void			Print(CChars* psz);
	void			Dump(void);
};


#endif // __META_W65C816_H__

