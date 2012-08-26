#include "SerialisedObject.h"
#include "ObjectIO.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CObjectIO::Unnamed(CSerialisedObject* pcSerialised, CChars* pszUnnamed)
{
	OIndex	oi;

	pszUnnamed->Init(OBJECT_UNNAMED_FILE"/");
	oi = pcSerialised->GetIndex();
	pszUnnamed->AppendHexHiLo(&oi, sizeof(OIndex));
	return pszUnnamed->Text();
}

