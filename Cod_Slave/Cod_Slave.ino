//----------------------------------LIBRARII-------------------------------------------------------------//
#include <IRremote.h>
#include <Wire.h>
#include <Servo.h>
#include <DHT.h>
//#include <EEPROM.h>

//----------------------------------INCLUDE C------------------------------------------------------------//
#include "control_motoare_BVD.c"
#include "ceas_BVD.c"

//----------------------------------SETARI PINI-----------------------------------------------------------//
// Setari telecomanda
const byte PIN_IR_RECEPTIE = 2;
IRrecv irrecv(PIN_IR_RECEPTIE);
decode_results results;
short valoare_recive = 0;
short control[6] = {-32131,765,-22441,-30601,2295,18615}; // Bine = 0, Anulare = 1, Inainte = 2, Inapoi = 3, Stanga = 4, Dreapta = 5;

// Setare pini motoare
const byte PINI_MOTOARE[] = {9,10,5,6}; // (PWM) M_stanga_+ = 9(B-1B), M_stanga_- = 10(B-1A), M_dreapta_+ = 5(A-1A), M_dreapta_- = 6(A-1B)

// Servo-motor
const byte PIN_SERVO = 11;
Servo servo;
byte pozitie = 90;// 0 = Dreapta, 90 = Centru, 180 = Stanga

// Ultrasunete
const byte PIN_TRIG = 8;
const byte PIN_ECHO = 7;

// DHT Senzor temperatura umiditate
#define DHTPIN 3
#define DHTTYPE DHT11
DHT dht = DHT(DHTPIN, DHTTYPE);
float temperatura = 0;

//----------------------------------DECLARARE VARIABILE----------------------------------------------------//
// ATENTIE, la mine sunt inversate
const boolean DAI[2] = {1,0}; // In loc de HIGH si LOW

// Ceas
byte zeci_secunde = 0;
unsigned int secunde_curente = 0;
boolean tic_tac_secunde = 0;

// Evitare obstacole
float masuratoare[7];// 3 = 90grade , 2 = 75 , 1 = 60 , 0 = 45, 4 = 105, 5 = 120, 6 = 135

//----------------------------------SETUP---------------------------------------------------------------------//

void setup() 
{ 
    Serial.begin(115200);    
    Serial.println("A pornit sistemul :))");

    // --SETUP pini------------------------------//
    for(byte i = 0; i < 4; i++)
    {
        pinMode(PINI_MOTOARE[i], OUTPUT);
        // Nu inteleg de ce a inceput sa merga pe dos si porneste din prima cu High
        digitalWrite(PINI_MOTOARE[i], DAI[0]);
    }    
    Serial.println("Pinii au fost setati.");

    // Pin Servo
    servo.attach(11);

    // Pini Ultrasunete
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
    
    // Senzor temperatura
    dht.begin();
    
    // --Start IR------------------------------//
    irrecv.enableIRIn();
    Serial.println("A pornit reciver-ul IR.");

//    comunicare(1);
    Serial.println("S-a stabilit conexiunea cu partenerul.");
    
    // --SETUP butoane------------------------------//
    //configurare_butoane();  

    fa_masuratorile();
    for(int i = 0; i < 7; i++)
    {
        Serial.print(masuratoare[i]);
        Serial.print("  ");
    }
}

//----------------------------------MAIN--------------------------------------------------------------------//
void loop() 
{
//    servo.write(90);
//    temperatura = dht.readTemperature();
//    Serial.print("Tempt = ");
//    Serial.print(temperatura);
//    Serial.print(" ");
//    Serial.println(masoara_distanta_cm());
//    //delay(200);
//    servo.write(pozitie);
//    Serial.println(masoara_distanta());
//    delay(timp);
//    servo.write(130);
//    Serial.println(masoara_distanta());
//    delay(timp);
//    servo.write(pozitie);
//    Serial.println(masoara_distanta());
//    delay(timp);
//    servo.write(50);
//    Serial.println(masoara_distanta());
//    delay(timp);

   
//    for(int i = 90; i <= 140; i += 10)
//    {
//        servo.write(i);
//        delay(timp/2);
//    }
//    servo.write(90);
//    for(int i = 90; i >= 40; i -= 10)
//    {
//        servo.write(i);
//        delay(timp/2);
//    }

}

