regB: JMP
cache: {_start}
regA:  6

SHLA:
	FROMA ; regA = A
	{regA}

	FROMB ; regB = B
	{regB}

	TOB   ; B = regA
	{regA}

	SUM	  ; A += B ( which is regA ) // Basically doubles A

	JMPC ; 1. If carry 
	{OVFLOW}

	JMP  ; Otherwise jump to end
	{SHLA_END}

	OVFLOW: CHNGA 0x1 ; 2. Then add one to A
  
    SHLA_END:

	TOB	; B = regB // Restore B
	{regB}

				JMP	; return
SHLA_RET_ADDR:	{HLT}

;eigth: 8
;SHRA:
;
;	FROMA
;	{regA}
;
;	FROMB
;	{regB}
;
;
;
;	TOB 
;	{SITE}
;	FROMB 
;	{SHLA_RET_ADDR}
;
;	TOB		; B = 8
;	{eigth}
;
;	SHRA_LOOP:
;		; if(--b == 0) goto SHRA_END;
;		FROMA
;		{regA}
;		CHNGA 0x3
;		CHNGB 0x0
;		TOA
;		{regA}
;		JMPZ 
;		{SHRA_END}
;
;		JMP
;		{SHLA}
;		SITE:
;	JMP
;	{SHRA_LOOP}
;
;	SHRA_END:
;
;	TOB
;	{regB}
;
;	JMP
;	{SHLA}
;
;	JMP 
;	{SHRA_RET_ADDR}


_start:

	TOA		; A = 6
	{regA}

	; SHLA_RET_ADDr = sSITE1 // mangles B
	TOB		; B = sSITE1
	{sSITE1_ADDR}
	FROMB	; SHLA_RET_ADDR = B
	{SHLA_RET_ADDR}

	; shla() // Double A, so now A should be 12
	JMP 
	{SHLA}

	sSITE1_ADDR: {sSITE1}
	sSITE1: 

	CHNGA 0x0	 ; A--
	CHNGA 0x0	 ; A--
	; A = 10 now, hopefully

HLT: JMP 
{HLT}
