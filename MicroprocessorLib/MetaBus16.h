#ifndef __META_BUS_16_H__
#define __META_BUS_16_H__
#include "BaseLib/PrimitiveTypes.h"
#include "MetaSignal.h"


class CMetaBus16 : CMetaSignal
{
protected:
	uint16	muiValue;

public:
	void	Init(void);
	void	Kill(void);

	void	Set(uint16 uiValue);
	uint16	Get(void);
	void	Print(CChars* psz, bool bAppend0x);
};


#endif // __META_BUS_16_H__

