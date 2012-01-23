/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LanguageLib

Codaphela LanguageLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LanguageLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LanguageLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
struct SChunkHeader
{
	int		iName;
	int		iChunkSize;
	int		iChunkIndexPos;  // -1 if no index.
};
struct SChunkStackElement
{
	SChunkHeader			sHeader;
	int						iChunkHeaderPos;
	int						iContainsChunks;
	CChunkIndex				cChunkIndex;
	int						iCurrChunk;
};
typedef CArrayTemplate<SChunkStackElement> CChunkStack;
class CChunkFile : public CFileExtended
{
private:
	CChunkStack		mcChunkStack;
	BOOL	__PrivateReadChunkBegin(void);
	BOOL	__PrivateReadChunkIndex(int iIndexPos, CChunkIndex* pcIndex);
public:
	int		FindFirstChunkWithName(int iName);
	int		FindNextChunkWithName(void);
	int		GetNumChunksWithName(int iName);
	int		GetNumChunks(void);
	int		GetChunkName(int iChunkNum);
	BOOL	VerifyChunkName(int iChunkNum, int iChunkName);
	BOOL	ReadOpen(char* szFilename);
	void	ReadClose(void);
	BOOL	ReadChunkBegin(int iChunkNum);
	BOOL	ReadIndexBegin(void);
	BOOL	ReadIndexEnd(void);
	BOOL	WriteOpen(char* szFilename);
	void*	WriteClose(void);
	BOOL	WriteChunkBegin(void);
	BOOL	WriteChunkEnd(int iChunkName);
};
struct STemp
{
	int i, j;
};
class SFloat2
{
public:
	float /*HI!*/ x, /*HI!*/ y;
	SFloat2() {};
	SFloat2(const float *);
	SFloat2(float x, float y);
	operator float* ();
	operator const float* () const;
	SFloat2& operator += (const SFloat2&);
	SFloat2& operator -= (const SFloat2&);
	SFloat2& operator *= (float);
	SFloat2& operator /= (float);
	SFloat2 operator + () const;
	SFloat2 operator - () const;
	SFloat2 operator + (const SFloat2&) const;
	SFloat2 operator - (const SFloat2&) const;
	SFloat2 operator * (float) const;
	SFloat2 operator / (float) const;
	friend SFloat2 operator * (float, const SFloat2&);
	BOOL operator == (const SFloat2&) const;
	BOOL operator != (const SFloat2&) const;
	void Init(float x, float y);
	void Init(int x, int y);
	void Zero(void);
	BOOL Save(CChunkFile* pcFile);
	BOOL Load(CChunkFile* pcFile);
};
class SFloat3
{
public:
	float x, y, z;
	SFloat3() {};
	SFloat3(const float *);
	SFloat3(const SFloat3& v);
	SFloat3(float x, float y, float z);
	operator float* ();
	operator const float* () const;
	SFloat3& operator += (const SFloat3&);
	SFloat3& operator -= (const SFloat3&);
	SFloat3& operator *= (float);
	SFloat3& operator /= (float);
	SFloat3 operator + () const;
	SFloat3 operator - () const;
	SFloat3 operator + (const SFloat3&) const;
	SFloat3 operator - (const SFloat3&) const;
	SFloat3 operator * (float) const;
	SFloat3 operator / (float) const;
	friend SFloat3 operator * (float, const SFloat3&);
	BOOL operator == (const SFloat3&) const;
	BOOL operator != (const SFloat3&) const;
	void Init(float x, float y, float z);
	void Zero(void);
	BOOL Save(CChunkFile* pcFile);
	BOOL Load(CChunkFile* pcFile);
	void Fix(void);
	void Dump(void);
	float Magnitude(void);
	float SquareMagnitude(void);
	void Normalize(void);
};
class SFloat4
{
public:
	float x, y, z, w;
	SFloat4() {};
	SFloat4(const float*);
	SFloat4(const SFloat3& v, float f);
	SFloat4(float x, float y, float z, float w);
	operator float* ();
	operator const float* () const;
	SFloat4& operator += (const SFloat4&);
	SFloat4& operator -= (const SFloat4&);
	SFloat4& operator *= (float);
	SFloat4& operator /= (float);
	SFloat4 operator + () const;
	SFloat4 operator - () const;
	SFloat4 operator + (const SFloat4&) const;
	SFloat4 operator - (const SFloat4&) const;
	SFloat4 operator * (float) const;
	SFloat4 operator / (float) const;
	friend SFloat4 operator * (float, const SFloat4&);
	BOOL operator == (const SFloat4&) const;
	BOOL operator != (const SFloat4&) const;	
	BOOL Save(CChunkFile* pcFile);
	BOOL Load(CChunkFile* pcFile);
	void Fix(void);
	void Dump(void);
};
class SFloat4x4
{
public:
	SFloat4	x;
	SFloat4	y;
	SFloat4 z;
	SFloat4 pos;
	void Init(void);
	BOOL Save(CChunkFile* pcFile);
	BOOL Load(CChunkFile* pcFile);
	void Fix(void);
	void Dump(void);
};
class SQuaternion
{
public:
	float x, y, z, w;
	SQuaternion() {}
	SQuaternion(const float *);
	SQuaternion(float x, float y, float z, float w);
	SQuaternion& operator += (const SQuaternion&);
	SQuaternion& operator -= (const SQuaternion&);
	SQuaternion& operator *= (const SQuaternion&);
	SQuaternion& operator *= (float);
	SQuaternion& operator /= (float);
	SQuaternion  operator + () const;
	SQuaternion  operator - () const;
	SQuaternion operator + (const SQuaternion&) const;
	SQuaternion operator - (const SQuaternion&) const;
	SQuaternion operator * (const SQuaternion&) const;
	SQuaternion operator * (float) const;
	SQuaternion operator / (float) const;
	friend SQuaternion operator * (float, const SQuaternion&);
	BOOL operator == (const SQuaternion&) const;
	BOOL operator != (const SQuaternion&) const;
};
float 		Float2Length(const SFloat2 *pV);
float 		Float2LengthSq (const SFloat2 *pV);
float 		Float2Dot(const SFloat2 *pV1, const SFloat2 *pV2);
float 		Float2CCW(const SFloat2 *pV1, const SFloat2 *pV2);
SFloat2* 	Float2Add(SFloat2 *pOut, const SFloat2 *pV1, const SFloat2 *pV2);
SFloat2* 	Float2Subtract(SFloat2 *pOut, const SFloat2 *pV1, const SFloat2 *pV2);
SFloat2* 	Float2Minimize(SFloat2 *pOut, const SFloat2 *pV1, const SFloat2 *pV2);
SFloat2* 	Float2Maximize(SFloat2 *pOut, const SFloat2 *pV1, const SFloat2 *pV2);
SFloat2* 	Float2Scale(SFloat2 *pOut, const SFloat2 *pV, float s);
SFloat2* 	Float2Lerp(SFloat2 *pOut, const SFloat2 *pV1, const SFloat2 *pV2, float s);
float 		Float3Length(const SFloat3 *pV);
float 		Float3LengthSq(const SFloat3 *pV);
float 		Float3Dot(const SFloat3 *pV1, const SFloat3 *pV2);
SFloat3* 	Float3Cross(SFloat3 *pOut, const SFloat3 *pV1, const SFloat3 *pV2);
SFloat3* 	Float3Add(SFloat3 *pOut, const SFloat3 *pV1, const SFloat3 *pV2);
SFloat3* 	Float3Subtract(SFloat3 *pOut, const SFloat3 *pV1, const SFloat3 *pV2);
SFloat3* 	Float3Minimize(SFloat3 *pOut, const SFloat3 *pV1, const SFloat3 *pV2);
SFloat3* 	Float3Maximize(SFloat3 *pOut, const SFloat3 *pV1, const SFloat3 *pV2);
SFloat3* 	Float3Scale(SFloat3 *pOut, const SFloat3 *pV, float s);
SFloat3* 	Float3Lerp(SFloat3 *pOut, const SFloat3 *pV1, const SFloat3 *pV2, float s);
float 		Float4Length(const SFloat4 *pV);
float 		Float4LengthSq(const SFloat4 *pV);
float 		Float4Dot(const SFloat4 *pV1, const SFloat4 *pV2);
SFloat4* 	Float4Add(SFloat4 *pOut, const SFloat4 *pV1, const SFloat4 *pV2);
SFloat4* 	Float4Subtract(SFloat4 *pOut, const SFloat4 *pV1, const SFloat4 *pV2);
SFloat4* 	Float4Minimize(SFloat4 *pOut, const SFloat4 *pV1, const SFloat4 *pV2);
SFloat4* 	Float4Maximize(SFloat4 *pOut, const SFloat4 *pV1, const SFloat4 *pV2);
SFloat4* 	Float4Scale(SFloat4 *pOut, const SFloat4 *pV, float s);
SFloat4* 	Float4Lerp(SFloat4 *pOut, const SFloat4 *pV1, const SFloat4 *pV2, float s);
void 		Float2Assign(SFloat2* psVecDest, float x, float y);
void 		Float2Assign(SFloat2* psVecDest, SFloat2* psVecSource);
void 		Float3Assign(SFloat3* psVecDest, float x, float y, float z);
void 		Float3Assign(SFloat3* psVecDest, SFloat3* psVecSource);
void 		Float3Zero(SFloat3* psVecDest);
void 		Float3InterpolatePosition(SFloat3* psVecDest, const SFloat3* psVec1, const SFloat3* psVec2, float fWeight);
void 		Float3InterpolateNormal(SFloat3* psVecDest, const SFloat3* psVec1, const SFloat3* psVec2, float fWeight);
void 		Float3RotateAboutNormalisedAxis(SFloat3* psVecDest, SFloat3 p, float fTheta, SFloat3 r);
void 		Float3Normalize(SFloat3 *pOut, const SFloat3 *pV);
void		QuaternionMultiply(SQuaternion* pOut, const SQuaternion* pQ1, const SQuaternion* pQ2);
