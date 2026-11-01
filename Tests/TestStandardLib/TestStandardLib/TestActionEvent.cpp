#include "TestLib/Assert.h"
#include "StandardLib/Action.h"


class CTargetForAction : public CListener
{
CONSTRUCTABLE(CTargetForAction)
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

	void Dingo(CUnknown* pcUnknown, void* pvContext)
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

	void Coyote(CUnknown* pcUnknown, void* pvContext)
	{
		miCorrectCode = 1;
	};

	void Sydney(CUnknown* pcUnknown, void* pvContext)
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

	void Nabia(CUnknown* pcUnknown, void* pvContext)
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
	cEvent.Init("Frisky", &CTargetForAction::Dingo);
	cEvent.AddListener(&cTarget);
	cEvent.Call((void*)"Eustance");
	cEvent.Kill();
	AssertInt(7, cTarget.miCorrectCode);
	cTarget.Kill();

	cTarget.Init();
	cEvent.Init("Frisky", &CTargetForAction::Coyote);
	cEvent.AddListener(&cTarget);
	cEvent.Call(NULL);
	cEvent.Kill();
	AssertInt(1, cTarget.miCorrectCode);
	cTarget.Kill();

	cTarget.Init();
	cEvent.Init("Frisky", &CTargetForAction::Sydney);
	cEvent.AddListener(&cTarget);
	f = 0.753f;
	cEvent.Call(&f);
	cEvent.Kill();
	AssertInt(4, cTarget.miCorrectCode);
	cTarget.Kill();

	cTarget.Init();
	cEvent.Init("Frisky", &CTargetForAction::Nabia);
	cEvent.AddListener(&cTarget);
	cEvent.Call((void*)NULL);
	cEvent.Kill();
	AssertInt(3, cTarget.miCorrectCode);
	cTarget.Kill();

	TestStatistics();
}

