; memcpy, memmove, memset, memcmp and strlen with 16 bit counts for the W65C816, and BaseLib's MemSwp16
; (MemSwp with a size length): what BaseLib's natural arrays (counted with size, so at most
; 65535 bytes) call through ARRAY_MEMCPY and the like (ArraySizeNatural.inl).  string816.asm
; has the C library's, with size_t (32 bit) counts.  The count is pushed as 2 bytes and kept
; in a word, but the pointers are 24 bit and a block can still cross into the next bank:
; the moves (mvn and mvp) stop at a bank's end as string816.asm's do.  Y never wraps in the
; others ([[D + n]:24 + Y] adds Y to all 24 bits).  The code changes itself (the moves'
; banks), so it isn't reentrant: an interrupt handler mustn't call these.
; The calling convention is the compiler's (IR816Backend.h): arguments are pushed last first
; (pointers 4 bytes, size and int 2), A and X are 16 bits wide on the way in and out, the
; result is in A (low) and X (high), D and the data bank are kept and A, X and Y change.
; The moves (but memset within one bank, which reads its arguments from S) push the data
; bank, 12 bytes of locals and D, and point D at the stack:
;	D + 1		the caller's D
;	D + 3		Count: the bytes of this move less 1
;	D + 5		Dst: the destination (memmove16 down: its last byte)
;	D + 9		Src: the source (memmove16 down: its last byte)
;	D + 13		Bytes: the bytes left
;	D + 15		the caller's data bank
;	D + 16		the return address
;	D + 19		the arguments: the destination first

Count		= 3
Dst			= 5
Src			= 9
Bytes		= 13
Argument	= 19


		.section text.MemMove816_16
		.global _memcpy16, _memmove16, _memset16

; void* memcpy16(void* pvDest, const void* pvSource, unsigned short uiBytes)
_memcpy16:
		.width (A, X), 16
		phb
		pea 0
		pea 0
		pea 0
		pea 0
		pea 0
		pea 0
		phd
		tsc
		tcd
		jsr Arguments
		brl MoveUp

; void* memmove16(void* pvDest, const void* pvSource, unsigned short uiBytes): up unless the
; destination is above the source and in it.
_memmove16:
		.width (A, X), 16
		phb
		pea 0
		pea 0
		pea 0
		pea 0
		pea 0
		pea 0
		phd
		tsc
		tcd
		jsr Arguments
		lda [D + Dst + 2]
		cmp [D + Src + 2]
		bne Ordered
		lda [D + Dst]
		cmp [D + Src]
Ordered:
		.width (A, X), 16
		bcc ToUp
		beq ToUp
		clc
		lda [D + Src]
		adc [D + Bytes]
		tax
		lda [D + Src + 2]
		adc 0
		cmp [D + Dst + 2]
		bne Ended
		txa
		cmp [D + Dst]
Ended:
		.width (A, X), 16
		bcc ToUp
		beq ToUp
		brl MoveDown
ToUp:
		.width (A, X), 16
		brl MoveUp

; void* memset16(void* pvDest, int iValue, unsigned short uiBytes): as string816.asm's memset.
; Within one bank, without a frame (the arguments are read from S), the data bank set to the
; destination's and the value's byte twice in a word stored 16 bytes at a time, then the
; words and the byte left; otherwise (or for 0 bytes) the first byte, then each from the one
; before.
_memset16:
		.width (A, X), 16
		lda [S + 10]
		beq MemsetMove
		clc
		adc [S + 4]
		bcs MemsetMove
		phb
		sep m
		lda [S + 7]
		pha
		plb
		lda [S + 9]
		xba
		lda [S + 9]
		rep m
		pha
		lda [S + 7]
		tay
		lda [S + 13]
		lsr A
		lsr A
		lsr A
		lsr A
		beq MemsetWords
		tax
		lda [S + 1]
MemsetSixteen:
		.width (A, X), 16
		sta [0 + Y]
		sta [2 + Y]
		sta [4 + Y]
		sta [6 + Y]
		sta [8 + Y]
		sta [10 + Y]
		sta [12 + Y]
		sta [14 + Y]
		tya
		clc
		adc 16
		tay
		lda [S + 1]
		dex
		bne MemsetSixteen
MemsetWords:
		.width (A, X), 16
		lda [S + 13]
		and 14
		lsr A
		beq MemsetByte
		tax
		lda [S + 1]
MemsetWord:
		.width (A, X), 16
		sta [0 + Y]
		iny
		iny
		dex
		bne MemsetWord
MemsetByte:
		.width (A, X), 16
		lda [S + 13]
		and 1
		beq MemsetDone
		sep m
		lda [S + 1]
		sta [0 + Y]
		rep m
MemsetDone:
		.width (A, X), 16
		ply
		plb
		lda [S + 6]
		tax
		lda [S + 4]
		rtl
