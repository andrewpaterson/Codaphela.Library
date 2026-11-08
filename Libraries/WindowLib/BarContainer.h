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
#ifndef __BAR_CONTAINER_H__
#define __BAR_CONTAINER_H__
#include "GridContainer.h"
#include "FillContainer.h"


//Components in a row, or a column, in the order they are appended, as a toolbar's or a status bar's: each its own
//size, centred across the bar.  The space left over goes to a stretch (AddStretch), so what is appended after it goes
//to the far end; without one it is after the last component.  The end gap, if any, stays after the last.  The bar is
//iThickness across and as long as it is laid out.
class CBarContainer : public CGridContainer
{
CONSTRUCTABLE(CBarContainer);
DESTRUCTABLE(CBarContainer);
protected:
	Ptr<CFillContainer>	mpEnd;  //After the last component: the space left over until there is a stretch, then the end gap.
	int32				miNextCell;  //Where the next component goes.
	int32				miEndGap;
	bool				mbVertical;
	bool				mbStretched;

public:
	void				Init(Ptr<CWindow> pWindow, bool bVertical, int32 iThickness, int32 iEndGap);
	void				Class(void) override;
	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	void				Append(Ptr<CComponent> pComponent);
	void				AddGap(int32 iPixels);
	void				AddStretch(void);
	bool				IsVertical(void);
	bool				IsStretched(void);

protected:
	bool				AppendStretch(Ptr<CComponent> pComponent);
	void				AppendLine(int32 iSpace, int32 iLength, ARGB32 sColour);
	void				AddAt(Ptr<CComponent> pComponent, int32 iCell, int iLengthWeight);
	void				PlaceEnd(void);
};


#endif // __BAR_CONTAINER_H__

