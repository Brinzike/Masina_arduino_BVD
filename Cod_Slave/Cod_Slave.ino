//----------------------------------LIBRARII---------------------------------------------//
#include <IRremote.h>

//----------------------------------INCLUDE C--------------------------------------------//

#include "control_motoare_BVD.c"
#include "ceas_BVD.c"

//----------------------------------SETARI PINI------------------------------------------//

// Setari telecomanda
const byte PIN_IR_RECEPTIE = 2;

// Setare pini motoare
// Trebuie sa fie PWM pentru a controla analog
const byte PINI_MOTOARE[] = {9,10,5,6};
//M_STANGA_PLUS = 9; // B-1B
//M_STANGA_MINUS = 10; // B-1A
//M_DREAPTA_PLUS = 5; // A-1A
//M_DREAPTA_MINUS = 6; // A-1B

//----------------------------------DECLARARE VARIABILE-----------------------------------//

// ATENTIE, LA MINE SUNT INVERSATE
const boolean DAI[2] = {1,0}; // In loc de HIGH si LOW

// Setari reciver IR
IRrecv irrecv(PIN_IR_RECEPTIE);
decode_results results;
short recive = 0;
short inainte = 0;
short inapoi = 0;
short drapta = 0;
short stanga = 0;

// Ceas
byte zeci_secunde = 0;
unsigned int secunde_curente = 0;
boolean tic_tac_secunde = 0;

//
unsigned int secunde_precedente = 0;
boolean cronometru_pornit = 0;

//----------------------------------SETUP------------------------------------------------//

void setup() 
{ 
    Serial.begin(9600);
    Serial.println("A pornit sitemul :))");

    // --SETUP pini------------------------------//
    for(byte i = 0; i < 4; i++)
    {
        pinMode(PINI_MOTOARE[i], OUTPUT);
        // Nu inteleg de ce a inceput sa merga pe dos si porneste din prima cu High
        digitalWrite(PINI_MOTOARE[i], DAI[0]);
    }    
    Serial.println("Pinii au fost setati.");

    irrecv.enableIRIn();
    Serial.println("A pornit reciver-ul IR.");
    Serial.println("Configuram butoanele.");
    Serial.println("Apasa o tasta pentru OK.");
    
}

//----------------------------------MAIN------------------------------------------------//

void loop() 
{
    short semnal = receptie();
    ceas(zeci_secunde, secunde_curente, tic_tac_secunde);
    Serial.print("Semnalul primit este : ");
    Serial.println(semnal);
    
}

short receptie()
{
    if( irrecv.decode( &results ) )
    {    
        if( results.value == -1)
        {
            irrecv.resume();
            return recive;
        }
        else
        {
            recive = results.value;
            irrecv.resume();
            return recive; 
        }      
    }
    return 0;   
}
