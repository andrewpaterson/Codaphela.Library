#ifndef __META_W65C02_H__
#define __META_W65C02_H__
#include "W65C02.h"
#include "MetaW65C.h"


class CMetaW65C02;
typedef void(*MetaW65C02TickHigh)(CMetaW65C02*, void*);
typedef void(*MetaW65C02TickLow)(CMetaW65C02*, void*);


class CMetaW65C02 : public CMetaW65C
{
protected:
	CW65C02					mcMPU;
	CW65C02Pins				mcPins;
	CMetaTrace 				mcSYNC;
	CMetaTrace 				mcSOB;

	MetaW65C02TickHigh		mfTickHigh;
	MetaW65C02TickLow		mfTickLow;

public:
	void			Init(MetaW65C02TickHigh fTickHigh, MetaW65C02TickLow fTickLow, void* pvContext);
	void			Kill(void);

	CW65CCore*		GetCore(void) override;
	CW65C02*		GetMPU(void);
	CMetaTrace* 	GetSYNC(void);
	CMetaTrace* 	GetSOB(void);

	bool			IsResetInstruction(void);
	bool			IsStopInstruction(void);

protected:
	bool			IsOpcodeFetchSignalled(void) override;
	void			LatchPhi2Low(void) override;
	void			CallTickHigh(void) override;
	void			CallTickLow(void) override;
};


#endif // __META_W65C02_H__

