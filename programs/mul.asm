; Instruction set:
; TOA, TOB, SUM, SUB, FROMA, FROMB, JMP, INCA, INCB, DECA, DECB, NEGA, NEGB, JMPC, JMPZ

;;;;;;;;;;;;;;;;;;;;;; HEADER
; NOTE: There cannot be more than 16 statements in the header
JMP {OUTSIDE_ADDR} ; ESCAPE HEADER
OUTSIDE_ADDR: {OUTSIDE} ; Store the location of the OUTSIDE label at some address and save that address int OUTSIDE_ADDR
END_LOOP_ADDR: {END_LOOP}
;-----------------
zero: 0x00
cache: 0x00
val1: 0x04
val2: 0x19
;;;;;;;;;;;;;;;;;;;;;;;

OUTSIDE:
 			; do {

    TOB {val2} 		; B = val2
    SUM			; A += B
    FROMA {cache} 		; cache = A

    TOA {zero} 		; A = 0

    TOB {val1} 		; B = val1
    DECB   		; B -= 1
    FROMB {val1} 		; val1 = B

    TOA {cache} 		; A = cache

    JMPZ {END_LOOP_ADDR}		; if(B == 0) break;

JMP {OUTSIDE_ADDR}		; } while(true);

END_LOOP: JMP {END_LOOP_ADDR} ; while(true);
