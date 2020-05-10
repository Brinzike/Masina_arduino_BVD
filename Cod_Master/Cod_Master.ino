#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//----------INCLUDE C----------//
#include "coduri_S-M_BVD.c"
#include "ceas_BVD.c"

//----------WIFI----------//
const char* ssid = "iancu-wifi";
const char* password = "bglnpbcr1";
ESP8266WebServer server(80);

//----------Display----------//
LiquidCrystal_I2C lcd(0x3F, 20, 4);
String temp = "0";
String linie_0 = "0";
String linie_1 = "0";
String linie_2 = "0";
String linie_3 = "0";
byte pozitie[4];
byte numaratoare = 1;

//----------Evitare obstacole----------//
float masuratoare[7];// 3 = 90grade , 2 = 75 , 1 = 60 , 0 = 45, 4 = 105, 5 = 120, 6 = 135
byte timp_rotire_servo = 100;
unsigned int secunde_precedente = 0;
byte nivel_viteza = 155;
boolean tic_tac_viteza = 0;

//----------Ceas----------//
byte zeci_secunde = 0;
byte zeci_secunde_precedente = 0;
boolean tic_tac_afisare = 0;
unsigned int secunde_curente = 0;
boolean tic_tac_secunde = 0;

//----------DECLARARE VARIABILE----------//
const int16_t I2C_SLAVE = 0x08;

