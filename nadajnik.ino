#include <SPI.h>            // biblioteka SPI
#include "nRF24L01.h"       // bibiloteki do obsługi modułu radiowego
#include "RF24.h"
#include <TimerOne.h>
#include "DHT.h"          // biblioteka czujnika temperatury DHT
#define DHTPIN 3          // numer pinu sygnałowego
#define VOLTAGE 4.865     // Wartość zasilanie arduino
DHT dht;                  // definicja czujnika
float arr[5];                // tablica zawierająca przesyłane dane
float t_new;                 // biezaca wartosc temperatury
float h_new;                 // biezaca wartosc wilgotnosci
int gas_ain = A1;            // definicja pinu dla czujnika CO
RF24 radio(7, 8);
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL }; // definicja pinów oraz kanału dla modułu radiowego
// float Command = 0;
/* czujnik jakośći powietrza */
const int numReadings = 100;    // ilość próbek
int readings[numReadings];      // odczyt z wejscia analogowego
int readIndex = 0;              // indeks obecenego odczytu
long int total = 0;             // zmienna pomocnicza
int latest_reading = 0;         // ostatni odczyt
int average_reading = 0;        // wartość średnia
int dustPin = A0;               // numer wejscia analogowego
uint32_t timer = millis();

void setup()
{
  pinMode(gas_ain, INPUT);
  dht.setup(DHTPIN);      // inicjalizacja czujnika
  Serial.begin(57600);    // otwarcie portu szeregowego
  radio.begin();
  radio.setRetries(15, 15);
  radio.openReadingPipe(1, pipes[1]);
  radio.startListening();
  radio.printDetails();
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1, pipes[1]);
  radio.stopListening();          // konfiguracja modułu radiowego
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(2, INPUT_PULLUP);       // wejscia dla czujnika zapylenia

  // Timer 1 w 16-bit mode w celu wygenerowania trwającego 0.32ms niskiego stanu LED  co 10ms
  // A0 potrzebne do próbkowaniac co 0.28ms po zboczu opadającym stanu LED
  Timer1.initialize(10000); // ustawienie timera na 10000 mikrosekund(lub 10ms - lub 100Hz)
  Timer1.pwm(10, 991); // Ustawienie wysokiego PWM  (10 - 0.32) * 1024 = 991
  Timer1.pwm(9, 999); // Ustawienie wysokiego PWM (10 - 0.28) * 1024 = 995
  attachInterrupt(0, takeReading, RISING); // obsługa przerwania  INT0
  // Utworzenie buforu dla próbek
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
}
//Funkcja probkowania dla czujnika zapylenia
void takeReading() {
  total = total - readings[readIndex];
  latest_reading = analogRead(dustPin);
  readings[readIndex] = latest_reading;
  total = total + latest_reading;
  readIndex = readIndex + 1;
  if (readIndex >= numReadings) readIndex = 0;
  average_reading = total / numReadings;
}

void loop(void)
{
  delay(dht.getMinimumSamplingPeriod());  // Miniamalne opóźnienie odczytu
  float t = dht.getTemperature();
  float h = dht.getHumidity(); // Odczyt temperatury i wilgotności powietrza
  if (timer > millis())  timer = millis(); // reset timera
  if (millis() - timer > 1000) {
    timer = millis(); // reset timera
    float MIN_VOLTAGE = 0.550;
    float latest_dust = latest_reading * (VOLTAGE / 1023.0);
    float average_dust = average_reading * (VOLTAGE / 1023.0);
    float x = latest_dust + MIN_VOLTAGE;
    float xa = average_dust + MIN_VOLTAGE;
    float latest_dustu = 12.92 * pow(x, 3) - 89.35 * pow(x, 2) + 354.2 * pow(x, 1) - 254;
    float average_dustu = 12.92 * pow(xa, 3) - 89.35 * pow(xa, 2) + 354.2 * pow(xa, 1) - 254; // obliczanie wartości zapylenia

    float ad_value = analogRead(gas_ain); // odczyt wartosci stężenia CO

    arr[0] = average_dustu;
    arr[1] = h_new;
    arr[2] = t_new;
    arr[3] = ad_value; // podstawianie danych do tablicy

    radio.stopListening();
    radio.write( &arr, sizeof(arr) ); // wysłanie tablicy za pomocą modułu radiowego
    radio.startListening();

    Serial.print(arr[2]);
    Serial.print("*C  ");
    Serial.print(arr[1]);
    Serial.print("%  ");
    Serial.print(arr[3]);
    Serial.print("CO  ");
    Serial.print(arr[0]);
    Serial.println("ug"); // wypisanie wartości za pomocą portu szeregowego

    // Sprawdzanie poprawności odczytu temperatury i wilgotności
    if (dht.getStatus())
    {
      h_new = h_new;
      t_new = t_new;
    }
    else
    {
      h_new = h;
      t_new = t;
    }
  }
}

