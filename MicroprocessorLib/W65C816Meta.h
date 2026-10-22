#ifndef __META_W65C816_H__
#define __META_W65C816_H__
#include "W65C816.h"
#include "W65CMeta.h"

class CW65C816Meta;
typedef void(*W65C816MetaTickHigh)(CW65C816Meta*, void*);
typedef void(*W65C816MetaTickLow)(CW65C816Meta*, void*);


class CW65C816Meta : public CW65CMeta
{
protected:
	CW65C816				mcMPU;
	CW65C816Pins			mcPins;
	CW65MetaTrace 				mcABORTB;
	CW65MetaTrace 				mcVPA;
	CW65MetaTrace 				mcVDA;
	CW65MetaTrace 				mcMX;
	CW65MetaTrace 				mcE;

	W65C816MetaTickHigh		mfTickHigh;
	W65C816MetaTickLow		mfTickLow;

	SW65TraceValue				msX;

public:
	void			Init(W65C816MetaTickHigh fTickHigh, W65C816MetaTickLow fTickLow, void* pvContext);
	void			Kill(void);

	CW65CCore*		GetCore(void) override;
	CW65C816*		GetMPU(void);
	CW65MetaTrace* 	GetABORTB(void);
	CW65MetaTrace* 	GetVPA(void);
	CW65MetaTrace* 	GetVDA(void);
	CW65MetaTrace* 	GetMX(void);
	CW65MetaTrace* 	GetE(void);

	bool			IsResetInstruction(void);
	bool			IsStopInstruction(void);

	void			SetX(SW65TraceValue sX);
	SW65TraceValue		GetX(void);

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

