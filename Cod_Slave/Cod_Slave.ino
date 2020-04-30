//----------------------------------LIBRARII---------------------------------------------//
#include <IRremote.h>

//----------------------------------INCLUDE C--------------------------------------------//

#include "control_motoare_BVD.c"
#include "ceas_BVD.c"
//#include "configurare_butoane_BVD.c"

//----------------------------------SETARI PINI------------------------------------------//

// Setari telecomanda
const byte PIN_IR_RECEPTIE = 2;

// Setare pini motoare
const byte PINI_MOTOARE[] = {9,10,5,6};
// (PWM) M_stanga_+ = 9(B-1B), M_stanga_- = 10(B-1A), M_dreapta_+ = 5(A-1A), M_dreapta_- = 6(A-1B)

//----------------------------------DECLARARE VARIABILE-----------------------------------//

// ATENTIE, la mine sunt inversate
const boolean DAI[2] = {1,0}; // In loc de HIGH si LOW

// Setari reciver IR
IRrecv irrecv(PIN_IR_RECEPTIE);
decode_results results;
short valoare_recive = 0;

// Setare butoane telecomanda IR
short control[6] = {-32131,765,-22441,-30601,2295,18615};
// Bine = 0, Anulare = 1, Inainte = 2, Inapoi = 3, Stanga = 4, Dreapta = 5;

// Ceas
byte zeci_secunde = 0;
unsigned int secunde_curente = 0;
boolean tic_tac_secunde = 0;

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

    // --Start IR------------------------------//
    irrecv.enableIRIn();
    Serial.println("A pornit reciver-ul IR.");

    // --SETUP butoane------------------------------//
    configurare_butoane();           
}

//----------------------------------MAIN------------------------------------------------//

void loop() 
{
    ceas(&zeci_secunde, &secunde_curente, &tic_tac_secunde); 
}

short receptie()
{
    if( irrecv.decode( &results ) )
    {    
        if( results.value == -1)
        {
            irrecv.resume();
            return valoare_recive;
        }
        else
        {
            valoare_recive = results.value;
            irrecv.resume();
            return valoare_recive; 
        }      
    }
    return 0;   
}

