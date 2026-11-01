#ifndef __META_BUS_16_H__
#define __META_BUS_16_H__
#include "BaseLib/PrimitiveTypes.h"
#include "MetaSignal.h"
#include "LogicBus.h"


//An ideal 16 bit bus.  A write is seen at once and the timeline is ignored, see CLogicBus.
class CMetaBus16 : public CMetaSignal, public CLogicBus
{
protected:
	uint16	muiValue;

public:
	void	Init(void);
	void	Kill(void);

	void	Set(uint16 uiValue);
	uint16	Get(void);
	void	Print(CChars* psz, bool bAppend0x);

	virtual void	Write(CTimeline* pcTimeline, uint32 uiValue);
	virtual void	WriteImpedance(CTimeline* pcTimeline);
	virtual uint32	Read(CTimeline* pcTimeline);
	virtual size	GetWidth(void);
};


#endif // __META_BUS_16_H__

