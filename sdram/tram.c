#define STAMPA_DBG
#include "includimi.h"

/******************************************

    Test della ram

******************************************/

#define SPIA		((BYTE) 0x55)

DWORD TRAM_AddrWalk1(DWORD Base, DWORD numByte)
{
	volatile BYTE * pRam = (BYTE *) Base ;
	DWORD uno = 1 ;

	// Segnalino
	pRam[0] = SPIA ;

	// Ciclo sugli indirizzi
	while (uno != numByte) {
		pRam[uno] = ~SPIA ;

		if (SPIA != pRam[0])
			break ;

		uno <<= 1 ;
	}

	if (numByte == uno)
		uno = 0 ;

	return uno ;
}

DWORD TRAM_AddrWalk0(DWORD Base, DWORD numByte)
{
	volatile BYTE * pRam = (BYTE *) Base ;
	DWORD uno = 1 ;    

	// Segnalino
	pRam[numByte-1] = SPIA ;

	// Ciclo sugli indirizzi
	while (uno != numByte) {
		DWORD zero = ~uno ;
		zero &= numByte - 1 ;

		pRam[zero] = ~SPIA ;

		if (SPIA != pRam[numByte-1])
			break ;

		uno <<= 1 ;
	}

	if (numByte == uno)
		uno = 0 ;

	return uno ;
}

DWORD TRAM_DataWalk1e0_16bit(DWORD Base)
{
	volatile WORD * pRam = (WORD *) Base ;
	WORD uno = 1 ;
	WORD zero = ~uno ;
	int i ;

	// Ciclo sui dati
	for (i=0 ; i<16 ; i++) {
		pRam[0] = uno ;
		pRam[1] = zero ;

		if (uno != pRam[0])
			break ;

		if (zero != pRam[1]) {
			uno = zero ;
			break ;
		} ;

		uno <<= 1 ;
		zero = ~uno ;
	}

	if (16 == i)
		uno = 0 ;

	return uno ;
}
