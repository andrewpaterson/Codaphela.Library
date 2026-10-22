#ifndef __META_BUS_32_H__
#define __META_BUS_32_H__
#include "BaseLib/PrimitiveTypes.h"
#include "W65MetaSignal.h"


class CW65MetaBus32 : public CW65MetaSignal
{
protected:
	uint32	muiValue;

public:
	void	Init(void);
	void	Kill(void);

	void	Set(uint32 uiValue);
	uint32	Get(void);
};


#endif // __META_BUS_32_H__

