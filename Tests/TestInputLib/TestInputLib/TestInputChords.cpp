#include "BaseLib/Logger.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "InputLib/Input.h"
//#include "WinInputLib/WinInput.h"
#include "TestLib/Assert.h"
#include "Player.h"


struct STestEventValue
{
	char*	szName;
	float	fValue;
};


typedef CArrayTemplate<STestEventValue> CArrayTestEventValue;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertEvents(CInput* pcInput, CProgramDeviceDetail* pcDetail, CPlayer* pcPlayer, char* szExpectedAction, char* szFirstEvent, ...)
{
	va_list						vaMarker;
	char*						pc;
	size						iCount;
	CArrayTestEventValue		cArray;
	size						iLen;
	float						fValue;
	size						i;
	STestEventValue*			pcEvent;

	cArray.Init();
	pcPlayer->GetEvents()->ReInit();

	iCount = 0;
	pc = szFirstEvent;
	va_start(vaMarker, szFirstEvent);
	while (pc != NULL)
	{
		pcEvent =  cArray.Add();

		pcEvent->szName = pc;
		iLen = (int)strlen(pc);
		fValue = (float)atof(&pc[iLen+1]);
		pcEvent->fValue = fValue;

		iCount++;
		pc = va_arg(vaMarker, char*);
	}

	for (i = 0; i < cArray.NumElements()-1; i++)
	{
		pcEvent = cArray.Get(i);
		pcDetail->AddEvent(pcEvent->szName, pcEvent->fValue);
		pcInput->Update();
		AssertSize(0, pcPlayer->GetEvents()->NumElements());
	}

	pcEvent = cArray.Get(i);
	pcDetail->AddEvent(pcEvent->szName, pcEvent->fValue);
	pcInput->Update();
	AssertSize(1, pcPlayer->GetEvents()->NumElements());
	AssertString(szExpectedAction, pcPlayer->GetEvents()->GetText(0));

	cArray.Kill();

	va_end(vaMarker);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CProgramDeviceDetail* AddProgrammaticDevice(CProgramInput*	pcProgram)
{
	CProgramDeviceDetail*	pcDetail;

	pcDetail = pcProgram->AddDetail("Programmatic: Device 1", "Happy Device");
	pcDetail->AddSource("W");
	pcDetail->AddSource("A");
	pcDetail->AddSource("S");
	pcDetail->AddSource("D");
	pcDetail->AddSource("Left Control");
	pcDetail->AddSource("Space");
	pcDetail->AddSource("Cursor Left");
	pcDetail->AddSource("Cursor Right");
	pcDetail->AddSource("Cursor Up");
	pcDetail->AddSource("Cursor Down");
	pcDetail->AddSource("Right Control");
	pcDetail->AddSource("Left Shift");
	pcDetail->AddSource("B");
	return pcDetail;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds one input, updates at uiTime and checks what the chords let
// through, as "Source value" or "[Chord]" separated by ", ".
//
//////////////////////////////////////////////////////////////////////////
void AssertChordsEvents(CInput* pcInput, CProgramDeviceDetail* pcDetail, CInputVirtualDevice* pcVirtual, char* szSource, float fValue, uint32 uiTime, char* szExpected)
{
	CChars					sz;
	CChars						szExpectedAt;
	CArrayInputChordsEvent*		pasEvents;
	SInputChordsEvent*			psEvent;
	CArrayInputDeviceTimeValue*	pasHistory;
	SInputDeviceTimeValue*		psHistory;
	size						i;

	if (szSource)
	{
		pcDetail->AddEvent(szSource, fValue);
	}
	pcInput->Update(uiTime);

	//The time is in both so a failure says which update it was.
	szExpectedAt.Init();
	szExpectedAt.Append((int)uiTime);
	szExpectedAt.Append(": ");
	szExpectedAt.Append(szExpected);

	sz.Init();
	sz.Append((int)uiTime);
	sz.Append(": ");
	pasEvents = pcVirtual->GetChords()->GetEvents();
	for (i = 0; i < pasEvents->NumElements(); i++)
	{
		psEvent = pasEvents->Get(i);
		if (i != 0)
		{
			sz.Append(", ");
		}
		if (psEvent->pcChord)
		{
			sz.Append("[");
			sz.Append(psEvent->pcChord->GetAction()->GetName());
			sz.Append("]");
		}
		else
		{
			sz.Append(psEvent->sValue.pcSource->GetFriendlyName());
			sz.Append(" ");
			sz.Append((int)psEvent->sValue.fValue);
		}
	}
	//On a failure the held inputs are shown too; c is carried.
	if (strcmp(szExpectedAt.Text(), sz.Text()) != 0)
	{
		pasHistory = &pcVirtual->GetChords()->macHistory;
		sz.Append(" | held:");
		for (i = 0; i < pasHistory->NumElements(); i++)
		{
			psHistory = pasHistory->Get(i);
			sz.Append(" ");
			sz.Append(psHistory->pcSource->GetFriendlyName());
			sz.Append((int)psHistory->fValue);
			sz.Append(psHistory->bCarried ? "c" : "");
		}
		sz.Append(pcVirtual->GetChords()->mpcPending ? " (pending)" : "");
	}
	AssertString(szExpectedAt.Text(), sz.Text());
	sz.Kill();
	szExpectedAt.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Left Control and Space, in either order, within the eviction age.
//
//////////////////////////////////////////////////////////////////////////
void TestInputChordsHeldAndEvicted(void)
{
	CInput						cInput;
	CPlayer						cPlayer;
	CProgramDeviceDetail*		pcDetail;
	CInputVirtualDevice*		pcVirtual;
	CInputDevice*				pcDevice;
	CInputChord*				pcChord;
	CGroupInputChordCriteria*	pcGroup;
	COrderedInputChordCriteria*	pcOrdered;

	cInput.Init(NULL);
	pcDetail = AddProgrammaticDevice(cInput.GetProgramInput());
	cInput.AddProgramInput();
	cPlayer.Init();

	pcDevice = cInput.GetDevices()->GetFirstVirtualDeviceInCategoryKeyboard(cInput.GetProgramInput()->GetCategoryName(), false)->GetDefaultDevice();
	pcVirtual = cInput.GetDevices()->CreateVirtualDevice("Held");
	AssertBool(true, pcVirtual->AddSources(pcDevice, "W", "Left Control", "Space", NULL));

	pcChord = pcVirtual->AddChordAction(cInput.GetActions()->AddActionWithListener("Control Space", &CPlayerListener::Up, (CPlayerListener*)&cPlayer));
	pcGroup = pcChord->AsGroup();
	pcGroup->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("Left Control"), ICT_Toggle));
	pcGroup->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("Space"), ICT_Toggle));
	AssertBool(true, pcChord->Done());

	//Tap control then Space: control is let go before the chord ends.
	pcChord = pcVirtual->AddChordAction(cInput.GetActions()->AddActionWithListener("Tap Control Space", &CPlayerListener::Up, (CPlayerListener*)&cPlayer));
	pcOrdered = pcChord->AsOrdered();
	pcOrdered->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("Left Control"), ICT_Toggle));
	pcOrdered->AddInactiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("Left Control"), ICT_Toggle));
	pcOrdered->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("Space"), ICT_Toggle));
	AssertBool(true, pcChord->Done());
	pcVirtual->Enable();

	//W can't be part of a chord so it isn't held.
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "W", 1.0f, 0, "W 1");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "W", 0.0f, 10, "W 0");

	//The chord completes in time; only the chord is let through.
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 1.0f, 1000, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, NULL, 0.0f, 1100, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Space", 1.0f, 1200, "[Control Space]");
	AssertSize(1, cPlayer.GetEvents()->NumElements());
	AssertString("Control Space: Up", cPlayer.GetEvents()->GetText(0));
	cPlayer.GetEvents()->ReInit();
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 0.0f, 1250, "Left Control 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Space", 0.0f, 1260, "Space 0");

	//Either order.
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Space", 1.0f, 1500, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 1.0f, 1550, "[Control Space]");
	AssertSize(1, cPlayer.GetEvents()->NumElements());
	cPlayer.GetEvents()->ReInit();
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Space", 0.0f, 1600, "Space 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 0.0f, 1610, "Left Control 0");

	//A held input never ages, however long until Space.
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 1.0f, 2000, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, NULL, 0.0f, 2400, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Space", 1.0f, 2500, "[Control Space]");
	AssertSize(1, cPlayer.GetEvents()->NumElements());
	cPlayer.GetEvents()->ReInit();

	//Control is still down so it is carried; Space again is the chord again.  The releases are let through.
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Space", 0.0f, 2510, "Space 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Space", 1.0f, 2520, "[Control Space]");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Space", 0.0f, 2530, "Space 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Space", 1.0f, 2540, "[Control Space]");
	AssertSize(2, cPlayer.GetEvents()->NumElements());
	cPlayer.GetEvents()->ReInit();
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Space", 0.0f, 2550, "Space 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 0.0f, 2560, "Left Control 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Space", 1.0f, 2570, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Space", 0.0f, 2580, "Space 1, Space 0");

	//A press no chord can take is let through; the carried inputs were used by their chord so they aren't.
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 1.0f, 2600, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Space", 1.0f, 2610, "[Control Space]");
	cPlayer.GetEvents()->ReInit();
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "W", 1.0f, 2620, "W 1");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "W", 0.0f, 2630, "W 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Space", 0.0f, 2640, "Space 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 0.0f, 2650, "Left Control 0");

	//Once everything is let go a partial chord has 300ms from the last release.
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 1.0f, 2800, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 0.0f, 2810, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, NULL, 0.0f, 3110, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, NULL, 0.0f, 3111, "Left Control 1, Left Control 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 1.0f, 3200, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 0.0f, 3210, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Space", 1.0f, 3500, "[Tap Control Space]");
	AssertSize(1, cPlayer.GetEvents()->NumElements());
	cPlayer.GetEvents()->ReInit();
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Space", 0.0f, 3510, "Space 0");

	//An input that isn't the chord's next lets the held one through, in order.  Control is still down and can start a
	//chord so it is carried: Space after is still control and space, as shift held while typing is for shift+tab after.
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 1.0f, 4000, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "W", 1.0f, 4050, "Left Control 1, W 1");
	AssertSize(0, cPlayer.GetEvents()->NumElements());
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "W", 0.0f, 4060, "W 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Space", 1.0f, 4070, "[Control Space]");
	AssertSize(1, cPlayer.GetEvents()->NumElements());
	cPlayer.GetEvents()->ReInit();
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Space", 0.0f, 4080, "Space 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 0.0f, 4090, "Left Control 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Space", 1.0f, 4100, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Space", 0.0f, 4110, "Space 1, Space 0");

	cPlayer.Kill();
	cInput.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A tapped key and then another, each chord with its own eviction age: a
