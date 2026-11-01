#ifndef __UNKNOWN_KILL_LIST_H__
#define __UNKNOWN_KILL_LIST_H__
#include "BaseLib/ArrayTemplate.h"


class CUnknownTestHelper;
class CKillList : public CArrayTemplate<CUnknownTestHelper*>
{
public:
	void AddKill(CUnknownTestHelper* pcKilled);
	bool Contains(CUnknownTestHelper* pcKilled);
};


#endif // __UNKNOWN_KILL_LIST_H__

