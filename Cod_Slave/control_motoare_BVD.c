#include "Arduino.h"

int directie (int directie, int viteza, byte pin[4], boolean DAI[2])
{
    // Ia valori de la 0 la 255
    // La mine 0 = HIGH, 1 = LOW
    if(DAI[0] == 1 && DAI[1] == 0)
    {
        viteza = 255 - viteza;
    }
    
    switch(directie)
    {
         case 0:
        {
              // Repaus
            digitalWrite(pin[0], DAI[0]);
            digitalWrite(pin[1], DAI[0]);
            digitalWrite(pin[2], DAI[0]);
            digitalWrite(pin[3], DAI[0]);         
        }
        break;
        
        case 1:
        {         
            // STOP
            digitalWrite(pin[0], DAI[1]);
            digitalWrite(pin[1], DAI[1]);
            digitalWrite(pin[2], DAI[1]);
            digitalWrite(pin[3], DAI[1]);           
        }
        break;

        case 2:
        {
            // In fata la viteza
            analogWrite(pin[0], viteza );
            digitalWrite(pin[1], DAI[0]);
            analogWrite(pin[2], viteza );
            digitalWrite(pin[3], DAI[0]);            
        }
        break;

        case 3:
        {
            // In spate
            digitalWrite(pin[0], DAI[0]);
            digitalWrite(pin[1], DAI[1]);
            digitalWrite(pin[2], DAI[0]);
            digitalWrite(pin[3], DAI[1]);           
        }
        break;

        case 4:
        {
              // Rotire Stanga
            digitalWrite(pin[0], DAI[0]);
            digitalWrite(pin[1], DAI[1]);
            digitalWrite(pin[2], DAI[1]);
            digitalWrite(pin[3], DAI[0]);        
        }
        break;
        
        case 5:
        {
            // Rotire Dreapta
            digitalWrite(pin[0], DAI[1]);
            digitalWrite(pin[1], DAI[0]);
            digitalWrite(pin[2], DAI[0]);
            digitalWrite(pin[3], DAI[1]); 
                       
        }
        break;

        case 6:
        {
            // Vireaza stanga
            analogWrite(pin[0], viteza );
            digitalWrite(pin[1], DAI[0]);
            digitalWrite(pin[2], DAI[1]);
            digitalWrite(pin[3], DAI[0]);
        }
        break;

        case 7:
        {
            //Vireaza dreapta
            digitalWrite(pin[0], DAI[1]);
            digitalWrite(pin[1], DAI[0]);
            analogWrite(pin[2], viteza );
            digitalWrite(pin[3], DAI[0]);           
        }
        break;
    }
}
