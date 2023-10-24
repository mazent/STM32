#define STAMPA_DBG
#include "utili.h"

// Rinominare main() generato dal cubo e sostituire
// la parte dopo l'ultima iniz.zione con un return qls
extern int cubo_main(void) ;

// Prima invocata (non hai niente)
extern void app_reset(void) ;
// Procedi con le tue robe (non deve tornare)
extern void app_iniz(void) ;

#pragma segment="RAM_IRQ"

int main(void)
{
    // O lancia il programma ...
    app_reset() ;

    // ... o proseguo
    if ( __section_size("RAM_IRQ") ) {
        // Se si usano [rom&ram]_startup_stm32h735xx.s, il block RAM_IRQ
        // ha dimensione non nulla e va inizializzato VTOR
        SCB->VTOR = UINTEGER( __section_begin("RAM_IRQ") ) ;
        __DSB() ;
    }

    // Inizializzo
    (void) cubo_main() ;

    app_iniz() ;
}

#ifdef USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(
    uint8_t * file,
    uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    DBG_PRINTF("%s %s %d", __func__, file, (int) line) ;
}

#endif /* USE_FULL_ASSERT */
