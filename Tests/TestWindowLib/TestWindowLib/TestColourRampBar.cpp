#include <stdlib.h>
#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Objects.h"
#include "SupportLib/PNGWriter.h"
#include "WindowLib/Window.h"
#include "WindowLib/Canvas.h"
#include "WindowLib/ColourRampBar.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/JewelTheme.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"


Ptr<CFont>		GenerateMenuFont(Ptr<CWindow> pWindow);
void			MoveTo(Ptr<CWindow> pWindow, SInt32Vec2 sPosition);


//Counts a ramp's changes and choices.
struct STestColourRampEvents
{
	int32	iChanges;
	int32	iChoices;

	void Changed(CColourRampBar* pcRamp) { iChanges++; }
	void Chosen(CColourRampBar* pcRamp) { iChoices++; }
};


//////////////////////////////////////////////////////////////////////////
//
// Where a stop's marker is in the window: under the bar at its place.
//
//////////////////////////////////////////////////////////////////////////
static SInt32Vec2 TestColourRampMarker(Ptr<CColourRampBar> pRamp, float32 fPlace)
{
	Ptr<CCanvas>	pBar;
	SInt32Vec2		sPosition;
	SInt32Vec2		sSize;

	pBar = pRamp->GetBar();
	sPosition = pBar->GetPosition();
	sSize = pBar->GetActualSize();
	return SInt32Vec2(sPosition.x + COLOUR_RAMP_MARKER_WIDTH + (int32)(fPlace * (sSize.x - 2 * COLOUR_RAMP_MARKER_WIDTH)), sPosition.y + sSize.y - COLOUR_RAMP_MARKER_HEIGHT / 2);
}


