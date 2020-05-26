#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <Servo.h>

//----------INCLUDE C----------//
#include "coduri_S-M_BVD.c"
#include "ceas_BVD.c"

//----------WIFI----------//
const char* ssid = "Brinzike-WiFi";
const char* password = "password";
ESP8266WebServer server(80);
boolean WiFi_conectat = 0;

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
#define DHTPIN D5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float temperatura;
const byte PIN_TRIG = D6;
const byte PIN_ECHO = D7;
Servo servo;
boolean tic_tac_send = 0;
float masuratoare[7];// 3 = 90grade , 2 = 75 , 1 = 60 , 0 = 45, 4 = 105, 5 = 120, 6 = 135
byte timp_rotire_servo = 100;
byte nivel_viteza = 155;
boolean tic_tac_viteza = 0;

//----------Ceas----------//
byte zeci_secunde = 0;
byte zeci_secunde_precedente = 0;
boolean tic_tac_afisare = 0;
unsigned int secunde_curente = 0;
unsigned int secunde_precedente = 0;
boolean tic_tac_secunde = 0;

//----------Comunicare----------//
const int16_t I2C_SLAVE = 0x08;
byte last_cod = 0;

//----------Variabile-----------//
byte mod = 0;

//========================================SETUP========================================//
void setup() 
{
    lcd.begin();
    Wire.begin();
    Serial.begin(115200);
    adauga_eveniment(" A pornit sistenuml :))  ");
    delay(500);

    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);

    servo.attach(D2);
    
    // ---------------------- Connect WiFi------------------//
    WiFi.begin(ssid, password);
    adauga_eveniment(" Astept sa ma conectez...  ");
    while(secunde_curente < 20 && !WiFi_conectat)
    {
        refresh_lcd();
        delay(50);
        
        if( WiFi.status() == WL_CONNECTED)
        {
            adauga_eveniment(" WiFi Conectat!  ");
            delay(500);
            
            adauga_eveniment(" Adresa IP: ");
            temp = " " + IpAddress2String(WiFi.localIP());
            adauga_eveniment(temp);
            delay(500);
            temp = "0";  
            WiFi_conectat = 1;     
        }
    }
    
    if(!WiFi_conectat)
    {
        adauga_eveniment(" Nu s-a conectat");
        adauga_eveniment(" Timp expirat");
    }

    dht.begin();
    Wire.begin();
    adauga_eveniment(" A pornit I2C  ");
}
//========================================END SETUP========================================//
//-----------------------------------------------------------------------------------------//
//========================================MAIN=============================================//
void loop() 
{
    temperatura = dht.readTemperature();
    ceas(&zeci_secunde, &secunde_curente, &tic_tac_secunde);
    refresh_lcd(); 
    verifica_mod(); 
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
        byte cod_primit = Wire.read(); // receive a byte as character

        if( cod_primit > 0 && cod_primit < 200 )
        {
            mesaje(cod_primit);
            // Daca este 50 - 52 schimba modul
            if(cod_primit >= 50 && cod_primit <= 52)
            {
                mod = cod_primit - 50;
            }
        }
    }
}

void trimite_slave(byte cod_de_trimis)
{  
    if(zeci_secunde % 3 == 0)
    {
        if( tic_tac_send == 0 )
        {
            tic_tac_send = 1;
            if(cod_de_trimis != last_cod)
            {
                last_cod = cod_de_trimis;
                
                Wire.beginTransmission(I2C_SLAVE); // transmit to device #8
                Wire.write(cod_de_trimis);              // sends one byte
                Wire.endTransmission();    // stop transmitting
            }
        }
    }
    else
    {
        tic_tac_send = 0;
    }        
}

