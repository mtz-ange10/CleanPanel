#include <Wire.h>
#include "RTClib.h"
#include <Preferences.h>
#include "BluetoothSerial.h"
#include <Arduino.h>

RTC_DS3231 rtc;
Preferences prefsCheck;
Preferences prefsClean;

// Sensado de limpieza
int cleanYesNo = 0;
int potPin = 36; //ADC1_0 VP
int valorADC = 0;
float voltajeMaxIdeal = 24, voltaje = 0, voltajeReal = 0;

// Variables para hora de chequeo
int alarmCheckHour;
int alarmCheckMinute;
int alarmCheckSecond;

// Variables para hora de limpieza
int alarmCleanHour;
int alarmCleanMinute;
int alarmCleanSecond;

// Pines I2C RTC
#define SDA_PIN 19
#define SCL_PIN 21

// Variables de control
bool cleanCycle = false;
bool activateMotors = false;

// Nebulizadores
const int nebu1 = 22;

// Puente H
const int pinENA = 13;
const int pinIN1 = 12;
const int pinIN2 = 14;
const int pinIN3 = 27;
const int pinIN4 = 26;
const int pinENB = 25;

// Bluetooth
String device_name = "CleanerMyPanel";
BluetoothSerial SerialBT;

// Función para mostrar hora programada de check
void showCheckAlarm() {
  Serial.printf("%02d:%02d:%02d\n", alarmCheckHour, alarmCheckMinute, alarmCheckSecond);
}

// Función para mostrar hora programada de limpieza
void showCleanAlarm() {
  Serial.printf("%02d:%02d:%02d\n", alarmCleanHour, alarmCleanMinute, alarmCleanSecond);
}