//========================================SETUP========================================//
void setup() 
{
    lcd.begin();
    Wire.begin();
    Serial.begin(115200);
    adauga_eveniment(" A pornit sistenuml :))  ");
    delay(500);
    
    // ---------------------- Connect WiFi------------------//
    WiFi.begin(ssid, password);
    adauga_eveniment(" Astept sa ma conectez...  ");
    while(WiFi.status() != WL_CONNECTED )
    {
        delay(500);
    }
    if( WiFi.status() == WL_CONNECTED)
    {
        adauga_eveniment(" WiFi Conectat!  ");
        delay(500);
        
        adauga_eveniment(" Adresa IP: ");
        temp = " " + IpAddress2String(WiFi.localIP());
        adauga_eveniment(temp);
        delay(500);
        temp = "0";       
    }
    else
    {
        adauga_eveniment(" Eroare!  ");
        adauga_eveniment(" Nu m-am conectat la WiFi  ");
        delay(1000);        
    }

    Wire.begin();
    adauga_eveniment(" A pornit I2C  ");
}
//========================================END SETUP========================================//
//-----------------------------------------------------------------------------------------//
//========================================MAIN=============================================//
void loop() 
{
    ceas(&zeci_secunde, &secunde_curente, &tic_tac_secunde);

    if( zeci_secunde == 3 )
    {
        if( tic_tac_afisare == 0 )
        {
            cerere_slave();
            afiseaza(linie_0, linie_1, linie_2, linie_3, pozitie, &numaratoare);
            tic_tac_afisare = 1;
        }
    }
    else if( zeci_secunde == 8 )
    {
        if( tic_tac_afisare == 0 )
        {
            afiseaza(linie_0, linie_1, linie_2, linie_3, pozitie, &numaratoare);
            tic_tac_afisare = 1;
        }
        
    }
    else
    {
        Serial.println("A intrat in else");
        tic_tac_afisare = 0;
    }

    
}
//========================================END MAIN========================================//
//----------------------------------------------------------------------------------------//
//========================================FUNCTII=========================================//
//------------------------------Comunicare----------------------------------//
void cerere_slave()
{    
    Wire.requestFrom(I2C_SLAVE, 1);    // request 1 bytes from slave device #8
    while (Wire.available()) 
    {
        byte cod = Wire.read(); // receive a byte as character
        switch(cod)
        {
            case 0:
            {
                // nu face nimic
            }
            break;

            case 1:
            {
                temp = " Partenerul este gata  ";
            }
            break;

            case 101:
            {
                temp = " Nu ai butoanele configurate  ";
            }
            break;
            
            case 102:
            {
                temp = " Incepem sa configuram butoanele  ";
            }
            break;
            
            case 103:
            {
                temp = " Ai butoane configurate  ";
                adauga_eveniment(temp);
                temp = "0";
                temp = " Daca doresti sa continui cu butoanele configurate, apasa 'Bine'  ";
                adauga_eveniment(temp);
                temp = "0";
                temp = " Daca doresti sa configurezi alta telecomanda apasa orice tasta  ";
            }
            break;
            
            case 104:
            {
                temp = " Ai pastrat configurarea butoanelor  ";
            }
            break;
            
            case 105:
            {
                temp = " Apasa o tasta pentru 'Bine'  ";
            }
            break;
            
            case 106:
            {
                temp = " Ai setat butonul 'Bine' ";
            }
            break;
            
            case 107:
            {
                temp = " Apasa o tasta pentru 'Anulare'  ";
            }
            break;
            
            case 108:
            {
                temp = " Ai setat butonul 'Anulare' ";
            }
            break;
            
            case 109:
            {
                temp = " Apasa o tasta pentru seta butonul `Inainte`  ";
            }
            break;
            
            case 110:
            {
                temp = " Ai setat butonul 'Inainte'  ";
            }
            break;
            
            case 111:
            {
                temp = " Apasa o tasta pentru seta butonul `Inapoi`  ";
            }
            break;
            
            case 112:
            {
                temp = " Ai setat butonul 'Inapoi'  ";
            }
            break;
            
            case 113:
            {
                temp = " Apasa o tasta pentru seta butonul `Stanga`  ";
            }
            break;
            
            case 114:
            {
                temp = " Ai setat butonul 'Stanga'  ";
            }
            break;
            
            case 115:
            {
                temp = " Apasa o tasta pentru seta butonul `Dreapta`  ";
            }
            break;
            
            case 116:
            {
                temp = " Ai setat butonul 'Dreapta'  ";
            }
            break;
            
            case 117:
            {
                temp = " Am primit un semnal  ";
                adauga_eveniment(temp);
                temp = "0";
                temp = " Apasa pe acelasi buton pentru a confirma  ";
            }
            break;
            
            case 118:
            {
                temp = " Nu a fost acelasi buton  ";
            }
            break;
            
            case 150:
            {
                temp = " Acesta tasta este deja utilizata  ";
                adauga_eveniment(temp);
                temp = "0";
                temp = " Te rog sa incerci din nou o alta tasta  ";
            }
            break;
            
            case 151:
            {
                temp = " A intervenit o eroare  ";
            }
            break;
            
            case 152:
            {
                temp = " Mai incercam o data sa setam butonul ";
            }
            break;

            default:
            {
                temp = " Nu am codul pe care l-a trimis partenerul  ";
            }
        }   

        if( cod )
        {
            adauga_eveniment(temp);
            temp = "0";
        }
    }
}
//------------------------------Ultrasunete----------------------------------//
//float masoara_distanta_cm()
//{
//    // Precizie masurata +- 0.50 cm 
//    float distanta[3] = {0.0, 0.0, 0.0};
//    long durata;
//    float eroare_calcul = 1.24; // Am o eroare de calcul de aprox de 1.29 ori mai putin
//    float eroare_pozitionare = 0.0; // Distanta de la botul masinutei la senzor
//
//    byte i = 0;
//    while( !distanta[2] ) // pana cand face ce-a de-a treia masuratoare
//    {
//        // Asteptam sa se faca liniste
//        digitalWrite(PIN_TRIG, LOW);
//        delayMicroseconds(15);
//        
//        // Trimitem inpulsurile
//        digitalWrite(PIN_TRIG, HIGH);
//        delayMicroseconds(10);
//        digitalWrite(PIN_TRIG, LOW);
//        
//        // Citim PIN_ECHO
//        durata = pulseIn(PIN_ECHO, HIGH);
//        
//        // speedofsound = 331.5+(0.607*temperature);
//        // distance = duration*speedofsound/10000/2;
//        // distanta reala = (distance * eroare_calcul) - eroare pozitionare
//        distanta[i] = ( (durata * (331.5 + (0.607 * temperatura) )/ 20000) * eroare_calcul - eroare_pozitionare );
//        i++;
//    }
//
//    // comparam cele 3 distante
//    // Selectam doar distantele apropiate (difernata < 2 cm) cel putin 2
//    if( modul(distanta[0] - distanta[1]) < 2.0) // se iau ambele distante
//    {
//        if( modul(distanta[0] - distanta[2]) < 2.0 ) // si a treia se ia in calcul
//        {
//            return ( (distanta[0] + distanta[1] + distanta[2]) / 3);
//        }
//        else // nu se ia si a treia distanta
//        {
//            return( (distanta[0] + distanta[1]) / 2);
//        }
//    }
//    else if( modul(distanta[1] - distanta[2]) < 2.0) // se iau doar a doua si a treia
//    {
//        return( (distanta[1] + distanta[2]) / 2);
//    }
//    else
//    {
//        return 0;
//    }
//
//    // in caz ca trece peste if unexpected
//    return 0;
//}
//
//byte modul(float termen)
//{
//    if(termen < 0.0)
//    {
//        return ( termen * (-1.0));
//    }
//    else
//    {
//        return termen;
//    }
//}
//
//void fa_masuratorile()
//{
//    byte j = 3; // 3 = masuratoare la 90 Greade
//    for( byte i = 90; i <= 135; i +=15) // spre stanga
//    {
////        servo.write(i);
//        delay(timp_rotire_servo);
//        masuratoare[j] = masoara_distanta_cm();
//        j++;      
//    }
//
//    j = 3;
//    for( byte i = 90; i >= 45; i -=15) // spre dreapta
//    {
////        servo.write(i);
//        delay(timp_rotire_servo);
//        masuratoare[j] = masoara_distanta_cm();
//        j--;      
//    }
////    servo.write(90);
//    delay(timp_rotire_servo);
//}

