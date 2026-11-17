#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/ColourField.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/Focus.h"
#include "WindowLib/JewelTheme.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"


Ptr<CFont>		GenerateMenuFont(Ptr<CWindow> pWindow);
SInt32Vec2		ComponentCentre(Ptr<CComponent> pComponent);
void			MoveTo(Ptr<CWindow> pWindow, SInt32Vec2 sPosition);
void			Click(Ptr<CWindow> pWindow);


//Counts a colour field's changes.
struct STestColourFieldChanges
{
	int32	iChanges;

	void Changed(CColourField* pcField)
	{
		iChanges++;
	}
};


//////////////////////////////////////////////////////////////////////////
//
// A colour picker's fields show its colour; one changed sets the others,
// a grey keeps the hue last chosen, and six hex digits are a colour.
//
//////////////////////////////////////////////////////////////////////////
void TestColourPickerFields(void)
{
	Ptr<CWinRefWindowFactory>	pNativeFactory;

	{
		Ptr<CWindow>			pWindow;
		Ptr<CFillContainer>		pFill;
		Ptr<CColourPicker>		pPicker;
		CPointer				pNull;
		float32*				pafColour;

		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, 300, 300, (char*)NULL);
		pWindow = OMalloc<CWindow>("Colour Picker Window", &pNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);
		pFill = OMalloc<CFillContainer>(pWindow);
		pWindow->SetContainer(pFill);
		pPicker = OMalloc<CColourPicker>(pWindow, CJewelTheme::CreateDarkTheme(pWindow, GenerateMenuFont(pWindow)), 1.0f, 0.5f, 0.0f);
		pPicker->SetAlignment(CSH_Left, CSV_Top);
		pFill->AddComponent(pPicker);
		pWindow->Layout();

		//Orange: a twelfth of the way round, full saturation and value.
		AssertDouble(1.0, pPicker->GetChannel(0)->GetValue(), 6);
		AssertDouble(0.5, pPicker->GetChannel(1)->GetValue(), 6);
		AssertDouble(0.0, pPicker->GetChannel(2)->GetValue(), 6);
		AssertDouble(0.0833333, pPicker->GetChannel(3)->GetValue(), 5);
		AssertDouble(1.0, pPicker->GetChannel(4)->GetValue(), 6);
		AssertDouble(1.0, pPicker->GetChannel(5)->GetValue(), 6);
		AssertString("FF8000", pPicker->GetHex()->GetText());

		//The value halved halves the red and green.
		pPicker->GetChannel(5)->SetValue(0.5);
		pPicker->HSVChanged(&pPicker->GetChannel(5));
		pafColour = pPicker->GetColour();
		AssertFloat(0.5f, pafColour[0], 5);
		AssertFloat(0.25f, pafColour[1], 5);
		AssertFloat(0.0f, pafColour[2], 5);
		AssertString("804000", pPicker->GetHex()->GetText());

		//No saturation is a grey, which keeps the hue it had; saturation back is orange again.
		pPicker->GetChannel(4)->SetValue(0.0);
		pPicker->HSVChanged(&pPicker->GetChannel(4));
		AssertFloat(0.5f, pafColour[0], 5);
		AssertFloat(0.5f, pafColour[1], 5);
		AssertFloat(0.5f, pafColour[2], 5);
		AssertDouble(0.0833333, pPicker->GetChannel(3)->GetValue(), 5);
		pPicker->GetChannel(4)->SetValue(1.0);
		pPicker->HSVChanged(&pPicker->GetChannel(4));
		AssertFloat(0.5f, pafColour[0], 5);
		AssertFloat(0.25f, pafColour[1], 5);
		AssertFloat(0.0f, pafColour[2], 5);

		//Blue set, the hue is two thirds.
		pPicker->GetChannel(0)->SetValue(0.0);
		pPicker->GetChannel(1)->SetValue(0.0);
		pPicker->GetChannel(2)->SetValue(1.0);
		pPicker->RGBChanged(&pPicker->GetChannel(2));
		AssertDouble(0.6666667, pPicker->GetChannel(3)->GetValue(), 5);
		AssertString("0000FF", pPicker->GetHex()->GetText());

		//Hex typed: only six digits are a colour, with or without a #.
		pPicker->GetHex()->SetText("40C0");
		AssertFloat(1.0f, pafColour[2], 5);
		pPicker->GetHex()->SetText("#40C080");
		AssertFloat(64.0f / 255.0f, pafColour[0], 5);
		AssertFloat(192.0f / 255.0f, pafColour[1], 5);
		AssertFloat(128.0f / 255.0f, pafColour[2], 5);
		AssertDouble(192.0 / 255.0, pPicker->GetChannel(5)->GetValue(), 5);
		pPicker->GetHex()->SetText("40C08G");
		AssertFloat(128.0f / 255.0f, pafColour[2], 5);

		//SetColour shows a colour.
		pPicker->SetColour(1.0f, 1.0f, 1.0f);
		AssertString("FFFFFF", pPicker->GetHex()->GetText());
		AssertDouble(0.0, pPicker->GetChannel(4)->GetValue(), 6);

		pPicker = NULL;
		pFill = NULL;
		ORoot()->Remove(pWindow);
		pWindow = NULL;
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	pNativeFactory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A colour field pressed opens its picker under itself; the picker's
// changes are the field's; pressed again, or a press elsewhere, closes
// it.
//
//////////////////////////////////////////////////////////////////////////
void TestColourFieldPicker(void)
{
	Ptr<CWinRefWindowFactory>	pNativeFactory;

	{
		Ptr<CWindow>				pWindow;
		Ptr<CFillContainer>			pFill;
		Ptr<CColourField>			pField;
		Ptr<CColourPicker>			pPicker;
		STestColourFieldChanges		sChanges;
		CPointer					pNull;

		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, 400, 400, (char*)NULL);
		pWindow = OMalloc<CWindow>("Colour Field Window", &pNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);
		pFill = OMalloc<CFillContainer>(pWindow);
		pFill->SetInsets(10, 10, 10, 10);
		pWindow->SetContainer(pFill);
		pField = OMalloc<CColourField>(pWindow, CJewelTheme::CreateDarkTheme(pWindow, GenerateMenuFont(pWindow)), "Colour", 0.2f, 0.4f, 0.6f);
		pField->SetDesiredSize(200, 24);
		pField->SetAlignment(CSH_Left, CSV_Top);
		pFill->AddComponent(pField);
		sChanges.iChanges = 0;
		pField->GetChangeEvent()->Add(&sChanges, &STestColourFieldChanges::Changed);
		pWindow->Layout();

		AssertTrue(pField->GetPicker().IsNull());
		AssertSize(0, pWindow->NumPopups());

		//Pressed, the picker opens under it showing its colour.
		MoveTo(pWindow, ComponentCentre(pField));
		Click(pWindow);
		pWindow->Layout();
		pPicker = pField->GetPicker();
		AssertTrue(pPicker.IsNotNull());
		AssertSize(1, pWindow->NumPopups());
		AssertTrue(pPicker->GetPosition().y >= pField->GetPosition().y + pField->GetActualSize().y);
		AssertDouble(0.2, pPicker->GetChannel(0)->GetValue(), 6);
		AssertInt(0, sChanges.iChanges);

		//The picker's red dragged to the right: the field's colour is the picker's.
		MoveTo(pWindow, ComponentCentre(pPicker->GetChannel(0)));
		pWindow->InputCommand(CIC_ActivateComponent, true);
		MoveTo(pWindow, SInt32Vec2(ComponentCentre(pPicker->GetChannel(0)).x + 500, ComponentCentre(pPicker->GetChannel(0)).y));
		pWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertFloat(1.0f, pField->GetColour()[0], 6);
		AssertFloat(0.4f, pField->GetColour()[1], 6);
		AssertTrue(sChanges.iChanges >= 1);
		AssertSize(1, pWindow->NumPopups());  //A press in the picker leaves it open.

		//The field's colour set shows in the open picker, and calls nothing.
		sChanges.iChanges = 0;
		pField->SetColour(0.0f, 1.0f, 0.0f);
		AssertString("00FF00", pPicker->GetHex()->GetText());
		AssertInt(0, sChanges.iChanges);

		//Pressed again it closes.
		pWindow->Layout();
		MoveTo(pWindow, ComponentCentre(pField));
		Click(pWindow);
		AssertTrue(pField->GetPicker().IsNull());
		AssertSize(0, pWindow->NumPopups());

		//Opened, a press on the background closes it.
		pField->OpenPicker();
		AssertSize(1, pWindow->NumPopups());
		pWindow->Layout();
		MoveTo(pWindow, SInt32Vec2(380, 380));
		Click(pWindow);
		AssertTrue(pField->GetPicker().IsNull());
		AssertSize(0, pWindow->NumPopups());

		pField->GetChangeEvent()->Remove(&sChanges);
		pPicker = NULL;
		pField = NULL;
		pFill = NULL;
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
void TestColourField(void)
{
	BeginTests();
	ObjectsInit();

	TestColourPickerFields();
	TestColourFieldPicker();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

