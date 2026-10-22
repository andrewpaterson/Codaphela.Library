#ifndef __META_BUS_8_H__
#define __META_BUS_8_H__
#include "BaseLib/PrimitiveTypes.h"
#include "W65MetaSignal.h"


class CW65MetaBus8 : public CW65MetaSignal
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

