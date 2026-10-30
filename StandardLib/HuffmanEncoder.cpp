/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <string.h>
#include "HuffmanEncoder.h"


#define HUFFMAN_MAX_NODES	(HUFFMAN_MAX_SYMBOLS * 2)


//////////////////////////////////////////////////////////////////////////
//
// Builds code lengths for the symbols with a non zero frequency.  If any
// code would be longer than iMaxBits the frequencies are flattened and
// the lengths built again.  At least two symbols always get a code, as
// some decoders reject a code with only one.
//
//////////////////////////////////////////////////////////////////////////
void CHuffmanEncoder::Init(uint32* puiFrequencies, int iNumSymbols, int iMaxBits)
{
	uint32	auiFrequencies[HUFFMAN_MAX_SYMBOLS];
	int		i;

	miNumSymbols = iNumSymbols;
	memcpy(auiFrequencies, puiFrequencies, (lsize)iNumSymbols * sizeof(uint32));
	while (!BuildLengths(auiFrequencies, iMaxBits))
	{
		for (i = 0; i < iNumSymbols; i++)
		{
			if (auiFrequencies[i] != 0)
			{
				auiFrequencies[i] = (auiFrequencies[i] >> 1) | 1;
			}
		}
	}
	BuildCodes();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHuffmanEncoder::Init(uint8* puiLengths, int iNumSymbols)
{
	miNumSymbols = iNumSymbols;
	memcpy(mauiLengths, puiLengths, iNumSymbols);
	BuildCodes();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHuffmanEncoder::Kill(void)
{
	miNumSymbols = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Builds a Huffman tree by repeatedly joining the two lightest nodes.
// Ties go to the lowest node so the result never varies.  Returns false
// if the deepest leaf is deeper than iMaxBits.
//
//////////////////////////////////////////////////////////////////////////
bool CHuffmanEncoder::BuildLengths(uint32* puiFrequencies, int iMaxBits)
{
	uint32	auiWeights[HUFFMAN_MAX_NODES];
	int16	aiParents[HUFFMAN_MAX_NODES];
	int16	aiSymbols[HUFFMAN_MAX_SYMBOLS];
	int16	aiActive[HUFFMAN_MAX_SYMBOLS];
	int		iNumLeaves;
	int		iNumNodes;
	int		iNumActive;
	int		iFirst;
	int		iSecond;
	int		i;
	int		iNode;
	int		iDepth;
	int		iSymbol;

	memset(mauiLengths, 0, miNumSymbols);
	iNumLeaves = 0;
	for (iSymbol = 0; iSymbol < miNumSymbols; iSymbol++)
	{
		if (puiFrequencies[iSymbol] != 0)
		{
			aiSymbols[iNumLeaves] = (int16)iSymbol;
			auiWeights[iNumLeaves] = puiFrequencies[iSymbol];
			aiParents[iNumLeaves] = -1;
			aiActive[iNumLeaves] = (int16)iNumLeaves;
			iNumLeaves++;
		}
	}

	if (iNumLeaves < 2)
	{
		//Give one or two unused symbols a code as well.
		mauiLengths[0] = 1;
		mauiLengths[1] = 1;
		if (iNumLeaves == 1)
		{
			mauiLengths[aiSymbols[0]] = 1;
			if (aiSymbols[0] > 1)
			{
				mauiLengths[1] = 0;
			}
		}
		return true;
	}

	iNumNodes = iNumLeaves;
	iNumActive = iNumLeaves;
	while (iNumActive > 1)
	{
		iFirst = -1;
		iSecond = -1;
		for (i = 0; i < iNumActive; i++)
		{
			iNode = aiActive[i];
			if ((iFirst == -1) || (auiWeights[iNode] < auiWeights[aiActive[iFirst]]))
			{
				iSecond = iFirst;
				iFirst = i;
			}
			else if ((iSecond == -1) || (auiWeights[iNode] < auiWeights[aiActive[iSecond]]))
			{
				iSecond = i;
			}
		}

		auiWeights[iNumNodes] = auiWeights[aiActive[iFirst]] + auiWeights[aiActive[iSecond]];
		aiParents[iNumNodes] = -1;
		aiParents[aiActive[iFirst]] = (int16)iNumNodes;
		aiParents[aiActive[iSecond]] = (int16)iNumNodes;

		//Replace the first with the new node and remove the second.
		aiActive[iFirst] = (int16)iNumNodes;
		aiActive[iSecond] = aiActive[iNumActive - 1];
		iNumActive--;
		iNumNodes++;
	}

	for (i = 0; i < iNumLeaves; i++)
	{
		iDepth = 0;
		for (iNode = i; aiParents[iNode] != -1; iNode = aiParents[iNode])
		{
			iDepth++;
		}
		if (iDepth > iMaxBits)
		{
			return false;
		}
		mauiLengths[aiSymbols[i]] = (uint8)iDepth;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Assigns canonical codes from the lengths (RFC 1951 section 3.2.2).
//
//////////////////////////////////////////////////////////////////////////
void CHuffmanEncoder::BuildCodes(void)
{
	uint16	auiCount[DEFLATE_MAX_BITS + 1];
	uint16	auiNextCode[DEFLATE_MAX_BITS + 1];
	uint16	uiCode;
	uint16	uiReversed;
	int		iLength;
	int		iSymbol;
	int		i;

	memset(auiCount, 0, sizeof(auiCount));
	for (iSymbol = 0; iSymbol < miNumSymbols; iSymbol++)
	{
		auiCount[mauiLengths[iSymbol]]++;
	}
	auiCount[0] = 0;

	uiCode = 0;
	for (iLength = 1; iLength <= DEFLATE_MAX_BITS; iLength++)
	{
		uiCode = (uint16)((uiCode + auiCount[iLength - 1]) << 1);
		auiNextCode[iLength] = uiCode;
	}

	for (iSymbol = 0; iSymbol < miNumSymbols; iSymbol++)
	{
		iLength = mauiLengths[iSymbol];
		if (iLength != 0)
		{
			uiCode = auiNextCode[iLength];
			auiNextCode[iLength]++;

			uiReversed = 0;
			for (i = 0; i < iLength; i++)
			{
				uiReversed = (uint16)((uiReversed << 1) | ((uiCode >> i) & 1));
			}
			mauiCodes[iSymbol] = uiReversed;
		}
		else
		{
			mauiCodes[iSymbol] = 0;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHuffmanEncoder::Write(CBitWriter* pcWriter, int iSymbol)
{
	pcWriter->WriteBits(mauiCodes[iSymbol], mauiLengths[iSymbol]);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CHuffmanEncoder::GetLength(int iSymbol)
{
	return mauiLengths[iSymbol];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8* CHuffmanEncoder::GetLengths(void)
{
	return mauiLengths;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CHuffmanEncoder::GetNumSymbols(void)
{
	return miNumSymbols;
}
