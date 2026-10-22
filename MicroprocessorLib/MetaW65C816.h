#ifndef __META_W65C816_H__
#define __META_W65C816_H__
#include "W65C816.h"
#include "MetaW65C.h"

class CMetaW65C816;
typedef void(*MetaW65C816TickHigh)(CMetaW65C816*, void*);
typedef void(*MetaW65C816TickLow)(CMetaW65C816*, void*);


class CMetaW65C816 : public CMetaW65C
{
protected:
	CW65C816				mcMPU;
	CW65C816Pins			mcPins;
	CMetaTrace 				mcABORTB;
	CMetaTrace 				mcVPA;
	CMetaTrace 				mcVDA;
	CMetaTrace 				mcMX;
	CMetaTrace 				mcE;

	MetaW65C816TickHigh		mfTickHigh;
	MetaW65C816TickLow		mfTickLow;

	STraceValue				msX;

public:
	void			Init(MetaW65C816TickHigh fTickHigh, MetaW65C816TickLow fTickLow, void* pvContext);
	void			Kill(void);

	CW65CCore*		GetCore(void) override;
	CW65C816*		GetMPU(void);
	CMetaTrace* 	GetABORTB(void);
	CMetaTrace* 	GetVPA(void);
	CMetaTrace* 	GetVDA(void);
	CMetaTrace* 	GetMX(void);
	CMetaTrace* 	GetE(void);

	bool			IsResetInstruction(void);
	bool			IsStopInstruction(void);

	void			SetX(STraceValue sX);
	STraceValue		GetX(void);

	void			Print(CChars* psz);
	void			Print(CChars* psz, bool bMnemonic, bool bCycle, bool bOperation, bool bA, bool bX, bool bY, bool bPC, bool bS, bool bDP, bool bDB, bool bP, bool bAddressBus, bool bDataBus, bool bBank, bool bVPB, bool bRDY, bool bMLB, bool bVPA, bool bVDA, bool bWM, bool bWX, bool bE, bool bRWB, bool bAddressOperation, bool bDataOperation, bool bDescription);
	void			Dump(void);

protected:
	bool			IsOpcodeFetchSignalled(void) override;
	void			LatchPhi2Low(void) override;
	void			CallTickHigh(void) override;
	void			CallTickLow(void) override;
};


#endif // __META_W65C816_H__

