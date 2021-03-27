; Instruction set:
; TOA, TOB, SUM, SUB, FROMA, FROMB, JMP, INCA, INCB, DECA, DECB, NEGA, NEGB, JMPC, JMPZ

;;;;;;;;;;;;;;;;;;;;;; HEADER
; NOTE: There cannot be more than 16 statements in the header
INCA
INCB
JMP {OUTSIDE_ADDR} ; ESCAPE HEADER
OUTSIDE_ADDR: {OUTSIDE} ; Store the location of the OUTSIDE label at some address and save that address int OUTSIDE_ADDR

;-----------------
cache: 0x00
;;;;;;;;;;;;;;;;;;;;;;;


OUTSIDE:
	FROMA {cache} ; c = a
	SUM ; a += b
	TOB {cache} ; b = c
JMP {OUTSIDE_ADDR}
