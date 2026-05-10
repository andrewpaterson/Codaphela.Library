#include "Numbers.h"
#include "IntVec4.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SIntVec4::SIntVec4(int x, int y, int z, int w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SIntVec4::Print(CChars* psx, int iWholeNumbers)
{
	CChars		szzz;
	int			iWidth;

	if (iWholeNumbers == -1)
	{
		iWholeNumbers = WholeNumbers();
		iWholeNumbers++;
		if (iWholeNumbers < 2)
		{
			iWholeNumbers = 2;
		}
	}

	iWidth = iWholeNumbers + 1;

	psx->Append('[');
	szzz.Init();
	szzz.Append(x);
	szzz.RightAlign(' ', iWidth);
	psx->Append(szzz);
	szzz.Kill();
	psx->Append(',');
	szzz.Init();
	szzz.Append(y);
	szzz.RightAlign(' ', iWidth);
	psx->Append(szzz);
	szzz.Kill();
	psx->Append(',');
	szzz.Init();
	szzz.Append(z);
	szzz.RightAlign(' ', iWidth);
	psx->Append(szzz);
	szzz.Kill();
	psx->Append(',');
	szzz.Init();
	szzz.Append(w);
	szzz.RightAlign(' ', iWidth);
	psx->Append(szzz);
	szzz.Kill();
	psx->Append(']');
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int SIntVec4::WholeNumbers(void)
{
	int i[4];

	i[0] = CountDigits(x);
	i[1] = CountDigits(y);
	i[2] = CountDigits(z);
	i[3] = CountDigits(w);

	return LargestInt(i, 4);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int IntVec4Dot(const SIntVec4* pV1, const SIntVec4* pV2)
{
	return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z + pV1->w * pV2->w;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIntVec4* IntVec4Add(SIntVec4* pOut, const SIntVec4* pV1, const SIntVec4* pV2)
{
	pOut->x = pV1->x + pV2->x;
	pOut->y = pV1->y + pV2->y;
	pOut->z = pV1->z + pV2->z;
	pOut->w = pV1->w + pV2->w;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIntVec4* IntVec4Subtract(SIntVec4* pOut, const SIntVec4* pV1, const SIntVec4* pV2)
{
	pOut->x = pV1->x - pV2->x;
	pOut->y = pV1->y - pV2->y;
	pOut->z = pV1->z - pV2->z;
	pOut->w = pV1->w - pV2->w;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIntVec4* IntVec4Minimize(SIntVec4* pOut, const SIntVec4* pV1, const SIntVec4* pV2)
{
	pOut->x = pV1->x < pV2->x ? pV1->x : pV2->x;
	pOut->y = pV1->y < pV2->y ? pV1->y : pV2->y;
	pOut->z = pV1->z < pV2->z ? pV1->z : pV2->z;
	pOut->w = pV1->w < pV2->w ? pV1->w : pV2->w;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIntVec4* IntVec4Maximize(SIntVec4* pOut, const SIntVec4* pV1, const SIntVec4* pV2)
{
	pOut->x = pV1->x > pV2->x ? pV1->x : pV2->x;
	pOut->y = pV1->y > pV2->y ? pV1->y : pV2->y;
	pOut->z = pV1->z > pV2->z ? pV1->z : pV2->z;
	pOut->w = pV1->w > pV2->w ? pV1->w : pV2->w;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIntVec4* IntVec4Scale(SIntVec4 *pOut, const SIntVec4 *pV, float32 s)
{
	pOut->x = (int)(pV->x * s);
	pOut->y = (int)(pV->y * s);
	pOut->z = (int)(pV->z * s);
	pOut->w = (int)(pV->w * s);
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIntVec4* IntVec4Lerp(SIntVec4 *pOut, const SIntVec4 *pV1, const SIntVec4 *pV2, float32 s)
{
	pOut->x = (int)(pV1->x + s * (pV2->x - pV1->x));
	pOut->y = (int)(pV1->y + s * (pV2->y - pV1->y));
	pOut->z = (int)(pV1->z + s * (pV2->z - pV1->z));
	pOut->w = (int)(pV1->w + s * (pV2->w - pV1->w));
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void IntVec4Swap(SIntVec4* ps1, SIntVec4* ps2)
{
	SIntVec4 temp;

	temp = *ps2;
	*ps2 = *ps1;
	*ps1 = temp;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void IntVec4MinMax(SIntVec4* psMin, SIntVec4* psMax, SIntVec4* asIn, int iInStride, int iNumPoints)
{
	int			i;
	SIntVec4*	psIn;

	if (iNumPoints > 0)
	{
		psIn = asIn;
		*psMin = *psIn;
		*psMax = *psIn;

		for (i = 1; i < iNumPoints; i++)
		{
			psIn = (SIntVec4*)RemapSinglePointer(asIn, i * iInStride);

			if (psIn->x < psMin->x)
			{
				psMin->x = psIn->x;
			}
			if (psIn->y < psMin->y)
			{
				psMin->y = psIn->y;
			}
			if (psIn->z < psMin->z)
			{
				psMin->z = psIn->z;
			}
			if (psIn->w < psMin->w)
			{
				psMin->w = psIn->w;
			}

			if (psIn->x > psMax->x)
			{
				psMax->x = psIn->x;
			}
			if (psIn->y > psMax->y)
			{
				psMax->y = psIn->y;
			}
			if (psIn->z > psMax->z)
			{
				psMax->z = psIn->z;
			}
			if (psIn->w > psMax->w)
			{
				psMax->w = psIn->w;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void IntVec4InterpolatePosition(SIntVec4* psVecDest, const SIntVec4* psVec1, const SIntVec4* psVec2, float32 fWeight)
{
	SIntVec4	sVec1;
	SIntVec4	sVec2;

	IntVec4Scale(&sVec1, psVec1, fWeight);
	IntVec4Scale(&sVec2, psVec2, 1.0f - fWeight);
	IntVec4Add(psVecDest, &sVec1, &sVec2);
}

