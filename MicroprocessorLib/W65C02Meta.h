#ifndef __META_W65C02_H__
#define __META_W65C02_H__
#include "W65C02.h"
#include "W65CMeta.h"


class CW65C02Meta;
typedef void(*W65C02MetaTickHigh)(CW65C02Meta*, void*);
typedef void(*W65C02MetaTickLow)(CW65C02Meta*, void*);


class CW65C02Meta : public CW65CMeta
{
protected:
	CW65C02					mcMPU;
	CW65C02Pins				mcPins;
	CW65MetaTrace 				mcSYNC;
	CW65MetaTrace 				mcSOB;

	W65C02MetaTickHigh		mfTickHigh;
	W65C02MetaTickLow		mfTickLow;

public:
	void			Init(W65C02MetaTickHigh fTickHigh, W65C02MetaTickLow fTickLow, void* pvContext);
	void			Kill(void);

	CW65CCore*		GetCore(void) override;
	CW65C02*		GetMPU(void);
	CW65MetaTrace* 	GetSYNC(void);
	CW65MetaTrace* 	GetSOB(void);

	bool			IsResetInstruction(void);
	bool			IsStopInstruction(void);

protected:
	bool			IsOpcodeFetchSignalled(void) override;
	void			LatchPhi2Low(void) override;
	void			CallTickHigh(void) override;
	void			CallTickLow(void) override;
};


#endif // __META_W65C02_H__

