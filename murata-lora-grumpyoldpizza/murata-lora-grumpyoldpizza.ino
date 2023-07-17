/*
*   Advanced coding using the grumpyOldPizza Tlera board
*   
*   https://github.com/GrumpyOldPizza/ArduinoCore-stm32l0
*
*   Which is then replaced by either 
*
*   https://github.com/facchinm/ArduinoCore-stm32l0/tree/vision_shield
*
*   or my version with a few examples at
*
*   https://github.com/hpssjellis/ArduinoCore-stm32l0/tree/vision_shield
*
*   The bridge program needs to be flashed on a PortentaH7 M7 core to allow using the board as an STM32L0 target.
*   Once flashed using the PortentaH7 core, you can switch the board to the Tlera --> protenta Vision Shield.
*   Presently this part only works well on Linux.
*   Note: Do not double press button into bootloader mode, must just be in regular mode until back to Portenta Core!
*
*   Serial works fine but eventually you need to flash the generic serial reader back onto the portenta to communicate 
*   This example code is in the public domain.
*/

#include "LoRaRadio.h"
#include "TimerMillis.h"

String message = "";

int receivedACK = 0;
int packageSent = 0;
int previous_bpm = 0;
int previous_ox_sat = 0;
int current_bpm;
int current_ox_sat;
int nivel;

int ox_sat_levels[4] = {96, 94, 92};
float bpm_level1[2] = {51, 90};
float bpm_level2[2] = {41, 110};
float bpm_level3 = 130;

static void myReceive(void);

TimerMillis myReceivePrintTimer, mySendTimer, mySendPrintTimer;
long lastSendTime = 0;
int interval = 1000;
int messageLen;

bool mySendThreadFree = true;
bool myReceiveThreadFree = true;

String mySendString, myReceiveString;

int myIncomingInt ;
int myRssi     ;
int mySnr      ;

using namespace std;

void sender(void){
  if (message.length() > 0){
    messageLen = message.length()+1;
    char mySendArray[messageLen];
  
    message.toCharArray(mySendArray, messageLen);

    mySendPrintIt();
    
    LoRaRadio.beginPacket();  
    LoRaRadio.write(mySendArray, messageLen);   
    LoRaRadio.endPacket();
  }
}


void mySendPrintIt(void) {
  Serial.println("Packages Sent.. " + String(packageSent));
  //Serial.println("Len:"+ String(message.length()));  
  //Serial.println(message);
  message = ""; 
}

void sendMessageLoRa(void) {
  
  if ((myReceiveString.substring(0,1)).equals("S")) {
    //Message from sensor
    String aux = myReceiveString.substring(2);
    String bpm = aux.substring(0,aux.indexOf("-"));
    String ox_sat = aux.substring(aux.indexOf("-")+1);

    if (bpm.toFloat() > bpm_level3 ) {
      //patient is under level 3
      //Serial.println("Level 3 bpm");
      current_bpm = 3;
    }else if (bpm.toFloat() < bpm_level2[0] or bpm.toFloat() >= bpm_level2[1]) {
      //patient is under level 2
      current_bpm = 2;
      //Serial.println("Level 2 bpm");
    }else if (bpm.toFloat() < bpm_level1[0] or bpm.toFloat() > bpm_level1[1]) {
      //patient is under level 1
      current_bpm = 1;
      //Serial.println("Level 1 bpm");
    }else {
      current_bpm = 0;
      //Serial.println("Level 0 bpm");
    }

    if (ox_sat.toInt() <= ox_sat_levels[2]) {
      //patient is under level 3
      current_ox_sat = 3;
      //Serial.println("Level 1 So2");
    }else if (ox_sat.toInt() <= ox_sat_levels[1]) {
      //patient is under level 2
      current_ox_sat = 2;
      //Serial.println("Level 2 So2");
    }else if (ox_sat.toInt() <= ox_sat_levels[0]) {
      //patient is under level 1
      current_ox_sat = 1;
      //Serial.println("Level 3 So2");
    }else {
      //patient is under level 0
      current_ox_sat = 0;
      //Serial.println("Level 0 So2");
    }
    
    Serial.println(previous_bpm);
    Serial.println(current_bpm);
    Serial.println(previous_ox_sat); 
    Serial.println(current_ox_sat);
    
    if (current_bpm != previous_bpm or current_ox_sat != previous_ox_sat) {

      if (current_bpm >= current_ox_sat) {
        nivel = current_bpm;
      }else {
        nivel = current_ox_sat;
      }
      
      //Must send LoRa message
      previous_bpm = current_bpm;
      previous_ox_sat = current_ox_sat;

      if (bpm.toFloat() < 100.0) {
        bpm = bpm + "0";
        //Serial.println(bpm);
      }

      message = "F-" + bpm + "-" + ox_sat.toInt() + "-" + nivel;
      //Serial.println(message);
      //message = "F-122.69-94.00-3";

      packageSent++;
      
    }
     
  }else if ((myReceiveString.substring(0,1)).equals("G")) {
    receivedACK++;
  }
}

void myReceivePrintIt(void){       
  Serial.println("ACK received.. " + String(receivedACK));
  //Serial.println("parsePacket:"+String(myIncomingInt) + ", RSSI:" + String(myRssi)+", SNR:" + String(mySnr) );
    
  LoRaRadio.receive();
  myReceiveString = "";
}

static void myReceive(void){
  //Serial.println("thread "+String(mySendThreadFree));
  
  if (mySendThreadFree){
    myReceiveThreadFree = false;

   myIncomingInt = LoRaRadio.parsePacket();   // must grab before read! 
   myRssi     = LoRaRadio.packetRssi();
   mySnr      = LoRaRadio.packetSnr();

   while (LoRaRadio.available() ) {
     myReceiveString.concat( (char)LoRaRadio.read());  // could be BYTE
   }
   
   Serial.println(myReceiveString);
   
   sendMessageLoRa();
   myReceivePrintIt();
   
   myReceiveThreadFree = true;
   
  }

  return;
}


void setup( void ){
    Serial.begin(9600);
        
    while (!Serial) { }   // non-blocking for the murata module on the Portenta

    LoRaRadio.begin(915000000);
    LoRaRadio.setFrequency(915000000);
    LoRaRadio.setTxPower(1);    //default 14
    LoRaRadio.setBandwidth(LoRaRadio.BW_125);
    LoRaRadio.setSpreadingFactor(LoRaRadio.SF_7);
    LoRaRadio.setCodingRate(LoRaRadio.CR_4_5);
    LoRaRadio.setLnaBoost(true);
   
    LoRaRadio.onReceive(myReceive);  // just telling it about the callback
    LoRaRadio.receive();            // is zero infinite, other upto milliseconds
}



void loop(void){
  if (millis() - lastSendTime > interval) {
    sender();
    lastSendTime = millis();
    interval = 1500;
    LoRaRadio.receive();
  }
}
