/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/StringHelper.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "ProgressDialog.h"


//////////////////////////////////////////////////////////////////////////
//
// A dialog titled szTitle saying szText over an empty bar, with Cancel
// and no OK: only the work finishing closes it as OK (Finish).
//
//////////////////////////////////////////////////////////////////////////
void CProgressDialog::Init(Ptr<CWindow> pOwner, SDialogLook* psLook, const char* szTitle, const char* szText)
{
	PreInit();

	msTicker.Clear();
	mpDialog = OMalloc<CDialog>(pOwner, psLook, szTitle);
	mpDialog->SetOKCaption(NULL);
	mpDialog->SetTicker(this, &CProgressDialog::DialogTick);

	mpText = OMalloc<CLabel>(mpDialog->GetWindow(), psLook->pcTheme->GetFont(), szText);
	mpText->SetAlignment(CSH_Stretch, CSV_Center);
	mpDialog->AddEditor(NULL, mpText);

	mpBar = OMalloc<CProgressBar>(mpDialog->GetWindow(), psLook->pcTheme->GetColour(TC_Separator), psLook->pcTheme->GetColour(TC_Accent), PROGRESS_DIALOG_BAR_HEIGHT);
	mpBar->SetAlignment(CSH_Stretch, CSV_Center);
	mpDialog->AddEditor(NULL, mpBar);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProgressDialog::Class(void)
{
	M_Pointer(mpDialog);
	M_Pointer(mpText);
	M_Pointer(mpBar);
	U_Unknown(SMemberCall, msTicker);
}


//////////////////////////////////////////////////////////////////////////
//
// The dialog, its text and its bar are freed with the pointers to them;
// the dialog is told not to tick this any more, if it's still here.
//
//////////////////////////////////////////////////////////////////////////
void CProgressDialog::Free(void)
{
	if (mpDialog.IsNotNull())
	{
		mpDialog->ClearTicker();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A progress dialog isn't saved.
//
//////////////////////////////////////////////////////////////////////////
bool CProgressDialog::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CProgressDialog::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// What is being done, on the line above the bar.  A ticker says it every
// tick, so the same again isn't laid out again.
//
//////////////////////////////////////////////////////////////////////////
void CProgressDialog::SetText(const char* szText)
{
	if (StringCompare(mpText->GetText(), szText) != 0)
	{
		mpText->SetText(szText);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The bar is filled iDone parts of iTotal (see CProgressBar).
//
//////////////////////////////////////////////////////////////////////////
void CProgressDialog::SetProgress(int64 iDone, int64 iTotal)
{
	mpBar->SetProgress(iDone, iTotal);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProgressDialog::ClearTicker(void)
{
	msTicker.Clear();
}


//////////////////////////////////////////////////////////////////////////
//
// Each of the dialog's ticks: the ticker looks at the work, and the next
// tick is asked for, as only the next is ever promised.
//
//////////////////////////////////////////////////////////////////////////
void CProgressDialog::DialogTick(void)
{
	if (msTicker.IsSet())
	{
		msTicker.Call(NULL);
	}
	mpDialog->GetWindow()->RequestTick(PROGRESS_DIALOG_TICK_MS);
}


//////////////////////////////////////////////////////////////////////////
//
// Shows the dialog until the work is finished (Finish) or it's cancelled.
// True if the work finished.
//
//////////////////////////////////////////////////////////////////////////
bool CProgressDialog::Show(void)
{
	return mpDialog->Show();
}


//////////////////////////////////////////////////////////////////////////
//
// The work is done: the dialog closes, as OK.  The ticker calls it.
//
//////////////////////////////////////////////////////////////////////////
void CProgressDialog::Finish(void)
{
	mpDialog->Close("OK");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CProgressDialog::GetText(void) { return mpText->GetText(); }
float CProgressDialog::GetFraction(void) { return mpBar->GetFraction(); }
Ptr<CDialog> CProgressDialog::GetDialog(void) const { return mpDialog; }

