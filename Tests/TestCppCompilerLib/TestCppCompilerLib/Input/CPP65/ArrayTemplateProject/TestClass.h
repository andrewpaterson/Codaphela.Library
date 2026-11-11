#ifndef __TEST_CLASS_H__
#define __TEST_CLASS_H__
#include "BaseLib/ArrayTemplate.h"


//An element for CArrayTemplate: not trivially constructible (CPostMalloc constructs each one) and polymorphic.
class CTestClass
{
public:
	static int	siMade;

	int			miValue;
	short		miSquare;

				CTestClass();
	void		Set(int iValue);
	virtual int	Describe(void);
};


typedef CArrayTemplate<CTestClass>	CArrayTestClass;


int SumDescriptions(CArrayTestClass* pacTests);


#endif // __TEST_CLASS_H__
