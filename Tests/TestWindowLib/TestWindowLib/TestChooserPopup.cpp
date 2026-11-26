#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/ClearCanvasDraw.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/JewelTheme.h"
#include "WindowLib/JewelColours.h"
#include "WindowLib/ChooserPopup.h"
#include "WindowLib/PromptPopup.h"
#include "WindowLib/InfoPopup.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"


void	Click(Ptr<CWindow> pWindow);


Ptr<CFont>			GenerateMenuFont(Ptr<CWindow> pWindow);
Ptr<CFontFamily>	CreateMonoTextAreaFamily(float32 fDefaultPixelSize);


//////////////////////////////////////////////////////////////////////////
//
// Counts a chooser's events.
//
//////////////////////////////////////////////////////////////////////////
class CTestChooserListener
{
public:
	int		miChosen;
	int		miSelected;
	int		miButton;
	int		miClosed;

	void Init(void) { miChosen = 0; miSelected = 0; miButton = 0; miClosed = 0; }
	void Chosen(void) { miChosen++; }
	void Selected(void) { miSelected++; }
	void Button(void) { miButton++; }
	void Closed(void) { miClosed++; }
};


//////////////////////////////////////////////////////////////////////////
//
// The rows' names as "a,b,c", a heading's in brackets.
//
//////////////////////////////////////////////////////////////////////////
static char* PrintChooserRows(Ptr<CChooserPopup> pChooser, CChars* pszDest)
{
	SGoToItem*	psItem;
	size		i;

	pszDest->Clear();
	for (i = 0; i < pChooser->NumRows(); i++)
	{
		if (i > 0)
		{
			pszDest->Append(",");
		}
		psItem = pChooser->GetRowItem(i);
		if (psItem->uiData == CHOOSER_HEADING)
		{
			pszDest->Append("[");
			pszDest->Append(psItem->szName.Text());
			pszDest->Append("]");
		}
		else
		{
			pszDest->Append(psItem->szName.Text());
		}
	}
	return pszDest->Text();
}


