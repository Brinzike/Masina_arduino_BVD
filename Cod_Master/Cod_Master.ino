#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <Servo.h>

//----------INCLUDE C----------//
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
float masuratoare[7];// 3 = 90grade , 2 = 75 , 1 = 60 , 0 = 45, 4 = 105, 5 = 120, 6 = 135
byte nr_masuratoare = 2;
byte nr_masuratoare_precedenta = 4;
boolean tic_tac_masuratoare = 0;
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
boolean tic_tac_send = 0;

//----------Variabile-----------//
byte mod = 0;
byte cod_primit = 0;
boolean _start = 0;
byte eroare_inaintare = 0;

//========================================SETUP========================================//
void setup() 
{
    lcd.begin(); 
    Serial.begin(115200);
    adauga_eveniment(" A pornit sistenuml :))  ");
    delay(500);

    //----------Comunicare------------------------------//
    Wire.begin();
    adauga_eveniment(" A pornit I2C  ");
    delay(500);

    //----------SETUP pini------------------------------//
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);

    //---------------------- Connect WiFi------------------//
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

    //----------------------Evitare Obstacole------------------//
    dht.begin(); 
    servo.attach(D2);

    //----------------------Start------------------//
    while( cod_primit != 1 )
    {
        ceas(&zeci_secunde, &secunde_curente, &tic_tac_secunde);
        refresh_lcd();
        trimite_slave(250);          
    }
    
    cod_primit = 0;
    _start = 1;
    adauga_eveniment(" Start");
}
//========================================END SETUP========================================//
//-----------------------------------------------------------------------------------------//
//========================================MAIN=============================================//
void loop() 
{
    temperatura = dht.readTemperature();
    ceas(&zeci_secunde, &secunde_curente, &tic_tac_secunde);
    refresh_lcd(); 

//    verifica_obstacole();
    verifica_mod(); 

    delay(50);
}
//========================================END MAIN========================================//
//----------------------------------------------------------------------------------------//
//========================================FUNCTII=========================================//
//------------------------------Verificare inaintare----------------------------------//
void verifica_obstacole()
{
    switch(eroare_inaintare)
    {
        case 0: // Nu are nici un obstacol 
        {
            servo.write(90);
            delay(timp_rotire_servo);

            masuratoare[3] = masoara_distanta_cm();
            if( masuratoare[3] > 3.0 && masuratoare[3] < 20.0 ) // Are un obstacol
            {
                eroare_inaintare = 2;
                trimite_slave(252);
            }
            else
            {
                eroare_inaintare = 0;
                trimite_slave(250);
            }
        }
        break;

        case 1:
        {
            
        }
        break;

        case 2:
        {
            if( mod == 2 )
            {
                trimite_slave(250); 
            }   
        }
        break;
    }
}

