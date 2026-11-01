#include "BaseLib/MemoryStack.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemoryStackAdd(void)
{
	CMemoryStack	cStack;
	char			szNumbers[] = {"0123456789"};
	char			szLetters[] = {"ABCDEFGHIJ"};
	char			szStuff[] = {"%^&"};
	char*			sz1;
	char*			sz2;
	char*			sz3;
	char*			sz4;

	cStack.Init(100);

	sz1 = (char*)cStack.Add(10);
	memcpy(sz1, szNumbers, 10);
	sz2 = (char*)cStack.Add(11);
	memcpy(sz2, szLetters, 11);
	
	AssertString("0123456789ABCDEFGHIJ", (char*)cStack.GetData());
	
	cStack.Remove();  //Removing doesn't do anything unless every item is removed.
	AssertString("0123456789ABCDEFGHIJ", (char*)cStack.GetData());
	((char*)cStack.GetData())[20] = '.';

	sz3 = (char*)cStack.Add(4);
	memcpy(sz3, szStuff, 4);	
	AssertString("0123456789ABCDEFGHIJ.%^&", (char*)cStack.GetData());

	cStack.Remove(2);
	sz4 = (char*)cStack.Add(4);
	memcpy(sz4, szStuff, 4);	
	AssertString("%^&", (char*)cStack.GetData());
	
	cStack.Kill();

	AssertNull(cStack.GetData());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemoryStackOutOfMemory(void)
{
	CMemoryStack	cStack;
	void*			pvData;

	cStack.Init(3);
	pvData = cStack.Add(2);
	AssertNotNull(pvData);

	pvData = cStack.Add(2);
	AssertNull(pvData);

	pvData = cStack.Add(1);
	AssertNotNull(pvData);

	pvData = cStack.Add(1);
	AssertNull(pvData);

	cStack.Clear();

	pvData = cStack.Add(3);
	AssertNotNull(pvData);

	pvData = cStack.Add(1);
	AssertNull(pvData);

	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemoryStack(void)
{
	BeginTests();

	TestMemoryStackAdd();
	TestMemoryStackOutOfMemory();

	TestStatistics();
}