MemsetMove:
		.width (A, X), 16
		phb
		pea 0
		pea 0
		pea 0
		pea 0
		pea 0
		pea 0
		phd
		tsc
		tcd
		lda [D + Argument + 6]
		bne Set
		brl Exit
Set:
		.width (A, X), 16
		sep m
		lda [D + Argument + 4]
		sta [[D + Argument]:24]
		rep m
		lda [D + Argument]
		sta [D + Src]
		clc
		adc 1
		sta [D + Dst]
		lda [D + Argument + 2]
		sta [D + Src + 2]
		adc 0
		sta [D + Dst + 2]
		lda [D + Argument + 6]
		dec A
		sta [D + Bytes]

; Moves up while there are bytes left: as many as the rest of the source's and the
; destination's banks hold.
MoveUp:
		.width (A, X), 16
		lda [D + Bytes]
		beq Exit
		dec A
		sta [D + Count]
		lda [D + Src]
		eor 65535
		cmp [D + Count]
		bcs UpSource
		sta [D + Count]
UpSource:
		.width (A, X), 16
		lda [D + Dst]
		eor 65535
		cmp [D + Count]
		bcs UpDestination
		sta [D + Count]
UpDestination:
		.width (A, X), 16
		sep m
		lda [D + Src + 2]
		sta [Up + 2]:24
		lda [D + Dst + 2]
		sta [Up + 1]:24
		rep m
		ldx [D + Src]
		ldy [D + Dst]
		lda [D + Count]
Up:
		mvn 0, 0
		sec
		lda [D + Src]
		adc [D + Count]
		sta [D + Src]
		lda [D + Src + 2]
		adc 0
		sta [D + Src + 2]
		sec
		lda [D + Dst]
		adc [D + Count]
		sta [D + Dst]
		lda [D + Dst + 2]
		adc 0
		sta [D + Dst + 2]
		jsr Moved
		bra MoveUp

; The result is the destination argument.
Exit:
		.width (A, X), 16
		lda [D + Argument]
		ldx [D + Argument + 2]
		pld
		ply
		ply
		ply
		ply
		ply
		ply
		plb
		rtl

; Moves down from the ends while there are bytes left, as many as the source's and the
; destination's banks hold below them.
MoveDown:
		.width (A, X), 16
		clc
		lda [D + Src]
		adc [D + Bytes]
		sta [D + Src]
		lda [D + Src + 2]
		adc 0
		sta [D + Src + 2]
		clc
		lda [D + Dst]
		adc [D + Bytes]
		sta [D + Dst]
		lda [D + Dst + 2]
		adc 0
		sta [D + Dst + 2]
		lda [D + Src]
		bne SourceEnd
		dec [D + Src + 2]
SourceEnd:
		.width (A, X), 16
		dec [D + Src]
		lda [D + Dst]
		bne DestinationEnd
		dec [D + Dst + 2]
DestinationEnd:
		.width (A, X), 16
		dec [D + Dst]
DownLoop:
		.width (A, X), 16
		lda [D + Bytes]
		beq Exit
		dec A
		sta [D + Count]
		lda [D + Src]
		cmp [D + Count]
		bcs DownSource
		sta [D + Count]
DownSource:
		.width (A, X), 16
		lda [D + Dst]
		cmp [D + Count]
		bcs DownDestination
		sta [D + Count]
DownDestination:
		.width (A, X), 16
		sep m
		lda [D + Src + 2]
		sta [Down + 2]:24
		lda [D + Dst + 2]
		sta [Down + 1]:24
		rep m
		ldx [D + Src]
		ldy [D + Dst]
		lda [D + Count]
Down:
		mvp 0, 0
		clc
		lda [D + Src]
		sbc [D + Count]
		sta [D + Src]
		lda [D + Src + 2]
		sbc 0
		sta [D + Src + 2]
		clc
		lda [D + Dst]
		sbc [D + Count]
		sta [D + Dst]
		lda [D + Dst + 2]
		sbc 0
		sta [D + Dst + 2]
		jsr Moved
		bra DownLoop

; Copies the destination, source and size arguments to Dst, Src and Bytes.
Arguments:
		.width (A, X), 16
		lda [D + Argument]
		sta [D + Dst]
		lda [D + Argument + 2]
		sta [D + Dst + 2]
		lda [D + Argument + 4]
		sta [D + Src]
		lda [D + Argument + 6]
		sta [D + Src + 2]
		lda [D + Argument + 8]
		sta [D + Bytes]
		rts

; Bytes -= Count + 1 (the clear carry takes the 1 more).
Moved:
		.width (A, X), 16
		clc
		lda [D + Bytes]
		sbc [D + Count]
		sta [D + Bytes]
		rts


