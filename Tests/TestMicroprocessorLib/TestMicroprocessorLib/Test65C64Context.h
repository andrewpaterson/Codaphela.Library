#ifndef __TEST_65C64_CONTEXT_H__
#define __TEST_65C64_CONTEXT_H__
#include "BaseLib/PrimitiveTypes.h"
#include "MicroprocessorLib/65C64Meta.h"


#define TEST_65C64_LOW_SIZE		0x40000
#define TEST_65C64_HIGH_BASE	0xFFFFF000
#define TEST_65C64_HIGH_SIZE	0x1000
#define TEST_65C64_MAX_CYCLES	256

//Instruction codes (the low 4 bits of an opcode).
#define C65_CPY		0
#define C65_AND		1
#define C65_OR		2
#define C65_XOR		3
#define C65_TST		4
#define C65_ADD		5
#define C65_SUB		6
#define C65_CMP		7

//Branch flags.  The first flag is one of 0, 1, c, z, n, v, f or g, the second one of c, z, n or v.
#define F65_FALSE	0
#define F65_TRUE	1
#define F65_C		2
#define F65_Z		3
#define F65_N		4
#define F65_V		5
#define F65_F		6
#define F65_G		7

//Branch conditions.
#define B65_CLEAR	0
#define B65_SET		1
#define B65_AND		2
#define B65_XOR		3

//Push and pull register masks.
#define M65_A		0x01
#define M65_B		0x02
#define M65_S		0x04
#define M65_V		0x08
#define M65_W		0x10
#define M65_X		0x20
#define M65_Y		0x40

//Find units.
#define U65_BIT		0
#define U65_CRUMB	1
#define U65_NYBBLE	2
#define U65_BYTE	3

//bist and stab index styles (0 to 5 is the bit index itself).
#define X65_INDEX_IMMEDIATE	6
#define X65_INDEX_REGISTER	7

//Block operation sub instructions and stop styles.
#define K65_COPY			0
#define K65_AND				2
#define K65_READ_COPY		12
#define STOP65_ZERO			0
#define STOP65_NOT_ZERO		1
#define STOP65_NEGATIVE		2
#define STOP65_NOT_NEGATIVE	3


//A bus cycle seen by the memory.
struct STest65C64Cycle
{
	uint32	uiAddress;
	uint16	uiData;
	bool	bRead;
	bool	bLower;
	bool	bUpper;
	bool	bOPV;
	bool	bVPA;
	bool	bVDA;
	bool	bVPB;
};


//Memory is 256KB at 0000:0000 and the 4KB vector page at FFFF:F000.
class CTest65C64Context
{
public:
	uint8			mauiLow[TEST_65C64_LOW_SIZE];
	uint8			mauiHigh[TEST_65C64_HIGH_SIZE];
	uint32			muiNext;
	size			muiBadAccesses;
	STest65C64Cycle	masCycles[TEST_65C64_MAX_CYCLES];
	size			muiNumCycles;
	size			muiTotalCycles;

	void	Init(uint32 uiResetAddress);
	uint8*	Byte(uint32 uiAddress);
	uint8	GetByte(uint32 uiAddress);
	void	SetByte(uint32 uiAddress, uint8 uiValue);
	uint16	GetWord(uint32 uiAddress);
	uint32	GetLong(uint32 uiAddress);
	uint64	GetQuad(uint32 uiAddress);
	void	SetWord(uint32 uiAddress, uint16 uiValue);
	void	SetLong(uint32 uiAddress, uint32 uiValue);
	void	SetQuad(uint32 uiAddress, uint64 uiValue);
	void	SetVector(uint16 uiVector, uint32 uiAddress);

	void	Org(uint32 uiAddress);
	void	Op(uint16 uiOpcode);
	void	Op(uint16 uiOpcode, uint16 uiWord);
	void	OpLong(uint16 uiOpcode, uint32 uiLong);
	void	OpQuad(uint16 uiOpcode, uint64 uiQuad);
	void	Word(uint16 uiWord);
	void	Long(uint32 uiLong);
	uint32	Here(void);

	void	ClearCycles(void);
};


void	Test65C64TickHigh(C65C64Meta* pcMPU, void* pvContext);
void	Test65C64TickLow(C65C64Meta* pcMPU, void* pvContext);
size	Test65C64Step(C65C64Meta* pcMPU, CTest65C64Context* pcContext);
size	Test65C64Run(C65C64Meta* pcMPU, CTest65C64Context* pcContext, size uiMaxInstructions);


//A processor running from reset in a test context.
struct STest65C64
{
	CTest65C64Context*	pcContext;
	C65C64Meta			cMeta;
};


