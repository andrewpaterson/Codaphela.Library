#ifndef __FAKE_TRACKER_H__
#define __FAKE_TRACKER_H__
#include "BaseLib/Define.h"
#include "BaseLib/IndexedGeneral.h"


class CFakeTracker
{
public:
	void* GetWithID(OIndex id);
	void* Get(int index);
	int   Num(void);
};


#endif // __FAKE_TRACKER_H__

