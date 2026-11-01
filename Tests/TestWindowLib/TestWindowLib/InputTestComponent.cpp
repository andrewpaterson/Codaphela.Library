#include "StandardLib/ClassDefines.h"
#include "WindowLib/Window.h"
#include "WindowLib/NativeWindowFactory.h"
#include "InputTestComponent.h"


//The commands the tests give, each logged by name.
static EComponentInputCommand	gaeInputTestCommands[] =
{
	CIC_Up, CIC_Left, CIC_WordLeft, CIC_End, CIC_LineUp, CIC_LineDown, CIC_SelectUp, CIC_SelectAll,
	CIC_Space, CIC_Tab, CIC_BackTab, CIC_Enter, CIC_EndEnter, CIC_Copy, CIC_Paste,
	CIC_ActivateComponent, CIC_ExecuteComponent, CIC_ContextMenu,
	CIC_StartDragScroll, CIC_ContDragScroll, CIC_StopDragScroll, CIC_StartDragSelect, CIC_ContDragSelect, CIC_StopDragSelect,
	CIC_Hover,
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputTestComponent::Init(Ptr<CWindow> pWindow, const char* szName, CChars* pszLog, bool bCanGetFocus)
{
	PreInit();

	CComponent::Init(pWindow);
	mszName.Init(szName);
	mpszLog = pszLog;
	SetCanGetFocus(bCanGetFocus);
	AddInputActions(gaeInputTestCommands, sizeof(gaeInputTestCommands) / sizeof(EComponentInputCommand), this, &CInputTestComponent::CommandAction);
	AddInputAction(IAF_Printable, NULL, this, &CInputTestComponent::PrintableAction);
	AddInputAction(IAF_Wheel, NULL, this, &CInputTestComponent::WheelAction);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputTestComponent::Class(void)
{
	CComponent::Class();

	U_String(mszName);
	U_Pointer(mpszLog);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputTestComponent::Free(void)
{
	mszName.Kill();
	CComponent::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputTestComponent::Log(const char* szCall)
{
	mpszLog->Append(&mszName);
	mpszLog->Append(':');
	mpszLog->Append(szCall);
	mpszLog->Append(';');
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputTestComponent::Log(const char* szCall, int x, int y)
{
	mpszLog->Append(&mszName);
	mpszLog->Append(':');
	mpszLog->Append(szCall);
	mpszLog->Append('(');
	mpszLog->Append((int32)x);
	mpszLog->Append(',');
	mpszLog->Append((int32)y);
	mpszLog->Append(");");
}


//////////////////////////////////////////////////////////////////////////
//
// Paste is logged as Paste() for an empty clipboard and Paste(...) for
// anything on it.
//
//////////////////////////////////////////////////////////////////////////
void CInputTestComponent::CommandAction(SComponentInputEvent* psEvent)
{
	CChars	szClipboard;
	int		x;
	int		y;

	x = psEvent->x;
	y = psEvent->y;
	switch (psEvent->eCommand)
	{
	case CIC_Up:				Log("Up");								break;
	case CIC_Left:				Log("Left");							break;
	case CIC_WordLeft:			Log("WordLeft");						break;
	case CIC_End:				Log("End");								break;
	case CIC_LineUp:			Log("LineUp");							break;
	case CIC_LineDown:			Log("LineDown");						break;
	case CIC_SelectUp:			Log("SelectUp");						break;
	case CIC_SelectAll:			Log("SelectAll");						break;
	case CIC_Space:				Log(psEvent->bInsert ? "Space(Insert)" : "Space(Overwrite)");	break;
	case CIC_Tab:				Log(psEvent->bInsert ? "Tab(Insert)" : "Tab(Overwrite)");		break;
	case CIC_BackTab:			Log("BackTab");							break;
	case CIC_Enter:				Log("Enter");							break;
	case CIC_EndEnter:			Log("EndEnter");						break;
	case CIC_Copy:				Log("Copy");							break;
	case CIC_ActivateComponent:	Log("Activate", x, y);					break;
	case CIC_ExecuteComponent:	Log("Execute", x, y);					break;
	case CIC_ContextMenu:		Log("ContextMenu", x, y);				break;
	case CIC_StartDragScroll:	Log("StartDragScroll", x, y);			break;
	case CIC_ContDragScroll:	Log("ContDragScroll", x, y);			break;
	case CIC_StopDragScroll:	Log("StopDragScroll", x, y);			break;
	case CIC_StartDragSelect:	Log("StartDragSelect", x, y);			break;
	case CIC_ContDragSelect:	Log("ContDragSelect", x, y);			break;
	case CIC_StopDragSelect:	Log("StopDragSelect", x, y);			break;
	case CIC_Hover:				Log("Hover", x, y);						break;
	case CIC_Paste:
		szClipboard.Init();
		mpWindow->GetFactory()->GetClipboardText(&szClipboard);
		Log(szClipboard.Empty() ? "Paste()" : "Paste(...)");
		szClipboard.Kill();
		break;
	default:
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputTestComponent::PrintableAction(SComponentInputEvent* psEvent)
{
	mpszLog->Append(&mszName);
	mpszLog->Append(":Printable(");
	mpszLog->Append(psEvent->cPrintable);
	mpszLog->Append(psEvent->bInsert ? ",Insert);" : ",Overwrite);");
}


//////////////////////////////////////////////////////////////////////////
//
// Logged then passed on, so the parents hear it too.
//
//////////////////////////////////////////////////////////////////////////
void CInputTestComponent::WheelAction(SComponentInputEvent* psEvent)
{
	Log("Wheel", psEvent->iRight, psEvent->iDown);
	psEvent->bPass = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputTestComponent::FocusGained(void) { Log("Gained"); }
void CInputTestComponent::FocusLost(void) { Log("Lost"); }
void CInputTestComponent::MovedInto(void) { Log("Into"); }
void CInputTestComponent::MovedOutof(void) { Log("Outof"); }