// partial chord waits as long as the longest of the chords that could
// still match it, which can be shorter than the chords' 300ms.
//
//////////////////////////////////////////////////////////////////////////
void AddTapChord(CInput* pcInput, CPlayer* pcPlayer, CInputVirtualDevice* pcVirtual, CInputDevice* pcDevice, char* szName, char* szTapped, char* szNext, uint32 uiEvictionAge)
{
	CInputChord*				pcChord;
	COrderedInputChordCriteria*	pcOrdered;

	pcChord = pcVirtual->AddChordAction(pcInput->GetActions()->AddActionWithListener(szName, &CPlayerListener::Up, (CPlayerListener*)pcPlayer));
	pcOrdered = pcChord->AsOrdered();
	pcOrdered->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource(szTapped), ICT_Toggle));
	pcOrdered->AddInactiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource(szTapped), ICT_Toggle));
	pcOrdered->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource(szNext), ICT_Toggle));
	pcChord->SetEvictionAge(uiEvictionAge);
	AssertBool(true, pcChord->Done());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestInputChordsEvictionAge(void)
{
	CInput						cInput;
	CPlayer						cPlayer;
	CProgramDeviceDetail*		pcDetail;
	CInputVirtualDevice*		pcVirtual;
	CInputDevice*				pcDevice;

	cInput.Init(NULL);
	pcDetail = AddProgrammaticDevice(cInput.GetProgramInput());
	cInput.AddProgramInput();
	cPlayer.Init();

	pcDevice = cInput.GetDevices()->GetFirstVirtualDeviceInCategoryKeyboard(cInput.GetProgramInput()->GetCategoryName(), false)->GetDefaultDevice();
	pcVirtual = cInput.GetDevices()->CreateVirtualDevice("Ages");
	AssertBool(true, pcVirtual->AddSources(pcDevice, "W", "B", "Left Control", "Space", NULL));
	AddTapChord(&cInput, &cPlayer, pcVirtual, pcDevice, "Slow", "Left Control", "Space", 500);
	AddTapChord(&cInput, &cPlayer, pcVirtual, pcDevice, "Usual", "Left Control", "W", 0);
	AddTapChord(&cInput, &cPlayer, pcVirtual, pcDevice, "Quick", "B", "W", 100);
	AddTapChord(&cInput, &cPlayer, pcVirtual, pcDevice, "Default", "Space", "W", 0);
	pcVirtual->Enable();

	//Control could start Slow or Usual, so it waits Slow's 500ms; Slow still matches after 300ms.
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 1.0f, 1000, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 0.0f, 1010, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, NULL, 0.0f, 1400, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Space", 1.0f, 1450, "[Slow]");
	cPlayer.GetEvents()->ReInit();
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Space", 0.0f, 1460, "Space 0");

	//Usual completes after 300ms too, while Slow could still match.
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 1.0f, 2000, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 0.0f, 2010, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "W", 1.0f, 2400, "[Usual]");
	cPlayer.GetEvents()->ReInit();
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "W", 0.0f, 2410, "W 0");

	//500ms after the release Control is let through.
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 1.0f, 3000, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 0.0f, 3010, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, NULL, 0.0f, 3510, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, NULL, 0.0f, 3511, "Left Control 1, Left Control 0");

	//Quick waits only 100ms.
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "B", 1.0f, 4000, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "B", 0.0f, 4010, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, NULL, 0.0f, 4110, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, NULL, 0.0f, 4111, "B 1, B 0");

	//A chord without its own age waits the chords' 300ms.
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Space", 1.0f, 5000, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Space", 0.0f, 5010, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, NULL, 0.0f, 5310, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, NULL, 0.0f, 5311, "Space 1, Space 0");
	AssertSize(0, cPlayer.GetEvents()->NumElements());

	cPlayer.Kill();
	cInput.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Both controls down in either order, then A down, then szLast down if
