; Garth Wilson's multiplication table (MULT.HEX, from wilsonminesco.com): the word at
; MathMult + ((a << 8) | b) * 2 is a * b for bytes a and b.  It is 128KB, two banks, and
; starts on a bank so the 816 runtime reaches it with a 16 bit index (MathMultiply in
; IR816Backend.cpp): bit 7 of a picks the bank.  Linked only when something uses it.
	.section rodata.MathMult
	.align $10000
	.global MathMult
MathMult:
	.inchex "MULT.HEX"
