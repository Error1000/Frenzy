; Note: We are never going to need to execute this "startup" code ever again once we do it, so we can repurpose it as data storage ... :)
regB3: TOA
regA3: {regA}

regB2: TOB
regA2: {regB}

JMP
{_start}

regA: 6
regB: 0


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


shla_times: 8
SHLA_RET_SITE_ADDR: {SHLA_RET_SITE}
SHRA:
	FROMB
	{regB2}


	TOB 
	{SHLA_RET_SITE_ADDR}
	FROMB 
	{SHLA_RET_ADDR}

	TOB		; B = shal_times
	{shla_times}

	SHRA_LOOP: ; unused but useful for clarity

                SHLA_RET_SITE: ; This is where SHLA Jump will jump back to
		; if(--b == 0) goto SHRA_END;
		FROMA   ; Save A
		{regA2}
		CHNGA 0x3 ; A = 0
		CHNGB 0x0 ; B--;
		TOA     ; Restore A
		{regA2}
		JMPZ 
		{SHRA_END}

		JMP
		{SHLA}
        ;------------------

	SHRA_END:

	TOB	; Restore B
	{regB2}

				JMP 
SHRA_RET_ADDR:	{HLT}; Will be overriden by caller


site1_addr: {site1}

_start:

	TOB
	{site1_addr}
	FROMB
	{SHRA_RET_ADDR}

	JMP
	{SHRA}

	site1:

HLT: JMP 
{HLT}