void configurare_butoane()
{
    // Daca este un buton care nu are configuratie, incepe configurarea butoanelor
    if( !control[0] || !control[1] || !control[2] || !control[3] || !control[4] || !control[5])
    {
        Serial.println("Nu ai butoanele configurate.");
        delay(500);
        Serial.println("Asa ca incepem sa configuram butoanele.");
        configurare_butoane();
    }
    else
    {
        Serial.println("Ai butoane configurate.");
        delay(1000);
        Serial.println("Daca doresti sa configurezi alta telecomanda.");
        delay(1000);
        Serial.println("Apasa orice tasta in afata de OK.");
        delay(1000);
        Serial.println("Daca doresti sa continui cu butoanele configurate, apasa 'Bine'.");
        do
        {
            int valoare = receptie();
            if(valoare != control[0] && valoare != 0)
            {
                Serial.println("");
                Serial.println("Incepem configurarea butoanelor.");
                Serial.println("");
                incepe_configurare_butoane();
            }
            if(valoare == control[0])
            {
               break;
            }
        }while(1);
    }
}
void incepe_configurare_butoane()
{
    boolean flag = 1;
    boolean flag_error = 0;
    // --------------------------Bine[0]--------------------------//
    do
    {
        Serial.println("Apasa o tasta pentru a seta butonul `Bine`.");
        if( setare_buton(&control[0]) )
        {
            Serial.println("Ai setat butonul 'Bine'.");
            Serial.println("");
            flag = 0;
        }              
    }while(flag);
    flag = 1;
    // --------------------------Anulare[1]--------------------------//
    do
    {
        Serial.println("Apasa o tasta pentru seta butonul `Anulare`.");
        if( setare_buton(&control[1]) )
        {
            if(control[1] != control[0])
            {
                Serial.println("Ai setat butonul 'Anulare'.");
                Serial.println("");
                flag = 0;
            }
            else
            {
                Serial.println("Acesta tasta este deja utilizata.");
                Serial.println("Te rog sa incerci din nou o alta tasta");
            }
            
        }       
    }while(flag);
    flag = 1;
    // --------------------------Inainte[2]--------------------------//
    do
    {
        flag_error = 0;
        
        Serial.println("Apasa o tasta pentru seta butonul `Inainte`.");
        if( setare_buton(&control[2]) )
        {
            for(byte i = 0; i < 2; i++)
            {
                if( control[2] == control[i] )
                {
                    Serial.println("Acesta tasta este deja utilizata.");
                    Serial.println("Te rog sa incerci din nou o alta tasta");
                    flag_error = 1;
                    continue;
                }
            }
            
            if(!flag_error)
            {
                Serial.println("Ai setat butonul 'Inainte'.");
                Serial.println("");
                flag = 0;
            }
            
        }       
    }while(flag);
    flag = 1;
    // --------------------------Inapoi[3]--------------------------//
    do
    {
        flag_error = 0;
        
        Serial.println("Apasa o tasta pentru seta butonul `Inapoi`.");
        if( setare_buton(&control[3]) )
        {
            for(byte i = 0; i < 3; i++)
            {
                if( control[3] == control[i] )
                {
                    Serial.println("Acesta tasta este deja utilizata.");
                    Serial.println("Te rog sa incerci din nou o alta tasta");
                    flag_error = 1;
                    continue;
                }
            }
            
            if(!flag_error)
            {
                Serial.println("Ai setat butonul 'Inapoi'.");
                Serial.println("");
                flag = 0;
            }
            
        }       
    }while(flag);
    flag = 1;
    // --------------------------Stanga[4]--------------------------//
    do
    {
        flag_error = 0;
        
        Serial.println("Apasa o tasta pentru seta butonul `Stanga`.");
        if( setare_buton(&control[4]) )
        {
            for(byte i = 0; i < 4; i++)
            {
                if( control[4] == control[i] )
                {
                    Serial.println("Acesta tasta este deja utilizata.");
                    Serial.println("Te rog sa incerci din nou o alta tasta");
                    flag_error = 1;
                    continue;
                }
            }
            
            if(!flag_error)
            {
                Serial.println("Ai setat butonul 'Stanga'.");
                Serial.println("");
                flag = 0;
            }
            
        }       
    }while(flag);
    flag = 1;
    // --------------------------Dreapta[5]--------------------------//
    do
    {
        flag_error = 0;
        
        Serial.println("Apasa o tasta pentru seta butonul `Dreapta`.");
        if( setare_buton(&control[5]) )
        {
            for(byte i = 0; i < 5; i++)
            {
                if( control[5] == control[i] )
                {
                    Serial.println("Acesta tasta este deja utilizata.");
                    Serial.println("Te rog sa incerci din nou o alta tasta");
                    flag_error = 1;
                    continue;
                }
            }
            
            if(!flag_error)
            {
                Serial.println("Ai setat butonul 'Dreapta'.");
                Serial.println("");
                flag = 0;
            }
            
        }       
    }while(flag);
    flag = 1;

}

boolean setare_buton(int *buton)
{
    *buton = 0;
    boolean tic_tac = 0; 
    int confirmare = 0;
    do
    {    
        *buton = receptie();
        delay(300); 
        if( *buton != 0)
        {
            do
            {               
                if(!tic_tac)
                {
                    Serial.println("Am primit un semnal.");
                    Serial.println("Apasa pe acelasi buton pentru a confirma.");
                    tic_tac = 1;
                } 
                confirmare = receptie();
                delay(300);          
                if(confirmare != 0 && confirmare == *buton)
                {
                    return 1;
                }
                        
                if(confirmare != 0 && confirmare != *buton)
                {
                    Serial.println("Nu a fost acelasi buton.");
                    return 0;    
                }                       
            }while(1);
        }
    }while(1);
}