//////////////////////////////////////////////////////////////////////////
//
// A chooser with a filter: typing filters its rows, the keys move through
// them from the field, enter chooses, escape closes, and what had the
// keys gets them back.
//
//////////////////////////////////////////////////////////////////////////
void TestChooserPopupFilter(void)
{
	Ptr<CWinRefWindowFactory>	pNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		SDialogLook					sLook;
		Ptr<CWindowTheme>			pTheme;
		Ptr<CFillContainer>			pFill;
		Ptr<CTextField>				pOwner;
		Ptr<CChooserPopup>			pChooser;
		CTestChooserListener		cListener;
		CChars						szRows;
		SInt32Vec2					sPosition;

		szRows.Init();
		cListener.Init();
		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, 800, 600, (char*)NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Chooser Owner", &pNativeFactory, pTick, OMalloc<CClearCanvasDraw>(JEWEL_EDITOR), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pTheme = CJewelTheme::CreateDarkTheme(pTestWindow, GenerateMenuFont(pTestWindow));
		sLook.pcTheme = &pTheme;
		sLook.pcDimFont = &pTheme->GetFont();

		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);
		pOwner = OMalloc<CTextField>(pTestWindow, pTheme, "owner");
		pFill->AddComponent(pOwner);
		pTestWindow->Layout();
		pTestWindow->GetFocus()->SetFocus(pOwner);

		pChooser = OMalloc<CChooserPopup>(pTestWindow, &sLook, pTheme->GetFont());
		ORoot()->Add(pChooser);
		pChooser->GetChosenEvent()->Add(&cListener, &CTestChooserListener::Chosen);
		pChooser->GetSelectedEvent()->Add(&cListener, &CTestChooserListener::Selected);
		pChooser->GetClosedEvent()->Add(&cListener, &CTestChooserListener::Closed);
		pChooser->SetTitle("Usages of Init  5 usages");
		pChooser->AddFilter();
		pChooser->AddItem("CTextArea", "TextArea.cpp:12", -1, 10);
		pChooser->AddItem("CTextField", "TextField.cpp:40", -1, 11);
		pChooser->AddItem("CTabbedContainer", "TabbedContainer.cpp:7", -1, 12);
		pChooser->AddItem("CListContainer", "ListContainer.cpp:90", -1, 13);
		pChooser->AddItem("CGoToDialog", "GoToDialog.cpp:3", -1, 14);
		AssertFalse(pChooser->IsOpen());
		AssertSize(5, pChooser->NumItems());

		//Opened with every item listed in the order added, the item asked for selected and the field with the keys.
		sPosition.Init(20, 30);
		pChooser->Open(sPosition, pOwner, 1);
		AssertTrue(pChooser->IsOpen());
		AssertSize(1, pTestWindow->NumPopups());
		AssertString("CTextArea,CTextField,CTabbedContainer,CListContainer,CGoToDialog", PrintChooserRows(pChooser, &szRows));
		AssertInt(1, pChooser->GetSelectedRow());
		AssertString("Usages of Init  5 usages", pChooser->GetTitle());
		AssertTrue(pTestWindow->GetFocus()->GetFocussedComponent() == pChooser->GetField());
		AssertInt(5, pChooser->GetList()->NumRows());

		//It is as wide as the theme says and tall enough for its title, its field and its five rows.
		pTestWindow->Layout();
		AssertInt(pTheme->GetMetric(TM_ChooserWidth), pChooser->GetPanel()->GetActualSize().x);
		AssertTrue(pChooser->GetPanel()->GetActualSize().y >= 5 * pTheme->GetMetric(TM_GoToRowHeight) + pChooser->GetField()->GetActualSize().y);
		AssertInt(5 * pTheme->GetMetric(TM_GoToRowHeight), pChooser->GetList()->GetParent()->GetActualSize().y);
		AssertTrue(pChooser->GetField()->GetActualSize().x > 100);

		//Typing filters, as Go to matches, the rows keeping their order; the first is selected.
		pTestWindow->InputPrintable('C', true);
		pTestWindow->InputPrintable('T', true);
		AssertString("CT", pChooser->GetFilterText());
		AssertString("CTextArea,CTextField,CTabbedContainer,CGoToDialog", PrintChooserRows(pChooser, &szRows));
		AssertInt(0, pChooser->GetSelectedRow());
		AssertSize(1, pChooser->GetRowItem(0)->sMatch.uiNumRanges);
		pTestWindow->InputPrintable('F', true);
		AssertString("CTextField", PrintChooserRows(pChooser, &szRows));
		AssertString("CTextField", pChooser->GetSelectedItem()->szName.Text());
		pTestWindow->InputCommand(CIC_Backspace, true);
		pTestWindow->InputCommand(CIC_Backspace, true);
		pTestWindow->InputCommand(CIC_Backspace, true);
		AssertSize(5, pChooser->NumRows());
		AssertSize(0, pChooser->GetRowItem(0)->sMatch.uiNumRanges);

		//Down and up move through the rows from the field, going round at the ends.
		pTestWindow->InputCommand(CIC_Down, true);
		AssertInt(1, pChooser->GetSelectedRow());
		pTestWindow->InputCommand(CIC_Up, true);
		pTestWindow->InputCommand(CIC_Up, true);
		AssertInt(4, pChooser->GetSelectedRow());
		pTestWindow->InputCommand(CIC_Down, true);
		AssertInt(0, pChooser->GetSelectedRow());
		AssertTrue(cListener.miSelected > 0);

		//Nothing matched, nothing to choose: enter leaves it open.
		pChooser->SetFilterText("zzz");
		AssertSize(0, pChooser->NumRows());
		AssertInt(-1, pChooser->GetSelectedRow());
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertTrue(pChooser->IsOpen());
		AssertInt(0, cListener.miChosen);

		//Enter chooses the selected row's item and closes the chooser; what had the keys has them back.
		pChooser->SetFilterText("list");
		AssertString("CListContainer", PrintChooserRows(pChooser, &szRows));
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertFalse(pChooser->IsOpen());
		AssertSize(0, pTestWindow->NumPopups());
		AssertInt(1, cListener.miChosen);
		AssertInt(1, cListener.miClosed);
		AssertTrue(pChooser->GetChosen() != NULL);
		AssertString("CListContainer", pChooser->GetChosen()->szName.Text());
		AssertSize(13, pChooser->GetChosen()->uiData);
		AssertTrue(pTestWindow->GetFocus()->GetFocussedComponent() == pOwner);

		//Opened again it starts unfiltered with nothing chosen; escape closes it and chooses nothing.
		pChooser->Open(sPosition, pOwner);
		AssertTrue(pChooser->IsOpen());
		AssertString("", pChooser->GetFilterText());
		AssertSize(5, pChooser->NumRows());
		AssertInt(0, pChooser->GetSelectedRow());
		AssertTrue(pChooser->GetChosen() == NULL);
		pTestWindow->InputCommand(CIC_ClearSelection, true);
		AssertFalse(pChooser->IsOpen());
		AssertInt(1, cListener.miChosen);
		AssertInt(2, cListener.miClosed);
		AssertTrue(pChooser->GetChosen() == NULL);
		AssertTrue(pTestWindow->GetFocus()->GetFocussedComponent() == pOwner);

		//Sorted, the best match is first whatever order they were added in.
		pChooser->SetSortMatches(true);
		pChooser->Open(sPosition, pOwner);
		pChooser->SetFilterText("ta");
		AssertString("CTabbedContainer,CTextArea", PrintChooserRows(pChooser, &szRows));
		pChooser->Close();
		AssertInt(3, cListener.miClosed);

		pChooser->GetChosenEvent()->Remove(&cListener);
		pChooser->GetSelectedEvent()->Remove(&cListener);
		pChooser->GetClosedEvent()->Remove(&cListener);
		szRows.Kill();
		pChooser = NULL;
		pOwner = NULL;
		pFill = NULL;
		pTheme = NULL;
		pTick = NULL;
		pTestWindow = NULL;
	}

	AssertTrue(ORoot()->RemoveAll());
	pNativeFactory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A chooser without a filter: the list has the keys.  Headings can't be
