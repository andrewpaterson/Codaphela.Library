#ifndef __ARRAY_TEMPLATE_TEST_CLASSES__
#define __ARRAY_TEMPLATE_TEST_CLASSES__
#include "BaseLib/PrimitiveTypes.h"


struct STestArrayTemplateItem
{
	int32	i1;
	int32	i2;
};


class CTestArrayTemplateItem : public STestArrayTemplateItem
{
public:
	virtual void	Init(int i);
	virtual const char*	GetName(void);
};


class CTestOverridenArrayTemplateItem : public CTestArrayTemplateItem
{
public:
	void	Init(int i);
	const char*	GetName(void);
};


#endif // __ARRAY_TEMPLATE_TEST_CLASSES__


