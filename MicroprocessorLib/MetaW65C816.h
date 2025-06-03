#ifndef __META_W65C816_H__
#define __META_W65C816_H__
#include "W65C816.h"
#include "Timeline.h"

class CMetaW65C816;
typedef void(*MetaW65C816TickHigh)(CMetaW65C816*, void*);
typedef void(*MetaW65C816TickLow)(CMetaW65C816*, void*);


class CMetaW65C816
{
protected:
	CW65C816				mcMPU;
	CW65C816Pins			mcPins;
	CTimeline				mcTimeline;
	CMetaBus16 				mcAddress;
	CMetaBus8 				mcData;
	CMetaTrace 				mcVPB;
	CMetaTrace 				mcRDY;
	CMetaTrace 				mcABORTB;
	CMetaTrace 				mcIRQB;
	CMetaTrace 				mcNMIB;
	CMetaTrace 				mcMLB;
	CMetaTrace 				mcVPA;
	CMetaTrace 				mcVDA;
	CMetaTrace 				mcRESB;
	CMetaTrace 				mcMX;
	CMetaTrace 				mcPHI2;
	CMetaTrace 				mcBE;
	CMetaTrace 				mcE;
	CMetaTrace 				mcRWB;

	MetaW65C816TickHigh		mfTickHigh;
	MetaW65C816TickLow		mfTickLow;
	void*					mpvContext;

	uint8					muiBank;
	STraceValue				msX;

public:
	void			Init(MetaW65C816TickHigh fTickHigh, MetaW65C816TickLow fTickLow, void* pvContext);
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

	bool			TickInstruction(void);
	bool			IsResetInstruction(void);
	bool			IsStopInstruction(void);
	bool			IsInstruction(CInstruction* pcExpectedInstruction);
	bool			IsFetchOpcodeCycle(void);

	void			SetBank(uint8 uiBank);
	uint8			GetBank(void);
	void			SetX(STraceValue sX);
	STraceValue		GetX(void);


	void			Print(CChars* psz);
	void			Print(CChars* psz, bool bMnemonic, bool bCycle, bool bOperation, bool bA, bool bX, bool bY, bool bPC, bool bS, bool bDP, bool bDB, bool bP, bool bAddressBus, bool bDataBus, bool bBank, bool bVPB, bool bRDY, bool bMLB, bool bVPA, bool bVDA, bool bWM, bool bWX, bool bE, bool bRWB, bool bAddressOperation, bool bDataOperation, bool bDescription);
	void			Dump(void);
};


#endif // __META_W65C816_H__

