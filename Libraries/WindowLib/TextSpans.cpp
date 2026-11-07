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
#include "TextSpans.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextSpans::Init(void)
{
	masSpans.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextSpans::Kill(void)
{
	masSpans.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The span is applied after those added before it.
//
//////////////////////////////////////////////////////////////////////////
void CTextSpans::Add(size uiStart, size uiLength, STextStyleOverride* psOverride)
{
	STextAreaSpan*	psSpan;

	psSpan = masSpans.Add();
	psSpan->uiStart = uiStart;
	psSpan->uiLength = uiLength;
	psSpan->sOverride = *psOverride;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextSpans::Clear(void) { masSpans.ReInit(); }
size CTextSpans::NumSpans(void) { return masSpans.NumElements(); }
STextAreaSpan* CTextSpans::Get(size uiIndex) { return masSpans.Get(uiIndex); }


//////////////////////////////////////////////////////////////////////////
//
// Applies every span that covers the byte at uiPosition to psStyle, in
// the order they were added.
//
//////////////////////////////////////////////////////////////////////////
void CTextSpans::ApplyAt(size uiPosition, STextStyle* psStyle)
{
	size			i;
	STextAreaSpan*	psSpan;

	for (i = 0; i < masSpans.NumElements(); i++)
	{
		psSpan = masSpans.Get(i);
		if ((uiPosition >= psSpan->uiStart) && (uiPosition < psSpan->uiStart + psSpan->uiLength))
		{
			psSpan->sOverride.ApplyTo(psStyle);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Applies the spans to the styles of bytes [uiFrom, uiTo), psStyles[0]
// being the style of byte uiFrom.
//
//////////////////////////////////////////////////////////////////////////
void CTextSpans::Apply(STextStyle* psStyles, size uiFrom, size uiTo)
{
	size			i;
	size			uiPosition;
	size			uiEnd;
	STextAreaSpan*	psSpan;

	for (i = 0; i < masSpans.NumElements(); i++)
	{
		psSpan = masSpans.Get(i);
		uiEnd = psSpan->uiStart + psSpan->uiLength;
		for (uiPosition = (psSpan->uiStart > uiFrom) ? psSpan->uiStart : uiFrom; (uiPosition < uiEnd) && (uiPosition < uiTo); uiPosition++)
		{
			psSpan->sOverride.ApplyTo(&psStyles[uiPosition - uiFrom]);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A span changes the font of its text (its face, size or baseline), so
// the text's size can change with it.
//
//////////////////////////////////////////////////////////////////////////
bool CTextSpans::ChangesFont(void)
{
	size	i;

	for (i = 0; i < masSpans.NumElements(); i++)
	{
		if (masSpans.Get(i)->sOverride.ChangesFont())
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A span applied to psStyle underlines, overlines or strikes through.
//
//////////////////////////////////////////////////////////////////////////
bool CTextSpans::HasDecorations(STextStyle* psStyle)
{
	size		i;
	STextStyle	sStyle;

	for (i = 0; i < masSpans.NumElements(); i++)
	{
		sStyle = *psStyle;
		masSpans.Get(i)->sOverride.ApplyTo(&sStyle);
		if (sStyle.GetDecorations() != 0)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextSpans::DocumentChanged(STextDocumentChange* psChange)
{
	if (psChange->eChange == TDC_Inserted)
	{
		TextInserted(psChange->uiStart, psChange->Length());
	}
	else
	{
		TextRemoved(psChange->uiStart, psChange->uiEnd);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Spans after the text move with it; a span it is put in, or at the end
// of, stretches over it.
//
//////////////////////////////////////////////////////////////////////////
void CTextSpans::TextInserted(size uiPosition, size uiLength)
{
	size			i;
	STextAreaSpan*	psSpan;

	for (i = 0; i < masSpans.NumElements(); i++)
	{
		psSpan = masSpans.Get(i);
		if (uiPosition <= psSpan->uiStart)
		{
			psSpan->uiStart += uiLength;
		}
		else if (uiPosition <= psSpan->uiStart + psSpan->uiLength)
		{
			psSpan->uiLength += uiLength;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Spans shrink by what was taken out of them and move back with the
// text after it.  A span left empty goes.
//
//////////////////////////////////////////////////////////////////////////
void CTextSpans::TextRemoved(size uiStart, size uiEnd)
{
	size			i;
	size			uiLength;
	STextAreaSpan*	psSpan;
	size			uiSpanStart;
	size			uiSpanEnd;

	uiLength = uiEnd - uiStart;
	i = masSpans.NumElements();
	while (i > 0)
	{
		i--;
		psSpan = masSpans.Get(i);
		uiSpanStart = psSpan->uiStart;
		uiSpanEnd = psSpan->uiStart + psSpan->uiLength;
		uiSpanStart = (uiSpanStart <= uiStart) ? uiSpanStart : ((uiSpanStart >= uiEnd) ? uiSpanStart - uiLength : uiStart);
		uiSpanEnd = (uiSpanEnd <= uiStart) ? uiSpanEnd : ((uiSpanEnd >= uiEnd) ? uiSpanEnd - uiLength : uiStart);
		if (uiSpanEnd == uiSpanStart)
		{
			masSpans.RemoveAt(i, true);
		}
		else
		{
			psSpan->uiStart = uiSpanStart;
			psSpan->uiLength = uiSpanEnd - uiSpanStart;
		}
	}
}

