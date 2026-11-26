#ifndef __META_TRACE_H__
#define __META_TRACE_H__
#include "BaseLib/PrimitiveTypes.h"
#include "TraceValue.h"
#include "MetaSignal.h"
#include "LogicSignal.h"


extern STraceValue gcMetaTraceUndriven;


//An ideal trace.  A write is seen at once and the timeline is ignored, see CLogicSignal.
class CMetaTrace : public CMetaSignal, public CLogicSignal
{
protected:
	STraceValue			msValue;

public:
	void			Init(void);
	void			Init(bool bValue);
	void			Kill(void);

	void			Set(bool bValue);
	STraceValue		Get(void) const;
	void			Invert(void);
	bool			IsHigh(void) const;
	bool			IsLow(void) const;

	void			Print(CChars* psz);

	virtual void			Write(CTimeline* pcTimeline, bool bValue);
	virtual void			WriteImpedance(CTimeline* pcTimeline);
	virtual STraceValue		Read(CTimeline* pcTimeline);
};


#endif // __META_TRACE_H__

