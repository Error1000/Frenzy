;;;;;;;;;;;;;;;;;;;;;; HEADER
; NOTE: There cannot be more than 16 statements in the header
JMP
{OUTSIDE} ; ESCAPE HEADER
;-----------------
cache_a: 0xA
cache_b: 0x2
res:     0x0
;;;;;;;;;;;;;;;;;;;;;;;

OUTSIDE:
	TOA {cache_a}
LOOP:
	TOB {res}
	CHNGB 0x1
	FROMB {res}

	TOB {cache_b}
	SUB
JMPC
{LOOP}

END:
	TOA {res}

HLT: JMP
     {HLT}



