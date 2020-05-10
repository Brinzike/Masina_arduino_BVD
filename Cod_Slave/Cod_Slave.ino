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
int control[6] = {-32131,765,-22441,-30601,2295,18615}; // Bine = 0, Schimba modul = 1, Inainte = 2, Inapoi = 3, Stanga = 4, Dreapta = 5;

// Setare pini motoare
const byte PINI_MOTOARE[] = {9,10,5,6}; // (PWM) M_stanga_+ = 9(B-1B), M_stanga_- = 10(B-1A), M_dreapta_+ = 5(A-1A), M_dreapta_- = 6(A-1B)

// Ceas
byte zeci_secunde = 0;
unsigned int secunde_curente = 0;
boolean tic_tac_secunde = 0;

// Comunicare I2C
const int16_t I2C_SLAVE = 0x08;
byte cod[10] = {0,0,0,0,0,0,0,0,0,0};

//========================================SETUP========================================//
void setup() 
{ 
    
    Serial.begin(115200);;
    Wire.begin(I2C_SLAVE);
    Wire.onRequest(requestEvent);

    // --SETUP pini------------------------------//
    for(byte i = 0; i < 4; i++)
    {
        pinMode(PINI_MOTOARE[i], OUTPUT);
    }
    
    // --Start IR------------------------------//
    irrecv.enableIRIn();
    
    update_cod(1);
    // --SETUP butoane------------------------------//
    configurare_butoane();  
}

void requestEvent() 
{
    Wire.write(cod[0]);
    // Update cod
    for(byte i = 0; i < 9; i ++)
    {
        if( cod[i + 1] != 0 )
        {
            cod[i] = cod[i + 1];
        }
        else
        {
            cod[i] = 0;
            break;
        }
    }
    if( cod[9] != 0 )
    {
        cod[9] = 0;
    }
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
//------------------------------Comunicare--------------------//
void update_cod(byte codul)
{
    // executa pana cand se elibereaza ultima pozitie
    while( cod[9] != 0 )
    {        
    }
    
    for( byte i = 0; i < 10; i++ )
    {
        if(cod[i] == 0 )
        {
            cod[i] = codul;
            break;
        }
    }
}
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
        update_cod(101);       
        incepe_configurare_butoane();
    }
    else
    {
        update_cod(103);

        boolean flag = 1;
        while(flag)
        {
            int valoare = receptie();
            if(valoare != control[0] && valoare != 0)
            {
                // Configureaza butoanele
                
                incepe_configurare_butoane();
                flag = 0;
            }else if(valoare == control[0])
            {
                // Pastreaza setarile
                update_cod(104);
                flag = 0;
            }
        }
    }
}

void incepe_configurare_butoane()
{
    update_cod(102);
    boolean flag_error = 1;
    // --------------------------Bine[0]--------------------------//
    while( !setare_buton(&control[0]) ) // 1- daca este este setat, 0- daca nu este setat
    {
        update_cod(105);
    }
    update_cod(106);
    // --------------------------Anulare[1]--------------------------//
    while( flag_error )
    {
        if( verifica_buton(2, 107) )
        {
            flag_error = 0;
        }
        else
        {
            update_cod(151);
            update_cod(152);      
        }
    }
    update_cod(108);
    flag_error = 1;
    // --------------------------Inainte[2]--------------------------//
    while( flag_error )
    {
        if( verifica_buton(2, 109) )
        {
            flag_error = 0;
        }
        else
        {
            update_cod(151);
            update_cod(152);      
        }
    }
    update_cod(110);
    flag_error = 1;
    // --------------------------Inapoi[3]--------------------------//
    while( flag_error )
    {
        if( verifica_buton(2, 111) )
        {
            flag_error = 0;
        }
        else
        {
            update_cod(151);
            update_cod(152);      
        }
    }
    update_cod(112);
    flag_error = 1;
    // --------------------------Stanga[4]--------------------------//
    while( flag_error )
    {
        if( verifica_buton(2, 113) )
        {
            flag_error = 0;
        }
        else
        {
            update_cod(151);
            update_cod(152);      
        }
    }
    update_cod(114);
    flag_error = 1;
    // --------------------------Dreapta[5]--------------------------//
    while( flag_error )
    {
        if( verifica_buton(2, 115) )
        {
            flag_error = 0;
        }
        else
        {
            update_cod(151);
            update_cod(152);      
        }
    }
    update_cod(116);
    flag_error = 1;
    // --------------------------Pnetru adaugare mai multe butoane--------------------------//
}

boolean verifica_buton(byte nr_buton, byte cod_mesaj)
{
    boolean flag = 1;     
    while( flag )
    {
        update_cod(cod_mesaj); 
        if( setare_buton(&control[nr_buton]) )
        {
            // verifica daca sunt alte butoane setate deja cu acest cod
            for(byte i = 0; i < nr_buton; i++)
            {
                flag = 0;
                // Daca gaseste un buton asemanator
                if( control[nr_buton] == control[i] )
                {
                    update_cod(150);
                    flag = 1;
                    continue;
                }
                // Daca nu pleaca mai departe cu flag 0
            }
        }
    } 
    return 1;
}

boolean setare_buton(int *buton)
{
    *buton = 0;
    boolean tic_tac = 0; 
    int confirmare = 0;
    while(1)
    {    
        *buton = receptie();
        delay(300); 
        if( *buton != 0)
        {
            while(1)
            {               
                if(!tic_tac)
                {
                    update_cod(117);
                    tic_tac = 1;
                } 
                confirmare = receptie();
                delay(300);          
                if(confirmare != 0 && confirmare == *buton)
                {
                    // Butonul a fost validat
                    return 1;
                }
                        
                if(confirmare != 0 && confirmare != *buton)
                {
                    // Nu a fost acelasi buton
                    update_cod(118);
                    return 0;    
                }                       
            }
        }
    }
}
