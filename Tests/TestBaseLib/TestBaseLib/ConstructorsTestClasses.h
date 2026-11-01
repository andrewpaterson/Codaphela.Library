#ifndef __CONSTRUCTORS_TEST_CLASSES_H__
#define __CONSTRUCTORS_TEST_CLASSES_H__


class CConstructorTestSuper1
{
public:
	int		i;

	virtual const char*	GetName(void) = 0;
	virtual int		GetSize(void) = 0;
	virtual int		Method1(void);
};


class CConstructorTestSuper2
{
public:
	int		j;

	virtual const char*	GetName(void);
	virtual int		GetSize(void);
	virtual int		Method2(void);
};


class CConstructorTestClass : public CConstructorTestSuper1, public CConstructorTestSuper2
{
public:
	int		k;

	const char*	GetName(void);
	int		GetSize(void);
	int		Method1(void);
};


#endif // __CONSTRUCTORS_TEST_CLASSES_H__