// chosen and stay over what is under them; the button and the preview
// are there when asked for.
//
//////////////////////////////////////////////////////////////////////////
void TestChooserPopupHeadings(void)
{
	Ptr<CWinRefWindowFactory>	pNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		SDialogLook					sLook;
		Ptr<CWindowTheme>			pTheme;
		Ptr<CFillContainer>			pFill;
		Ptr<CChooserPopup>			pChooser;
		Ptr<CChooserPopup>			pFiltered;
		CTestChooserListener		cListener;
		CChars						szRows;
		SInt32Vec2					sPosition;

		szRows.Init();
		cListener.Init();
		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, 1200, 700, (char*)NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Chooser Owner", &pNativeFactory, pTick, OMalloc<CClearCanvasDraw>(JEWEL_EDITOR), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pTheme = CJewelTheme::CreateDarkTheme(pTestWindow, GenerateMenuFont(pTestWindow));
		sLook.pcTheme = &pTheme;
		sLook.pcDimFont = &pTheme->GetFont();
		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);
		pTestWindow->Layout();

		pChooser = OMalloc<CChooserPopup>(pTestWindow, &sLook, pTheme->GetFont());
		ORoot()->Add(pChooser);
		pChooser->GetChosenEvent()->Add(&cListener, &CTestChooserListener::Chosen);
		pChooser->GetButtonClickEvent()->Add(&cListener, &CTestChooserListener::Button);
		pChooser->GetClosedEvent()->Add(&cListener, &CTestChooserListener::Closed);
		pChooser->AddPreview(pTheme->GetFont(), CreateMonoTextAreaFamily(14.0f));
		pChooser->AddButton("Open in Find Window");
		pChooser->AddHeading("Super methods");
		pChooser->AddItem("CVessel::Fill", "Vessel.h:10", -1, 1);
		pChooser->AddHeading("Implementations");
		pChooser->AddItem("CDrum::Fill", "Drum.h:22", -1, 2);
		pChooser->AddItem("CRack::Fill", "Rack.h:31", -1, 3);
		AssertTrue(pChooser->GetField().IsNull());
		AssertTrue(pChooser->GetPreview().IsNotNull());
		AssertTrue(pChooser->GetButton().IsNotNull());

		//The first row that can be chosen is selected, not the heading over it; the list has the keys.
		sPosition.Init(10, 10);
		pChooser->Open(sPosition, pFill);
		AssertString("[Super methods],CVessel::Fill,[Implementations],CDrum::Fill,CRack::Fill", PrintChooserRows(pChooser, &szRows));
		AssertInt(1, pChooser->GetSelectedRow());
		AssertTrue(pTestWindow->GetFocus()->GetFocussedComponent() == pChooser->GetList());

		//With a preview it is the theme's most rows tall, the preview beside them; the button is under them.
		pTestWindow->Layout();
		AssertInt(pTheme->GetMetric(TM_ChooserWidth) + pTheme->GetMetric(TM_GoToGap) + pTheme->GetMetric(TM_ChooserPreviewWidth), pChooser->GetPanel()->GetActualSize().x);
		AssertInt(pTheme->GetMetric(TM_ChooserRows) * pTheme->GetMetric(TM_GoToRowHeight), pChooser->GetPreview()->GetActualSize().y);
		AssertTrue(pChooser->GetPreview()->GetActualSize().x >= pTheme->GetMetric(TM_ChooserPreviewWidth));
		AssertTrue(pChooser->GetButton()->GetPosition().y > pChooser->GetPreview()->GetPosition().y);
		AssertFalse(pChooser->GetList()->IsRowEnabled(0));
		AssertTrue(pChooser->GetList()->IsRowEnabled(1));

		//Down steps over a heading; a heading can't be selected or chosen.
		pTestWindow->InputCommand(CIC_Down, true);
		AssertInt(3, pChooser->GetSelectedRow());
		AssertFalse(pChooser->SelectRow(2));
		AssertInt(3, pChooser->GetSelectedRow());

		//With no field, typing finds a row by its name: "ra" is CRack::Fill, and "ve" back round the end CVessel::Fill.
		pTestWindow->InputPrintable('r', true);
		pTestWindow->InputPrintable('a', true);
		AssertTrue(pChooser->GetList()->IsFinding());
		AssertInt(4, pChooser->GetSelectedRow());
		pTestWindow->InputCommand(CIC_Backspace, true);
		pTestWindow->InputCommand(CIC_Backspace, true);
		AssertFalse(pChooser->GetList()->IsFinding());
		pTestWindow->InputPrintable('v', true);
		pTestWindow->InputPrintable('e', true);
		AssertInt(1, pChooser->GetSelectedRow());
		pTestWindow->InputCommand(CIC_Backspace, true);
		pTestWindow->InputCommand(CIC_Backspace, true);
		AssertTrue(pChooser->IsOpen());
		pChooser->SelectRow(3);

		//Enter in the list chooses.
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertFalse(pChooser->IsOpen());
		AssertInt(1, cListener.miChosen);
		AssertSize(2, pChooser->GetChosen()->uiData);

		//The button closes the chooser and calls its event; nothing is chosen.
		pChooser->Open(sPosition, pFill, 4);
		AssertInt(4, pChooser->GetSelectedRow());
		pChooser->GetButton()->Click();
		AssertFalse(pChooser->IsOpen());
		AssertInt(1, cListener.miButton);
		AssertInt(1, cListener.miChosen);
		AssertTrue(pChooser->GetChosen() == NULL);
		AssertInt(2, cListener.miClosed);

		//The items can be replaced between openings.
		pChooser->RemoveAllItems();
		pChooser->AddItem("One", NULL, -1, 7);
		pChooser->Open(sPosition, pFill);
		AssertString("One", PrintChooserRows(pChooser, &szRows));
		pChooser->Close();

		//Opened above: the position is where its bottom left goes.
		pChooser->SetOpensAbove(true);
		sPosition.Init(10, 500);
		pChooser->Open(sPosition, pFill);
		pTestWindow->Layout();
		AssertInt(500 - pChooser->GetPanel()->GetActualSize().y, pChooser->GetPopup()->GetPosition().y);
		AssertInt(10, pChooser->GetPopup()->GetPosition().x);
		pChooser->Close();
		pChooser->SetOpensAbove(false);
		sPosition.Init(10, 10);

		//Filtered, a heading is listed only while something under it is.
		pFiltered = OMalloc<CChooserPopup>(pTestWindow, &sLook, pTheme->GetFont());
		ORoot()->Add(pFiltered);
		pFiltered->AddFilter();
		pFiltered->AddHeading("Super methods");
		pFiltered->AddItem("CVessel::Fill", "Vessel.h:10", -1, 1);
		pFiltered->AddHeading("Implementations");
		pFiltered->AddItem("CDrum::Fill", "Drum.h:22", -1, 2);
		pFiltered->AddItem("CRack::Fill", "Rack.h:31", -1, 3);
		pFiltered->Open(sPosition, pFill);
		pFiltered->SetFilterText("rack");
		AssertString("[Implementations],CRack::Fill", PrintChooserRows(pFiltered, &szRows));
		AssertInt(1, pFiltered->GetSelectedRow());
		pFiltered->SetFilterText("fill");
		AssertString("[Super methods],CVessel::Fill,[Implementations],CDrum::Fill,CRack::Fill", PrintChooserRows(pFiltered, &szRows));

		//A chooser freed while it is open closes its popup.
		AssertSize(1, pTestWindow->NumPopups());
		ORoot()->Remove(pFiltered);
		pFiltered = NULL;
		AssertSize(0, pTestWindow->NumPopups());

		pChooser->GetChosenEvent()->Remove(&cListener);
		pChooser->GetButtonClickEvent()->Remove(&cListener);
		pChooser->GetClosedEvent()->Remove(&cListener);
		szRows.Kill();
		pChooser = NULL;
		pFill = NULL;
		pTheme = NULL;
		pTick = NULL;
		pTestWindow = NULL;
	}

	AssertTrue(ORoot()->RemoveAll());
	pNativeFactory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Counts a prompt's events.
