#include "Arduino.h"

void directie (byte directie, unsigned short viteza, byte pin[4])
{
    byte viteza_2 = 30;
    if(viteza == 0)
    {
        viteza = 255;
        viteza_2 = 255;
    }

    switch(directie)
    {
         case 0:
        {
              // Repaus
            digitalWrite(pin[0], LOW);
            digitalWrite(pin[1], LOW);
            digitalWrite(pin[2], LOW);
            digitalWrite(pin[3], LOW);         
        }
        break;
        
        case 1:
        {         
            // STOP
            digitalWrite(pin[0], HIGH);
            digitalWrite(pin[1], HIGH);
            digitalWrite(pin[2], HIGH);
            digitalWrite(pin[3], HIGH);           
        }
        break;

        case 2:
        {
            // In fata la viteza
            analogWrite(pin[0], viteza - 10);
            digitalWrite(pin[1], LOW);
            analogWrite(pin[2], viteza );
            digitalWrite(pin[3], LOW);            
        }
        break;

        case 3:
        {
            // In spate la viteza
            digitalWrite(pin[0], LOW);
            digitalWrite(pin[1], viteza - 10);
            digitalWrite(pin[2], LOW);
            digitalWrite(pin[3], viteza);           
        }
        break;

        case 4:
        {
              // Rotire Stanga
            digitalWrite(pin[0], LOW);
            digitalWrite(pin[1], HIGH);
            digitalWrite(pin[2], HIGH);
            digitalWrite(pin[3], LOW);        
        }
        break;
        
        case 5:
        {
            // Rotire Dreapta
            digitalWrite(pin[0], HIGH);
            digitalWrite(pin[1], LOW);
            digitalWrite(pin[2], LOW);
            digitalWrite(pin[3], HIGH); 
                       
        }
        break;

        case 6:
        {
            // Vireaza stanga
            analogWrite(pin[0], viteza - viteza_2 );
            digitalWrite(pin[1], LOW);
            analogWrite(pin[2], viteza);
            digitalWrite(pin[3], LOW);
        }
        break;

        case 7:
        {
            //Vireaza dreapta
            digitalWrite(pin[0], viteza);
            digitalWrite(pin[1], LOW);
            analogWrite(pin[2], viteza - viteza_2 );
            digitalWrite(pin[3], LOW);           
        }
        break;
    }
}
