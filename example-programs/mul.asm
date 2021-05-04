;;;;;;;;;;;;;;;;;;;;;; HEADER
JMP 
{OUTSIDE} ; ESCAPE HEADER
;-----------------
cache: 0x00
; Multiply val1 by val2, put result in A
val1: 10
val2: 10
;;;;;;;;;;;;;;;;;;;;;;;

OUTSIDE:
 			; do {

    TOB 
    {val2} 		; B = val2
    
    SUM			; A += B
    
    FROMA 
    {cache} 		; cache = A

    CHNGA 0x3 		; A = 0

    TOB 
    {val1} 		; B = val1
    
    CHNGB 0x0  		; B -= 1
    
    FROMB 
    {val1} 		; val1 = B

    TOA 
    {cache} 		; A = cache

    JMPZ 
    {HLT}		; if(B == 0) break;

JMP 
{OUTSIDE}		; } while(true);

HLT: JMP 
     {HLT} ; while(true);