void mesaje(byte codul)
{
    switch(codul)
        {
            case 1:
            {
                temp = " Partenerul este gata  ";
            }
            break;

            case 50:
            {
                temp = " Telecomanda";
                mod = 0;
            }
            break;

            case 51:
            {
                temp = " Urmareste Linia";
                mod = 1;
            }
            break;

            case 52:
            {
                temp = " Evita Obstacole";
                mod = 2;
            }
            break;

            case 99:
            {
                temp = " Nu detectez suprafata  ";
            }
            break;

            case 101:
            {
                temp = " Butoane neconfigurate  ";
            }
            break;
            
            case 102:
            {
                temp = " Configuram butoanele  ";
            }
            break;
            
            case 103:
            {
                temp = " Ai butoane configurate  ";
                adauga_eveniment(temp);
                temp = "0";
                temp = " Pt continuare cu butoanele configurate, apasa 'Bine'  ";
                adauga_eveniment(temp);
                temp = "0";
                temp = " Pt conf alta telecomanda apasa orice tasta  ";
            }
            break;
            
            case 104:
            {
                temp = " Ai pastrat conf butoanelor  ";
            }
            break;
            
            case 105:
            {
                temp = " Apasa pt BINE";
            }
            break;
            
            case 106:
            {
                temp = " Setat BINE";
            }
            break;
            
            case 107:
            {
                temp = " Apasa pt ANULARE";
            }
            break;
            
            case 108:
            {
                temp = " Setat ANULARE";
            }
            break;
            
            case 109:
            {
                temp = " Apasa pt INAINTE";
            }
            break;
            
            case 110:
            {
                temp = " Setat INAINTE";
            }
            break;
            
            case 111:
            {
                temp = " Apasa pt INAPOI";
            }
            break;
            
            case 112:
            {
                temp = " Setat INAPOI";
            }
            break;
            
            case 113:
            {
                temp = " Apasa pt STANGA";
            }
            break;
            
            case 114:
            {
                temp = " Setat STANGA";
            }
            break;
            
            case 115:
            {
                temp = " Apasa pt DREAPTA";
            }
            break;
            
            case 116:
            {
                temp = " Setat DREAPTA";
            }
            break;
            
            case 117:
            {
                temp = " Am primit semnal";
                adauga_eveniment(temp);
                temp = "0";
                temp = " Apasa din nou";
            }
            break;
            
            case 118:
            {
                temp = " Nu a fost acelasi buton  ";
            }
            break;

            case 120:
            {
                temp = " Stop";
            }
            break;
            
            case 121:
            {
                temp = " Inainte";
            }
            break;

            case 122:
            {
                temp = " Inapoi";
            }
            break;

            case 123:
            {
                temp = " Usor Stanga";
            }
            break;

            case 124:
            {
                temp = " Usor Dreapta";
            }
            break;

            case 125:
            {
                temp = " Stanga";
            }
            break;

            case 126:
            {
                temp = " Dreapta";
            }
            break;
            
            case 150:
            {
                temp = " Tasta utilizata";
                adauga_eveniment(temp);
                temp = "0";
                temp = " Incearca alta tasta  ";
            }
            break;
            
            case 151:
            {
                temp = " A intervenit o eroare  ";
            }
            break;
            
            case 152:
            {
                temp = " Incercam inca o data sa setam butonul ";
            }
            break;

            default:
            {
                temp = " Nu am codul pe care l-a trimis partenerul  ";
            }
        }   

        adauga_eveniment(temp);
        temp = "0";
}

//------------------------------Verificare mod--------------------//
void verifica_mod()
{
    // Pentru mod 0 si 1 nu are nici o actiune, doar afiseaza mesajele
    if(mod == 2)
    {
        evita_obstacolele();
    }
}

