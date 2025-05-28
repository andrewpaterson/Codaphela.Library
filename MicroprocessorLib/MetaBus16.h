#ifndef __META_BUS_16_H__
#define __META_BUS_16_H__
#include "BaseLib/PrimitiveTypes.h"


class CMetaBus16
{
protected:
	uint16	muiValue;

public:
	void	Init(void);
	void	Kill(void);

	void	Set(uint16 uiValue);
	uint16	Get(void);
};


#endif // __META_BUS_16_H__

