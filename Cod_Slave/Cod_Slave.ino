//----------------------------------LIBRARII---------------------------------------------//
#include <IRremote.h>
#include <Wire.h>
#include <EEPROM.h>

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
    Serial.begin(115200);    
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

    comunicare(1);
    Serial.println("S-a stabilit conexiunea cu partenerul.");
    
    // --SETUP butoane------------------------------//
    configurare_butoane();               
}

//----------------------------------MAIN------------------------------------------------//
void loop() 
{
    ceas(&zeci_secunde, &secunde_curente, &tic_tac_secunde); 
}

//----------------------------------Comunicare------------------------------------------------//
void comunicare(byte cod)
{
    if( cod == 1 && cod <= 3)
    {
        ordine(cod);
    }
    else
    {
        if( ordine(2) ) // A returnat 1
        {
            Serial.write((byte)cod);
        }
        comunicare(1000);
    }

}

boolean ordine(byte cod)
{
    boolean confirmare_serial = 0;
    switch(cod)
    {
        // ----------Confirmare sincronizare----------//
        case 1:
        {
            // Trimite un semnal
            Serial.write((boolean)1);

            // Asteapta sa confirme ca este sincronizat
            while(!confirmare_serial)
            {
                if(Serial.available() > 0)
                {
                    confirmare_serial = Serial.read();
                }
            }

            // Ii trimit ca am auzit semnalul lui
            Serial.write(confirmare_serial);
        }
        break;

        // ----------Vreau sa zic ceva----------//
        case 2:
        {
            // Trimit ca vreau sa ma asculte
            // Ca nu cumva sa vrea el sa imi spuna ceva
            Serial.write((byte)cod);

            // Asteapta sa confirme ca este sincronizat
            // Si este disponibil sa asculte
            while(!confirmare_serial)
            {
                if(Serial.available() > 0)
                {
                    confirmare_serial = Serial.read();
                }
            }
            
            // Are ceva mai bun de zis
            if(confirmare_serial == 3)
            {
                return 0;
            }
            // Este disponibil sa ma asculte
            else
            {
                confirmare_serial = 0;
            }
                     
            // El executa ce ii zic
            // Codul merge mai departe 
            return 1; 
        }
        break;

        // ----------Trebuie sa ascult----------//
        case 3:
        {
            
        }
        break;

        // ---------Confirmare ca va executa-------- //
        case 1000:
        {
            // Astept confirmare ca v-a executa
            while(!confirmare_serial)
            {
                if(Serial.available() > 0)
                {
                    confirmare_serial = Serial.read();
                }
            }
        }
    }

    // Daca nu este nici un caz
    return 0;
}

//----------------------------------Telecomanda IR------------------------------------------------//
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

//----------------------------------Configurarea butoanelor------------------------------------------------//
void configurare_butoane()
{
    // Daca este un buton care nu are configuratie, incepe configurarea butoanelor
    if( !control[0] || !control[1] || !control[2] || !control[3] || !control[4] || !control[5])
    {
        Serial.println("Nu ai butoanele telecomenzii configurate.");
        comunicare(101);
        delay(500);
        Serial.println("Incepem sa configuram butoanele.");
        comunicare(102);
        configurare_butoane();
    }
    else
    {
        Serial.println("Ai butoane configurate.");
        comunicare(120);        
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
                Serial.println("Incepem sa configuram butoanele.");
                comunicare(104);
                Serial.println("");
                incepe_configurare_butoane();
            }
            if(valoare == control[0])
            {
                Serial.println("Ai pastrat configurarea butoanelor.");
                comunicare(121);
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
        comunicare(103);
        if( setare_buton(&control[0]) )
        {
            Serial.println("Ai setat butonul 'Bine'.");
            comunicare(104);
            Serial.println("");
            flag = 0;
        }              
    }while(flag);
    flag = 1;
    // --------------------------Anulare[1]--------------------------//
    do
    {
        Serial.println("Apasa o tasta pentru seta butonul `Anulare`.");
        comunicare(105);
        if( setare_buton(&control[1]) )
        {
            if(control[1] != control[0])
            {
                Serial.println("Ai setat butonul 'Anulare'.");
                comunicare(106);
                Serial.println("");
                flag = 0;
            }
            else
            {
                Serial.println("Acesta tasta este deja utilizata.");
                Serial.println("Te rog sa incerci din nou o alta tasta");
                comunicare(150);
            }
            
        }       
    }while(flag);
    flag = 1;
    // --------------------------Inainte[2]--------------------------//
    do
    {
        flag_error = 0;
        
        Serial.println("Apasa o tasta pentru seta butonul `Inainte`.");
        comunicare(107);
        if( setare_buton(&control[2]) )
        {
            for(byte i = 0; i < 2; i++)
            {
                if( control[2] == control[i] )
                {
                    Serial.println("Acesta tasta este deja utilizata.");
                    Serial.println("Te rog sa incerci din nou o alta tasta");
                    comunicare(150);
                    flag_error = 1;
                    continue;
                }
            }
            
            if(!flag_error)
            {
                Serial.println("Ai setat butonul 'Inainte'.");
                Serial.println("");
                comunicare(108);
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
        comunicare(109);
        if( setare_buton(&control[3]) )
        {
            for(byte i = 0; i < 3; i++)
            {
                if( control[3] == control[i] )
                {
                    Serial.println("Acesta tasta este deja utilizata.");
                    Serial.println("Te rog sa incerci din nou o alta tasta");
                    comunicare(150);
                    flag_error = 1;
                    continue;
                }
            }
            
            if(!flag_error)
            {
                Serial.println("Ai setat butonul 'Inapoi'.");
                Serial.println("");
                comunicare(110);
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
        comunicare(111);
        if( setare_buton(&control[4]) )
        {
            for(byte i = 0; i < 4; i++)
            {
                if( control[4] == control[i] )
                {
                    Serial.println("Acesta tasta este deja utilizata.");
                    Serial.println("Te rog sa incerci din nou o alta tasta");
                    comunicare(150);
                    flag_error = 1;
                    continue;
                }
            }
            
            if(!flag_error)
            {
                Serial.println("Ai setat butonul 'Stanga'.");
                Serial.println("");
                comunicare(112);
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
        comunicare(113);
        if( setare_buton(&control[5]) )
        {
            for(byte i = 0; i < 5; i++)
            {
                if( control[5] == control[i] )
                {
                    Serial.println("Acesta tasta este deja utilizata.");
                    Serial.println("Te rog sa incerci din nou o alta tasta");
                    comunicare(150);
                    flag_error = 1;
                    continue;
                }
            }
            
            if(!flag_error)
            {
                Serial.println("Ai setat butonul 'Dreapta'.");
                Serial.println("");
                comunicare(114);
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
                    comunicare(151);
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
                    comunicare(152);
                    return 0;    
                }                       
            }while(1);
        }
    }while(1);
}
