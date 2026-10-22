#ifndef __META_W65C_H__
#define __META_W65C_H__
#include "W65CCore.h"
#include "Timeline.h"


//The traces and clocking the W65C02S and W65C816S meta processors share.  A subclass owns
//the processor and its pins, latches what the processor puts on the data bus while PHI2
//is low and calls the tick callbacks.
class CInstruction;
class CMetaW65C
{
protected:
	CTimeline				mcTimeline;
	CMetaBus16 				mcAddress;
	CMetaBus8 				mcData;
	CMetaTrace 				mcVPB;
	CMetaTrace 				mcRDY;
	CMetaTrace 				mcIRQB;
	CMetaTrace 				mcNMIB;
	CMetaTrace 				mcMLB;
	CMetaTrace 				mcRESB;
	CMetaTrace 				mcPHI2;
	CMetaTrace 				mcBE;
	CMetaTrace 				mcRWB;

	void*					mpvContext;
	uint8					muiBank;

public:
	virtual CW65CCore*		GetCore(void) = 0;

	CMetaBus16*		GetAddress(void);
	CMetaBus8* 		GetData(void);
	CMetaTrace* 	GetVPB(void);
	CMetaTrace* 	GetRDY(void);
	CMetaTrace* 	GetIRQB(void);
	CMetaTrace* 	GetNMIB(void);
	CMetaTrace* 	GetMLB(void);
	CMetaTrace* 	GetRESB(void);
	CMetaTrace* 	GetPHI2(void);
	CMetaTrace* 	GetBE(void);
	CMetaTrace* 	GetRWB(void);

	bool			TickInstruction(void);
	bool			TestTickCycle(void);
	bool			IsInstruction(CInstruction* pcExpectedInstruction);
	bool			IsFetchOpcodeCycle(void);

	void			SetBank(uint8 uiBank);
	uint8			GetBank(void);

protected:
	void			InitTraces(void* pvContext);
	void			KillTraces(void);
	void			InputTransition(void);

	virtual bool	IsOpcodeFetchSignalled(void) = 0;
	virtual void	LatchPhi2Low(void) = 0;
	virtual void	CallTickHigh(void) = 0;
	virtual void	CallTickLow(void) = 0;
};


#endif // __META_W65C_H__

