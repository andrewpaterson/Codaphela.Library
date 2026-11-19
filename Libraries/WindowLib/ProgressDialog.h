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
#ifndef __PROGRESS_DIALOG_H__
#define __PROGRESS_DIALOG_H__
#include "Dialog.h"
#include "Label.h"
#include "ProgressBar.h"


//How long a progress dialog waits between looks at how its work is going.
#define PROGRESS_DIALOG_TICK_MS		15


//How high a progress dialog's bar is.
#define PROGRESS_DIALOG_BAR_HEIGHT	4


//How something long is going, in a dialog of its own shown modally over its owner's window, as IntelliJ's: a line
//saying what is being done, a bar that fills as it is, and Cancel.  The work is done somewhere else (other threads,
//say), so the dialog draws and Cancel can be clicked meanwhile.
//
//While it's shown the dialog calls its ticker every PROGRESS_DIALOG_TICK_MS.  The ticker says how the work is going
//(SetText, SetProgress) and calls Finish once it's done, which closes the dialog.  Show is true if the work finished
//and false if Cancel (or escape, or the window's close button) closed the dialog first: stopping the work then is
//the caller's.  Show it from a tick, not from input (see CWindow::ShowModal).
class CProgressDialog : public CObject
{
CONSTRUCTABLE(CProgressDialog);
DESTRUCTABLE(CProgressDialog);
protected:
	Ptr<CDialog>		mpDialog;
	Ptr<CLabel>			mpText;
	Ptr<CProgressBar>	mpBar;
	SMemberCall			msTicker;  //Not set if there is none.

public:
	void				Init(Ptr<CWindow> pOwner, SDialogLook* psLook, const char* szTitle, const char* szText);
	void				Class(void) override;
	void				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	void				SetText(const char* szText);
	void				SetProgress(int64 iDone, int64 iTotal);
	template<class M, class O>
	void				SetTicker(O* pcObject, void(M::*fTick)(void));
	void				ClearTicker(void);

	bool				Show(void);
	void				Finish(void);

	char*				GetText(void);
	float				GetFraction(void);
	Ptr<CDialog>		GetDialog(void) const;

	void				DialogTick(void);
};


//////////////////////////////////////////////////////////////////////////
//
// fTick is called every PROGRESS_DIALOG_TICK_MS while the dialog is
// shown, outside its input.  The object isn't held; it calls ClearTicker
// before it goes.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class O>
void CProgressDialog::SetTicker(O* pcObject, void(M::*fTick)(void))
{
	msTicker.Init(pcObject, fTick);
}


#endif // __PROGRESS_DIALOG_H__

