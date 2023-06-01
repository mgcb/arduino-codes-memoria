#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include "ArduinoLowPower.h"
#include "RTClib.h"

RTC_DS3231 rtc;

#define REPORTING_PERIOD_MS     10000

int counter = 1;
String mensaje;
float bit_rate;
float ox_sat; 

uint32_t tsLastReport = 0;

PulseOximeter pox;

void digitalClockDisplay(){
  DateTime now = rtc.now();
  
  // digital clock display of the time
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
}

/*void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}*/

void onBeatDetected() {
  Serial.println("Beat!");
}

void send_lora(float beat_rate, float ox_sat) {
  
  mensaje = "(" + String(counter) + ")" + String(pox.getHeartRate()) + "-" + String(pox.getSpO2());

  if (beat_rate != 0 and ox_sat != 0) {
      Serial.println("Enviando medición:");
      Serial.println(mensaje);
    
      // send packet
      LoRa.beginPacket();
      LoRa.print("S-");
      LoRa.print(beat_rate);
      LoRa.print("-");
      LoRa.println(ox_sat);
      LoRa.endPacket();

      digitalClockDisplay();

      counter++;
    
    } else {
      Serial.println("Medición inválida");
    }
}


void setup() {
  if (!LoRa.begin(915E6)) {
    Serial.println("LoRa failed to initialize");
    while (1);
  }
  
  Serial.begin(115200);
  while (Serial.available());
  Serial.println("Initializing LoRa Sensorial Node...");

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Comprobando que instancia para Pulsioxímetro se inció correctamente
  if (!pox.begin()) {
        Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
    }

  //Callback para detección de latido
  pox.setOnBeatDetectedCallback(onBeatDetected);
  
}

void loop() {
  pox.update();

  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
  
    send_lora(pox.getHeartRate(), pox.getSpO2());
   
    tsLastReport = millis();
  }

  /*Serial.println("Starting sleep mode");
  LowPower.deepSleep(1000);
  digitalWrite(LORA_IRQ_DUMB, LOW);
  digitalWrite(LORA_RESET, LOW);
  digitalWrite(LORA_BOOT0, LOW);*/
}
