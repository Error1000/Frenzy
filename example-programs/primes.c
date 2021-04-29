

typedef unsigned char byte;


int main(){
	byte sieve[16] = {0};

	byte regA = 0;
	byte regB = 0;
	byte cache = 0; // cache

	regA = 3;
	MAIN_LOOP: 
		addr = (regA-1)/2;

		state = (sieve[addr/8] & (1 << (addr%8)) == 0;

		if(state){
			asm("lb ra, regA"); 	//printf("%u\n", regA);

			regB = regA*3;
			MARK_MULTIPLES_LOOP:
				sieve[(regB-1)/2/8] |= (1 << ((regB-1)/2)%8);
				cache = regB;
				regB += regA;
				regB += regA;
				if(regB < cache){ regB = cache; goto MULTIPLES_LOOP_EXIT; }
			goto MARK_MULTIPLES_LOOP;

		}

		MULTIPLES_LOOP_EXIT:

		regA += 2;
		if(regA > 85) goto MAIN_LOOP_EXIT;
	goto MAIN_LOOP;

	MAIN_LOOP_EXIT:

	PRINT_LOOP:
		if( (sieve[(regA-1)/2/8] & (1 << ((regA-1)/2)%8)) == 0 ){
			asm("lb ra, regA");     // printf("%u\n", regA);
		}
		cache = regA;
		regA += 2;
		if(regA < cache){ regA = cache; goto HLT; }
	goto PRINT_LOOP;


    HLT: goto HLT;
}
