#include "PointerRemapper.h"
#include "Define.h"


//The commnets and names in this class are weird because it began life as a CSV file row.
template <class M>
class CListTemplateMinimal
{
protected:
	size	miNumElements;
	int		miRowLength;  //The number of characters in the string including all terminating zeros.
	size	miTotalSize;

	size	miUsedElements;
	size	miUsedLength;

	int		maiFieldOffsets[1];	//This is just the first fields positions, the size of the struct will be increased to contain iNumFields char*'s.
							//...
							//The size of the struct will also include all the zero terminated strings lengths.
public:
	size	TotalSize(int iNumElements, int iRowLength);

	void	Init(int iNumFields, int iRowLength);
	void	Kill(void);

	M*		Get(size iIndex);
	M*		Get(size iIndex, size* piSize);
	M*		Add(M* pv, size iSize);
	int		NumElements(void);
	int		AllocatedElements(void);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size CListTemplateMinimal<M>::TotalSize(int iNumElements, int iRowLength)
{
	return sizeof(CListTemplateMinimal<M>) + iNumElements * sizeof(M*) + iRowLength;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CListTemplateMinimal<M>::Init(int iNumElements, int iRowLength)
{
	miNumElements = iNumElements;
	miRowLength = iRowLength;

	miUsedElements = 0;
	miUsedLength = 0;

	miTotalSize = TotalSize(iNumElements, iRowLength);
	maiFieldOffsets[0] = sizeof(CListTemplateMinimal<M>) + miNumElements * sizeof(M*);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CListTemplateMinimal<M>::Kill(void)
{
	miUsedElements = 0;
	miUsedLength = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CListTemplateMinimal<M>::Get(size iIndex)
{
	return (M*)RemapSinglePointer(this, maiFieldOffsets[iIndex]);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CListTemplateMinimal<M>::Get(size iIndex, size* piSize)
{
	if (iIndex < miNumElements - 1)
	{
		*piSize = maiFieldOffsets[iIndex + 1] - maiFieldOffsets[iIndex];
	}
	else
	{
		*piSize = miTotalSize - maiFieldOffsets[iIndex];
	}
	return (M*)RemapSinglePointer(this, maiFieldOffsets[iIndex]);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CListTemplateMinimal<M>::Add(M* pv, size iSize)
{
	size	iStringStart;
	M*		pvDest;

	iStringStart = (int)maiFieldOffsets[0];

	pvDest = (M*)RemapSinglePointer(this, iStringStart + miUsedLength);
	memcpy(pvDest, pv, iSize);
	maiFieldOffsets[miUsedElements] = (int)(iStringStart + miUsedLength);
	miUsedLength += iSize;
	miUsedElements++;

	return pvDest;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CListTemplateMinimal<M>::NumElements(void)
{
	return miUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CListTemplateMinimal<M>::AllocatedElements(void)
{
	return miNumElements;
}

