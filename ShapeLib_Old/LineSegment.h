#ifndef __LINE_SEGMENT_H__
#define __LINE_SEGMENT_H__
#include <d3dx9.h>
#include "CoreLib\ChunkFile.h"


class CLineSegmentImm
{
public:
	D3DVECTOR	msStart;  //starting point of line
	D3DVECTOR	msDirection;  //unit direction from start to msEnd
	float		mfLength;		
	D3DVECTOR	msEnd;	//msEnd of line

	void Set(D3DVECTOR* sStart, D3DVECTOR* sEnd);
	void Set(float x1, float y1, float z1, float x2, float y2, float z2);
	void Set(void);
	void Copy(CLineSegmentImm* source);

	void Save(CChunkFile* pcFile);
	void Load(CChunkFile* pcFile);
};


#endif __LINE_SEGMENT_H__

