; Note: We are never going to need to execute this "startup" code ever again once we do it, so we can repurpose it as data storage ... :)
regB: JMP
cache: {_start}
regA:  6
sSITE1_ADDR: {sSITE1}


SHLA:
	FROMA ; regA = A
	{regA}

	TOB   ; B = regA
	{regA}

	SUM	  ; A += B ( which is regA ) // Basically doubles A

	JMPC ; 1. If carry 
	{OVFLOW}

	JMP  ; Otherwise jump to end
	{SHLA_END}

	OVFLOW: CHNGA 0x1 ; 2. Then add one to A
  
    SHLA_END:	JMP	; return
SHLA_RET_ADDR:	{HLT} ; Address will be overriden by caller

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

	TOA		
	{regA}

	; SHLA_RET_ADDR = sSITE1 // mangles B
	TOB		; B = sSITE1
	{sSITE1_ADDR}
	FROMB	; SHLA_RET_ADDR = B
	{SHLA_RET_ADDR}

	; shla() // Double A
	JMP 
	{SHLA}

	sSITE1: ; This is where the SHLA jump wil jump back to

	
HLT: JMP 
{HLT}