void	Test65C64Begin(STest65C64* psTest, uint32 uiResetAddress);
size	Test65C64Reset(STest65C64* psTest);
size	Test65C64Step(STest65C64* psTest);
C65C64*	Test65C64MPU(STest65C64* psTest);
void	Test65C64End(STest65C64* psTest);
uint16	Test65C64Flags(STest65C64* psTest);

//Encoders.  Registers are E65Register values.
uint16	Op65RR(uint8 uiCode, E65Register eFirst, E65Register eSecond, bool bSigned = false);
uint16	Op65RI(uint8 uiCode, E65Register eFirst, uint8 uiBytes, bool bSigned = false);
uint16	Op65RE(uint8 uiCode, E65Register eFirst, int iValue);
uint16	Op65M(uint8 uiCode, bool bSwap, E65Register eRegister, uint8 uiMode, E65Register eAddress, bool bSigned = false);
uint16	Op65ME(uint8 uiCode, int iValue, uint8 uiMode, E65Register eAddress);
uint16	Op65M2(uint8 uiCode, bool bSwap, E65Register eRegister, uint8 uiMode, E65Register eFirst, E65Register eSecond, bool bSigned = false);
uint16	Op65Jump(E65Register eStack, uint8 uiMode, uint8 uiField, bool bPushP, bool bPushC, bool bRelative);
uint16	Op65Return(E65Register eStack, bool bPullP, bool bPullC, bool bRti);
uint16	Op65Sep(uint8 uiMask);
uint16	Op65Cep(uint8 uiMask);
uint16	Op65Wth(E65Register eRegister, uint8 uiBytes);
uint16	Op65Wtth(E65Register eFirst, E65Register eSecond, uint8 uiBytes);
uint16	Op65Brk(uint16 uiVector);
uint16	Op65Stp(void);
uint16	Op65StpWake(uint8 uiWakeMask);
uint16	Op65Mul(void);
uint16	Op65Div(void);
uint16	Op65Subcopy(bool bBits, E65Register eSource, E65Register eDestination, bool bCountRegister, uint8 uiCountOrRegister);
uint16	Op65SubcopyIndices(bool bBits, bool bSourceRegister, uint8 uiSource, bool bDestinationRegister, uint8 uiDestination, uint8 uiEmbeddedBitCount = 0);
uint16	Op65Neg(E65Register eDestination, E65Register eSource);
uint16	Op65Abs(E65Register eDestination, E65Register eSource);
uint16	Op65Find(bool bSet, uint8 uiUnit, E65Register eRegister, E65Register eIndex, bool bDecrement);
uint16	Op65Bist(uint8 uiFlag, E65Register eRegister, uint8 uiIndexStyle);
uint16	Op65Stab(uint8 uiFlag, E65Register eRegister, uint8 uiIndexStyle);
uint16	Op65Stat(uint8 uiDestinationFlag, uint8 uiSourceFlag);
uint16	Op65ShiftRegister(bool bLeft, E65Register eFirst, E65Register eSecond);
uint16	Op65PushOperand(E65Register eStack, uint8 uiMode, uint8 uiField);
uint16	Op65PullOperand(E65Register eStack, uint8 uiMode, uint8 uiField);
uint16	Op65Block(uint8 uiSub, bool bDecrement, bool bAllowStop, uint8 uiStopStyle, E65Register eRegister);
uint16	Op65Vector(uint8 uiSub, bool bDecrementV, bool bAllowStop, uint8 uiStopStyle, bool bDecrementW, uint8 uiWidthCode);
uint16	Op65Gwth(E65Register eQuery, bool bDecrement, E65Register eDestination);
uint16	Op65Wthx(E65Register eRegister, bool bSigned, uint8 uiBytes);
uint16	Op65Branch(uint8 uiCondition, uint8 uiFirst, uint8 uiSecond, int iOffset);
uint16	Op65BranchImmediate(uint8 uiCondition, uint8 uiFirst, uint8 uiSecond, uint8 uiBytes);
uint16	Op65BranchEqual(bool bNotEqual, uint8 uiFirst, uint8 uiSecond, int iOffset);
uint16	Op65BranchEqualImmediate(bool bNotEqual, uint8 uiFirst, uint8 uiSecond, uint8 uiBytes);
uint16	Op65Cpyx(bool bSwap, E65Register eRegister, uint8 uiMode, E65Register eAddress, uint8 uiBytes);
uint16	Op65CpyxSigned(E65Register eRegister, uint8 uiMode, E65Register eAddress, uint8 uiBytes);
uint16	Op65Bal(int iOffset);
uint16	Op65Nop(uint8 uiWait);
uint16	Op65Shift(bool bLeft, E65Register eFirst, uint8 uiAmount);
uint16	Op65Push(E65Register eStack, uint8 uiMask);
uint16	Op65Pull(E65Register eStack, uint8 uiMask);


#endif // __TEST_65C64_CONTEXT_H__

