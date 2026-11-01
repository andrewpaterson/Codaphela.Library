#ifndef __CONSTRUCTOR_CALL_TEST_CLASSES_H__
#define __CONSTRUCTOR_CALL_TEST_CLASSES_H__
#include "BaseLib/ConstructorCall.h"
#include "ConstructorsTestClasses.h"


class CConstructorCallTest1
{
public:
	const char*	GetName(void);
};


class CConstructorCallTest2
{
public:
	virtual const char*	GetName(void);
};


class CConstructorCallTest3
{
public:
	virtual const char*	GetName(void) =0;
};


class CConstructorCaller1 : public CPostMalloc<CConstructorCallTest1>
{
};


class CConstructorCaller2 : public CPostMalloc<CConstructorCallTest2>
{
};


class CConstructorCaller3 : public CPostMalloc<CConstructorCallTest3>
{
};


class CConstructorCaller4 : public CPostMalloc<CConstructorTestClass>
{
};


#endif // __CONSTRUCTOR_CALL_TEST_CLASSES_H__

