;;;;;;;;;;;;;;;;;;;;;; HEADER
JMP 
{OUTSIDE} ; ESCAPE HEADER
;-----------------
cache: 0x00
;;;;;;;;;;;;;;;;;;;;;;;


; Show fibonacci numbers in B, stop at biggest one
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
