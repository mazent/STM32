#define STAMPA_DBG
#include "utili.h"
#include "i2c.h"
#include "bsp.h"

static bool iniz = false ;

// Comandi
#define CMD_WIR    (0 << 4) //  Write to Input Register
#define CMD_UDAC   (1 << 4) //  Update (Power Up) DAC Register
#define CMD_WUDAC  (3 << 4) //  Write to and Update (Power Up) DAC Register
#define CMD_PD     (4 << 4) //  Power Down
#define CMD_SIR    (6 << 4) //  Select Internal Reference
#define CMD_SER    (7 << 4) //  Select External Reference

bool LTC2631_iniz(void)
{
	ASSERT(!iniz) ;

	if (!iniz) {
		iniz = I2C_iniz() ;
		ASSERT(iniz) ;
	}

	return iniz ;
}

bool LTC2631_wir(uint16_t x)
{
	bool esito = false ;

	ASSERT(iniz) ;

	do {
		if (!iniz)
			break ;

		uint8_t msg[3] = {
			CMD_WIR
		} ;
		memcpy(msg + 1, &x, sizeof(x)) ;

		esito = I2C_scrivi(LTC2631_IND, msg, sizeof(msg)) ;
	} while (false) ;

	return esito ;
}

bool LTC2631_udac(void)
{
	bool esito = false ;

	ASSERT(iniz) ;

	do {
		if (!iniz)
			break ;

		uint8_t msg[3] = {
			CMD_UDAC
		} ;

		esito = I2C_scrivi(LTC2631_IND, msg, sizeof(msg)) ;
	} while (false) ;

	return esito ;
}

bool LTC2631_wudac(uint16_t x)
{
	bool esito = false ;

	ASSERT(iniz) ;

	do {
		if (!iniz)
			break ;

		uint8_t msg[3] = {
			CMD_WUDAC
		} ;
		memcpy(msg + 1, &x, sizeof(x)) ;

		esito = I2C_scrivi(LTC2631_IND, msg, sizeof(msg)) ;
	} while (false) ;

	return esito ;
}

bool LTC2631_pd(void)
{
	bool esito = false ;

	ASSERT(iniz) ;

	do {
		if (!iniz)
			break ;

		uint8_t msg[3] = {
			CMD_PD
		} ;

		esito = I2C_scrivi(LTC2631_IND, msg, sizeof(msg)) ;
	} while (false) ;

	return esito ;
}

bool LTC2631_sir(void)
{
	bool esito = false ;

	ASSERT(iniz) ;

	do {
		if (!iniz)
			break ;

		uint8_t msg[3] = {
			CMD_SIR
		} ;

		esito = I2C_scrivi(LTC2631_IND, msg, sizeof(msg)) ;
	} while (false) ;

	return esito ;
}

bool LTC2631_ser(void)
{
	bool esito = false ;

	ASSERT(iniz) ;

	do {
		if (!iniz)
			break ;

		uint8_t msg[3] = {
			CMD_SER
		} ;

		esito = I2C_scrivi(LTC2631_IND, msg, sizeof(msg)) ;
	} while (false) ;

	return esito ;
}