//------------------------------Comunicare----------------------------------//
void cerere_slave()
{    
    Wire.requestFrom(I2C_SLAVE, 1);    // request 1 bytes from slave device #8
    while (Wire.available()) 
    {
        cod_primit = Wire.read(); // receive a byte as character
        if( _start ) // == 1
        {
            mesaje(cod_primit);
            cod_primit = 0;
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
            case 0: // nimic
            break;
            
            case 2:
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

            case 98:
            {
                temp = " Am un obstacol";
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
                //temp = " Stop";
            }
            break;
            
            case 121:
            {
                //temp = " Inainte";
            }
            break;

            case 122:
            {
                //temp = " Inapoi";
            }
            break;

            case 123:
            {
                //temp = " Usor Stanga";
            }
            break;

            case 124:
            {
                //temp = " Usor Dreapta";
            }
            break;

            case 125:
            {
                //temp = " Stanga";
            }
            break;

            case 126:
            {
                //temp = " Dreapta";
            }
            break;

            case 130:
            {
                temp = " Aseaza-ma pe suprafata fara linie";
                adauga_eveniment(temp);
                temp = "0";
                temp = " Si apasa OK";
            }
            break;

            case 131:
            {
                temp = " S Stanga pe linie";
            }
            break;

            case 132:
            {
                temp = " S Mijloc pe linie";
            }
            break;

            case 133:
            {
                temp = " S Dreapta pe linie";
            }
            break;

            case 134:
            {
                temp = " Aseaza-ma pe margine masa";
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

        if( temp != "0" )
        {
            adauga_eveniment(temp);
            temp = "0";
        }
}

//------------------------------Verificare mod--------------------//
void verifica_mod()
{
    // Pentru mod 0 si 1 nu are nici o actiune, doar afiseaza mesajele
    if(mod == 2)
    {
        evita_obstacole();
    }
}

//------------------------------Ultrasunete----------------------------------//
float masoara_distanta_cm()
{
    // Precizie masurata +- 0.50 cm 
    float distanta[3] = {0.0, 0.0, 0.0};
    long durata;

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
void evita_obstacole()
{
    servo.write(90);
    delay(timp_rotire_servo);
    float distanta = masoara_distanta_cm();

    if( distanta >= 30.0 ) // Inainte full
    {
        trimite_slave(202);
    }
    else if( distanta >= 20.0 && distanta < 30.0 ) // Ininte incet
    {
        trimite_slave(201);
    }
    else if( distanta >= 6.0 && distanta < 20.0 ) // Stop faci masuratori
    {
        trimite_slave(200);
        servo.write(60);
        delay(timp_rotire_servo);
        masuratoare[1] = masoara_distanta_cm();

        servo.write(120);
        delay(timp_rotire_servo);
        masuratoare[5] = masoara_distanta_cm();

        if( masuratoare[1] < masuratoare[5] ) // In stanga are mai mult loc
        {
            // Rotire Stanga
            trimite_slave(207);
            delay(50);
        }
        else
        {
            // Rotire Dreapta
            trimite_slave(208);
            delay(50);
        }

    }
    else if( distanta >= 3.0 && distanta < 6.0 ) // Inapoi
    {
        trimite_slave(203);
    }
    else // Stop
    {
        trimite_slave(200);
    }
    
}

void evita_obstacolele()
{
    // Intra cu nr_masuratoare = 2 si nr_masuratoare_precedenta = 4
    float dist = 0;

    servo.write(75 + ( (nr_masuratoare - 2) * 15) ); // Invarte la 75 apoi la 90 apoi la 105
    delay(timp_rotire_servo);

    masuratoare[nr_masuratoare] = masoara_distanta_cm(); // Masoara pt pozitia curenta

    // Compara si ia cea mai mica valoare valida
    byte termen_final = compara(nr_masuratoare, nr_masuratoare_precedenta);
    if( !termen_final ) // ==0
    {
        dist = 0;
    }
    else
    {
        dist = masuratoare[termen_final];
    }

    nr_masuratoare_precedenta = nr_masuratoare; 
    if( !tic_tac_masuratoare )
    {        
        nr_masuratoare++;
    }
    else
    {
        nr_masuratoare--;
    }

    if( nr_masuratoare == 4 )
    {
        tic_tac_masuratoare = 1;
    }
    else if( nr_masuratoare == 2 )
    {
        tic_tac_masuratoare = 0;
    }    
    
    if(dist > 30.0) // Full inainte
    {
        trimite_slave(202);
    }
    else if(dist <= 30.0 && dist > 20.0) // incet inainte
    {
        trimite_slave(201);
    }
    else if( dist <= 20.0 && dist > 10 ) // fa masuratorile si decide in ce directie sa iei
    {
        //fa_masuratorile();
        // 45Grade si 135 grade
        servo.write(45);
        delay( 2 * timp_rotire_servo );
        masuratoare[0] = masoara_distanta_cm();

        servo.write(135);
        delay( 2 * timp_rotire_servo );
        masuratoare[6] = masoara_distanta_cm();

        byte termen_mai_mic = compara(0,6);
    }
    else if( dist <= 10.0 && dist >= 3.0 ) // da in spate
    {
        trimite_slave(203);
    }
    else // Stop
    {
        trimite_slave(200);
    }

}

byte compara(byte nr_termen1, byte nr_termen2)
{
    if( masuratoare[nr_termen1] > 3.0 )
    {
        if( masuratoare[nr_termen2] > 3.0 )
        {
            if( masuratoare[nr_termen1] < masuratoare[nr_termen2] )
            {
                return nr_termen1;
            }
            else
            {
                return nr_termen2;
            }
        }
        else
        {
            return nr_termen1;
        }     
    }
    else if( masuratoare[nr_termen2] > 3.0 ) // daca masuratoarea precedenta este valida
    {
        return nr_termen2;
    }
    
    return 0;
}

//------------------------------Convertiri--------------------------------//
String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ;
}