//----------------------------------Ultrasunete------------------------------------------------//
float masoara_distanta_cm()
{
    // Precizie masurata +- 0.50 cm 
    float distanta[3] = {0.0, 0.0, 0.0};
    long durata;
    float eroare_calcul = 1.24; // Am o eroare de calcul de aprox de 1.29 ori mai putin
    float eroare_pozitionare = 0.0; // Distanta de la botul masinutei la senzor

    byte i = 0;
    while( !distanta[2] ) // pana cand face ce-a de-a treia masuratoare
    {
        // Asteptam sa se faca liniste
        digitalWrite(PIN_TRIG, LOW);
        delayMicroseconds(15);
        
        // Trimitem inpulsurile
        digitalWrite(PIN_TRIG, HIGH);
        delayMicroseconds(10);
        digitalWrite(PIN_TRIG, LOW);
        
        // Citim PIN_ECHO
        durata = pulseIn(PIN_ECHO, HIGH);
        
        // speedofsound = 331.5+(0.607*temperature);
        // distance = duration*speedofsound/10000/2;
        // distanta reala = (distance * eroare_calcul) - eroare pozitionare
        distanta[i] = ( (durata * (331.5 + (0.607 * temperatura) )/ 20000) * eroare_calcul - eroare_pozitionare );
        i++;
    }

    // comparam cele 3 distante
    // Selectam doar distantele apropiate (difernata < 2 cm) cel putin 2
    if( modul(distanta[0] - distanta[1]) < 2.0) // se iau ambele distante
    {
        if( modul(distanta[0] - distanta[2]) < 2.0 ) // si a treia se ia in calcul
        {
            return ( (distanta[0] + distanta[1] + distanta[2]) / 3);
        }
        else // nu se ia si a treia distanta
        {
            return( (distanta[0] + distanta[1]) / 2);
        }
    }
    else if( modul(distanta[1] - distanta[2]) < 2.0) // se iau doar a doua si a treia
    {
        return( (distanta[1] + distanta[2]) / 2);
    }
    else
    {
        return 0;
    }

    // in caz ca trece peste if unexpected
    return 0;
}

byte modul(float termen)
{
    if(termen < 0.0)
    {
        return ( termen * (-1.0));
    }
    else
    {
        return termen;
    }
}

//----------------------------------Evitare obstacole------------------------------------------------//
void evitare_obstacole()
{
    // te uiti +- 10 grade stanga dreapta
        // te uiti la 90 Grade
        // faci masuratoare

        // te uiti la 80 Grade
        // faci masuratoare

        // te uiti la 100 Grade
        // faci masuratoare
    
    // compari rezultatele si trebuie sa afli care este ce-a mai apropiata de adevar
    
    // daca distanta > 30 cm
        // maresti viteza progresiv
        
    // daca distnata < 30 cm si > 20
        // incetinesti progresiv
        
    // daca distanta < 20 cm
        // fa toate masuratorile
            // decide in ce parte sa mergi
    
}

void fa_masuratorile()
{
    byte j = 3; // 3 = masuratoare la 90 Greade
    for( byte i = 90; i <= 135; i +=15) // spre stanga
    {
        servo.write(i);
        delay(150);
        masuratoare[j] = masoara_distanta_cm();
        j++;      
    }

    j = 3;
    for( byte i = 90; i >= 45; i -=15) // spre dreapta
    {
        servo.write(i);
        delay(150);
        masuratoare[j] = masoara_distanta_cm();
        j--;      
    }
    servo.write(90);
    delay(150);
}

//----------------------------------Telecomanda IR---------------------------------------------------------//
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