// Función de limpieza
void cleanPanel() {
  cleanCycle = true;
  delay(2000);
  while (cleanCycle) {
    // Nebulizadores
    digitalWrite(nebu1, LOW);
    Serial.println("Click");
    delay(500);
    digitalWrite(nebu1, HIGH);
    Serial.println("Des-click");
    delay(500);

    Serial.println("Nebulizing...");
    SerialBT.println("NEBULIZING");
    SerialBT.println(";");
    delay(50000); // limpiar: 300,000

    digitalWrite(nebu1, LOW);
    Serial.println("Click");
    delay(500);
    digitalWrite(nebu1, HIGH);
    Serial.println("Des-click");
    delay(500);

    Serial.println("Nebulized finishing");
    delay(5000);

    // Motores
    activateMotors = true;
    delay(200);

    if (activateMotors) {
      Serial.println("Brushing...");
      SerialBT.println("BRUSHING");
      SerialBT.println(";");

      // Adelante
      digitalWrite(pinIN3, LOW);
      digitalWrite(pinIN4, HIGH);
      analogWrite(pinENB, 450);
      digitalWrite(pinIN1, HIGH);
      digitalWrite(pinIN2, LOW);
      analogWrite(pinENA, 170);
      delay(47000);

      // Parar
      digitalWrite(pinIN3, LOW);
      digitalWrite(pinIN4, LOW);
      digitalWrite(pinIN1, LOW);
      digitalWrite(pinIN2, LOW);
      delay(5000);

      // Atrás
      digitalWrite(pinIN3, HIGH);
      digitalWrite(pinIN4, LOW);
      analogWrite(pinENB, 450);
      digitalWrite(pinIN1, LOW);
      digitalWrite(pinIN2, HIGH);
      analogWrite(pinENA, 170);
      delay(52000);

      // Parar
      digitalWrite(pinIN3, LOW);
      digitalWrite(pinIN4, LOW);
      digitalWrite(pinIN1, LOW);
      digitalWrite(pinIN2, LOW);
      delay(2000);

      Serial.println("Brushing finished");
      Serial.println("Process finished");
      SerialBT.println("CLEANFINISHED");
      SerialBT.println(";");
    }

    activateMotors = false;
    cleanCycle = false;
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  if (!rtc.begin()) {
    Serial.println("RTC not found");
    while (1);
  }

  // Ajusta hora solo la primera vez
  //if (!prefs.getBool("horaAjustada", false)) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    //prefs.putBool("horaAjustada", true);
    //Serial.println("Hora ajustada por primera vez.");
  //}

  // Cargar hora guardada o establecer por defecto
  if (prefsCheck.isKey("checkHour")) {
    alarmCheckHour = prefsCheck.getInt("checkHour");
    alarmCheckMinute = prefsCheck.getInt("checkMinute");
    alarmCheckSecond = prefsCheck.getInt("checkSecond");
  } else {
    alarmCheckHour = 12;
    alarmCheckMinute = 30;
    alarmCheckSecond = 0;

    prefsCheck.putInt("checkHour", alarmCheckHour);
    prefsCheck.putInt("checkMinute", alarmCheckMinute);
    prefsCheck.putInt("checkSecond", alarmCheckSecond);
  }

  // Cargar hora guardada o establecer por defecto
  if (prefsClean.isKey("cleanHour")) {
    alarmCleanHour = prefsClean.getInt("cleanHour");
    alarmCleanMinute = prefsClean.getInt("cleanMinute");
    alarmCleanSecond = prefsClean.getInt("cleanSecond");
  } else {
    alarmCleanHour = 21;
    alarmCleanMinute = 30;
    alarmCleanSecond = 0;

    prefsClean.putInt("cleanHour", alarmCleanHour);
    prefsClean.putInt("cleanMinute", alarmCleanMinute);
    prefsClean.putInt("cleanSecond", alarmCleanSecond);
  }

  Serial.println("Hora de chequeo actual:");
  showCheckAlarm();
  
  Serial.println("Hora de limpieza actual:");
  showCleanAlarm();
  
  // Configurar pines
  pinMode(nebu1, OUTPUT);
  pinMode(pinIN1, OUTPUT);
  pinMode(pinIN2, OUTPUT);
  pinMode(pinIN3, OUTPUT);
  pinMode(pinIN4, OUTPUT);
  pinMode(pinENA, OUTPUT);
  pinMode(pinENB, OUTPUT);

  SerialBT.begin(device_name);
  Serial.println("Bluetooth ready");
}

void loop() {
  // put your main code here, to run repeatedly:
  DateTime now = rtc.now();
  Serial.printf("%02d:%02d:%02d\n", now.hour(), now.minute(), now.second());
  delay(1000);

  if (now.hour() == alarmCheckHour && now.minute() == alarmCheckMinute && now.second() == 1){
      valorADC = analogRead(potPin);
      // Convertir a voltaje (0 - 3.3V)
      voltaje = (valorADC * 3.3) / 4095.0; //7.9 es el voltaje de conversión
      voltajeReal = voltaje * 7.9;

      Serial.print("Valor ADC: ");
      Serial.print(valorADC);
      Serial.print(" | Voltaje: ");
      Serial.print(voltaje, 3);
      Serial.print(" | Voltaje Real: ");
      Serial.println(voltajeReal, 3);

      if (voltajeReal <= (voltajeMaxIdeal * .9)){
        cleanYesNo = 1;
        Serial.println("Cleaning needed");
      } else {
        cleanYesNo = 0;
        Serial.println("Cleaning no-needed");
      }
  }
  
  // Ejecutar limpieza automática
  if (now.hour() == alarmCleanHour && now.minute() == alarmCleanMinute && now.second() == alarmCleanSecond) {
    if (cleanYesNo == 1){
      Serial.println("Starting automatically cleaning");
      cleanPanel();
      } else {
        Serial.println("Cleaning no-needed");
      }
  }

  // Comprobar mensajes Bluetooth
  if (SerialBT.available()) {
    String serialBTmessage = SerialBT.readStringUntil('\n');
    serialBTmessage.trim();
    Serial.println("Message received: " + serialBTmessage);

    // Enviar hora actual de checado
    if (serialBTmessage == "getcheckmemory") {
      String savedCheckHour = String(alarmCheckHour) + ":" + String(alarmCheckMinute);
      SerialBT.println("CHECKINGHOUR" + savedCheckHour);
      SerialBT.println(";");
    }
    
    // Enviar hora actual de limpieza
    if (serialBTmessage == "getcleanmemory") {
      String savedCleanHour = String(alarmCleanHour) + ":" + String(alarmCleanMinute);
      SerialBT.println("CLEANINGHOUR" + savedCleanHour);
      SerialBT.println(";");
    }

    if (serialBTmessage == "autoclean") {
      cleanPanel();
    }

    if (serialBTmessage.indexOf("newhourcheck") != -1) {
      Serial.println("New hour received for check");

      String horaYminCh = serialBTmessage.substring(String("newhourcheck").length());
      horaYminCh.trim();
      int posSeparadorCh = horaYminCh.indexOf(':');

      if (posSeparadorCh != -1) {
        String newHourCh = horaYminCh.substring(0, posSeparadorCh);
        String newMinuteCh = horaYminCh.substring(posSeparadorCh + 1);

        alarmCheckHour = newHourCh.toInt();
        alarmCheckMinute = newMinuteCh.toInt();
        alarmCheckSecond = 1;

        prefsCheck.putInt("checkHour", alarmCheckHour);
        prefsCheck.putInt("checkMinute", alarmCheckMinute);
        prefsCheck.putInt("checkSecond", alarmCheckSecond);

        Serial.print("New time for check recorded:");
        showCheckAlarm();
      }
    }

    if (serialBTmessage.indexOf("newhourclean") != -1) {
      Serial.println("New hour received for clean");

      String horaYmin = serialBTmessage.substring(String("newhourclean").length());
      horaYmin.trim();
      int posSeparador = horaYmin.indexOf(':');

      if (posSeparador != -1) {
        String newHour = horaYmin.substring(0, posSeparador);
        String newMinute = horaYmin.substring(posSeparador + 1);

        alarmCleanHour = newHour.toInt();
        alarmCleanMinute = newMinute.toInt();
        alarmCleanSecond = 1;

        prefsClean.putInt("cleanHour", alarmCleanHour);
        prefsClean.putInt("cleanMinute", alarmCleanMinute);
        prefsClean.putInt("cleanSecond", alarmCleanSecond);

        Serial.print("New time for clean recorded:");
        showCleanAlarm();
      }
    }

    if (serialBTmessage == "calibrate") {
      valorADC = analogRead(potPin);
      // Convertir a voltaje (0 - 3.3V)
      voltaje = (valorADC * 3.3) / 4095.0; //7.9 es el voltaje de conversión
      voltajeMaxIdeal = voltaje * 7.9;

      Serial.print("Valor ADC: ");
      Serial.print(valorADC);
      Serial.print(" | Voltaje: ");
      Serial.print(voltaje, 3);
      Serial.print(" | Voltaje Maximo Ideal: ");
      Serial.println(voltajeMaxIdeal, 3);
      SerialBT.println("Value calibrated");
      SerialBT.println(";");
    }

  }
  delay(200);
}
