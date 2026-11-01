#include "ObjectTestClasses.h"
#include "StandardLib/Unknown.h"


class CUnknownObjectHolder : public CUnknown
{
CONSTRUCTABLE(CUnknownObjectHolder)
public:
	CTestObject*				mpcTestObject;
	STestObjectFreedNotifier*	mpsFreedNotifier;

	void	Init(STestObjectFreedNotifier* psKilledNotifier);
	void	Kill(void);
};


class CObjectWithUnknown : public CObject
{
CONSTRUCTABLE(CObjectWithUnknown)
DESTRUCTABLE(CObjectWithUnknown)
public:
	CUnknownObjectHolder*		mpcHolder;
	STestObjectFreedNotifier*	mpsFreedNotifier;

	void	Init(STestObjectFreedNotifier* psKilledNotifier);
	void	Class(void) override;
	void	Free(void) override;

	bool	Save(CObjectWriter* pcFile) override;
	bool	Load(CObjectReader* pcFile) override;

	void	AllocateUnknown(STestObjectFreedNotifier* psFreedNotifier);
};

