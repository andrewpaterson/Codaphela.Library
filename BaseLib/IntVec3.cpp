#include "Numbers.h"
#include "IntegerHelper.h"
#include "IntVec3.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SIntVec3::Print(CChars* psx, int iWholeNumbers)
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
	psx->Append(']');
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int SIntVec3::WholeNumbers(void)
{
	int i[3];

	i[0] = CountDigits(x);
	i[1] = CountDigits(y);
	i[2] = CountDigits(z);

	return LargestInt(i, 3);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int Int32Vec3Dot(const SIntVec3* pV1, const SIntVec3* pV2)
{
	return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIntVec3* Int32Vec3Cross(SIntVec3* pOut, const SIntVec3* pV1, const SIntVec3* pV2)
{
	SIntVec3 v;

	v.x = pV1->y * pV2->z - pV1->z * pV2->y;
	v.y = pV1->z * pV2->x - pV1->x * pV2->z;
	v.z = pV1->x * pV2->y - pV1->y * pV2->x;

	*pOut = v;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIntVec3* Int32Vec3Add(SIntVec3* pOut, const SIntVec3* pV1, const SIntVec3* pV2)
{
	pOut->x = pV1->x + pV2->x;
	pOut->y = pV1->y + pV2->y;
	pOut->z = pV1->z + pV2->z;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIntVec3* Int32Vec3Subtract(SIntVec3* pOut, const SIntVec3* pV1, const SIntVec3* pV2)
{
	pOut->x = pV1->x - pV2->x;
	pOut->y = pV1->y - pV2->y;
	pOut->z = pV1->z - pV2->z;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIntVec3* Int32Vec3Minimize(SIntVec3* pOut, const SIntVec3* pV1, const SIntVec3* pV2)
{
	pOut->x = pV1->x < pV2->x ? pV1->x : pV2->x;
	pOut->y = pV1->y < pV2->y ? pV1->y : pV2->y;
	pOut->z = pV1->z < pV2->z ? pV1->z : pV2->z;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIntVec3* Int32Vec3Maximize(SIntVec3* pOut, const SIntVec3* pV1, const SIntVec3* pV2)
{
	pOut->x = pV1->x > pV2->x ? pV1->x : pV2->x;
	pOut->y = pV1->y > pV2->y ? pV1->y : pV2->y;
	pOut->z = pV1->z > pV2->z ? pV1->z : pV2->z;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIntVec3* Int32Vec3Scale(SIntVec3 *pOut, const SIntVec3 *pV, float32 s)
{
	pOut->x = (int)(pV->x * s);
	pOut->y = (int)(pV->y * s);
	pOut->z = (int)(pV->z * s);
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIntVec3* Int32Vec3Lerp(SIntVec3 *pOut, const SIntVec3 *pV1, const SIntVec3 *pV2, float32 s)
{
	pOut->x = (int)(pV1->x + s * (pV2->x - pV1->x));
	pOut->y = (int)(pV1->y + s * (pV2->y - pV1->y));
	pOut->z = (int)(pV1->z + s * (pV2->z - pV1->z));
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Int32Vec3Swap(SIntVec3* ps1, SIntVec3* ps2)
{
	SIntVec3 temp;

	temp = *ps2;
	*ps2 = *ps1;
	*ps1 = temp;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Int32Vec3MinMax(SIntVec3* psMin, SIntVec3* psMax, SIntVec3* asIn, int iInStride, int iNumPoints)
{
	int			i;
	SIntVec3*	psIn;

	if (iNumPoints > 0)
	{
		psIn = asIn;
		*psMin = *psIn;
		*psMax = *psIn;

		for (i = 1; i < iNumPoints; i++)
		{
			psIn = (SIntVec3*)RemapSinglePointer(asIn, i * iInStride);

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
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Int32Vec3InterpolatePosition(SIntVec3* psVecDest, const SIntVec3* psVec1, const SIntVec3* psVec2, float32 fWeight)
{
	SIntVec3	sVec1;
	SIntVec3	sVec2;

	Int32Vec3Scale(&sVec1, psVec1, fWeight);
	Int32Vec3Scale(&sVec2, psVec2, 1.0f - fWeight);
	Int32Vec3Add(psVecDest, &sVec1, &sVec2);
}

