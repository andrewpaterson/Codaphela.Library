#include "TestLib/Assert.h"
#include "BaseLib/CharsHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCharsHelperIntersect(void)
{
    CArrayChars   aszOne;
    CArrayChars   aszTwo;
    CArrayChars   aszResult;

    aszOne.Init();
    aszOne.Add("Hello");
    aszOne.Add("World");
    aszOne.Add("How");
    aszOne.Add("Are");
    aszOne.Add("You?");

    aszTwo.Init();
    aszTwo.Add("My");
    aszTwo.Add("There");
    aszTwo.Add("Hello");
    aszTwo.Add("Are");
    aszTwo.Add("How");
    aszTwo.Add("You");
    aszTwo.Add("Doing");

    aszResult.Init();
    CCharsHelper::Intersect(&aszResult, &aszOne, &aszTwo);

    AssertSize(3, aszResult.NumElements());
    AssertString("Are", aszResult.Get(0)->Text());
    AssertString("Hello", aszResult.Get(1)->Text());
    AssertString("How", aszResult.Get(2)->Text());

    aszOne.Kill();
    aszTwo.Kill();
    aszResult.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCharsHelper(void)
{
    BeginTests();

    TestCharsHelperIntersect();

    TestStatistics();
}