; int memcmp16(const void* pv1, const void* pv2, unsigned short uiBytes): words while there
; are two bytes left, then the byte left; the bytes of a word that differs are compared first
; to last.  It pushes 4 bytes of locals and D, and points D at the stack:
;	D + 1		the caller's D
;	D + 3		Save: the two bytes that differ
;	D + 7		the return address
;	D + 10		the arguments: the first, the second at D + 14, the count at D + 18

Save		= 3
First		= 10
Second		= 14
Length		= 18


		.section text.Memcmp816_16
		.global _memcmp16

_memcmp16:
		.width (A, X), 16
		pea 0
		pea 0
		phd
		tsc
		tcd
		ldy 0
		lda [D + Length]
		lsr A
		beq MemcmpOdd
		tax
MemcmpWord:
		.width (A, X), 16
		lda [[D + First]:24 + Y]
		cmp [[D + Second]:24 + Y]
		bne MemcmpWordDiffers
		iny
		iny
		dex
		bne MemcmpWord
MemcmpOdd:
		.width (A, X), 16
		lda [D + Length]
		and 1
		beq MemcmpSame
		sep m
		lda [[D + First]:24 + Y]
		cmp [[D + Second]:24 + Y]
		bne Difference
		rep m
MemcmpSame:
		.width (A, X), 16
		lda 0
		pld
		ply
		ply
		rtl
MemcmpWordDiffers:
		.width (A, X), 16
		sep m
		lda [[D + First]:24 + Y]
		cmp [[D + Second]:24 + Y]
		bne Difference
		iny
		lda [[D + First]:24 + Y]

; The result of a compare that found two bytes that differ at Y: the first's (in A, 8 bits
; wide) less the second's, as unsigned chars.
Difference:
		.width A, 8
		.width X, 16
		rep m
		and 255
		sta [D + Save]
		lda [[D + Second]:24 + Y]
		and 255
		sta [D + Save + 2]
		lda [D + Save]
		sec
		sbc [D + Save + 2]
		pld
		ply
		ply
		rtl


		.section text.MemSwp816_16
		.global _MemSwp16__FPvPvt

