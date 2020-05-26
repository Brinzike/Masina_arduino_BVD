//----------LIBRARII----------//
#include <IRremote.h>
#include <Wire.h>
//#include <EEPROM.h>
#include <FastLED.h>

//-----------INCLUDE C----------//
#include "control_motoare_BVD.c"
#include "ceas_BVD.c"

//----------SETARI------------//

//-----Setari telecomanda-----//
const byte PIN_IR_RECEPTIE = 2;
IRrecv irrecv(PIN_IR_RECEPTIE);
decode_results results;
int valoare_recive = 0;
int control[6] = {-32131,765,-22441,-30601,2295,18615}; // Bine = 0, Schimba modul = 1, Inainte = 2, Inapoi = 3, Stanga = 4, Dreapta = 5;
boolean tic_tac_comanda = 0;

//-----Setare pini motoare-----//
const byte PINI_MOTOARE[] = {9,10,5,6}; // (PWM) M_stanga_+ = 9(B-1A), M_stanga_- = 10(B-1B), M_dreapta_+ = 5(A-1A), M_dreapta_- = 6(A-1B)

//-----Ceas-------------------//
byte zeci_secunde = 0;
unsigned int secunde_curente = 0;
boolean tic_tac_secunde = 0;

//-----Comunicare I2C--------//
const int16_t I2C_SLAVE = 0x08;
byte cod[100];
byte cod_primit = 0;
byte last_cod = 0;

//-----Urmaritor linie-------//
const byte PIN_SENZOR_IR[] = {A0, A1, A2}; // 0 = Stanga , 1 = Mijloc, 2 = Dreapta
unsigned short valori_senzori_ir[3];
byte valoare_detectare_linie = 100;
byte cazul_precedent = 0;

//-----FastLED------------//
#define LED_PIN 3
#define NUM_LEDS 8
CRGB semnalizare[NUM_LEDS];

//-----Variabile------------//
byte mod = 0;
const byte PIN_POWER_MOTOR = 4; 
byte eroare_inaintare = 0;

//========================================SETUP========================================//
void setup() 
{ 
    //----------FastLED------------------------------//
    FastLED.addLeds<WS2812, LED_PIN, GRB>(semnalizare, NUM_LEDS);
    semnalizare[0] = CRGB(0, 50, 0);
    delay(400);
    FastLED.show();

    //----------Comunicare------------------------------//
    Serial.begin(115200);
    semnalizare[1] = CRGB(0, 50, 0);
    FastLED.show();
    delay(400);
    
    Wire.begin(I2C_SLAVE);
    semnalizare[2] = CRGB(0, 50, 0);
    FastLED.show();
    delay(400);
    
    Wire.onRequest(requestEvent);
    Wire.onReceive(receiveEvent);
    
    semnalizare[3] = CRGB(0, 50, 0);
    FastLED.show();
    delay(400);

    //----------SETUP pini------------------------------//
    pinMode(PIN_POWER_MOTOR, OUTPUT);
    digitalWrite(PIN_POWER_MOTOR, HIGH);
    
    for(byte i = 0; i < 4; i++)
    {
        pinMode(PINI_MOTOARE[i], OUTPUT);
    }

    for(byte i = 0; i < 3; i++)
    {
        pinMode(PIN_SENZOR_IR[i], INPUT);
    }
    
    semnalizare[4] = CRGB(0, 50, 0);
    FastLED.show();
    delay(400);
    
    //----------Start IR------------------------------//
    irrecv.enableIRIn();
    update_cod(1); // A terminat initializarea
    semnalizare[5] = CRGB(0, 50, 0);
    FastLED.show();
    delay(400);
    
    //----------SETUP butoane------------------------------//
    configurare_butoane();  
    semnalizare[6] = CRGB(0, 50, 0);
    FastLED.show();
    delay(400);

    //----------Start------------------------------//
    digitalWrite(PIN_POWER_MOTOR, LOW);
    semnalizare[7] = CRGB(0, 50, 0);
    FastLED.show();
    delay(400);
    Serial.println("A pornit");
}

void requestEvent() 
{
    Wire.write(cod[0]);
    // Update cod
    for(byte i = 0; i < 99; i ++)
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
    if( cod[99] != 0 )
    {
        cod[99] = 0;
    }
}

void receiveEvent(int howMany) // nu stiu de ce este howMany
{
    cod_primit = Wire.read();
}

