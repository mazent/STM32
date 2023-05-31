	SECTION `.text`:CODE:NOROOT(2)
	THUMB


	PUBLIC stack_e_salta

	// I parametri sono le prime due locazioni della
	// tabella delle interruzioni
	// _Noreturn
	// void stack_e_salta(uint32_t stack, uint32_t indirizzo)
	//					           r0               r1
stack_e_salta:
	// Imposto il suo stack
    MSR msp, r0

    // Scarico la pipeline
    ISB

	// Passo il controllo al programma
	BX r1




	END