//////////////////////////////////////////////////////////////////////////
//
// A ramp's stops are in the order of their places; its colour is its
// stops' and a blend between them; a marker pressed is chosen, and
// dragged it's moved past the others; + adds one half way to the next,
// - removes the chosen one but not the last; the chosen stop's place and
// colour are changed by their fields.
//
//////////////////////////////////////////////////////////////////////////
void TestColourRampBarStops(void)
{
	Ptr<CWinRefWindowFactory>	pNativeFactory;

	{
		Ptr<CWindow>				pWindow;
		Ptr<CFillContainer>			pFill;
		Ptr<CColourRampBar>			pRamp;
		STestColourRampEvents		sEvents;
		CPointer					pNull;
		float32						afColour[3];
		SInt32Vec2					sMarker;

		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, 320, 200, (char*)NULL);
		pWindow = OMalloc<CWindow>("Colour Ramp Window", &pNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);
		pFill = OMalloc<CFillContainer>(pWindow);
		pFill->SetInsets(10, 10, 10, 10);
		pWindow->SetContainer(pFill);
		pRamp = OMalloc<CColourRampBar>(pWindow, CJewelTheme::CreateDarkTheme(pWindow, GenerateMenuFont(pWindow)));
		pRamp->SetAlignment(CSH_Stretch, CSV_Top);
		pFill->AddComponent(pRamp);
		sEvents.iChanges = 0;
		sEvents.iChoices = 0;
		pRamp->GetChangeEvent()->Add(&sEvents, &STestColourRampEvents::Changed);
		pRamp->GetChoiceEvent()->Add(&sEvents, &STestColourRampEvents::Chosen);

		//Black with no stops.  Added out of order they're kept in order; the first added is the chosen one.
		pRamp->At(0.5f, afColour);
		AssertFloat(0.0f, afColour[0], 6);
		AssertInt(-1, pRamp->GetChosen());
		AssertInt(0, pRamp->AddStop(1.0f, 1.0f, 1.0f, 1.0f, 7));
		AssertInt(0, pRamp->AddStop(0.0f, 1.0f, 0.0f, 0.0f, 5));
		AssertInt(1, pRamp->AddStop(0.5f, 0.0f, 0.0f, 1.0f, 6));
		AssertSize(3, pRamp->NumStops());
		AssertInt(5, pRamp->GetStop(0)->iData);
		AssertInt(6, pRamp->GetStop(1)->iData);
		AssertInt(7, pRamp->GetStop(2)->iData);
		AssertInt(2, pRamp->GetChosen());
		AssertInt(0, sEvents.iChanges);
		pWindow->Layout();

		//Red, through purple, to blue, through pale blue, to white; the end stops' colours beyond the ends.
		pRamp->At(0.25f, afColour);
		AssertFloat(0.5f, afColour[0], 6);
		AssertFloat(0.0f, afColour[1], 6);
		AssertFloat(0.5f, afColour[2], 6);
		pRamp->At(0.75f, afColour);
		AssertFloat(0.5f, afColour[0], 6);
		AssertFloat(0.5f, afColour[1], 6);
		AssertFloat(1.0f, afColour[2], 6);
		pRamp->At(-1.0f, afColour);
		AssertFloat(1.0f, afColour[0], 6);
		AssertFloat(0.0f, afColour[2], 6);
		pRamp->At(2.0f, afColour);
		AssertFloat(1.0f, afColour[1], 6);

		//The chosen stop's place and colour are in the fields.
		AssertDouble(1.0, pRamp->GetPlace()->GetValue(), 6);
		AssertFloat(1.0f, pRamp->GetColour()->GetColour()[1], 6);

		//The middle marker pressed is chosen; let go where it is, nothing moved.
		sMarker = TestColourRampMarker(pRamp, 0.5f);
		MoveTo(pWindow, sMarker);
		pWindow->InputCommand(CIC_ActivateComponent, true);
		AssertInt(1, pRamp->GetChosen());
		AssertInt(1, sEvents.iChoices);
		AssertDouble(0.5, pRamp->GetPlace()->GetValue(), 6);
		AssertFloat(0.0f, pRamp->GetColour()->GetColour()[1], 6);
		pWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertInt(0, sEvents.iChanges);

		//Dragged a quarter of the way left it's moved; dragged off the left end it's the first stop, at 0.
		MoveTo(pWindow, sMarker);
		pWindow->InputCommand(CIC_ActivateComponent, true);
		MoveTo(pWindow, TestColourRampMarker(pRamp, 0.25f));
		AssertInt(1, pRamp->GetChosen());
		AssertFloat(0.25f, pRamp->GetStop(1)->fPlace, 2);
		AssertTrue(sEvents.iChanges >= 1);
		MoveTo(pWindow, SInt32Vec2(-50, sMarker.y));
		pWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertFloat(0.0f, pRamp->GetStop(0)->fPlace, 6);
		AssertFloat(0.0f, pRamp->GetStop(1)->fPlace, 6);
		AssertInt(6, pRamp->GetStop(pRamp->GetChosen())->iData);
		pWindow->Layout();

		//A press away from every marker chooses nothing else.
		sEvents.iChoices = 0;
		MoveTo(pWindow, TestColourRampMarker(pRamp, 0.6f));
		pWindow->InputCommand(CIC_ActivateComponent, true);
		pWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertInt(0, sEvents.iChoices);

		//Its place set by the field: past the white stop it's the last.
		sEvents.iChanges = 0;
		pRamp->GetPlace()->SetValue(0.8);
		pRamp->PlaceChanged(&pRamp->GetPlace());
		AssertFloat(0.8f, pRamp->GetStop(pRamp->GetChosen())->fPlace, 6);
		AssertInt(1, sEvents.iChanges);
		AssertInt(6, pRamp->GetStop(1)->iData);

		//Its colour set by the field.
		pRamp->GetColour()->SetColour(0.0f, 1.0f, 0.0f);
		pRamp->ColourChanged(&pRamp->GetColour());
		AssertFloat(1.0f, pRamp->GetStop(1)->afColour[1], 6);
		AssertFloat(0.0f, pRamp->GetStop(1)->afColour[2], 6);
		AssertInt(2, sEvents.iChanges);

		//+ adds a stop half way to the next, 0.9, the ramp's colour there (green half way to white), and chooses it.
		pRamp->AddClicked(NULL);
		AssertSize(4, pRamp->NumStops());
		AssertInt(2, pRamp->GetChosen());
		AssertFloat(0.9f, pRamp->GetStop(2)->fPlace, 6);
		AssertFloat(0.5f, pRamp->GetStop(2)->afColour[0], 5);
		AssertFloat(1.0f, pRamp->GetStop(2)->afColour[1], 5);
		AssertInt(-1, pRamp->GetStop(2)->iData);
		AssertInt(3, sEvents.iChanges);

		//NODE_EDITOR_PICTURE's neighbour: COLOUR_RAMP_PICTURE=file saves the window, to look at.
		pWindow->Layout();
		pWindow->Paint();
		if (getenv("COLOUR_RAMP_PICTURE") != NULL)
		{
			SavePNG(pWindow->GetCanvas()->GetImageOrNull(), getenv("COLOUR_RAMP_PICTURE"));
		}

		//- removes the chosen one and chooses the one before; the last one stays.
		pRamp->RemoveClicked(NULL);
		AssertSize(3, pRamp->NumStops());
		AssertInt(1, pRamp->GetChosen());
		pRamp->RemoveClicked(NULL);
		pRamp->RemoveClicked(NULL);
		AssertSize(1, pRamp->NumStops());
		AssertInt(0, pRamp->GetChosen());
		pRamp->RemoveClicked(NULL);
		AssertSize(1, pRamp->NumStops());
		AssertInt(7, pRamp->GetStop(0)->iData);

		pRamp->RemoveAllStops();
		AssertSize(0, pRamp->NumStops());
		AssertInt(-1, pRamp->GetChosen());

		pRamp->GetChangeEvent()->Remove(&sEvents);
		pRamp->GetChoiceEvent()->Remove(&sEvents);
		pRamp = NULL;
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
void TestColourRampBar(void)
{
	BeginTests();
	ObjectsInit();

	TestColourRampBarStops();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

