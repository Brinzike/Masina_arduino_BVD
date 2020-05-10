//----------LIBRARII----------//
#include <IRremote.h>
#include <Wire.h>
//#include <EEPROM.h>

//-----------INCLUDE C----------//
#include "control_motoare_BVD.c"
#include "ceas_BVD.c"

//----------SETARI----------//
// Setari telecomanda
const byte PIN_IR_RECEPTIE = 2;
IRrecv irrecv(PIN_IR_RECEPTIE);
decode_results results;
short valoare_recive = 0;
short control[6] = {-32131,765,-22441,-30601,2295,18615}; // Bine = 0, Schimba modul = 1, Inainte = 2, Inapoi = 3, Stanga = 4, Dreapta = 5;

// Setare pini motoare
const byte PINI_MOTOARE[] = {9,10,5,6}; // (PWM) M_stanga_+ = 9(B-1B), M_stanga_- = 10(B-1A), M_dreapta_+ = 5(A-1A), M_dreapta_- = 6(A-1B)

// Ceas
byte zeci_secunde = 0;
unsigned int secunde_curente = 0;
boolean tic_tac_secunde = 0;

// Comunicare I2C
const int16_t I2C_SLAVE = 0x08;
byte cod = 0;

//========================================SETUP========================================//
void setup() 
{ 
    Serial.begin(115200);
    Wire.begin(I2C_SLAVE);
    Wire.onRequest(requestEvent);

    // --SETUP pini------------------------------//
    for(byte i = 0; i < 4; i++)
    {
        pinMode(PINI_MOTOARE[i], OUTPUT);
    }
    
    // --Start IR------------------------------//
    irrecv.enableIRIn();

    cod = 1;
    
    // --SETUP butoane------------------------------//
    //configurare_butoane();  

    cod = 1;
}

void requestEvent() 
{
    Wire.write(cod);
    cod = 0;
}
//========================================END SETUP========================================//
//-----------------------------------------------------------------------------------------//
//========================================MAIN=============================================//
void loop() 
{
  
}
//========================================END MAIN========================================//
//----------------------------------------------------------------------------------------//
//========================================FUNCTII=========================================//

//------------------------------Telecomanda IR--------------------//
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

//------------------------------Configurarea butoanelor-----------//
void configurare_butoane()
{
    // Daca este un buton care nu are configuratie, incepe configurarea butoanelor
    if( !control[0] || !control[1] || !control[2] || !control[3] || !control[4] || !control[5])
    {
        Serial.println("Nu ai butoanele telecomenzii configurate.");
//        comunicare(101);
        delay(500);
        Serial.println("Incepem sa configuram butoanele.");
//        comunicare(102);
        configurare_butoane();
    }
    else
    {
        Serial.println("Ai butoane configurate.");
//        comunicare(120);        
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
//                comunicare(104);
                Serial.println("");
                incepe_configurare_butoane();
            }
            if(valoare == control[0])
            {
                Serial.println("Ai pastrat configurarea butoanelor.");
//                comunicare(121);
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
//        comunicare(103);
        if( setare_buton(&control[0]) )
        {
            Serial.println("Ai setat butonul 'Bine'.");
//            comunicare(104);
            Serial.println("");
            flag = 0;
        }              
    }while(flag);
    flag = 1;
    // --------------------------Anulare[1]--------------------------//
    do
    {
        Serial.println("Apasa o tasta pentru seta butonul `Anulare`.");
//        comunicare(105);
        if( setare_buton(&control[1]) )
        {
            if(control[1] != control[0])
            {
                Serial.println("Ai setat butonul 'Anulare'.");
//                comunicare(106);
                Serial.println("");
                flag = 0;
            }
            else
            {
                Serial.println("Acesta tasta este deja utilizata.");
                Serial.println("Te rog sa incerci din nou o alta tasta");
//                comunicare(150);
            }
            
        }       
    }while(flag);
    flag = 1;
    // --------------------------Inainte[2]--------------------------//
    do
    {
        flag_error = 0;
        
        Serial.println("Apasa o tasta pentru seta butonul `Inainte`.");
//        comunicare(107);
        if( setare_buton(&control[2]) )
        {
            for(byte i = 0; i < 2; i++)
            {
                if( control[2] == control[i] )
                {
                    Serial.println("Acesta tasta este deja utilizata.");
                    Serial.println("Te rog sa incerci din nou o alta tasta");
//                    comunicare(150);
                    flag_error = 1;
                    continue;
                }
            }
            
            if(!flag_error)
            {
                Serial.println("Ai setat butonul 'Inainte'.");
                Serial.println("");
//                comunicare(108);
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
//        comunicare(109);
        if( setare_buton(&control[3]) )
        {
            for(byte i = 0; i < 3; i++)
            {
                if( control[3] == control[i] )
                {
                    Serial.println("Acesta tasta este deja utilizata.");
                    Serial.println("Te rog sa incerci din nou o alta tasta");
//                    comunicare(150);
                    flag_error = 1;
                    continue;
                }
            }
            
            if(!flag_error)
            {
                Serial.println("Ai setat butonul 'Inapoi'.");
                Serial.println("");
//                comunicare(110);
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
//        comunicare(111);
        if( setare_buton(&control[4]) )
        {
            for(byte i = 0; i < 4; i++)
            {
                if( control[4] == control[i] )
                {
                    Serial.println("Acesta tasta este deja utilizata.");
                    Serial.println("Te rog sa incerci din nou o alta tasta");
//                    comunicare(150);
                    flag_error = 1;
                    continue;
                }
            }
            
            if(!flag_error)
            {
                Serial.println("Ai setat butonul 'Stanga'.");
                Serial.println("");
//                comunicare(112);
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
//        comunicare(113);
        if( setare_buton(&control[5]) )
        {
            for(byte i = 0; i < 5; i++)
            {
                if( control[5] == control[i] )
                {
                    Serial.println("Acesta tasta este deja utilizata.");
                    Serial.println("Te rog sa incerci din nou o alta tasta");
//                    comunicare(150);
                    flag_error = 1;
                    continue;
                }
            }
            
            if(!flag_error)
            {
                Serial.println("Ai setat butonul 'Dreapta'.");
                Serial.println("");
//                comunicare(114);
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
//                    comunicare(151);
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
//                    comunicare(152);
                    return 0;    
                }                       
            }while(1);
        }
    }while(1);
}
