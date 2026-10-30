/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "LineBucket.h"
#include "LineMinimiser.h"


//////////////////////////////////////////////////////////////////////////
//
// Adds the fewest lines covering pacInput to pacOutput (Logicim's
// LineMinimiser): lines on the same horizontal or vertical that overlap or
// meet end to end become one line.  Lines that only cross aren't joined.
// The output keeps the order of the first line of each merged group.
//
//////////////////////////////////////////////////////////////////////////
void MinimiseLines(CArrayLine* pacInput, CArrayLine* pacOutput)
{
	CArrayLineBucket	acBuckets;
	CLineBucket*		pcBucket;
	CLineBucket*		pcOther;
	size				i;
	size				j;
	bool				bMerged;
	size				uiNumElements;
	size				uiNumBuckets;

	acBuckets.Init();
	uiNumElements = pacInput->NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcBucket = acBuckets.Add();
		pcBucket->Init(pacInput->Get(i));
	}

	//A bucket is checked against those after it again each time it grows.
	i = 0;
	while (i < acBuckets.NumElements())
	{
		bMerged = false;
		pcBucket = acBuckets.Get(i);
		uiNumBuckets = acBuckets.NumElements();
		for (j = i + 1; j < uiNumBuckets; j++)
		{
			pcOther = acBuckets.Get(j);
			if (pcBucket->Overlaps(pcOther))
			{
				pcBucket->Merge(pcOther);
				acBuckets.RemoveAt(j);
				bMerged = true;
				break;
			}
		}
		if (!bMerged)
		{
			i++;
		}
	}

	uiNumElements = acBuckets.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcBucket = acBuckets.Get(i);
		pacOutput->Add(pcBucket->GetLine());
		pcBucket->Kill();
	}
	acBuckets.Kill();
}

