#include "TestLib/Assert.h"
#include "StandardLib/Action.h"


class CTargetForAction
{
public:
	int		miCorrectCode;

	void Init(void)
	{
		miCorrectCode = 0;
	}

	void Kill(void)
	{
		miCorrectCode = 0;
	}

	void Dingo(CAction* pcAction, void* pvContext)
	{
		char* szCode;

		szCode = (char*)pvContext;

		if (strcmp(szCode, "Eustance") == 0)
		{
			miCorrectCode = 7;
		}
		else
		{
			miCorrectCode = 6;
		}
	};

	void Coyote(CAction* pcAction, void* pvContext)
	{
		miCorrectCode = 1;
	};

	void Sydney(CAction* pcAction, void* pvContext)
	{
		float fValue;

		fValue = *((float*)pvContext);
		if (fValue == 0.753f)
		{
			miCorrectCode = 4;
		}
		else
		{
			miCorrectCode = 6;
		}
	};

	void Named(CAction* pcAction, void* pvContext)
	{
		if (pcAction->Is("Frisky"))
		{
			miCorrectCode = 5;
		}
		else
		{
			miCorrectCode = 6;
		}
	};

	void Nabia(CAction* pcAction, void* pvContext)
	{
		if (pvContext == NULL)
		{
			miCorrectCode = 3;
		}
		else
		{
			miCorrectCode = 6;
		}
	};
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestActionEvent(void)
{
	BeginTests();

	CAction				cEvent;
	CTargetForAction	cTarget;
	float				f;

	cTarget.Init();
	cEvent.Init("Frisky");
	cEvent.Add(&cTarget, &CTargetForAction::Dingo);
	cEvent.Call((void*)"Eustance");
	cEvent.Kill();
	AssertInt(7, cTarget.miCorrectCode);
	cTarget.Kill();

	cTarget.Init();
	cEvent.Init("Frisky");
	cEvent.Add(&cTarget, &CTargetForAction::Coyote);
	cEvent.Call(NULL);
	cEvent.Kill();
	AssertInt(1, cTarget.miCorrectCode);
	cTarget.Kill();

	cTarget.Init();
	cEvent.Init("Frisky");
	cEvent.Add(&cTarget, &CTargetForAction::Sydney);
	f = 0.753f;
	cEvent.Call(&f);
	cEvent.Kill();
	AssertInt(4, cTarget.miCorrectCode);
	cTarget.Kill();

	cTarget.Init();
	cEvent.Init("Frisky");
	cEvent.Add(&cTarget, &CTargetForAction::Nabia);
	cEvent.Call((void*)NULL);
	cEvent.Kill();
	AssertInt(3, cTarget.miCorrectCode);
	cTarget.Kill();

	cTarget.Init();
	cEvent.Init("Frisky");
	cEvent.Add(&cTarget, &CTargetForAction::Named);
	cEvent.Call(NULL);
	AssertInt(5, cTarget.miCorrectCode);
	AssertTrue(cEvent.Remove(&cTarget));
	AssertSize(0, cEvent.NumCalls());
	cTarget.miCorrectCode = 0;
	cEvent.Call(NULL);
	AssertInt(0, cTarget.miCorrectCode);
	cEvent.Kill();
	cTarget.Kill();

	TestStatistics();
}

