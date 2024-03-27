/******************************************

    Attesa di microsecondi utilizzando il timer di sistema

    $Id$

******************************************/

	SECTION `.text`:CODE:NOROOT(2)
	THUMB

	PUBLIC attesa_clock

	// void attesa_clock(uint32_t clock)
	//							   r0

attesa_clock:
	PUSH {r4, r5, r6}

	; r0 = clock da attendere

	; r1 = reload (cicli per tick)
	LDR.N r1, indSysTickLOAD
	LDR r1, [r1]
	ADD r1, r1, #1

    ; r5 = contatore clock
    EORS r5, r5, r5

    ; r6 = puntatore a systick->VAL
    LDR.N r6, indSysTickVAL

    ; r2 = systick->VAL precedente
    LDR r2, [r6]
cicloAC:
	; r3 = systick->VAL corrente
	LDR r3, [r6]
	; r4 = tempo trascorso
	SUBS r4, r2, r3
	BPL posAC
		; negativo
	SUB r4, r1, r3
	ADD r4, r4, r2
posAC:
	ADDS r5, r5, r4
	CMP r5, r0
	BPL esciAC
    MOV r2, r3
	B cicloAC

esciAC:
	POP {r4, r5, r6}
	BX lr

    SECTION `.text`:CODE:NOROOT(2)
    DATA

indSysTickVAL:
    DC32     0xE000E018

indSysTickLOAD:
    DC32     0xE000E014


	END

