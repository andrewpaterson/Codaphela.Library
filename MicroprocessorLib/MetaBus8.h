#ifndef __META_BUS_8_H__
#define __META_BUS_8_H__
#include "BaseLib/PrimitiveTypes.h"


class CMetaBus8
{
protected:
	uint8	muiValue;

public:
	void	Init(void);
	void	Kill(void);

	void	Set(uint8 uiValue);
	uint8	Get(void);
};


#endif // __META_BUS_8_H__

