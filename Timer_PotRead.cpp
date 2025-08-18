#include <Wire.h>
#include "RTClib.h"

RTC_DS3231 rtc;

// Pines I2C RTC
#define SDA_PIN 19
#define SCL_PIN 21

int potPin = 36; //ADC1_0 VP
int valorADC = 0;
float voltaje = 0;

void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:
  Wire.begin(SDA_PIN, SCL_PIN);

  if (!rtc.begin()) {
    Serial.println("RTC not found");
    while (1);
  }

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

}

void loop() {
  // put your main code here, to run repeatedly:
  DateTime now = rtc.now();
  Serial.printf("%02d:%02d:%02d\n", now.hour(), now.minute(), now.second());
  delay(1000);

  int segundo = now.second();
  int resSegundo = segundo % 5;

  if (resSegundo == 0){
  valorADC = analogRead(potPin);
  // Convertir a voltaje (0 - 3.3V)
  voltaje = (valorADC * 3.3) / 4095.0;

  Serial.print("Valor ADC: ");
  Serial.print(valorADC);
  Serial.print(" | Voltaje: ");
  Serial.println(voltaje, 2); // 2 decimales
  }

  delay(500);
}
