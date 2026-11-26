#ifndef __META_W65C_H__
#define __META_W65C_H__
#include "W65CCore.h"
#include "MetaBus8.h"
#include "MetaBus16.h"
#include "MetaTrace.h"
#include "Timeline.h"


//The traces and clocking the W65C02S and W65C816S meta processors share.  A subclass owns
//the processor and its pins, latches what the processor puts on the data bus while PHI2
//is low and calls the tick callbacks.
class CW65Instruction;
class CW65CMeta
{
protected:
	CTimeline					mcTimeline;
	CMetaBus16					mcAddress;
	CMetaBus8					mcData;
	CMetaTrace					mcVPB;
	CMetaTrace					mcRDY;
	CMetaTrace					mcIRQB;
	CMetaTrace					mcNMIB;
	CMetaTrace					mcMLB;
	CMetaTrace					mcRESB;
	CMetaTrace					mcPHI2;
	CMetaTrace					mcBE;
	CMetaTrace					mcRWB;

	void*						mpvContext;
	uint8						muiBank;

public:
	virtual CW65CCore*		GetCore(void) = 0;

	CMetaBus16*		GetAddress(void);
	CMetaBus8*		GetData(void);
	const CMetaTrace*	GetVPB(void) const;
	const CMetaTrace*	GetRDY(void) const;
	CMetaTrace*		GetIRQB(void);
	CMetaTrace*		GetNMIB(void);
	const CMetaTrace*	GetMLB(void) const;
	CMetaTrace*		GetRESB(void);
	const CMetaTrace*	GetPHI2(void) const;
	const CMetaTrace*	GetBE(void) const;
	const CMetaTrace*	GetRWB(void) const;

	bool			TickInstruction(void);
	bool			TestTickCycle(void);
	bool			IsInstruction(CW65Instruction* pcExpectedInstruction);
	bool			IsFetchOpcodeCycle(void);

	void			SetBank(uint8 uiBank);
	uint8			GetBank(void) const;

protected:
	void			InitTraces(void* pvContext);
	void			KillTraces(void);
	void			InputTransition(void);

	virtual bool	IsOpcodeFetchSignalled(void) const = 0;
	virtual void	LatchPhi2Low(void) = 0;
	virtual void	CallTickHigh(void) = 0;
	virtual void	CallTickLow(void) = 0;
};


#endif // __META_W65C_H__

