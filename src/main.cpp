#include <Arduino.h>
#include "DHT.h"
#include <math.h>
#define DHTTYPE DHT11
#include <DHT.h>

const int PIN_TEMP = 16;
const int PIN_BUTTON = 17;
const int PIN_TRANS = 18;

float referenz_temp = 0;
float aktuelle_temp = 0;
float abweichung = 2;
bool referenz_gesetzt = false;
bool aktuell_button = false;
bool letzter_button = HIGH;

unsigned long start_zeit = 0; 
unsigned long dauer_buzzer = 4000;
bool led_an = false;
bool alarm_ausgeloest = false;

DHT dht(PIN_TEMP, DHTTYPE);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(500);

  Serial.println("System wird gestartet.");

  dht.begin();

  // Input und output Pins initialisieren
  pinMode(PIN_BUTTON, INPUT_PULLUP); // damit gilt: nicht gedrückt=HIGH, gedrückt=LOW
  pinMode(PIN_TRANS, OUTPUT);

  // Buzzer erstmal low, da Buzzer am transistor hängt, diesen erstmal nicht leiten lassen, also Basis auf LOW
  digitalWrite(PIN_TRANS, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  // aktuellen Button Zustand speichern
  aktuell_button = digitalRead(PIN_BUTTON);

  if (aktuell_button == LOW && letzter_button == HIGH) {
    float temp = dht.readTemperature();

    // Falls die Referenztemperatur quatsch ist, loop neu starten und nochmal WErt suchen
    if (temp < 5 || temp > 40 || isnan(temp)) {
      Serial.println("Temperaturwert ist Müll, evtl. Sensorfehler");
    } else {
      referenz_temp = temp;
      referenz_gesetzt = true;
      alarm_ausgeloest = false; 
      led_an = false;
      digitalWrite(PIN_TRANS, LOW);

      Serial.print("Referenz Temperatur gespeichert: ");
      Serial.println(referenz_temp);
    }
  }

  // Wenn referenz temp gesetzt ist, kann aktuelle temperatur gemessen werden
  if (referenz_gesetzt) {
    float temp = dht.readTemperature();

    if (temp < 5 || temp > 40 || isnan(temp)) {
      Serial.println("Temperatur ist Müll.");
    } else {
      aktuelle_temp = temp;

      Serial.print("Aktuelle Temperatur beträgt: ");
      Serial.println(aktuelle_temp);

      // Wenn aktuelle Temperatur um 2 Grad sinkt, genug gelüftet, Buzzer an, nur wenn buzzer noch nicht an war
      if (aktuelle_temp <= referenz_temp - abweichung && !led_an && !alarm_ausgeloest) {
        digitalWrite(PIN_TRANS, HIGH);
        start_zeit = millis();
        led_an = true;
      }

      if (led_an && millis() - start_zeit >= dauer_buzzer) {
        // ansonsten buzzer wieder aus
        digitalWrite(PIN_TRANS, LOW);
        led_an = false;

        // variable die mir speichert ob schon ausgelöst wurde
        alarm_ausgeloest = true;
      }
    }
  }

  letzter_button = aktuell_button;

  // nur alle 2 Sekunden messen
  delay(2000);
}