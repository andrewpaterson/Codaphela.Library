#ifndef __BUS_CYCLE_ARRAY_H__
#define __BUS_CYCLE_ARRAY_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "BusCycle.h"


typedef CArrayTemplatePtr<CBusCycle> __CBusCycleArray;


class CBusCycleArray : public __CBusCycleArray
{
public:
	void Kill(void);
};


#endif // __BUS_CYCLE_ARRAY_H__


