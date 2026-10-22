#ifndef __BUS_CYCLE_ARRAY_H__
#define __BUS_CYCLE_ARRAY_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "W65BusCycle.h"


typedef CArrayTemplatePtr<CW65BusCycle> __CW65BusCycleArray;


class CW65BusCycleArray : public __CW65BusCycleArray
{
public:
	void Kill(void);
};


#endif // __BUS_CYCLE_ARRAY_H__


