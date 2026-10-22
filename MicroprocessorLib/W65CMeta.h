#ifndef __META_W65C_H__
#define __META_W65C_H__
#include "W65CCore.h"
#include "W65Timeline.h"


//The traces and clocking the W65C02S and W65C816S meta processors share.  A subclass owns
//the processor and its pins, latches what the processor puts on the data bus while PHI2
//is low and calls the tick callbacks.
class CW65Instruction;
class CW65CMeta
{
protected:
	CW65Timeline				mcTimeline;
	CW65MetaBus16 				mcAddress;
	CW65MetaBus8 				mcData;
	CW65MetaTrace 				mcVPB;
	CW65MetaTrace 				mcRDY;
	CW65MetaTrace 				mcIRQB;
	CW65MetaTrace 				mcNMIB;
	CW65MetaTrace 				mcMLB;
	CW65MetaTrace 				mcRESB;
	CW65MetaTrace 				mcPHI2;
	CW65MetaTrace 				mcBE;
	CW65MetaTrace 				mcRWB;

	void*					mpvContext;
	uint8					muiBank;

public:
	virtual CW65CCore*		GetCore(void) = 0;

	CW65MetaBus16*		GetAddress(void);
	CW65MetaBus8* 		GetData(void);
	CW65MetaTrace* 	GetVPB(void);
	CW65MetaTrace* 	GetRDY(void);
	CW65MetaTrace* 	GetIRQB(void);
	CW65MetaTrace* 	GetNMIB(void);
	CW65MetaTrace* 	GetMLB(void);
	CW65MetaTrace* 	GetRESB(void);
	CW65MetaTrace* 	GetPHI2(void);
	CW65MetaTrace* 	GetBE(void);
	CW65MetaTrace* 	GetRWB(void);

	bool			TickInstruction(void);
	bool			TestTickCycle(void);
	bool			IsInstruction(CW65Instruction* pcExpectedInstruction);
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