//========================================END SETUP========================================//
//#########################################################################################//
//========================================MAIN=============================================//
void loop() 
{
    ceas(&zeci_secunde, &secunde_curente, &tic_tac_secunde);
    
    citeste_senzorii();
    if(valori_senzori_ir[0] > 400 || valori_senzori_ir[1] > 400 || valori_senzori_ir[2] > 400 ) // Un senzor a iesit de pe masa
    {
        // Stop
        mergi(0, 0, 0);
        update_cod(99);
        eroare_inaintare = 1;
    }
    else
    {
        eroare_inaintare = 0;
    }
    verifica_mod();
     
    delay(20);
}
//========================================END MAIN========================================//
//#########################################################################################//
//========================================FUNCTII=========================================//
//------------------------------Comunicare--------------------//
void update_cod(byte codul)
{
    // executa pana cand se elibereaza ultima pozitie
    while( cod[99] != 0 )
    {        
    }
    
    for( byte i = 0; i < 99; i++ )
    {
        if(cod[i] == 0 )
        {
            cod[i] = codul;
            break;
        }
    }
}

void mergi(byte cod, byte orientare, unsigned short viteza )
{    
    if(last_cod != cod && mod == 0)
    {
        last_cod = cod;
        update_cod(cod);
    }
    switch(orientare)
    {
        case 0:// Repaus
        {
            semnalizare[0] = CRGB(255,0,0);
            semnalizare[1] = CRGB(0,0,0);
            semnalizare[2] = CRGB(0,0,0);
            semnalizare[3] = CRGB(255,0,0);
            semnalizare[4] = CRGB(255,0,0);
            semnalizare[5] = CRGB(0,0,0);
            semnalizare[6] = CRGB(0,0,0);
            semnalizare[7] = CRGB(255,0,0);
            FastLED.show();
        }
        break;
        
        case 2: // Inainte
        {
            semnalizare[0] = CRGB(30,0,0);
            semnalizare[1] = CRGB(0,0,0);
            semnalizare[2] = CRGB(0,0,0);
            semnalizare[3] = CRGB(0,0,0);
            semnalizare[4] = CRGB(0,0,0);
            semnalizare[5] = CRGB(0,0,0);
            semnalizare[6] = CRGB(0,0,0);
            semnalizare[7] = CRGB(30,0,0);
            FastLED.show();
        }
        break;

        case 3: // Inapoi
        {
            semnalizare[0] = CRGB(30,0,0);
            semnalizare[1] = CRGB(0,0,0);
            semnalizare[2] = CRGB(200,200,200);
            semnalizare[3] = CRGB(200,200,200);
            semnalizare[4] = CRGB(200,200,200);
            semnalizare[5] = CRGB(200,200,200);
            semnalizare[6] = CRGB(0,0,0);
            semnalizare[7] = CRGB(30,0,0);
            FastLED.show();
        }
        break;

        case 6: // Stanga
        {
            semnalizare[0] = CRGB(150,130,0);
            semnalizare[1] = CRGB(150,130,0);
            semnalizare[2] = CRGB(0,0,0);
            semnalizare[3] = CRGB(0,0,0);
            semnalizare[4] = CRGB(0,0,0);
            semnalizare[5] = CRGB(0,0,0);
            semnalizare[6] = CRGB(0,0,0);
            semnalizare[7] = CRGB(30,0,0);
            FastLED.show();
        }
        break;

        case 7: // Dreapta
        {
            semnalizare[0] = CRGB(30,0,0);
            semnalizare[1] = CRGB(0,0,0);
            semnalizare[2] = CRGB(0,0,0);
            semnalizare[3] = CRGB(0,0,0);
            semnalizare[4] = CRGB(0,0,0);
            semnalizare[5] = CRGB(0,0,0);
            semnalizare[6] = CRGB(150,130,0);
            semnalizare[7] = CRGB(150,130,0);
            FastLED.show();
        }
        break;   
    }
    directie(orientare, viteza, PINI_MOTOARE);
}

//------------------------------Verificare mod--------------------//
void verifica_mod()
{
    int comanda = 0; 
    // Face sa mearga controlul de la telecomanda mai bine
    if( mod == 0 )
    { 
        comanda = receptie();
        delay(200);
    }
    else if( mod >= 1 )
    {
        if( zeci_secunde % 3 == 1)
        {
            if( tic_tac_comanda == 0 )
            {
                comanda = receptie();
                if(comanda)
                {
                    tic_tac_comanda = 1;
                }
            }
        }
        else
        {
            tic_tac_comanda = 0;
        }
    }
    
    if( comanda == control[0] )
    {
        mod++;
        if( mod == 3 )
        {
            mod = 0;
        }
        update_cod( (50 + mod) );
    }

    switch(mod)
    {
        case 0:
        {
            control_telecomanda(comanda);
        }
        break;

        case 1:
        {
            urmareste_linia();
        }
        break;

        case 2:
        {
            evita_obstacolele();
        }
        break;
    }
}

