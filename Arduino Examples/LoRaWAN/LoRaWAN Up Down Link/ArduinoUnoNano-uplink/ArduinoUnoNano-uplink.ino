#include <rn2xx3.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3); // RX, TX

rn2xx3 myLora(mySerial);
  
int sensorPin = 10;

void setup(){

  // Custom
  pinMode(sensorPin, INPUT);
  pinMode(9, OUTPUT);

  //output LED pin
  pinMode(13, OUTPUT);
  led_on();

  Serial.begin(9600); //serial port to computer
  mySerial.begin(57600); //serial port to radio
  Serial.println("Startup");

  initialize_radio();                        

  myLora.tx("KT LoRa Mote Test Ping");

  led_off();
  delay(2000);
}

void initialize_radio(){
  //reset rn2483
  pinMode(12, OUTPUT);
  digitalWrite(12, LOW);
  delay(500);
  digitalWrite(12, HIGH);

  delay(100); //wait for the RN2xx3's startup message
  mySerial.flush();

  //Autobaud the rn2483 module to 9600. The default would otherwise be 57600.
  myLora.autobaud();
  myLora.setDR(0); // sf12, bw 125khz, bps 250 

  //check communication with radio
  String hweui = myLora.hweui();
  while(hweui.length() != 16){
    Serial.println("Communication with RN2xx3 unsuccessful. Power cycle the board.");
    Serial.println(hweui);
    delay(10000);
    hweui = myLora.hweui();
  }

  //print out the HWEUI so that we can register it via ttnctl
  Serial.println("When using OTAA, register this DevEUI: ");
  Serial.println(myLora.hweui());
  Serial.println("RN2xx3 firmware version:");
  Serial.println(myLora.sysver());

  //configure your keys and join the network
  Serial.println("Trying to join loraserver");
  bool join_result = false;

  /*
   * ABP: initABP(String addr, String AppSKey, String NwkSKey);
   * Paste the example code from the TTN console here:
   */

  // Keys Node 2
  const char *devAddr = "067edabb";
  const char *nwkSKey = "13ed40fbea439c6223bc359a2894ef39";
  const char *appSKey = "7165c75fd637f75b0511f735e1a86713";

  join_result = myLora.initABP(devAddr, appSKey, nwkSKey);

  /*
   * OTAA: initOTAA(String AppEUI, String AppKey);
   * If you are using OTAA, paste the example code from the TTN console here:
   */
  //const char *appEui = "70B3D57ED00001A6";
  //const char *appKey = "A23C96EE13804963F8C2BD6285448198";

  //join_result = myLora.initOTAA(appEui, appKey);

  while(!join_result){
    Serial.println("Unable to join. Are your keys correct, and do you have TTN coverage?");
    delay(60000); //delay a minute before retry
    join_result = myLora.init();
  }
  Serial.println("Successfully joined TTN");

}

// the loop routine runs over and over again forever:
void loop(){
    led_on();

    int sensor= digitalRead(sensorPin);;
    String data = "0004a30b001fecea,";
    data += sensor;

    Serial.println("TXing " + data);
    Serial.println(myLora.txCnf(data)); //one byte, blocking function

    String received = myLora.getRx();
    received = myLora.base16decode(received);
    Serial.println("Received downlink: " + received);

    led_off();
    delay(5000);
}

void led_on(){
  digitalWrite(13, 1);
}

void led_off(){
  digitalWrite(13, 0);
}

