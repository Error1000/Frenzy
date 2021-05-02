; Note: We are never going to need to execute this "startup" code ever again once we do it, so we can repurpose it as data storage ... :)
regB: JMP
cache: {_start}
regA:  6

SHLA:
	FROMA 
	{regA}
	FROMB 
	{regB}

	TOB  
	{regA}

	SUM

	JMPC ; 1. If carry 
	{OVFLOW}

	JMP  ; Otherwise jump to end
	{SHLA_END}

	OVFLOW: CHNGA 0x1 ; 2. Then add one to A
  
SHLA_END:
	TOB     ; Restore B
	{regB}
	JMP	; return
SHLA_RET_ADDR:	{HLT} ; Address will be overriden by caller


eigth: 8
SHLA_RET_SITE_ADDR: {SHLA_RET_SITE}
SHRA:
	FROMB
	{regB}


	TOB 
	{SHLA_RET_SITE_ADDR}
	FROMB 
	{SHLA_RET_ADDR}

	TOB		; B = 8
	{eigth}

	SHRA_LOOP: ; unused but useful for clarity

                SHLA_RET_SITE: ; This is where SHLA Jump will jump back to
		; if(--b == 0) goto SHRA_END;
		FROMA   ; Save A
		{regA}
		CHNGA 0x3 ; A = 0
		CHNGB 0x0 ; B--;
		TOA     ; Restore A
		{regA}
		JMPZ 
		{SHRA_END}

		JMP
		{SHLA}
        ;------------------

	SHRA_END:

	TOB	; Restore B
	{regB}

		JMP 
SHRA_RET_ADDR:	{HLT}; Will be overriden by caller


sSITE1_ADDR: {sSITE1}
_start:

	TOA		
	{regA}

	; SHRA_RET_ADDR = sSITE1 // mangles B
	TOB		; B = sSITE1
	{sSITE1_ADDR}
	FROMB	; SHRA_RET_ADDR = B
	{SHRA_RET_ADDR}

	; shra() // Double A
	JMP 
	{SHRA}

	sSITE1: ; This is where the SHRA jump will jump back to

	
HLT: JMP 
{HLT}