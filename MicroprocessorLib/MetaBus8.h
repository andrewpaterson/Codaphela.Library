#ifndef __META_BUS_8_H__
#define __META_BUS_8_H__
#include "BaseLib/PrimitiveTypes.h"
#include "MetaSignal.h"


class CMetaBus8 : public CMetaSignal
{
protected:
	uint8	muiValue;

public:
	void	Init(void);
	void	Kill(void);

	void	Set(uint8 uiValue);
	uint8	Get(void);

	void	Print(CChars* psz, bool bAppend0x);
};


#endif // __META_BUS_8_H__