// it isn't NULL.  There are no releases in the chord.
//
//////////////////////////////////////////////////////////////////////////
void AddControlsChord(CInput* pcInput, CInputVirtualDevice* pcVirtual, CInputDevice* pcDevice, CPlayer* pcPlayer, char* szName, char* szLast)
{
	CInputChord*				pcChord;
	COrderedInputChordCriteria*	pcOrdered;
	CGroupInputChordCriteria*	pcGroup;

	pcChord = pcVirtual->AddChordAction(pcInput->GetActions()->AddActionWithListener(szName, &CPlayerListener::Up, (CPlayerListener*)pcPlayer));
	pcOrdered = pcChord->AsOrdered();
	pcGroup = pcOrdered->AddGroupAction();
	pcGroup->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("Left Control"), ICT_Toggle));
	pcGroup->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("Right Control"), ICT_Toggle));
	pcOrdered->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("A"), ICT_Toggle));
	if (szLast)
	{
		pcOrdered->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource(szLast), ICT_Toggle));
	}
	pcChord->Done();
}


//////////////////////////////////////////////////////////////////////////
//
// The controls then A is the start of the controls then A then D.  The
// shorter waits to see if the longer matches.
//
//////////////////////////////////////////////////////////////////////////
void TestInputChordsPrefix(void)
{
	CInput						cInput;
	CPlayer						cPlayer;
	CProgramDeviceDetail*		pcDetail;
	CInputVirtualDevice*		pcVirtual;
	CInputDevice*				pcDevice;

	cInput.Init(NULL);
	pcDetail = AddProgrammaticDevice(cInput.GetProgramInput());
	cInput.AddProgramInput();
	cPlayer.Init();

	pcDevice = cInput.GetDevices()->GetFirstVirtualDeviceInCategoryKeyboard(cInput.GetProgramInput()->GetCategoryName(), false)->GetDefaultDevice();
	pcVirtual = cInput.GetDevices()->CreateVirtualDevice("Prefix");
	AssertBool(true, pcVirtual->AddSources(pcDevice, "W", "A", "D", "Left Control", "Right Control", NULL));
	AddControlsChord(&cInput, pcVirtual, pcDevice, &cPlayer, "Controls A", NULL);
	AddControlsChord(&cInput, pcVirtual, pcDevice, &cPlayer, "Controls A D", "D");
	pcVirtual->Enable();

	//The longer chord matches; the shorter is never called.
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 1.0f, 0, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Right Control", 1.0f, 10, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "A", 1.0f, 20, "");
	AssertSize(0, cPlayer.GetEvents()->NumElements());
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "D", 1.0f, 30, "[Controls A D]");
	AssertSize(1, cPlayer.GetEvents()->NumElements());
	AssertString("Controls A D: Up", cPlayer.GetEvents()->GetText(0));
	cPlayer.GetEvents()->ReInit();
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "D", 0.0f, 40, "D 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "A", 0.0f, 50, "A 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 0.0f, 60, "Left Control 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Right Control", 0.0f, 70, "Right Control 0");

	//Letting go of A stops the longer chord so the shorter is called, then the release is let through.
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Right Control", 1.0f, 1000, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 1.0f, 1010, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "A", 1.0f, 1020, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "A", 0.0f, 1030, "[Controls A], A 0");
	AssertSize(1, cPlayer.GetEvents()->NumElements());
	AssertString("Controls A: Up", cPlayer.GetEvents()->GetText(0));
	cPlayer.GetEvents()->ReInit();
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 0.0f, 1040, "Left Control 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Right Control", 0.0f, 1050, "Right Control 0");

	//An input that isn't part of a chord does the same.
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 1.0f, 2000, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Right Control", 1.0f, 2010, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "A", 1.0f, 2020, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "W", 1.0f, 2030, "[Controls A], W 1");
	AssertSize(1, cPlayer.GetEvents()->NumElements());
	cPlayer.GetEvents()->ReInit();
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "W", 0.0f, 2040, "W 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "A", 0.0f, 2050, "A 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 0.0f, 2060, "Left Control 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Right Control", 0.0f, 2070, "Right Control 0");

	//Held down the shorter chord waits however long; letting go of A calls it.  The controls are carried, D can't use them.
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 1.0f, 3000, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Right Control", 1.0f, 3010, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "A", 1.0f, 3020, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, NULL, 0.0f, 3500, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "A", 0.0f, 3510, "[Controls A], A 0");
	AssertSize(1, cPlayer.GetEvents()->NumElements());
	AssertString("Controls A: Up", cPlayer.GetEvents()->GetText(0));
	cPlayer.GetEvents()->ReInit();
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "D", 1.0f, 3520, "D 1");

	//With the controls still down A again is the shorter chord again.
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "D", 0.0f, 3530, "D 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 0.0f, 3540, "Left Control 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Right Control", 0.0f, 3550, "Right Control 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 1.0f, 3600, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Right Control", 1.0f, 3610, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "A", 1.0f, 3620, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "A", 0.0f, 3630, "[Controls A], A 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "A", 1.0f, 3640, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "A", 0.0f, 3650, "[Controls A], A 0");
	AssertSize(2, cPlayer.GetEvents()->NumElements());
	cPlayer.GetEvents()->ReInit();

	cPlayer.Kill();
	cInput.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Left Control and Left Shift down in either order.
