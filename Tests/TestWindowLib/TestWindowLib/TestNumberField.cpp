#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/Focus.h"
#include "WindowLib/JewelTheme.h"
#include "WindowLib/NumberField.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"


Ptr<CFont>		GenerateMenuFont(Ptr<CWindow> pWindow);
SInt32Vec2		ComponentCentre(Ptr<CComponent> pComponent);
void			MoveTo(Ptr<CWindow> pWindow, SInt32Vec2 sPosition);
void			Click(Ptr<CWindow> pWindow);


//Counts a number field's changes.
struct STestNumberFieldChanges
{
	int32		iChanges;
	float64		fLast;

	void Changed(CNumberField* pcField)
	{
		iChanges++;
		fLast = pcField->GetValue();
	}
};


//////////////////////////////////////////////////////////////////////////
//
// A number field 200 wide at the top left of a window.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CNumberField> CreateTestNumberField(Ptr<CWindow> pWindow, const char* szName, float64 fValue)
{
	Ptr<CFillContainer>		pFill;
	Ptr<CNumberField>		pField;

	pFill = OMalloc<CFillContainer>(pWindow);
	pFill->SetInsets(10, 10, 10, 10);
	pWindow->SetContainer(pFill);
	pField = OMalloc<CNumberField>(pWindow, CJewelTheme::CreateDarkTheme(pWindow, GenerateMenuFont(pWindow)), szName, fValue);
	pField->SetDesiredSize(200, pField->GetDesiredSize().y);
	pField->SetAlignment(CSH_Left, CSV_Top);
	pFill->AddComponent(pField);
	return pField;
}


//////////////////////////////////////////////////////////////////////////
//
// The pointer pressed in the middle of pField, moved iRight and let go.
//
//////////////////////////////////////////////////////////////////////////
static void DragTestNumberField(Ptr<CWindow> pWindow, Ptr<CNumberField> pField, int32 iRight)
{
	SInt32Vec2	sCentre;

	sCentre = ComponentCentre(pField);
	MoveTo(pWindow, sCentre);
	pWindow->InputCommand(CIC_ActivateComponent, true);
	MoveTo(pWindow, SInt32Vec2(sCentre.x + iRight, sCentre.y));
	pWindow->InputCommand(CIC_ExecuteComponent, true);
}


