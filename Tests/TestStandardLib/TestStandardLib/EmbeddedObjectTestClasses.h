#ifndef __EMBEDDED_OBJECT_TEST_CLASSES_H__
#define __EMBEDDED_OBJECT_TEST_CLASSES_H__
#include "StandardLib/Object.h"
#include "StandardLib/Pointer.h"
#include "StandardLib/Array.h"
#include "StandardLib/ObjectWriter.h"
#include "StandardLib/ObjectReader.h"
#include "ObjectTestClasses.h"


class CEmbeddedTest : public CObject
{
CONSTRUCTABLE(CEmbeddedTest)
DESTRUCTABLE(CEmbeddedTest)
public:
	int32	miAmANumber;
	float	mfSoAmI;
	Ptr<>	mpTest;

	void	Class(void) override;
	void	Init(void);
	void	Free(void) override;

	bool	Save(CObjectWriter* pcFile) override;
	bool	Load(CObjectReader* pcFile) override;
};


class CEmbeddedContainer : public CObject
{
CONSTRUCTABLE(CEmbeddedContainer)
DESTRUCTABLE(CEmbeddedContainer)
public:
	char				msz[4];
	CEmbeddedTest		mcOne;
	int32				mi;
	CEmbeddedTest		mcTwo;
	float				mf;
	Ptr<>				mpTest;

	void				Class(void) override;
	void				Init(void);
	void				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;
	Ptr<CEmbeddedTest>	GetOne(void);
	Ptr<CEmbeddedTest>	GetTwo(void);
};


class CEmbeddedComplex : public CObject
{
CONSTRUCTABLE(CEmbeddedComplex)
DESTRUCTABLE(CEmbeddedComplex)
public:
	CEmbeddedTest		mcSimple;
	int32				mai[2];
	CArray<>			ma;
	CEmbeddedContainer	mcContainer;
	Ptr<>				mpTest;

	void	Class(void) override;
	void	Init(void);
	void	Free(void) override;

	void	Add(Ptr<> pObject);

	bool	Save(CObjectWriter* pcFile) override;
	bool	Load(CObjectReader* pcFile) override;
};


class CEmbeddedTestWithNotifier : public CObject
{
CONSTRUCTABLE(CEmbeddedTestWithNotifier)
DESTRUCTABLE(CEmbeddedTestWithNotifier)
public:
	STestObjectFreedNotifier*	mpsFreedNotifier;
	Ptr<>						mpTest;

	void	Class(void) override;
	void	Init(STestObjectFreedNotifier* psFreedNotifier, Ptr<> pTest);
	void	Free(void) override;

	bool	Save(CObjectWriter* pcFile) override;
	bool	Load(CObjectReader* pcFile) override;
};


class CEmbeddedContainerWithNotifier : public CObject
{
CONSTRUCTABLE(CEmbeddedContainerWithNotifier)
DESTRUCTABLE(CEmbeddedContainerWithNotifier)
public:
	CEmbeddedTestWithNotifier	mcOne;
	CEmbeddedTestWithNotifier	mcTwo;
	CEmbeddedTestWithNotifier	mcThree;
	STestObjectFreedNotifier*	mpsFreedNotifier;
	Ptr<>						mpTest;

	void							Class(void) override;
	void							Init(STestObjectFreedNotifier* psFreedNotifier, Ptr<> pTest, Ptr<> pOne, Ptr<> pTwo, Ptr<> pThree, STestObjectFreedNotifier* psFreedOne, STestObjectFreedNotifier* psFreedTwo, STestObjectFreedNotifier* psFreedThree);
	void							Free(void) override;

	bool							Save(CObjectWriter* pcFile) override;
	bool							Load(CObjectReader* pcFile) override;
	Ptr<CEmbeddedTestWithNotifier>	GetOne(void);
	Ptr<CEmbeddedTestWithNotifier>	GetTwo(void);
	Ptr<CEmbeddedTestWithNotifier>	GetThree(void);
};


#endif // __EMBEDDED_OBJECT_TEST_CLASSES_H__

