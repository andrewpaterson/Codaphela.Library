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
#ifndef __TEXT_SPANS_H__
#define __TEXT_SPANS_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/TextDocument.h"
#include "SupportLib/TextStyle.h"


//A range of the text in a style.  uiStart and uiLength are in bytes of the UTF8 text.
struct STextAreaSpan
{
	size				uiStart;
	size				uiLength;
	STextStyleOverride	sOverride;
};


typedef CArrayTemplate<STextAreaSpan>	CArrayTextAreaSpan;


//The spans of a text area's text (CTextArea::AddSpan), in the order they were added: where spans overlap each property
//takes the value of the last span added that sets it.  They move and stretch with the text as it is edited, so text put
//in takes the style of the character before it, and a span whose text is all taken out goes.
class CTextSpans : public CTextDocumentListener
{
protected:
	CArrayTextAreaSpan	masSpans;

public:
	void			Init(void);
	void			Kill(void);

	void			Add(size uiStart, size uiLength, STextStyleOverride* psOverride);
	void			Clear(void);
	size			NumSpans(void);
	STextAreaSpan*	Get(size uiIndex);

	void			ApplyAt(size uiPosition, STextStyle* psStyle);
	void			Apply(STextStyle* psStyles, size uiFrom, size uiTo);
	bool			ChangesFont(void);
	bool			HasDecorations(STextStyle* psStyle);

	void			DocumentChanged(STextDocumentChange* psChange) override;

protected:
	void			TextInserted(size uiPosition, size uiLength);
	void			TextRemoved(size uiStart, size uiEnd);
};


#endif // __TEXT_SPANS_H__