//
//////////////////////////////////////////////////////////////////////////
class CTestPromptListener
{
public:
	CPromptPopup*		mpcPrompt;  //Not held.
	int					miAccepted;
	int					miClosed;
	CChars				mszText;

	void Init(CPromptPopup* pcPrompt) { mpcPrompt = pcPrompt; miAccepted = 0; miClosed = 0; mszText.Init(); }
	void Kill(void) { mszText.Kill(); }
	void Closed(void) { miClosed++; }

	//A number is taken and the prompt closed; anything else is said to be wrong.
	void Accepted(void)
	{
		miAccepted++;
		mszText.Set(mpcPrompt->GetText());
		if ((mszText.Length() > 0) && (mszText.Text()[0] >= '0') && (mszText.Text()[0] <= '9'))
		{
			mpcPrompt->Close();
		}
		else
		{
			mpcPrompt->SetProblem("Type a number.");
		}
	}
};


//////////////////////////////////////////////////////////////////////////
//
// A prompt opens with its text selected and the keys; Enter gives the
// text to whoever listens, who closes it or says what is wrong, which
// stays until it is typed in again; Escape closes it, and what had the
// keys has them back.
//
//////////////////////////////////////////////////////////////////////////
void TestPromptPopup(void)
{
	Ptr<CWinRefWindowFactory>	pNativeFactory;

	ObjectsInit();
	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		SDialogLook					sLook;
		Ptr<CWindowTheme>			pTheme;
		Ptr<CFillContainer>			pFill;
		Ptr<CTextField>				pOwner;
		Ptr<CPromptPopup>			pPrompt;
		CTestPromptListener			cListener;
		SInt32Vec2					sPosition;

		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, 800, 600, (char*)NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Prompt Owner", &pNativeFactory, pTick, OMalloc<CClearCanvasDraw>(JEWEL_EDITOR), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pTheme = CJewelTheme::CreateDarkTheme(pTestWindow, GenerateMenuFont(pTestWindow));
		sLook.pcTheme = &pTheme;
		sLook.pcDimFont = &pTheme->GetFont();
		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);
		pOwner = OMalloc<CTextField>(pTestWindow, pTheme, "owner");
		pFill->AddComponent(pOwner);
		pTestWindow->Layout();
		pTestWindow->GetFocus()->SetFocus(pOwner);

		pPrompt = OMalloc<CPromptPopup>(pTestWindow, &sLook);
		ORoot()->Add(pPrompt);
		cListener.Init(&pPrompt);
		pPrompt->GetAcceptedEvent()->Add(&cListener, &CTestPromptListener::Accepted);
		pPrompt->GetClosedEvent()->Add(&cListener, &CTestPromptListener::Closed);
		AssertFalse(pPrompt->IsOpen());

		//Opened with its caption and its text, the field with the keys; it has a size.
		sPosition.Init(40, 40);
		pPrompt->Open(sPosition, pOwner, "Line[:column]:", "12:5");
		AssertTrue(pPrompt->IsOpen());
		AssertSize(1, pTestWindow->NumPopups());
		AssertString("Line[:column]:", pPrompt->GetCaption());
		AssertString("12:5", pPrompt->GetText());
		AssertString("", pPrompt->GetProblem());
		AssertTrue(pTestWindow->GetFocus()->GetFocussedComponent() == pPrompt->GetField());
		pTestWindow->Layout();
		AssertTrue(pPrompt->GetPanel()->GetActualSize().x > 150);
		AssertTrue(pPrompt->GetPanel()->GetActualSize().y > pPrompt->GetField()->GetActualSize().y);

		//Typing replaces the text, which was selected.  Enter gives it to the listener, who says what is wrong: it stays open.
		pTestWindow->InputPrintable('x', true);
		AssertString("x", pPrompt->GetText());
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertInt(1, cListener.miAccepted);
		AssertString("x", cListener.mszText.Text());
		AssertTrue(pPrompt->IsOpen());
		AssertString("Type a number.", pPrompt->GetProblem());

		//Typing again takes what was wrong away; Enter on what is right closes it and gives the keys back.
		pTestWindow->InputCommand(CIC_Backspace, true);
		AssertString("", pPrompt->GetProblem());
		pTestWindow->InputPrintable('4', true);
		pTestWindow->InputPrintable('2', true);
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertInt(2, cListener.miAccepted);
		AssertString("42", cListener.mszText.Text());
		AssertFalse(pPrompt->IsOpen());
		AssertInt(1, cListener.miClosed);
		AssertSize(0, pTestWindow->NumPopups());
		AssertTrue(pTestWindow->GetFocus()->GetFocussedComponent() == pOwner);

		//Escape closes it and accepts nothing.
		pPrompt->Open(sPosition, pOwner, "Line[:column]:", "7");
		AssertString("7", pPrompt->GetText());
		pTestWindow->InputCommand(CIC_ClearSelection, true);
		AssertFalse(pPrompt->IsOpen());
		AssertInt(2, cListener.miAccepted);
		AssertInt(2, cListener.miClosed);
		AssertTrue(pTestWindow->GetFocus()->GetFocussedComponent() == pOwner);

		pPrompt->GetAcceptedEvent()->Remove(&cListener);
		pPrompt->GetClosedEvent()->Remove(&cListener);
		cListener.Kill();
		pPrompt = NULL;
		pOwner = NULL;
		pFill = NULL;
		pTheme = NULL;
		pTick = NULL;
		pTestWindow = NULL;
	}

	AssertTrue(ORoot()->RemoveAll());
	pNativeFactory = NULL;
	ObjectsFlush();
	ObjectsKill();
}