//////////////////////////////////////////////////////////////////////////
//
// With no range a pixel's drag is a hundredth of the number's size, the
// drag measured from where it was pressed; SetValue calls nothing.
//
//////////////////////////////////////////////////////////////////////////
void TestNumberFieldDragged(void)
{
	Ptr<CWinRefWindowFactory>	pNativeFactory;

	{
		Ptr<CWindow>				pWindow;
		Ptr<CNumberField>			pField;
		STestNumberFieldChanges		sChanges;
		CPointer					pNull;

		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, 300, 100, (char*)NULL);
		pWindow = OMalloc<CWindow>("Number Field Window", &pNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);
		pField = CreateTestNumberField(pWindow, "Scale", 2.0);
		sChanges.iChanges = 0;
		sChanges.fLast = 0;
		pField->GetChangeEvent()->Add(&sChanges, &STestNumberFieldChanges::Changed);
		pWindow->Layout();

		AssertInt(200, pField->GetActualSize().x);
		AssertString("2.000", pField->GetNumberText());
		AssertFalse(pField->IsEditing());

		//50 pixels right of a 2 is 50 hundredths of 2 more.
		DragTestNumberField(pWindow, pField, 50);
		AssertFalse(pField->IsEditing());
		AssertDouble(3.0, pField->GetValue(), 6);
		AssertString("3.000", pField->GetNumberText());
		AssertInt(1, sChanges.iChanges);
		AssertDouble(3.0, sChanges.fLast, 6);

		//And left takes it down, through nothing.
		DragTestNumberField(pWindow, pField, -150);
		AssertDouble(-1.5, pField->GetValue(), 6);
		AssertString("-1.500", pField->GetNumberText());
		AssertInt(2, sChanges.iChanges);

		//A number smaller than one changes as one does, so it can leave nothing.
		pField->SetValue(0);
		AssertInt(2, sChanges.iChanges);
		DragTestNumberField(pWindow, pField, 25);
		AssertDouble(0.25, pField->GetValue(), 6);
		AssertInt(3, sChanges.iChanges);

		pField->GetChangeEvent()->Remove(&sChanges);
		pField = NULL;
		ORoot()->Remove(pWindow);
		pWindow = NULL;
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	pNativeFactory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// With a range the field's width is the range, the value stops at its
// ends, and a whole number moves a step at a time.
//
//////////////////////////////////////////////////////////////////////////
void TestNumberFieldRangeAndWhole(void)
{
	Ptr<CWinRefWindowFactory>	pNativeFactory;

	{
		Ptr<CWindow>				pWindow;
		Ptr<CNumberField>			pField;
		STestNumberFieldChanges		sChanges;
		CPointer					pNull;

		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, 300, 100, (char*)NULL);
		pWindow = OMalloc<CWindow>("Number Field Window", &pNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);
		pField = CreateTestNumberField(pWindow, "Amount", 5.0);
		sChanges.iChanges = 0;
		pField->GetChangeEvent()->Add(&sChanges, &STestNumberFieldChanges::Changed);
		pWindow->Layout();

		//Setting a range keeps the value in it.
		pField->SetRange(0.0, 1.0);
		AssertDouble(1.0, pField->GetValue(), 6);
		AssertInt(0, sChanges.iChanges);

		//A quarter of the width is a quarter of the range.
		DragTestNumberField(pWindow, pField, -50);
		AssertDouble(0.75, pField->GetValue(), 6);
		DragTestNumberField(pWindow, pField, -500);
		AssertDouble(0.0, pField->GetValue(), 6);
		AssertInt(2, sChanges.iChanges);
		DragTestNumberField(pWindow, pField, -20);  //Already at the end: nothing changes.
		AssertInt(2, sChanges.iChanges);

		//A whole number with no range: one for every NUMBER_FIELD_INTEGER_PIXELS.
		pField->SetRange(0.0, 0.0);
		pField->SetWhole(true);
		pField->SetValue(4.4);
		AssertDouble(4.0, pField->GetValue(), 6);
		AssertString("4", pField->GetNumberText());
		DragTestNumberField(pWindow, pField, 3 * NUMBER_FIELD_INTEGER_PIXELS);
		AssertDouble(7.0, pField->GetValue(), 6);
		AssertString("7", pField->GetNumberText());

		pField->GetChangeEvent()->Remove(&sChanges);
		pField = NULL;
		ORoot()->Remove(pWindow);
		pWindow = NULL;
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	pNativeFactory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Clicked, it's typed into.  Enter takes a number, kept to the range;
// Escape and anything that isn't a number put back what there was.
//
//////////////////////////////////////////////////////////////////////////
void TestNumberFieldTyped(void)
{
	Ptr<CWinRefWindowFactory>	pNativeFactory;

	{
		Ptr<CWindow>				pWindow;
		Ptr<CNumberField>			pField;
		Ptr<CFocus>					pFocus;
		STestNumberFieldChanges		sChanges;
		CPointer					pNull;

		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, 300, 100, (char*)NULL);
		pWindow = OMalloc<CWindow>("Number Field Window", &pNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);
		pFocus = pWindow->GetFocus();
		pField = CreateTestNumberField(pWindow, "Scale", 2.0);
		sChanges.iChanges = 0;
		pField->GetChangeEvent()->Add(&sChanges, &STestNumberFieldChanges::Changed);
		pWindow->Layout();

		//A click opens a text field of the number, all selected, with the focus: typing replaces it.
		MoveTo(pWindow, ComponentCentre(pField));
		Click(pWindow);
		AssertTrue(pField->IsEditing());
		pWindow->Layout();
		AssertString("2.000", pField->GetEdit()->GetText());
		AssertTrue(pFocus->GetFocussedComponent() == pField->GetEdit());
		pWindow->InputPrintable('1', true);
		pWindow->InputPrintable('.', true);
		pWindow->InputPrintable('5', true);
		AssertString("1.5", pField->GetEdit()->GetText());
		AssertInt(0, sChanges.iChanges);
		pWindow->InputCommand(CIC_Enter, true);
		AssertFalse(pField->IsEditing());
		AssertDouble(1.5, pField->GetValue(), 6);
		AssertString("1.500", pField->GetNumberText());
		AssertInt(1, sChanges.iChanges);
		pWindow->Layout();

		//Escape puts back what there was.
		MoveTo(pWindow, ComponentCentre(pField));
		Click(pWindow);
		AssertTrue(pField->IsEditing());
		pWindow->InputPrintable('9', true);
		pWindow->InputCommand(CIC_ClearSelection, true);
		AssertFalse(pField->IsEditing());
		AssertDouble(1.5, pField->GetValue(), 6);
		AssertInt(1, sChanges.iChanges);
		pWindow->Layout();

		//So does something that isn't a number.
		pField->BeginEdit();
		pWindow->InputPrintable('1', true);
		pWindow->InputPrintable('x', true);
		pWindow->InputCommand(CIC_Enter, true);
		AssertFalse(pField->IsEditing());
		AssertDouble(1.5, pField->GetValue(), 6);
		AssertInt(1, sChanges.iChanges);

		//What's typed is kept to the range.
		pField->SetRange(0.0, 10.0);
		pField->BeginEdit();
		pWindow->InputPrintable('2', true);
		pWindow->InputPrintable('5', true);
		pWindow->InputCommand(CIC_Enter, true);
		AssertDouble(10.0, pField->GetValue(), 6);
		AssertInt(2, sChanges.iChanges);

		//Focus going elsewhere takes what was typed.
		pField->BeginEdit();
		pWindow->InputPrintable('3', true);
		pFocus->SetFocus(Ptr<CComponent>());
		AssertFalse(pField->IsEditing());
		AssertDouble(3.0, pField->GetValue(), 6);
		AssertInt(3, sChanges.iChanges);

		pField->GetChangeEvent()->Remove(&sChanges);
		pFocus = NULL;
		pField = NULL;
		ORoot()->Remove(pWindow);
		pWindow = NULL;
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	pNativeFactory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNumberField(void)
{
	BeginTests();
	ObjectsInit();

	TestNumberFieldDragged();
	TestNumberFieldRangeAndWhole();
	TestNumberFieldTyped();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