//------------------------------Ultrasunete----------------------------------//
float masoara_distanta_cm()
{
    // Precizie masurata +- 0.50 cm 
    float distanta[3] = {0.0, 0.0, 0.0};
    long durata;
    float eroare_calcul = 1; //1.24; // Am o eroare de calcul de aprox de 1.24 ori mai putin
    float eroare_pozitionare = 0.0; // Distanta de la botul masinutei la senzor

    for(byte i = 0; i < 3; i++) // pana cand face ce-a de-a treia masuratoare
    {
        // Asteptam sa se faca liniste
        digitalWrite(PIN_TRIG, LOW);
        delayMicroseconds(2);
        
        // Trimitem inpulsurile
        digitalWrite(PIN_TRIG, HIGH);
        delayMicroseconds(10);
        digitalWrite(PIN_TRIG, LOW);
        
        // Citim PIN_ECHO
        durata = pulseIn(PIN_ECHO, HIGH);
        
        // speedofsound = 331.5+(0.607*temperature);
        // distance = duration*speedofsound/10000/2;
        // distanta reala = (distance * eroare_calcul) - eroare pozitionare
        if( temperatura != 0.0 )
        {
            distanta[i] = durata * (331.5 + (0.607 * temperatura) )/ 20000;
        }
        else
        {
            distanta[i] = durata * 0,03434 / 2;
        }
        
        if( distanta[i] < 3.0 )
        {
            distanta[i] = 0.0;
        }
    }

    // comparam cele 3 distante
    // Selectam doar distantele apropiate (difernata < 2 cm) cel putin 2
    if( modul(distanta[0] - distanta[1]) < 2.0) // se iau ambele distante
    {
        if( (modul(distanta[0] - distanta[2]) < 2.0) || (modul(distanta[1] - distanta[2]) < 2.0)) // si a treia se ia in calcul
        {
            return ( (distanta[0] + distanta[1] + distanta[2]) / 3);
        }
        else // nu se ia si a treia distanta
        {
            return( (distanta[0] + distanta[1]) / 2);
        }
    }
    else if( modul(distanta[0] - distanta[2]) < 2.0 ) // se iau doar a doua si a treia
    {
        return( (distanta[0] + distanta[2]) / 2);
    }
    else if( modul(distanta[1] - distanta[2]) < 2.0 ) // se ia prima si ultima valoare
    {
        return( (distanta[1] + distanta[2]) / 2);
    }
    else
    {
        return 0;
    }

    // in caz ca trece peste if (unexpected)
    return 0;
}

float modul(float termen)
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

void fa_masuratorile()
{
    byte j = 3; // 3 = masuratoare la 90 Greade
    for( byte i = 90; i <= 135; i +=15) // spre stanga
    {
        servo.write(i);
        delay(timp_rotire_servo);
        masuratoare[j] = masoara_distanta_cm();
        j++;      
    }

    j = 3;
    for( byte i = 90; i >= 45; i -=15) // spre dreapta
    {
        servo.write(i);
        delay(timp_rotire_servo);
        masuratoare[j] = masoara_distanta_cm();
        j--;      
    }
    servo.write(90);
    delay(timp_rotire_servo);
}

//------------------------------Display----------------------------------//
void refresh_lcd()
{
    ceas(&zeci_secunde, &secunde_curente, &tic_tac_secunde);
    if( zeci_secunde == 5 )
    {
        if( tic_tac_afisare == 0 )
        {
            cerere_slave();
            afiseaza(linie_0, linie_1, linie_2, linie_3, pozitie, &numaratoare);
            tic_tac_afisare = 1;
        }
    }
    else
    {
        tic_tac_afisare = 0;
    }
}

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
    afiseaza(linie_0, linie_1, linie_2, linie_3, pozitie, &numaratoare);
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
    
    if( text.length() > nr_caractere)
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

//------------------------------Evita Obstacolele--------------------//
void evita_obstacolele()
{
    servo.write(90);
    delay(timp_rotire_servo);
    float dist = masoara_distanta_cm();
    if(dist > 30.0)
    {
        trimite_slave(202);
    }
    else if(dist <= 30.0 && dist > 20.0)
    {
        trimite_slave(201);
    }
    else if( dist <= 10.0 && dist >= 3.0 )
    {
        trimite_slave(203);
    }
    else
    {
        trimite_slave(200);
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
