#include "Numbers.h"
#include "Int32Vec4.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SInt32Vec4::SInt32Vec4(int32 x, int32 y, int32 z, int32 w)
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
void SInt32Vec4::Print(CChars* psx, int iWholeNumbers)
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
int SInt32Vec4::WholeNumbers(void)
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
int32 Int32Vec4Dot(const SInt32Vec4* pV1, const SInt32Vec4* pV2)
{
	return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z + pV1->w * pV2->w;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec4* Int32Vec4Add(SInt32Vec4* pOut, const SInt32Vec4* pV1, const SInt32Vec4* pV2)
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
SInt32Vec4* Int32Vec4Subtract(SInt32Vec4* pOut, const SInt32Vec4* pV1, const SInt32Vec4* pV2)
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
SInt32Vec4* Int32Vec4Minimize(SInt32Vec4* pOut, const SInt32Vec4* pV1, const SInt32Vec4* pV2)
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
SInt32Vec4* Int32Vec4Maximize(SInt32Vec4* pOut, const SInt32Vec4* pV1, const SInt32Vec4* pV2)
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
SInt32Vec4* Int32Vec4Scale(SInt32Vec4 *pOut, const SInt32Vec4 *pV, float32 s)
{
	pOut->x = (int32)(pV->x * s);
	pOut->y = (int32)(pV->y * s);
	pOut->z = (int32)(pV->z * s);
	pOut->w = (int32)(pV->w * s);
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec4* Int32Vec4Lerp(SInt32Vec4 *pOut, const SInt32Vec4 *pV1, const SInt32Vec4 *pV2, float32 s)
{
	pOut->x = (int32)(pV1->x + s * (pV2->x - pV1->x));
	pOut->y = (int32)(pV1->y + s * (pV2->y - pV1->y));
	pOut->z = (int32)(pV1->z + s * (pV2->z - pV1->z));
	pOut->w = (int32)(pV1->w + s * (pV2->w - pV1->w));
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Int32Vec4Swap(SInt32Vec4* ps1, SInt32Vec4* ps2)
{
	SInt32Vec4 temp;

	temp = *ps2;
	*ps2 = *ps1;
	*ps1 = temp;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Int32Vec4MinMax(SInt32Vec4* psMin, SInt32Vec4* psMax, SInt32Vec4* asIn, int iInStride, int iNumPoints)
{
	int			i;
	SInt32Vec4*	psIn;

	if (iNumPoints > 0)
	{
		psIn = asIn;
		*psMin = *psIn;
		*psMax = *psIn;

		for (i = 1; i < iNumPoints; i++)
		{
			psIn = (SInt32Vec4*)RemapSinglePointer(asIn, i * iInStride);

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
void Int32Vec4InterpolatePosition(SInt32Vec4* psVecDest, const SInt32Vec4* psVec1, const SInt32Vec4* psVec2, float32 fWeight)
{
	SInt32Vec4	sVec1;
	SInt32Vec4	sVec2;

	Int32Vec4Scale(&sVec1, psVec1, fWeight);
	Int32Vec4Scale(&sVec2, psVec2, 1.0f - fWeight);
	Int32Vec4Add(psVecDest, &sVec1, &sVec2);
}