; void MemSwp16(const void* pv1, const void* pv2, size uiLength) (BaseLib's StringHelper.h; MemSwp,
; with an lsize length, is string816.asm's): a word at a time, then the byte left,
; Y running up from both pointers.  D points at the stack: the pointers are at D + 6 and
; D + 10, the length at D + 14.
_MemSwp16__FPvPvt:
		.width (A, X), 16
		phd
		tsc
		tcd
		ldy 0
		lda [D + 14]
		lsr A
		beq MemSwpOdd
		tax
MemSwpWord:
		.width (A, X), 16
		lda [[D + 6]:24 + Y]
		pha
		lda [[D + 10]:24 + Y]
		sta [[D + 6]:24 + Y]
		pla
		sta [[D + 10]:24 + Y]
		iny
		iny
		dex
		bne MemSwpWord
MemSwpOdd:
		.width (A, X), 16
		lda [D + 14]
		and 1
		beq MemSwpDone
		sep m
		lda [[D + 6]:24 + Y]
		pha
		lda [[D + 10]:24 + Y]
		sta [[D + 6]:24 + Y]
		pla
		sta [[D + 10]:24 + Y]
		rep m
MemSwpDone:
		.width (A, X), 16
		pld
		rtl


		.section text.Strlen816_16
		.global _strlen16

; unsigned short strlen16(const char* sz): sz's length, counted in Y (a word), or 65535 (MAX_SIZE)
; when there is no zero in its first 65535 bytes: what BaseLib's StrLen (natural strings) uses.
; D points at the stack: sz is at D + 6.
_strlen16:
		.width (A, X), 16
		phd
		tsc
		tcd
		sep m
		ldy 0
Strlen16Char:
		.width A, 8
		.width X, 16
		lda [[D + 6]:24 + Y]
		beq Strlen16Done
		iny
		cpy 65535
		bne Strlen16Char
Strlen16Done:
		.width A, 8
		.width X, 16
		rep m
		tya
		pld
		rtl


; The bit array functions (BaseLib's IntegerHelper.h) with size (16 bit) counts and results:
; FindFirstSetBit, FindFirstClearBit, FindLastSetBit and FindLastClearBit (FindNextSetBit and
; FindNextClearBit are C++ on them), as string816.asm's Large ones but with the count of words
; in X (first) or Y the byte offset (last) only: a size count of bits is at most 8KB.  And
; FindFirstClearBit(uint32) and FindFirstSetBit(uint64): the lowest clear (set) bit's index, or
; the width when there is none (as __builtin_ctzl and ctzll).  ARRAY_ELEMENT_NOT_FOUND ($FFFF)
; when the array has none.  The array functions push 2 bytes of locals and D, and point D at
; the stack:
;	D + 1		the caller's D
;	D + 3		BitInvert16: 0 or $FFFF
;	D + 5		the return address
;	D + 8		the arguments: the array, then the count of bits at D + 12

BitInvert16	= 3
BitArray16	= 8
BitBits16	= 12


		.section text.Bits816_16
		.relax
		.global _FindFirstSetBit__FPvt, _FindFirstClearBit__FPvt, _FindLastSetBit__FPvt, _FindLastClearBit__FPvt
		.global _FindFirstClearBit__Fj, _FindFirstSetBit__Fy

; size FindFirstSetBit(void* pvArray, size iArraySize)
_FindFirstSetBit__FPvt:
		.width (A, X), 16
		pea 0
		bra FindFirst16

; size FindFirstClearBit(void* pvArray, size iArraySize)
_FindFirstClearBit__FPvt:
		.width (A, X), 16
		pea $FFFF
FindFirst16:
		.width (A, X), 16
		phd
		tsc
		tcd
		ldy 0
		lda [D + BitBits16]
		lsr A
		lsr A
		lsr A
		lsr A
		beq FindFirst16Part
		tax
FindFirst16Word:
		.width (A, X), 16
		lda [[D + BitArray16]:24 + Y]
		eor [D + BitInvert16]
		bne FindFirst16Found
		iny
		iny
		dex
		bne FindFirst16Word
FindFirst16Part:
		.width (A, X), 16
		lda [D + BitBits16]
		and 15
		beq FindFirst16None
		asl A
		tax
		lda [[D + BitArray16]:24 + Y]
		eor [D + BitInvert16]
		and [BitsMasks16 + X]:24
		bne FindFirst16Found
FindFirst16None:
		.width (A, X), 16
		lda $FFFF
		bra Bits16Return
FindFirst16Found:
		.width (A, X), 16
		jsl BitsCtz16
		sta [D + BitInvert16]
		tya
		asl A
		asl A
		asl A
		clc
		adc [D + BitInvert16]
Bits16Return:
		.width (A, X), 16
		pld
		ply
		rtl

; size FindLastSetBit(void* pvArray, size iArraySize)
_FindLastSetBit__FPvt:
		.width (A, X), 16
		pea 0
		bra FindLast16

; size FindLastClearBit(void* pvArray, size iArraySize): Y starts at the last (partial) word.
_FindLastClearBit__FPvt:
		.width (A, X), 16
		pea $FFFF
FindLast16:
		.width (A, X), 16
		phd
		tsc
		tcd
		lda [D + BitBits16]
		lsr A
		lsr A
		lsr A
		lsr A
		asl A
		tay
		lda [D + BitBits16]
		and 15
		beq FindLast16Words
		asl A
		tax
		lda [[D + BitArray16]:24 + Y]
		eor [D + BitInvert16]
		and [BitsMasks16 + X]:24
		bne FindLast16Found
FindLast16Words:
		.width (A, X), 16
		cpy 0
		beq FindLast16None
		dey
		dey
		lda [[D + BitArray16]:24 + Y]
		eor [D + BitInvert16]
		beq FindLast16Words
FindLast16Found:
		.width (A, X), 16
		jsl BitsLog2_16
		sta [D + BitInvert16]
		tya
		asl A
		asl A
		asl A
		clc
		adc [D + BitInvert16]
		bra Bits16Return
FindLast16None:
		.width (A, X), 16
		lda $FFFF
		bra Bits16Return

; size FindFirstClearBit(uint32 uiBits): D points at the stack, uiBits is at D + 6.
_FindFirstClearBit__Fj:
		.width (A, X), 16
		phd
		tsc
		tcd
		lda [D + 6]
		eor $FFFF
		bne FindWord32Found
		lda [D + 8]
		eor $FFFF
		beq FindWord32None
		jsl BitsCtz16
		clc
		adc 16
		pld
		rtl
FindWord32None:
		.width (A, X), 16
		lda 32
		pld
		rtl
FindWord32Found:
		.width (A, X), 16
		jsl BitsCtz16
		pld
		rtl

; size FindFirstSetBit(uint64 uiBits): D points at the stack, uiBits is at D + 6; Y is the
; found word's first bit.
_FindFirstSetBit__Fy:
		.width (A, X), 16
		phd
		tsc
		tcd
		ldy 0
		lda [D + 6]
		bne FindWord64Found
		ldy 16
		lda [D + 8]
		bne FindWord64Found
		ldy 32
		lda [D + 10]
		bne FindWord64Found
		ldy 48
		lda [D + 12]
		bne FindWord64Found
		lda 64
		pld
		rtl
FindWord64Found:
		.width (A, X), 16
		jsl BitsCtz16
		phy
		clc
		adc [S + 1]
		ply
		pld
		rtl