class CTestInfoListener
{
public:
	int		miPressed;
	size	muiPosition;

	void Init(void) { miPressed = 0; muiPosition = MAX_SIZE; }
	void Pressed(CInfoPopup* pcPopup, size uiPosition) { miPressed++; muiPosition = uiPosition; }
};


//////////////////////////////////////////////////////////////////////////
//
// An info popup shows a few lines of text at a point of the window, under
// it or over it, and is kept inside the window; the pointer passes
// through it and it takes no keys.  Setting its text while it is shown
// sizes it again.  Hidden, a popup opened over it stays open; a press
// anywhere closes it as it does any popup.  With a maximum height taller
// text scrolls, by lines and by pages; taking the pointer, the wheel
// scrolls it and a press on it calls its press event and leaves it open,
// the keys taken from what had them.
//
//////////////////////////////////////////////////////////////////////////
void TestInfoPopup(void)
{
	Ptr<CWinRefWindowFactory>	pNativeFactory;

	ObjectsInit();
	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		SDialogLook					sLook;
		Ptr<CWindowTheme>			pTheme;
		Ptr<CFillContainer>			pFill;
		Ptr<CTextField>				pOwner;
		Ptr<CInfoPopup>				pInfo;
		Ptr<CPromptPopup>			pPrompt;
		SInt32Vec2					sPoint;
		SInt32Vec2					sSize;
		SInt32Vec2					sWide;
		CTestInfoListener			cListener;
		Ptr<CScrollPane>			pPane;
		Ptr<CTextArea>				pText;
		CInt32Rect					cLine;
		CChars						szLines;
		int32						iLine;
		int32						iMaximum;
		int							i;

		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, 800, 600, (char*)NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Info Owner", &pNativeFactory, pTick, OMalloc<CClearCanvasDraw>(JEWEL_EDITOR), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pTheme = CJewelTheme::CreateDarkTheme(pTestWindow, GenerateMenuFont(pTestWindow));
		sLook.pcTheme = &pTheme;
		sLook.pcDimFont = &pTheme->GetFont();
		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);
		pOwner = OMalloc<CTextField>(pTestWindow, pTheme, "owner");
		pFill->AddComponent(pOwner);
		pTestWindow->Layout();
		pTestWindow->GetFocus()->SetFocus(pOwner);

		pInfo = OMalloc<CInfoPopup>(pTestWindow, pTheme, CreateMonoTextAreaFamily(14.0f));
		ORoot()->Add(pInfo);
		AssertFalse(pInfo->IsShown());
		AssertFalse(pInfo->Hide());

		//Shown under a point: its top is a little under it, its left on it, and the keys stay where they were.
		pInfo->SetText("int Area(void)");
		AssertString("int Area(void)", pInfo->GetText());
		sPoint.Init(100, 200);
		pInfo->Show(sPoint, false);
		AssertTrue(pInfo->IsShown());
		AssertSize(1, pTestWindow->NumPopups());
		pTestWindow->Layout();
		AssertInt(100, pInfo->GetPopup()->GetPosition().x);
		AssertInt(200 + INFO_POPUP_GAP, pInfo->GetPopup()->GetPosition().y);
		sSize = pInfo->GetPopup()->GetActualSize();
		AssertTrue(sSize.x > 80);
		AssertTrue(sSize.y > 14);
		AssertTrue(pTestWindow->GetFocus()->GetFocussedComponent() == pOwner);

		//The pointer passes through it to what is under it.
		AssertTrue(pInfo->GetPopup()->FindComponentAt(104, 206).IsNull());

		//Shown over a point: its bottom is a little over it.  Shown again it moves, still one popup.
		pInfo->Show(sPoint, true);
		AssertSize(1, pTestWindow->NumPopups());
		pTestWindow->Layout();
		AssertInt(200 - INFO_POPUP_GAP - sSize.y, pInfo->GetPopup()->GetPosition().y);
		AssertTrue(pInfo->IsAbove());

		//With no room over the point it goes under it; past the right edge it is moved back in.
		sPoint.Init(790, 4);
		pInfo->Show(sPoint, true);
		pTestWindow->Layout();
		AssertInt(4 + INFO_POPUP_GAP, pInfo->GetPopup()->GetPosition().y);
		AssertInt(800 - sSize.x, pInfo->GetPopup()->GetPosition().x);

		//More text while it is shown: it is wider and taller, and a span is taken.
		sPoint.Init(100, 200);
		pInfo->Show(sPoint, false);
		pInfo->SetText("int Area(void)\nvirtual void Draw(int iX, int iY)");
		AssertTrue(pInfo->AddSpan(0, 3, Set32BitColour((uint8)0x30, (uint8)0x90, (uint8)0xF0)));
		pTestWindow->Layout();
		sWide = pInfo->GetPopup()->GetActualSize();
		AssertTrue(sWide.x > sSize.x);
		AssertTrue(sWide.y > sSize.y);
		AssertSize(1, pInfo->GetTextArea()->NumSpans());
		pInfo->SetText("x");
		AssertSize(0, pInfo->GetTextArea()->NumSpans());

		//A popup opened over it stays when it is hidden.
		pPrompt = OMalloc<CPromptPopup>(pTestWindow, &sLook);
		ORoot()->Add(pPrompt);
		sPoint.Init(300, 300);
		pPrompt->Open(sPoint, pOwner, "Line:", "1");
		AssertSize(2, pTestWindow->NumPopups());
		AssertTrue(pInfo->Hide());
		AssertFalse(pInfo->IsShown());
		AssertSize(1, pTestWindow->NumPopups());
		AssertTrue(pPrompt->IsOpen());
		pPrompt->Close();
		AssertSize(0, pTestWindow->NumPopups());

		//Every popup closing closes it too, and it can be shown again.
		sPoint.Init(100, 200);
		pInfo->Show(sPoint, false);
		AssertTrue(pInfo->IsShown());
		pTestWindow->CloseAllPopups();
		AssertFalse(pInfo->IsShown());
		pInfo->Show(sPoint, false);
		AssertTrue(pInfo->IsShown());
		AssertSize(1, pTestWindow->NumPopups());
		AssertTrue(pInfo->Hide());
		AssertSize(0, pTestWindow->NumPopups());

		//Text that fits doesn't scroll, with a maximum height or without.
		AssertInt(-1, pInfo->GetMaximumHeight());
		pInfo->SetText("one\ntwo");
		pInfo->Show(sPoint, false);
		AssertFalse(pInfo->IsScrolling());
		AssertInt(0, pInfo->GetScrollPosition());
		AssertFalse(pInfo->Scroll(1));
		AssertFalse(pInfo->ScrollPages(1));
		AssertTrue(pInfo->Hide());
		pText = pInfo->GetTextArea();
		pInfo->SetMaximumHeight(100);
		AssertInt(100, pInfo->GetMaximumHeight());
		pInfo->Show(sPoint, false);
		pTestWindow->Layout();
		AssertFalse(pInfo->IsScrolling());
		AssertTrue(pInfo->GetPopup()->GetActualSize().y < 100);
		AssertTrue(pInfo->Hide());

		//Forty lines in a hundred pixels: the popup is as tall as its maximum and its insets, and the text scrolls a
		//line at a time, a page at a time, and no further than its ends.
		szLines.Init();
		for (i = 0; i < 40; i++)
		{
			szLines.Append("line ");
			szLines.Append(i);
			szLines.Append('\n');
		}
		pInfo->SetText(szLines.Text());
		szLines.Kill();
		pInfo->Show(sPoint, false);
		pTestWindow->Layout();
		AssertTrue(pInfo->IsScrolling());
		sSize = pInfo->GetPopup()->GetActualSize();
		AssertTrue(sSize.y >= 100);
		AssertTrue(sSize.y < 120);
		pPane = pInfo->GetPopup()->GetScrollPane();
		AssertTrue(pPane.IsNotNull());
		AssertTrue(pText->GetLineBounds(0, &cLine));
		iLine = cLine.GetHeight();
		iMaximum = pPane->GetMaximumScrollPosition().y;
		AssertTrue(iMaximum > 20 * iLine);
		AssertInt(0, pPane->GetScrollPosition().y);
		AssertInt(0, pInfo->GetScrollPosition());
		AssertTrue(pInfo->Scroll(2));
		AssertInt(2 * iLine, pPane->GetScrollPosition().y);
		AssertInt(2 * iLine, pInfo->GetScrollPosition());
		AssertTrue(pInfo->Scroll(-1));
		AssertInt(iLine, pPane->GetScrollPosition().y);
		AssertTrue(pInfo->Scroll(-5));
		AssertInt(0, pPane->GetScrollPosition().y);
		AssertTrue(pInfo->ScrollPages(1));
		AssertTrue(pPane->GetScrollPosition().y > 2 * iLine);
		AssertTrue(pPane->GetScrollPosition().y <= 100);
		AssertTrue(pInfo->ScrollPages(100));
		AssertInt(iMaximum, pPane->GetScrollPosition().y);
		AssertTrue(pInfo->ScrollPages(-100));
		AssertInt(0, pPane->GetScrollPosition().y);

		//The pointer still passes through it, so the wheel over it isn't its own and a press calls nothing.
		cListener.Init();
		pInfo->GetPressEvent()->Add(&cListener, &CTestInfoListener::Pressed);
		AssertFalse(pInfo->IsTakesPointer());
		AssertTrue(pInfo->GetPopup()->FindComponentAt(110, 230).IsNull());

		//Taking the pointer, the wheel over it scrolls it, and a press on a character of its text calls the press
		//event with where the character is, and it stays open.  Nothing has the keys after: it can't, and what had
		//them was pressed away from.
		pInfo->SetTakesPointer(true);
		AssertTrue(pInfo->IsTakesPointer());
		AssertTrue(pInfo->GetPopup()->FindComponentAt(110, 230).IsNotNull());
		pTestWindow->PointerMoved(110, 230);
		pTestWindow->InputCommand(CIC_WheelDown, true);
		pTestWindow->Layout();
		AssertTrue(pPane->GetScrollPosition().y > 0);
		AssertTrue(pInfo->ScrollPages(-100));
		pTestWindow->Layout();
		pTestWindow->PointerMoved(pText->GetPosition().x + (pText->GetPositionX(1, false) + pText->GetPositionX(2, false)) / 2, pText->GetPosition().y + iLine + iLine / 2);
		Click(pTestWindow);
		AssertInt(1, cListener.miPressed);
		AssertSize(8, cListener.muiPosition);  //The 'i' of "line 1": "line 0" and its line feed are seven.
		AssertTrue(pInfo->IsShown());
		AssertTrue(pTestWindow->GetFocus()->GetFocussedComponent().IsNull());
		pTestWindow->GetFocus()->SetFocus(pOwner);

		//After the end of its first line, which is shorter than its last, a press says that no character was pressed.
		i = pText->GetPositionX(1, false) - pText->GetPositionX(0, false);
		pTestWindow->PointerMoved(pText->GetPosition().x + 6 * i + i / 2, pText->GetPosition().y + iLine / 2);
		Click(pTestWindow);
		AssertInt(2, cListener.miPressed);
		AssertSize(MAX_SIZE, cListener.muiPosition);
		AssertTrue(pInfo->IsShown());
		pTestWindow->GetFocus()->SetFocus(pOwner);

		//Letting the pointer pass again, a press where it is is not its own: it closes it, as a press anywhere does.
		pInfo->SetTakesPointer(false);
		pTestWindow->PointerMoved(pText->GetPosition().x + i / 2, pText->GetPosition().y + iLine / 2);
		Click(pTestWindow);
		AssertInt(2, cListener.miPressed);
		AssertFalse(pInfo->IsShown());
		pInfo->Show(sPoint, false);
		pTestWindow->Layout();

		//Without a maximum again it is as tall as its text, and scrolls nothing.
		pInfo->GetPressEvent()->Remove(&cListener);
		AssertTrue(pInfo->Hide());
		pInfo->SetMaximumHeight(-1);
		pInfo->SetText("one\ntwo\nthree\nfour\nfive\nsix\nseven\neight\nnine\nten");
		pInfo->Show(sPoint, false);
		pTestWindow->Layout();
		AssertFalse(pInfo->IsScrolling());
		AssertTrue(pInfo->GetPopup()->GetActualSize().y > 9 * iLine);
		AssertTrue(pInfo->Hide());
		AssertSize(0, pTestWindow->NumPopups());

		pPane = NULL;
		pText = NULL;
		pPrompt = NULL;
		pInfo = NULL;
		pOwner = NULL;
		pFill = NULL;
		pTheme = NULL;
		pTick = NULL;
		pTestWindow = NULL;
	}

	AssertTrue(ORoot()->RemoveAll());
	pNativeFactory = NULL;
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// A driven chooser: the keys stay with what opened it, which filters it,
// moves through it and chooses.  It has no title and no field.  Items
// added while it is open are listed, the selected one still selected,
// and it grows to its rows; a note is a line under them.  When it closes
// the keys are where they were.
//
//////////////////////////////////////////////////////////////////////////
void TestChooserPopupDriven(void)
{
	Ptr<CWinRefWindowFactory>	pNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		SDialogLook					sLook;
		Ptr<CWindowTheme>			pTheme;
		Ptr<CFillContainer>			pFill;
		Ptr<CTextField>				pOwner;
		Ptr<CChooserPopup>			pChooser;
		CTestChooserListener		cListener;
		CChars						szRows;
		SInt32Vec2					sPosition;
		int32						iRowHeight;
		int32						iHeight;

		szRows.Init();
		cListener.Init();
		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, 800, 600, (char*)NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Driven Owner", &pNativeFactory, pTick, OMalloc<CClearCanvasDraw>(JEWEL_EDITOR), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pTheme = CJewelTheme::CreateDarkTheme(pTestWindow, GenerateMenuFont(pTestWindow));
		sLook.pcTheme = &pTheme;
		sLook.pcDimFont = &pTheme->GetFont();
		iRowHeight = pTheme->GetMetric(TM_GoToRowHeight);

		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);
		pOwner = OMalloc<CTextField>(pTestWindow, pTheme, "owner");
		pFill->AddComponent(pOwner);
		pTestWindow->Layout();
		pTestWindow->GetFocus()->SetFocus(pOwner);

		pChooser = OMalloc<CChooserPopup>(pTestWindow, &sLook, pTheme->GetFont());
		ORoot()->Add(pChooser);
		pChooser->GetChosenEvent()->Add(&cListener, &CTestChooserListener::Chosen);
		pChooser->GetClosedEvent()->Add(&cListener, &CTestChooserListener::Closed);
		AssertFalse(pChooser->IsDriven());
		pChooser->SetDriven(true);
		pChooser->SetSortMatches(true);
		AssertTrue(pChooser->IsDriven());
		pChooser->AddItem("GetValue", "int", -1, 10);
		pChooser->AddItem("GetSelectedNode", "CNode*", -1, 11);
		pChooser->AddItem("SetValue", "bool", -1, 12);

		//Opened, the keys are still its owner's; it has no field and is as tall as its three rows.
		sPosition.Init(20, 30);
		pChooser->Open(sPosition, pOwner);
		AssertTrue(pChooser->IsOpen());
		AssertTrue(pTestWindow->GetFocus()->GetFocussedComponent() == pOwner);
		AssertTrue(pChooser->GetField().IsNull());
		AssertString("GetValue,GetSelectedNode,SetValue", PrintChooserRows(pChooser, &szRows));
		AssertInt(0, pChooser->GetSelectedRow());
		pTestWindow->Layout();
		AssertInt(3 * iRowHeight, pChooser->GetPanel()->GetActualSize().y);
		AssertInt(20, pChooser->GetPopup()->GetPosition().x);

		//Its owner moves the selection, round the ends.
		pChooser->Down();
		AssertInt(1, pChooser->GetSelectedRow());
		pChooser->Down();
		pChooser->Down();
		AssertInt(0, pChooser->GetSelectedRow());
		pChooser->Up();
		AssertInt(2, pChooser->GetSelectedRow());
		AssertTrue(pTestWindow->GetFocus()->GetFocussedComponent() == pOwner);

		//Filtered by what its owner gives it, as a field's text would: the names' capitals match, and it is as tall
		//as the rows left.  Nothing matching leaves no rows, and none can be chosen.
		pChooser->SetFilterText("GSN");
		AssertString("GSN", pChooser->GetFilterText());
		AssertString("GetSelectedNode", PrintChooserRows(pChooser, &szRows));
		AssertInt(0, pChooser->GetSelectedRow());
		pTestWindow->Layout();
		AssertInt(iRowHeight, pChooser->GetPanel()->GetActualSize().y);
		pChooser->SetFilterText("Zz");
		AssertString("", PrintChooserRows(pChooser, &szRows));
		AssertFalse(pChooser->ChooseSelected());
		AssertTrue(pChooser->IsOpen());
		pChooser->SetFilterText("Get");
		AssertString("GetValue,GetSelectedNode", PrintChooserRows(pChooser, &szRows));
		pChooser->Down();
		AssertInt(1, pChooser->GetSelectedRow());

		//Items added while it is open: listed if they match, where their match puts them, the selected item still
		//selected, and it grows.
		pChooser->AddItem("GetLimit", "int", -1, 13);
		pChooser->AddItem("Reset", "void", -1, 14);
		pChooser->ItemsChanged();
		AssertString("GetValue,GetLimit,GetSelectedNode", PrintChooserRows(pChooser, &szRows));
		AssertInt(2, pChooser->GetSelectedRow());
		AssertSize(11, pChooser->GetSelectedItem()->uiData);
		pTestWindow->Layout();
		AssertInt(3 * iRowHeight, pChooser->GetPanel()->GetActualSize().y);

		//A note is a line under the rows, and goes when it is emptied.
		AssertString("", pChooser->GetNote());
		pChooser->SetNote("Working...");
		AssertString("Working...", pChooser->GetNote());
		pTestWindow->Layout();
		iHeight = pChooser->GetPanel()->GetActualSize().y;
		AssertTrue(iHeight > 3 * iRowHeight);
		pChooser->SetNote("");
		pTestWindow->Layout();
		AssertInt(3 * iRowHeight, pChooser->GetPanel()->GetActualSize().y);

		//Its owner chooses: the selected item, and it has closed with the keys where they were.
		AssertTrue(pChooser->ChooseSelected());
		AssertFalse(pChooser->IsOpen());
		AssertInt(1, cListener.miChosen);
		AssertInt(1, cListener.miClosed);
		AssertSize(11, pChooser->GetChosen()->uiData);
		AssertTrue(pTestWindow->GetFocus()->GetFocussedComponent() == pOwner);

		//Opened again it is filtered by nothing, and Close chooses nothing.
		pChooser->Open(sPosition, pOwner);
		AssertString("", pChooser->GetFilterText());
		AssertString("GetValue,GetSelectedNode,SetValue,GetLimit,Reset", PrintChooserRows(pChooser, &szRows));
		pChooser->Close();
		AssertFalse(pChooser->IsOpen());
		AssertTrue(pChooser->GetChosen() == NULL);
		AssertInt(1, cListener.miChosen);
		AssertInt(2, cListener.miClosed);

		//It is driven or not from before it is first opened.
		pChooser->SetDriven(false);
		AssertTrue(pChooser->IsDriven());

		pChooser->GetChosenEvent()->Remove(&cListener);
		pChooser->GetClosedEvent()->Remove(&cListener);
		szRows.Kill();
		pChooser = NULL;
		pOwner = NULL;
		pFill = NULL;
		pTheme = NULL;
		pTick = NULL;
		pTestWindow = NULL;
	}

	AssertTrue(ORoot()->RemoveAll());
	pNativeFactory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChooserPopup(void)
{
	BeginTests();
	ObjectsInit();

	TestChooserPopupFilter();
	TestChooserPopupHeadings();
	TestChooserPopupDriven();

	ObjectsFlush();
	ObjectsKill();

	TestPromptPopup();
	TestInfoPopup();
	TestStatistics();
}

