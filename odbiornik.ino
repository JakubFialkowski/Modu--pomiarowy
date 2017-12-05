#include "nRF24L01.h"  //bibiloteka dla modułu radiowego
#include "RF24.h"
#include <SPI.h>      //bibiloteka SPI
#include <Wire.h>     //bibiloteka wire
#include <LiquidCrystal_I2C.h>   //bibiloteka obsługi wyświetlacza LCD

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); // definicja wyświetlacza LCD
// defnicja modułu radiowego
RF24 radio(7, 8);
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
typedef enum { role_ping_out = 1, role_pong_back } role_e;
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};
role_e role = role_pong_back;

void setup(void)
{
  lcd.begin(20, 4); // określenie wielkości wyświetlacza LCD
  Serial.begin(57600); // otwarcie transmisji szeregowej
  // initializacja modułu radiowego
  radio.begin();
  radio.setRetries(15, 15);
  radio.openReadingPipe(1, pipes[1]);
  radio.startListening();
  radio.printDetails();
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1, pipes[0]);
  radio.startListening();
}

void loop(void)
{

  if ( radio.available() )
  {
    float arr[5]; // tablica przesyłanych danych
    
    radio.read( &arr, sizeof(arr) ); //odczyt tablicy
    // wyświetlenie danych za pomocą portu szeregowego
    Serial.print(arr[2]);
    Serial.print("*C  ");
    Serial.print(arr[1]);
    Serial.print("%  ");
    Serial.print(arr[0]);
    Serial.println("ug");
    //wyświetlenie danych za pomocą wyświetlacza LCD
    lcd.setCursor(0, 0);
    lcd.print("Temperatura");
    lcd.setCursor(13, 0);
    lcd.print(arr[2], 0);
    lcd.print("*C");
    lcd.setCursor(0, 1);
    lcd.print("Wilgotnosc");
    lcd.setCursor(13, 1);
    lcd.print(arr[1], 0);
    lcd.print("%");
    lcd.setCursor(0, 3);
    lcd.print("CO");
    lcd.setCursor(13, 3);
    lcd.print(arr[3], 0);
    lcd.print("ppm");
    lcd.setCursor(0, 2);
    lcd.print("Zapylenie");
    lcd.setCursor(13, 2);
    lcd.print(arr[0], 0);
    lcd.print("ug/m3");
    delay(20);
  }
}

