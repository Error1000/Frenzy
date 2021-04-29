;;;;;;;;;;;;;;;;;;;;;; HEADER
; NOTE: There cannot be more than 16 statements in the header
JMP 
{OUTSIDE} ; ESCAPE HEADER
;-----------------
cache: 0x00
;;;;;;;;;;;;;;;;;;;;;;;


OUTSIDE:
	CHNGA 0x1
	CHNGB 0x1

LOOP:	FROMA 
	{cache} ; c = a
	
	SUM ; a += b	
	
	TOB 
	{cache} ; b = c
	JMPC
	{HLT}
JMP 
{LOOP}

HLT: JMP
{HLT}
