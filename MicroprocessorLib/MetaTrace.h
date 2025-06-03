#ifndef __META_TRACE_H__
#define __META_TRACE_H__
#include "BaseLib/PrimitiveTypes.h"
#include "TraceValue.h"
#include "MetaSignal.h"


extern STraceValue gcMetaTraceUndriven;


class CMetaTrace : public CMetaSignal
{
protected:
	STraceValue		msValue;

public:
	void			Init(void);
	void			Init(bool bValue);
	void			Kill(void);

	void			Set(bool bValue);
	STraceValue		Get(void);
	void			Invert(void);
	bool			IsHigh(void);
	bool			IsLow(void);

	void			Print(CChars* psz);
};


#define SafeSetTrace(p, b) if (p) { p->Set(b); }
#define SafeGetTrace(p) if (p) { return p->Get(); } else { return gcMetaTraceUndriven; }


#endif // !__META_TRACE_H__

