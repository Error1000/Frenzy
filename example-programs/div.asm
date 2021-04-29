;;;;;;;;;;;;;;;;;;;;;; HEADER
; NOTE: There cannot be more than 16 statements in the header
JMP
{OUTSIDE} ; ESCAPE HEADER
;-----------------
cache_a: 69
cache_b: 0x2
res:     0x0
;;;;;;;;;;;;;;;;;;;;;;;

OUTSIDE:
	TOA 	;A = cache_a
	{cache_a}
LOOP:
	TOB ; B = res
	{res}
	
	CHNGB 0x1 ; B--
	
	FROMB ; res = B
	{res}

	TOB 	; B = cache_b
	{cache_b}
	
	SUB	; A -= B
JMPC	; If carry then loop back
{LOOP} ; Carry should be on after SUB, except when overflowing signed arithmetic, also SUB leaves B inverted, but that dosen't matter for us
; CHNGB is done before SUB because it updates flags too

END:
	TOA 
	{res}

HLT: JMP
     {HLT}