//------------------------------Display----------------------------------//
void adauga_eveniment(String eveniment)
{
    if( linie_3[0] == '0' )
    {
        if( linie_2[0] == '0')
        {
            if( linie_1[0] == '0' )
            {
                if( linie_0[0] == '0' )
                {
                    linie_0 = eveniment;
                }
                else
                {
                    linie_1 = eveniment;
                }
            }
            else
            {
                linie_2 = eveniment;
            }
        }
        else
        {
            linie_3 = eveniment;
        }
    }
    else
    {
        linie_0 = "0";
        pozitie[0] = pozitie[1];
        linie_0 = linie_1;
        linie_1 = "0";
        pozitie[1] = pozitie[2];
        linie_1 = linie_2;
        linie_2 = "0";
        pozitie[2] = pozitie[3];
        linie_2 = linie_3;
        linie_3 = "0";
        pozitie[3] = 0;
        linie_3 = eveniment;
        numaratoare++;
    }

    if( (numaratoare + 3) == 99 )
    {
        linie_0 = " A trebuit sa dau refresh la ecran  ";
        numaratoare = 1;
        linie_1 = "0";
        linie_2 = "0";
        linie_3 = "0";
    }
    //afiseaza(linie_0, linie_1, linie_2, linie_3, pozitie, &numaratoare);
}

void afiseaza(String text_0, String text_1, String text_2, String text_3, byte poz[4], byte *ordine)
{
    lcd.clear();
    
    //----------Printeaza linia 0
    if(text_0[0] != '0')
    {
        if( *ordine < 10)
        {
            lcd.print("0");
        }
        lcd.print(*ordine);
        lcd.print(".");
        printeaza_linie(text_0, &poz[0]);  
    }
    else
    {
        lcd.print("00.");
        printeaza_linie(" Nu este nimic de afisat  ", &poz[0]);
    }

    //----------Printeaza linia 1
    if(text_1[0] != '0')
    {
        lcd.setCursor(0,1);
        if( (*ordine + 1) < 10)
        {
            lcd.print("0");
        }
        lcd.print((*ordine + 1));
        lcd.print(".");
        printeaza_linie(text_1, &poz[1]);
    } 

    //----------Printeaza linia 2
    if(text_2[0] != '0')
    {
        lcd.setCursor(0,2);
        if( (*ordine + 2) < 10)
        {
            lcd.print("0");
        }
        lcd.print((*ordine + 2));
        lcd.print(".");
        printeaza_linie(text_2, &poz[2]);
    }

    //----------Printeaza linia 2
    if(text_3[0] != '0')
    {
        lcd.setCursor(0,3);
        if( (*ordine + 3) < 10)
        {
            lcd.print("0");
        }
        lcd.print((*ordine + 3));
        lcd.print(".");
        printeaza_linie(text_3, &poz[3]);
    }
}

void printeaza_linie(String text, byte *pozitie)
{
    byte nr_caractere = 17;
    
    if( text.length() >= nr_caractere)
    {
        if( text.length() >= (*pozitie + nr_caractere) )
        {
            // Daca textul pana la pozitie este > nr_caractere
            for(byte i = *pozitie; i < (*pozitie + nr_caractere); i++)
            {      
                lcd.print(text[i]);
            }
        }
        else
        {
            for(byte i = *pozitie; i < text.length(); i++)
            {      
                lcd.print(text[i]);
                if( i == (text.length() - 1) )
                {
                    for(byte j = 0; j < (*pozitie - (text.length() - nr_caractere) ); j++)
                    {
                        lcd.print(text[j]);
                    }
                }
            }
            
        }
        
        *pozitie = *pozitie + 1;
        if( *pozitie == text.length() )
        {
           *pozitie = 0;
        }        
    }
    else
    {
        lcd.print(text);
    }
}

//------------------------------Comunicare--------------------------------//
String mesaje(byte codul)
{
    switch(codul)
    {
        case 0:
        {
            // nimic
        }
        break;
        
        case 1:
        {
            String text = " Partenerul este gata  ";
            return text;
        }
        break;
    }
}

//------------------------------Convertiri--------------------------------//
String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ;
}