//------------------------------Telecomanda IR--------------------//
int receptie()
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
            delay(300);
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
    while( flag_error )
    {
        if( verifica_buton(0, 105) )
        {
            flag_error = 0;
        }
        else
        {
            update_cod(151);
            update_cod(152);      
        }
    }
    update_cod(106);
    flag_error = 1;
    // --------------------------Anulare[1]--------------------------//
    while( flag_error )
    {
        if( verifica_buton(1, 107) )
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
        if( verifica_buton(3, 111) )
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
        if( verifica_buton(4, 113) )
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
        if( verifica_buton(5, 115) )
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
            if( nr_buton )
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
                        break;
                    }
                    // Daca nu pleaca mai departe cu flag 0
                }
            } 
            else
            {
                return 1;
            }
        }
    } 
    return 1;
}

boolean setare_buton(int *buton)
{
    delay(300);
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

//------------------------------Control Telecomanda IR--------------------//
void control_telecomanda(int comanda)
{
    if( comanda == control[2] ) // Inainte
        {
            if( eroare_inaintare == 0 )
            {
                mergi(121, 2, 200);
            }
        }
        else if( comanda == control[3] ) // Inapoi
        {
            mergi(122, 3, 170);
        }
        else if( comanda == control[4] ) // Vireaza Stanga
        {
            if( eroare_inaintare == 0 )
            {
                mergi(125, 6, 0);
            }
        }
        else if( comanda == control[5] ) // Vireaza Dreapta
        {
            if( eroare_inaintare == 0 )
            {
                mergi(126, 7, 0);
            }
        }
        else // Stop
        {
            mergi(120, 0, 0);
        }
}

//------------------------------Urmaritor linie--------------------//
void urmareste_linia()
{
    if( valori_senzori_ir[0] > valoare_detectare_linie && valori_senzori_ir[1] < valoare_detectare_linie && valori_senzori_ir[2] < valoare_detectare_linie ) // Este pe stanga
    {
        mergi(0, 6, 160);
        cazul_precedent = 1;
    }
    else if( valori_senzori_ir[0] < valoare_detectare_linie && valori_senzori_ir[1] < valoare_detectare_linie && valori_senzori_ir[2] > valoare_detectare_linie ) // Este pe dreapta
    {
        mergi(0, 7, 160);
        cazul_precedent = 2;
    }
    else if( valori_senzori_ir[0] < valoare_detectare_linie && valori_senzori_ir[1] > valoare_detectare_linie && valori_senzori_ir[2] < valoare_detectare_linie ) // Este pe mijloc
    {
        mergi(0, 2, 180);
        cazul_precedent = 3;
    }
    else if( valori_senzori_ir[0] > valoare_detectare_linie && valori_senzori_ir[1] > valoare_detectare_linie && valori_senzori_ir[2] > valoare_detectare_linie ) // Este pe toti senzorii
    {
        mergi(0, 0, 0);
    }
    else if( valori_senzori_ir[0] < valoare_detectare_linie && valori_senzori_ir[1] < valoare_detectare_linie && valori_senzori_ir[2] < valoare_detectare_linie ) // Nu mai este pe nici un senzor
    {
        if( cazul_precedent == 1 ) // A fost pe stanga
        {
            mergi(0, 6, 0);
        }
        else if( cazul_precedent == 2 ) // A fost pe dreapta
        {
            mergi(0, 7, 0);
        }
    }

    // gaseste linie pe doi dintre senzori
    // Nu gaseste nici o linie
}

void citeste_senzorii()
{
    Serial.print(" Valori senzori : ");
    for(byte i = 0; i < 3; i++)
    {
        valori_senzori_ir[i] = analogRead(PIN_SENZOR_IR[i]);
        Serial.print(valori_senzori_ir[i]);
        Serial.print(" ");
    }
    Serial.println("");
}

//------------------------------Evita Obstacolele--------------------//
void evita_obstacolele()
{
    // Cand primeste un cod executa pana ii vine alt cod
    switch(cod_primit)
    {
        case 200: // Stop
        {
            mergi(120, 0, 255);
        }
        break;

        case 201: // Inainte Usor
        {
            mergi(121, 2, 150);
        }
        break;

        case 202: // Inainte Full
        {
            mergi(121, 2, 255);
        }
        break;

        case 203: // Inapoi usor
        {
            mergi(122, 3, 150);
        }
        break;

        case 204: // Inapoi full
        {
            mergi(122, 3, 255);
        }
        break;

        case 205: // Stanga usor
        {
            mergi(123, 6, 160);
        }
        break;

        case 206: // Dreapta usor
        {
            mergi(124, 7, 160);
        }
        break;

        case 207: // Stanga maxim
        {
            mergi(125, 6, 0);
        }
        break;

        case 208: // Dreapta maxim
        {
            mergi(126, 7, 0);
        }
        break;

        case 209: // Rotire spre Stanga
        {
            mergi(125, 4, 255);
        }
        break;

        case 210: // Rotire spre Dreapta
        {
            mergi(126, 5, 255);
        }
        break;

        default:
        break;
    }
    
    
}
