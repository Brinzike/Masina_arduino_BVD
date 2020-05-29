#include "Arduino.h"

void ceas(byte *zeci_secunde, unsigned int *secunde_curente, boolean *tic_tac_secunde)
{
    *zeci_secunde = ((millis() % 1000) / 100) + 1;
    
    if( *zeci_secunde == 10)
    {
        if(*tic_tac_secunde == 1)
        {
            *secunde_curente = *secunde_curente + 1;
            *tic_tac_secunde = 0;
        }
    }
    else
    {
        *tic_tac_secunde = 1;
    }
    
}