//
//////////////////////////////////////////////////////////////////////////
void AddControlShiftGroup(COrderedInputChordCriteria* pcOrdered, CInputChord* pcChord, CInputDevice* pcDevice)
{
	CGroupInputChordCriteria*	pcGroup;

	pcGroup = pcOrdered->AddGroupAction();
	pcGroup->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("Left Control"), ICT_Toggle));
	pcGroup->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("Left Shift"), ICT_Toggle));
}


//////////////////////////////////////////////////////////////////////////
//
// Three chords that start the same:
//   ((ctrl+shift) a)						ctrl and shift in either order, then a.
//   ((ctrl+shift) (a+b))					ctrl and shift in either order, then a and b in either order.  All four
//											stay down; letting one go before the last is down stops the chord.
//   (((ctrl+shift) a) (-ctrl-shift-a) b)	ctrl and shift in either order then a, then all three let go in any
//											order, then b.
//
//////////////////////////////////////////////////////////////////////////
void TestInputChordsCommonStart(void)
{
	CInput						cInput;
	CPlayer						cPlayer;
	CProgramDeviceDetail*		pcDetail;
	CInputVirtualDevice*		pcVirtual;
	CInputDevice*				pcDevice;
	CInputChord*				pcChord;
	COrderedInputChordCriteria*	pcOrdered;
	COrderedInputChordCriteria*	pcInner;
	CGroupInputChordCriteria*	pcGroup;
	CInputChord*				pcMatching;
	CInputChord*				pcAPlusB;

	cInput.Init(NULL);
	pcDetail = AddProgrammaticDevice(cInput.GetProgramInput());
	cInput.AddProgramInput();
	cPlayer.Init();

	pcDevice = cInput.GetDevices()->GetFirstVirtualDeviceInCategoryKeyboard(cInput.GetProgramInput()->GetCategoryName(), false)->GetDefaultDevice();
	pcVirtual = cInput.GetDevices()->CreateVirtualDevice("Common Start");
	AssertBool(true, pcVirtual->AddSources(pcDevice, "W", "A", "B", "Left Control", "Left Shift", NULL));

	pcChord = pcVirtual->AddChordAction(cInput.GetActions()->AddActionWithListener("CS A", &CPlayerListener::Up, (CPlayerListener*)&cPlayer));
	pcOrdered = pcChord->AsOrdered();
	AddControlShiftGroup(pcOrdered, pcChord, pcDevice);
	pcOrdered->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("A"), ICT_Toggle));
	AssertBool(true, pcChord->Done(&pcMatching));
	AssertNull(pcMatching);
	AssertSize(3, pcChord->GetTotalCriteria());

	pcChord = pcVirtual->AddChordAction(cInput.GetActions()->AddActionWithListener("CS A+B", &CPlayerListener::Up, (CPlayerListener*)&cPlayer));
	pcOrdered = pcChord->AsOrdered();
	AddControlShiftGroup(pcOrdered, pcChord, pcDevice);
	pcGroup = pcOrdered->AddGroupAction();
	pcGroup->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("A"), ICT_Toggle));
	pcGroup->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("B"), ICT_Toggle));
	AssertBool(true, pcChord->Done());
	AssertSize(4, pcChord->GetTotalCriteria());
	pcAPlusB = pcChord;

	//ctrl, shift, a then b: (a+b) matches that too so both would be called.  The new chord isn't added.
	pcChord = pcVirtual->AddChordAction(cInput.GetActions()->AddActionWithListener("CS A then B", &CPlayerListener::Up, (CPlayerListener*)&cPlayer));
	pcOrdered = pcChord->AsOrdered();
	pcInner = pcOrdered->AddOrderedAction();
	AddControlShiftGroup(pcInner, pcChord, pcDevice);
	pcInner->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("A"), ICT_Toggle));
	pcOrdered->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("B"), ICT_Toggle));
	AssertBool(false, pcChord->Done(&pcMatching));
	AssertPointer(pcAPlusB, pcMatching);

	//The same chord written the other way round.
	pcChord = pcVirtual->AddChordAction(cInput.GetActions()->AddActionWithListener("SC B+A", &CPlayerListener::Up, (CPlayerListener*)&cPlayer));
	pcOrdered = pcChord->AsOrdered();
	pcGroup = pcOrdered->AddGroupAction();
	pcGroup->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("Left Shift"), ICT_Toggle));
	pcGroup->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("Left Control"), ICT_Toggle));
	pcGroup = pcOrdered->AddGroupAction();
	pcGroup->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("B"), ICT_Toggle));
	pcGroup->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("A"), ICT_Toggle));
	AssertBool(false, pcChord->Done(&pcMatching));
	AssertPointer(pcAPlusB, pcMatching);

	//Four long but b can't be second, so nothing else matches it.
	pcChord = pcVirtual->AddChordAction(cInput.GetActions()->AddActionWithListener("C B S A", &CPlayerListener::Up, (CPlayerListener*)&cPlayer));
	pcOrdered = pcChord->AsOrdered();
	pcOrdered->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("Left Control"), ICT_Toggle));
	pcOrdered->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("B"), ICT_Toggle));
	pcOrdered->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("Left Shift"), ICT_Toggle));
	pcOrdered->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("A"), ICT_Toggle));
	AssertBool(true, pcChord->Done(&pcMatching));
	AssertNull(pcMatching);
	pcVirtual->GetChords()->RemoveChord(pcChord);

	//The inner ordered criteria are made before anything follows them in the outer.
	pcChord = pcVirtual->AddChordAction(cInput.GetActions()->AddActionWithListener("CS A, B", &CPlayerListener::Up, (CPlayerListener*)&cPlayer));
	pcOrdered = pcChord->AsOrdered();
	pcInner = pcOrdered->AddOrderedAction();
	AddControlShiftGroup(pcInner, pcChord, pcDevice);
	pcInner->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("A"), ICT_Toggle));
	pcGroup = pcOrdered->AddGroupAction();
	pcGroup->AddInactiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("Left Control"), ICT_Toggle));
	pcGroup->AddInactiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("Left Shift"), ICT_Toggle));
	pcGroup->AddInactiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("A"), ICT_Toggle));
	pcOrdered->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("B"), ICT_Toggle));
	AssertBool(true, pcChord->Done());
	AssertSize(7, pcChord->GetTotalCriteria());

	pcVirtual->Enable();

	//ctrl, shift, a then b with a still down: only (a+b).
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 1.0f, 0, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Shift", 1.0f, 10, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "A", 1.0f, 20, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "B", 1.0f, 30, "[CS A+B]");
	AssertSize(1, cPlayer.GetEvents()->NumElements());
	AssertString("CS A+B: Up", cPlayer.GetEvents()->GetText(0));
	cPlayer.GetEvents()->ReInit();
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "B", 0.0f, 40, "B 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "A", 0.0f, 50, "A 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Shift", 0.0f, 60, "Left Shift 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 0.0f, 70, "Left Control 0");

	//shift, ctrl, b then a: only (a+b) takes b first.
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Shift", 1.0f, 1000, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 1.0f, 1010, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "B", 1.0f, 1020, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "A", 1.0f, 1030, "[CS A+B]");
	AssertSize(1, cPlayer.GetEvents()->NumElements());
	cPlayer.GetEvents()->ReInit();
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "A", 0.0f, 1040, "A 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "B", 0.0f, 1050, "B 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 0.0f, 1060, "Left Control 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Shift", 0.0f, 1070, "Left Shift 0");

	//ctrl, shift, a, all let go in any order, then b: (ctrl+shift) a waits through the releases and the longest is called.
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 1.0f, 2000, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Shift", 1.0f, 2010, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "A", 1.0f, 2020, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Shift", 0.0f, 2030, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "A", 0.0f, 2040, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 0.0f, 2050, "");
	AssertSize(0, cPlayer.GetEvents()->NumElements());
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "B", 1.0f, 2060, "[CS A, B]");
	AssertSize(1, cPlayer.GetEvents()->NumElements());
	AssertString("CS A, B: Up", cPlayer.GetEvents()->GetText(0));
	cPlayer.GetEvents()->ReInit();
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "B", 0.0f, 2070, "B 0");

	//Letting go of a keeps (-ctrl-shift-a) b going, W stops it so (ctrl+shift) a is called and the rest let through.
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 1.0f, 2500, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Shift", 1.0f, 2510, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "A", 1.0f, 2520, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "A", 0.0f, 2530, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "W", 1.0f, 2540, "[CS A], A 0, W 1");
	AssertSize(1, cPlayer.GetEvents()->NumElements());
	AssertString("CS A: Up", cPlayer.GetEvents()->GetText(0));
	cPlayer.GetEvents()->ReInit();
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "W", 0.0f, 2550, "W 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Shift", 0.0f, 2560, "Left Shift 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 0.0f, 2570, "Left Control 0");

	//Letting go of ctrl before b is down stops (a+b), and b with shift and a still down stops (-ctrl-shift-a) b.
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 1.0f, 2700, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Shift", 1.0f, 2710, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "A", 1.0f, 2720, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 0.0f, 2730, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "B", 1.0f, 2740, "[CS A], Left Control 0, B 1");
	AssertSize(1, cPlayer.GetEvents()->NumElements());
	cPlayer.GetEvents()->ReInit();
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "B", 0.0f, 2750, "B 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "A", 0.0f, 2760, "A 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Shift", 0.0f, 2770, "Left Shift 0");

	//All let go but no b: (ctrl+shift) a is called 300ms after the last release, then the releases are let through.
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 1.0f, 2800, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Shift", 1.0f, 2810, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "A", 1.0f, 2820, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "A", 0.0f, 2830, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 0.0f, 2840, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Shift", 0.0f, 2850, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, NULL, 0.0f, 3150, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, NULL, 0.0f, 3151, "[CS A], A 0, Left Control 0, Left Shift 0");
	AssertSize(1, cPlayer.GetEvents()->NumElements());
	cPlayer.GetEvents()->ReInit();

	//ctrl, shift, a then W, which no chord has.  The carried keys are dropped so their releases are let through.
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 1.0f, 3200, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Shift", 1.0f, 3210, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "A", 1.0f, 3220, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "W", 1.0f, 3230, "[CS A], W 1");
	AssertSize(1, cPlayer.GetEvents()->NumElements());
	cPlayer.GetEvents()->ReInit();
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "W", 0.0f, 3240, "W 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "A", 0.0f, 3250, "A 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Shift", 0.0f, 3260, "Left Shift 0");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 0.0f, 3270, "Left Control 0");

	//ctrl, shift, a held: nothing ages while they are down.  Letting go keeps (-ctrl-shift-a) b going, then 300ms after
	//the last release (ctrl+shift) a is called.
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 1.0f, 4000, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Shift", 1.0f, 4010, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "A", 1.0f, 4020, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, NULL, 0.0f, 4400, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "A", 0.0f, 4410, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Shift", 0.0f, 4420, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 0.0f, 4430, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, NULL, 0.0f, 4730, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, NULL, 0.0f, 4731, "[CS A], A 0, Left Shift 0, Left Control 0");
	AssertSize(1, cPlayer.GetEvents()->NumElements());
	cPlayer.GetEvents()->ReInit();

	//ctrl, shift, b then W: nothing matches so all are let through in order.
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Control", 1.0f, 5000, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "Left Shift", 1.0f, 5010, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "B", 1.0f, 5020, "");
	AssertChordsEvents(&cInput, pcDetail, pcVirtual, "W", 1.0f, 5030, "Left Control 1, Left Shift 1, B 1, W 1");
	AssertSize(0, cPlayer.GetEvents()->NumElements());

	cPlayer.Kill();
	cInput.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestInputChords(void)
{
	TypesInit();
	TypeConverterInit();
	MemoryInit();
	DataIOInit();
	ObjectsInit();

	BeginTests();

	CInput						cInput;
	CPlayer						cPlayer1;
	CPlayer						cPlayer2;
	CProgramDeviceDetail*		pcDetail;
	CInputVirtualDevice*		pcPlayer1;
	bool						bResult;
	CInputDevice*				pcDevice;
	CInputVirtualDevice*		pcPlayer2;
	CInputChord*				pcChord;
	COrderedInputChordCriteria*	pcOrdered;
	CGroupInputChordCriteria*	pcGroup;

	cInput.Init(NULL);
	pcDetail = AddProgrammaticDevice(cInput.GetProgramInput());
	cInput.AddProgramInput();
	
	cPlayer1.Init();
	pcPlayer1 = cInput.GetDevices()->GetFirstVirtualDeviceInCategoryKeyboard(cInput.GetProgramInput()->GetCategoryName(), false);
	pcDevice = pcPlayer1->GetDefaultDevice();

	pcPlayer1 = cInput.GetDevices()->CreateVirtualDevice("Player 1");
	bResult = pcPlayer1->AddSources(pcDevice, "W", "A", "S", "D", "Left Control", NULL);
	AssertBool(true, bResult);

	pcPlayer1->AddOrderedAction(cInput.GetActions()->AddActionWithListener("Player 1 Up", &CPlayerListener::Up, (CPlayerListener*)&cPlayer1), "W", "W", NULL);
	pcPlayer1->AddOrderedAction(cInput.GetActions()->AddActionWithListener("Player 1 Down", &CPlayerListener::Down, (CPlayerListener*)&cPlayer1), "S", "W", "W", "W", NULL);
	pcPlayer1->AddOrderedAction(cInput.GetActions()->AddActionWithListener("Player 1 Left", &CPlayerListener::Left, (CPlayerListener*)&cPlayer1), "A", "D", "A", "A", NULL);
	pcPlayer1->AddOrderedAction(cInput.GetActions()->AddActionWithListener("Player 1 Right", &CPlayerListener::Right, (CPlayerListener*)&cPlayer1), "A", "D", "A", "D", NULL);

	pcPlayer1->Enable();

	pcDetail->AddEvent("W", 1.0f);
	cInput.Update();
	AssertSize(0, cPlayer1.GetEvents()->NumElements());

	pcDetail->AddEvent("W", 0.0f);
	cInput.Update();
	AssertSize(0, cPlayer1.GetEvents()->NumElements());

	pcDetail->AddEvent("W", 1.0f);
	cInput.Update();
	AssertSize(1, cPlayer1.GetEvents()->NumElements());
	AssertString("Player 1 Up: Up", cPlayer1.GetEvents()->GetText(0));
	cPlayer1.GetEvents()->ReInit();

	pcDetail->AddEvent("W", 0.0f);
	cInput.Update();
	AssertSize(0, cPlayer1.GetEvents()->NumElements());

	AssertEvents(&cInput, pcDetail, &cPlayer1, "Player 1 Up: Up",  "W\0 1", "W\0 0", "W\0 1", NULL);
	pcDetail->AddEvent("W", 0.0f);
	cInput.Update();

	AssertEvents(&cInput, pcDetail, &cPlayer1, "Player 1 Down: Down",  "S\0 1", "S\0 0", "W\0 1", "W\0 0", "W\0 1", "W\0 0", "W\0 1", NULL);
	pcDetail->AddEvent("W", 0.0f);
	cInput.Update();

	AssertEvents(&cInput, pcDetail, &cPlayer1, "Player 1 Left: Left",  "A\0 1", "A\0 0", "D\0 1", "D\0 0", "A\0 1", "A\0 0", "A\0 1", NULL);
	pcDetail->AddEvent("A", 0.0f);
	cInput.Update();

	AssertEvents(&cInput, pcDetail, &cPlayer1, "Player 1 Right: Right",  "A\0 1", "A\0 0", "D\0 1", "D\0 0", "A\0 1", "A\0 0", "D\0 1", NULL);
	pcDetail->AddEvent("D", 0.0f);
	cInput.Update();

	AssertEvents(&cInput, pcDetail, &cPlayer1, "Player 1 Up: Up",  "S\0 1", "S\0 0", "A\0 1", "A\0 0", "W\0 1", "W\0 0", "W\0 1", NULL);
	pcDetail->AddEvent("W", 0.0f);
	cInput.Update();

	cPlayer2.Init();
	pcPlayer2 = cInput.GetDevices()->CreateVirtualDevice("Player 2");
	bResult = pcPlayer2->AddSources(pcDevice, "Cursor Up", "Cursor Left", "Cursor Down", "Cursor Right", "Right Control", NULL);
	AssertBool(true, bResult);

	pcChord = pcPlayer2->AddChordAction(cInput.GetActions()->AddActionWithListener("Player 2 Up", &CPlayerListener::Up, (CPlayerListener*)&cPlayer2));
	pcOrdered = pcChord->AsOrdered();
	pcGroup = pcOrdered->AddGroupAction();
	pcGroup->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("Cursor Left"), ICT_Toggle));
	pcGroup->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("Cursor Right"), ICT_Toggle));
	pcGroup = pcOrdered->AddGroupAction();
	pcGroup->AddInactiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("Cursor Left"), ICT_Toggle));
	pcGroup->AddInactiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("Cursor Right"), ICT_Toggle));
	pcOrdered->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("Cursor Up"), ICT_Toggle));
	pcOrdered->AddInactiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("Cursor Up"), ICT_Toggle));
	pcChord->Done();
	pcChord = pcPlayer2->AddChordAction(cInput.GetActions()->AddActionWithListener("Player 2 Down", &CPlayerListener::Down, (CPlayerListener*)&cPlayer2));
	pcOrdered = pcChord->AsOrdered();
	pcGroup = pcOrdered->AddGroupAction();
	pcGroup->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("Cursor Left"), ICT_Toggle));
	pcGroup->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("Cursor Right"), ICT_Toggle));
	pcGroup = pcOrdered->AddGroupAction();
	pcGroup->AddInactiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("Cursor Left"), ICT_Toggle));
	pcGroup->AddInactiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("Cursor Right"), ICT_Toggle));
	pcOrdered->AddActiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("Cursor Down"), ICT_Toggle));
	pcOrdered->AddInactiveAction(pcChord->AddEvaluatorSpecificSource(pcDevice, pcDevice->GetSource("Cursor Down"), ICT_Toggle));
	pcChord->Done();
	pcPlayer2->Enable();

	AssertEvents(&cInput, pcDetail, &cPlayer2, "Player 2 Up: Up", "Cursor Left\0 1", "Cursor Right\0 1", "Cursor Left\0 0", "Cursor Right\0 0", "Cursor Up\0 1", "Cursor Up\0 0", NULL);
	AssertEvents(&cInput, pcDetail, &cPlayer2, "Player 2 Up: Up", "Cursor Left\0 1", "Cursor Right\0 1", "Cursor Right\0 0", "Cursor Left\0 0", "Cursor Up\0 1", "Cursor Up\0 0", NULL);
	AssertEvents(&cInput, pcDetail, &cPlayer2, "Player 2 Up: Up", "Cursor Right\0 1", "Cursor Left\0 1", "Cursor Left\0 0", "Cursor Right\0 0", "Cursor Up\0 1", "Cursor Up\0 0", NULL);
	AssertEvents(&cInput, pcDetail, &cPlayer2, "Player 2 Up: Up", "Cursor Right\0 1", "Cursor Left\0 1", "Cursor Right\0 0", "Cursor Left\0 0", "Cursor Up\0 1", "Cursor Up\0 0", NULL);

	AssertEvents(&cInput, pcDetail, &cPlayer2, "Player 2 Down: Down", "Cursor Left\0 1", "Cursor Right\0 1", "Cursor Left\0 0", "Cursor Right\0 0", "Cursor Down\0 1", "Cursor Down\0 0", NULL);
	AssertEvents(&cInput, pcDetail, &cPlayer2, "Player 2 Down: Down", "Cursor Left\0 1", "Cursor Right\0 1", "Cursor Right\0 0", "Cursor Left\0 0", "Cursor Down\0 1", "Cursor Down\0 0", NULL);
	AssertEvents(&cInput, pcDetail, &cPlayer2, "Player 2 Down: Down", "Cursor Right\0 1", "Cursor Left\0 1", "Cursor Left\0 0", "Cursor Right\0 0", "Cursor Down\0 1", "Cursor Down\0 0", NULL);
	AssertEvents(&cInput, pcDetail, &cPlayer2, "Player 2 Down: Down", "Cursor Right\0 1", "Cursor Left\0 1", "Cursor Right\0 0", "Cursor Left\0 0", "Cursor Down\0 1", "Cursor Down\0 0", NULL);

	cPlayer1.GetEvents()->ReInit();
	pcDetail->AddEvent("W", 1.0f);
	cInput.Update();
	AssertSize(0, cPlayer1.GetEvents()->NumElements());

	pcDetail->AddEvent("Cursor Left", 1.0f);
	cInput.Update();
	AssertSize(0, cPlayer1.GetEvents()->NumElements());

	pcDetail->AddEvent("W", 0.0f);
	cInput.Update();
	AssertSize(0, cPlayer1.GetEvents()->NumElements());

	pcDetail->AddEvent("W", 1.0f);
	cInput.Update();
	AssertSize(1, cPlayer1.GetEvents()->NumElements());
	AssertString("Player 1 Up: Up", cPlayer1.GetEvents()->GetText(0));
	cPlayer1.GetEvents()->ReInit();

	cPlayer2.Kill();
	cPlayer1.Kill();
	cInput.Kill();

	TestInputChordsHeldAndEvicted();
	TestInputChordsEvictionAge();
	TestInputChordsPrefix();
	TestInputChordsCommonStart();

	TestStatistics();

	ObjectsKill();
	DataIOKill();
	MemoryKill();
	TypeConverterKill();
	TypesKill();
}

