; A 128KB slice of Garth Wilson's reciprocal table (INVERT.HEX, from wilsonminesco.com): the
; long at MathInvert + (d - $8000) * 4 is round(2^32 / d) for d = $8000 to $FFFF (a 17 bit
; value: its third byte is 1, or 2 for d = $8000).  The file has every d from 0, at $40000 +
; d * 4; only the entries for a divisor shifted up until its top bit is set are kept.  It
; starts on a bank so the 816 runtime's divide reaches it with a 16 bit index: bit 14 of d
; picks the bank.  Linked only when something uses it.
	.section rodata.MathInvert
	.align $10000
	.global MathInvert
MathInvert:
	.inchex "INVERT.HEX", $60000, $20000
