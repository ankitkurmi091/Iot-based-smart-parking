/* 
IoT Based Smart Home
*/
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <time.h>
#include <HTTPClient.h>
#include <WebServer.h> 
#include <ESPmDNS.h> 
#include<ESP32Servo.h>
#include <ArduinoJson.h>
WebServer server(80); 
StaticJsonDocument<384>

// ********Global variables for webpage******
String id;
String enter_time_ris;
String exit_time_ris;
String enter_time_dev;
String exit_time_dev;
String total_time_ris;
String total_time_dev;
String slot_1;
String slot_2;
String slot_3;
String slot_4;
String slot_5;
String slot_6;
int ris_in;
int dev_in;
int ris_out;
int dev_out;

//****************************************
//#define ThermistorPin 35
//#define LDR_PIN       34
#define R1  10000
#define C1  (float)1.009249522e-03
#define C2  (float)2.378405444e-04
#define C3  (float)2.019202697e-07
String GOOGLE_SCRIPT_ID = "AKfycbx_8gecM8ZVzvbwo3m9K6BnkVuHcbOy335BUdOOgIVdq4j627Oo_6RFtgOwGzWDX19o"; 
//https://script.google.com/macros/s/AKfycbwhcnmC6cSE_P5MKice-T6GX8pfe4rFFNiKUnY9-vuwE9hwzCA/exec?tag=adc_A0&value=123
const int sendInterval = 5000; 
float avg[3]={0,0,0};

char light_array[7];
char temp_array[7];

//updated 04.12.2019
const char * root_ca=\
"your cartificate";

char auth[] = "Blynk Auth Token";
char ssid[] = "WiFi Name";
char pass[] = "WiFi Password";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

WiFiClientSecure client;
void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(  &timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}
//for servo ................................
Servo tap_servo;
Servo tap_servo1;
int sensor_pin = 36;
int tap_servo_pin =16;
int sensor1_pin=39;
int tap_servo1_pin =17;
int donir1;
int donir2;

//for rfid & timming ...........................
#include <WiFiUdp.h>
#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
#include <NTPClient.h>

const long utcOffsetInSeconds = 19800;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

#define RST_PIN         22        
#define SS_1_PIN        4         
#define SS_2_PIN        5         
#define NR_OF_READERS   2
byte ssPins[] = {SS_1_PIN, SS_2_PIN};

MFRC522 mfrc522[NR_OF_READERS];  
int i = 1;
byte code[10];
String uidString;
int count0 = 0;
int count1 = 0;

// for lcd .............................
#include <LiquidCrystal.h>
LiquidCrystal lcd(2, 12, 13, 14, 15, 25);  

//for ir ................................
BlynkTimer timer;
int IR_1 = 26;  
int IR_2 = 27;
int IR_3 = 32;
int IR_4 = 33;  
int IR_5 = 34;
int IR_6 = 35;

WidgetLED led1(V1);
WidgetLED led2(V2);
WidgetLED led3(V3);
WidgetLED led4(V4);
WidgetLED led5(V5);
WidgetLED led6(V6);

void setup(){
  
Serial.begin(115200);
WiFi.begin(ssid, pass);
Blynk.begin(auth, ssid, pass, IPAddress(13, 233, 230, 91), 8080);
//for servo .....................
  pinMode(sensor_pin,INPUT);
  tap_servo.attach(tap_servo_pin);
  pinMode(sensor1_pin,INPUT);
  tap_servo1.attach(tap_servo1_pin);
// for rfid & timming ...............
 SPI.begin();        
   Serial.println("RFID is ready for reading card");
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN); // Init each MFRC522 card
    Serial.print(F("Reader "));
    Serial.print(reader);
    Serial.print(F(": "));
    mfrc522[reader].PCD_DumpVersionToSerial();
  }
//init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

// for lcd ..................
lcd.begin(16, 2); 
lcd.setCursor (0,0);
lcd.print("  IOT  Parking  ");
lcd.setCursor (0,1);
lcd.print("     System     ");
delay (2000);
lcd.clear();

//for ir input ...............  
  pinMode(IR_1, INPUT);
  pinMode(IR_2,INPUT);
  pinMode(IR_3,INPUT);
  pinMode(IR_4,INPUT);
  pinMode(IR_5,INPUT);
  pinMode(IR_6,INPUT);

// for blynk slot ..................
  timer.setInterval(1000L, slot1 );
  timer.setInterval(1600L, slot2 );
  timer.setInterval(1500L, slot3 );
  timer.setInterval(1200L, slot4 );
  timer.setInterval(1500L, slot5 );
  timer.setInterval(1800L, slot6 );
  timer.setInterval(500L, web_server ); //**********
  timer.setInterval(1000L, time_update );
  timer.setInterval(1100L, servo1 );
  
 //****************************************
    Serial.println("\n\n\n\n\n\n\n");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("\n\n\n\n\n\n");
    if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }
  server.on("/", handleRoot);
  server.on("/data.json", data_json);
  server.onNotFound(handleNotFound);
  server.begin();
  HTTPClient http;
  Serial.println("HTTP server started");
  lcd.begin(16, 2); 
  lcd.setCursor (0,0);
  lcd.print(WiFi.localIP());
  delay (5000);
  lcd.clear();
//**********************************
}

void loop(){
//for rfid loop .................

  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    if(reader == 0){
      reader0(reader);
    }
    else if(reader == 1){
      reader1 (reader);
    }      
  }
 Blynk.run();
 timer.run();
 
 String lightpercentage = timeClient.getFormattedTime();
  //float temp = getTemperature();
  //String temp_s(temp);
  //String lightPer_s(lightpercentage);

  sendData("tag=adc_A0&value="+lightpercentage);
  delay(sendInterval);

}

//delay(3000);
void reader0(int reader){       
if ( ! mfrc522[reader].PICC_IsNewCardPresent()) 
  {
    return; }
 if ( ! mfrc522[reader].PICC_ReadCardSerial()) 
  {
    return;  }
//delay(3000);
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522[reader].uid.size; i++) {
     Serial.print(mfrc522[reader].uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522[reader].uid.uidByte[i], HEX);
     content.concat(String(mfrc522[reader].uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522[reader].uid.uidByte[i], HEX));
     //Serial.print("forrrr");
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase(); 
      count0++;
       if (content.substring(1) == "21 73 00 1D") {
    Serial.println("Authorized access");
    Serial.println("Car orange entered ..................");
    Serial.print("Car orange entrance timming......");
    printLocalTime();
    id = "RISHABH_IN";
    enter_time_ris =  String(timeClient.getHours()) +  String(":")+String(timeClient.getMinutes()) +String(":")+ String(timeClient.getSeconds());
     ris_in = timeClient.getMinutes()*60 + timeClient.getSeconds();
  }

   if (content.substring(1) == "60 26 4E 21") //our car id
  {
    Serial.println("car green enter ..................");
    Serial.print("car green entrance timming......");
    printLocalTime();
    id = "DEVANSH_IN";
    enter_time_dev =  String(timeClient.getHours()) +  String(":")+String(timeClient.getMinutes()) +String(":")+ String(timeClient.getSeconds());
     dev_in = timeClient.getMinutes()*60 + timeClient.getSeconds();
  }
   }  

void reader1 (int reader){
 if ( ! mfrc522[reader].PICC_IsNewCardPresent())   {
    return; }
 if ( ! mfrc522[reader].PICC_ReadCardSerial()) {
    return;
  }
  String content2= "";
  byte letter2;
  for (byte i = 0; i < mfrc522[reader].uid.size; i++) 
  {
     Serial.print(mfrc522[reader].uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522[reader].uid.uidByte[i], HEX);
     content2.concat(String(mfrc522[reader].uid.uidByte[i] < 0x10 ? " 0" : " "));
     content2.concat(String(mfrc522[reader].uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content2.toUpperCase();
      count0++;
   if (content2.substring(1) == "21 73 00 1D") //our car id 1
  {
    Serial.println("car orange exited ..................1");
    Serial.print("car orange exit timming.......");
    printLocalTime();
    id = "RISHABH_OUT";
    exit_time_ris =  String(timeClient.getHours()) +  String(":")+String(timeClient.getMinutes()) +String(":")+ String(timeClient.getSeconds());
   ris_out = timeClient.getMinutes()*60 + timeClient.getSeconds();
   total_time_ris = String(ris_out - ris_in);
  }

  if (content2.substring(1) == "60 26 4E 21") //our car id 2
  {
    Serial.println("car green exit ..................2");
    Serial.print("car green exiting timing........");
    printLocalTime();
    id = "DEVANSH_OUT";
    exit_time_dev =  String(timeClient.getHours()) +  String(":")+String(timeClient.getMinutes()) +String(":")+ String(timeClient.getSeconds());
    dev_out = timeClient.getMinutes()*60 + timeClient.getSeconds();
    total_time_dev = String(dev_out - dev_in);
  }
}  

// for servo ,,,,,,,,,,,,,,,,,,,
void servo1 (){
  donir1 = digitalRead(sensor_pin);
  if (donir1==0)
  {
//   delay(3000);
   tap_servo.write(0); 
  }
  if (donir1==1)
  {
//    delay(3000);
    tap_servo.write(180);
   }
 donir2 = digitalRead(sensor1_pin);
 if (donir2==0)
  {
//    delay(3000);
   tap_servo1.write(0); 
  }
  if (donir2==1)
  {
//    delay(3000);
    tap_servo1.write(180);
   }
  } 
  
// for IR 1 ..........................................
void slot1(){
  int ir1 = digitalRead(IR_1);
  if (ir1 == HIGH) {
    led1.on();
    lcd.setCursor (0,0);
    lcd.print("1-F");
    Serial.println("ir1 is high");
    slot_1 = "UNBOOKED";
  }
  else {
    led1.off();
    lcd.setCursor (0,0);
    lcd.print("1-E");
    Serial.println("ir1 is low");
    slot_1 = "BOOKED";
  }
}

// for IR 2 ..........................................
void slot2(){
  int ir2 = digitalRead(IR_2);
  if (ir2 == HIGH) {
    led2.on();
    lcd.setCursor (5,0);
    lcd.print("2-F");
    Serial.println("ir2 is high");
    slot_2 = "UNBOOKED";
  }
  else {
    led2.off();
    lcd.setCursor (5,0);
    lcd.print("2-E");
    Serial.println("ir2 is low");
    slot_2 = "BOOKED";
  }
}

//for IR3 ..............................................
void slot3(){
  int ir3 = digitalRead(IR_3);
  if (ir3 == HIGH) {
    led3.on();
    lcd.setCursor (10,0);
    lcd.print("3-F");
    Serial.println("ir3 is high");
    slot_3 = "UNBOOKED";
}
  else {
    led3.off();
     lcd.setCursor (10,0);
     lcd.print("3-E");
     Serial.println("ir3 is low");
     slot_3 = "BOOKED";
 }
}
/// for IR 4 ..........................................
void slot4(){
  int ir4 = digitalRead(IR_4);
  if (ir4 == HIGH) {
    led4.on();
    lcd.setCursor (0,1);
    lcd.print("4-F");
    Serial.println("ir 4 is high");
    slot_4 = "UNBOOKED";
  }
  else {
    led4.off();
    lcd.setCursor (0,1);
    lcd.print("4-E");
    Serial.println("ir4 is low");
    slot_4 = "BOOKED";
  }
}

//// for IR 5 ..........................................
void slot5(){
  int ir5 = digitalRead(IR_5);
  if (ir5 == HIGH) {
    led5.on();
    lcd.setCursor (5,1);
    lcd.print("5-F");
    Serial.println("ir5 is high");
   slot_5 = "UNBOOKED";
  }
  else {
    led5.off();
    lcd.setCursor (5,1);
    lcd.print("5-E");
    Serial.println("ir5 is low");
    slot_5 = "BOOKED";
  }
}

////for IR 6 ..............................................
void slot6(){
  int ir6 = digitalRead(IR_6);
  if (ir6 == HIGH) {
    led6.on();
    lcd.setCursor (10,1);
    lcd.print("6-F");
    lcd.setCursor (14,0);
    lcd.print("↑");
    lcd.setCursor (14,1);
    lcd.print("|");
    Serial.println("ir6 is high");
    slot_6 = "UNBOOKED";
  }
  else {
    led6.off();
    lcd.setCursor (10,1);
    lcd.print("6-E");
    Serial.println("ir6 is low");
    slot_6 = "BOOKED";
  }
}


//********************************************************
void time_update(){
  
  timeClient.update();
  }

void handleRoot() {

  server.send(200, "text/html", "<html>  <head> <title>Parking Dashboard </title>  <style> #details { margin: 0px; position: absolute; left: 1250; }  h1 { background-color: rgb(4, 60, 82); font-family: Arial, Helvetica, sans-serif; margin: 0px; text-align: center; color: rgb(230, 225, 225); }  #slots-1 { width: 65%; height: 50%; background-color: darkcyan; display: flex; flex-direction: row; }  #slots-2 { width: 65%; height: 50%; background-color: darkcyan; display: flex; flex-direction: row; }  .slot1 { width: 230px; height: 350px; background-color: rgb(4, 60, 82); border: 2px solid rgb(17, 13, 13); margin: 65px; display: flex; flex-direction: row; padding: 10px; position: relative; }  .inner1 { width: 230px; height: 50px; background-color: rgb(68, 146, 53); border: 2px solid rgb(17, 13, 13); margin-top: 300px; display: flex; flex-direction: row; justify-content: center; position: absolute; }  .circle1 { width: 400px; height: 230px; background-color: darkgreen; margin-right: 10px; border-radius: 50%; position: relative; right: -1%; top: 15%; }  .circle-1 { margin-top: 44%; text-align: center; justify-content: center; justify-items: center; align-content: center; align-items: center; }  .slot2 { width: 230px; height: 350px; background-color: rgb(4, 60, 82); border: 2px solid rgb(12, 17, 10); margin: 65px; display: flex; flex-direction: row; padding: 10px; }  .inner2 { width: 230px; height: 50px; background-color: rgb(68, 146, 53); border: 2px solid rgb(17, 13, 13); margin-top: 300px; display: flex; flex-direction: row; justify-content: center; position: absolute; }  .circle2 { width: 400px; height: 230px; background-color: darkgreen; margin-right: 10px; border-radius: 50%; position: relative; right: -1%; top: 15%; }  .circle-2 { margin-top: 44%; text-align: center; justify-content: center; justify-items: center; align-content: center; align-items: center; }  .slot3 { width: 230px; height: 350px; background-color: rgb(4, 60, 82); border: 2px solid rgb(12, 17, 10); margin: 65px; display: flex; flex-direction: row; padding: 10px; }  .inner3 { width: 230px; height: 50px; background-color: rgb(68, 146, 53); border: 2px solid rgb(17, 13, 13); margin-top: 300px; display: flex; flex-direction: row; justify-content: center; position: absolute; }  .circle3 { width: 400px; height: 230px; background-color: darkgreen; margin-right: 10px; border-radius: 50%; position: relative; right: -1%; top: 15%; }  .circle-3 { margin-top: 44%; text-align: center; justify-content: center; justify-items: center; align-content: center; align-items: center; }  .slot4 { width: 230px; height: 350px; background-color: rgb(4, 60, 82); border: 2px solid rgb(12, 17, 10); margin: 65px; display: flex; flex-direction: row; padding: 10px; }  .inner4 { width: 230px; height: 50px; background-color: rgb(68, 146, 53); border: 2px solid rgb(17, 13, 13); margin-top: 300px; display: flex; flex-direction: row; justify-content: center; position: absolute; }  .circle4 { width: 400px; height: 230px; background-color: darkgreen; margin-right: 10px; border-radius: 50%; position: relative; right: -1%; top: 15%; }  .circle-4 { margin-top: 44%; text-align: center; justify-content: center; justify-items: center; align-content: center; align-items: center; }  .slot5 { width: 230px; height: 350px; background-color: rgb(4, 60, 82); border: 1px solid rgb(12, 17, 10); margin: 65px; display: flex; flex-direction: row; padding: 10px; }  .inner5 { width: 230px; height: 50px; background-color: rgb(68, 146, 53); border: 2px solid rgb(17, 13, 13); margin-top: 300px; display: flex; flex-direction: row; justify-content: center; position: absolute; }  .circle5 { width: 400px; height: 230px; background-color: darkgreen; margin-right: 10px; border-radius: 50%; position: relative; right: -1%; top: 15%; }  .circle-5 { margin-top: 44%; text-align: center; justify-content: center; justify-items: center; align-content: center; align-items: center; }  .slot6 { width: 230px; height: 350px; background-color: rgb(4, 60, 82); border: 2px solid rgb(12, 17, 10); margin: 65px; display: flex; flex-direction: row; padding: 10px; }  .inner6 { width: 230px; height: 50px; background-color: rgb(68, 146, 53); border: 2px solid rgb(17, 13, 13); margin-top: 300px; display: flex; flex-direction: row; justify-content: center; position: absolute; }  .circle6 { width: 400px; height: 230px; background-color: darkgreen; margin-right: 10px; border-radius: 50%; position: relative; right: -1%; top: 15%; }  .circle-6 { margin-top: 44%; text-align: center; justify-content: center; justify-items: center; align-content: center; align-items: center; } </style> </head>  <body> <div class='container' id='cont'>  <h1>CAR PARKING LOT</h1>  <div class='slots' id='slots-1'>  <div id='details'> <h2> LOADING...</h2>     </div> <div class='slot1'> <div class='inner1'> Slot-1</div> <div class='circle1' id='circle1'> <h3 class='circle-1' id='circ1'>sLoading</h3> </div> </div> <div class='slot2'> <div class='inner2'>Slot-2</div> <div class='circle2'> <h3 class='circle-2' id='circ2'> Loading </h3> </div> </div> <div class='slot3'> <div class='inner3'>Slot-3</div> <div class='circle3'> <h3 class='circle-3' id='circ3'>Loading </h3> </div> </div> </div> <div class='slots' id='slots-2'> <div class='slot4'> <div class='inner4'>Slot-4</div> <div class='circle4'> <h3 class='circle-4' id='circ4'> Slot unbooked </h3> </div> </div> <div class='slot5'> <div class='inner5'>Slot-5</div> <div class='circle5'> <h3 class='circle-5' id='circ5'> Slot unbooked </h3> </div> </div> <div class='slot6'> <div class='inner6'>Slot-6</div> <div class='circle6'> <h3 class='circle-6' id='circ6'> Slot unbooked </h3> </div> </div> </div> <script> let res;  function foo() { var request = new XMLHttpRequest(); request.open('GET', '/data.json', false); request.send(null); res = request.responseText;  document.getElementById('circ1').innerText = JSON.parse(res)['slot1'];  document.getElementById('circ2').innerText = JSON.parse(res)['slot2'];  document.getElementById('circ3').innerText = JSON.parse(res)['slot3'];  document.getElementById('circ4').innerText = JSON.parse(res)['slot4'];  document.getElementById('circ5').innerText = JSON.parse(res)['slot5'];  document.getElementById('circ6').innerText = JSON.parse(res)['slot6'];   if (JSON.parse(res)['id'] == '') {  enter_time = JSON.parse(res)['enter_time_ris']; let new_html = '<h2>NO CARS PRESENT</h2>';  document.getElementById('details').innerHTML = new_html; };  if (JSON.parse(res)['id'] == 'RISHABH_IN') {  enter_time = JSON.parse(res)['enter_time_ris']; let new_html = '<h2>        CAR ENTERY     </h2><h3> CAR MODEL: TESLA MODEL S</h3><br><h3> NUMBER: MP15FC6969</h3><br><h3> OWNER : RISHABH TIWARI</h3><br><h3> ENTRY TIME :' + enter_time + '</h3>';  document.getElementById('details').innerHTML = new_html; };  if (JSON.parse(res)['id'] == 'DEVANSH_IN') {  enter_time = JSON.parse(res)['enter_time_dev']; let new_html = '<h2>        CAR ENTERY     </h2><h3> CAR MODEL: FERRARI F8</h3><br><h3> NUMBER: MP69F0707</h3><br><h3> OWNER : DEVANSH PATEL</h3><br><h3> ENTRY TIME :' + enter_time + '</h3>';  document.getElementById('details').innerHTML = new_html; };      if (JSON.parse(res)['id'] == 'RISHABH_OUT') {  exit_time = JSON.parse(res)['exit_time_ris']; enter_time = JSON.parse(res)['enter_time_ris']; total_time = JSON.parse(res)['total_time_ris'] / 60;  let new_html = '<h2>        CAR EXIT     </h2><h3> CAR MODEL: TESLA MODEL S</h3><br><h3> NUMBER: MP15FC6969</h3><br><h3> OWNER : RISHABH TIWARI</h3><br><h3> ENTRY TIME :' + enter_time + '</h3><br><h3> EXIT TIME : ' + exit_time + '</h3><br><h3>TOTAL TIME :' + total_time + ' MINUTES' + '</h3><br><h3> TOTAL PAY :' + total_time * 2 + '</h3><br><h3> PAYMENT MODE : FASTCARD</h3>';  document.getElementById('details').innerHTML = new_html; };  if (JSON.parse(res)['id'] == 'DEVANSH_OUT') {  exit_time = JSON.parse(res)['exit_time_dev']; enter_time = JSON.parse(res)['enter_time_dev']; total_time = JSON.parse(res)['total_time_dev'] / 60;  let new_html = '<h2>        CAR EXIT     </h2><h3> CAR MODEL: FERRARI F8 </h3><br><h3> NUMBER: MP69Fk0707</h3><br><h3> OWNER : DEVANSH PATEL</h3><br><h3> ENTRY TIME :' + enter_time + '</h3><br><h3> EXIT TIME : ' + exit_time + '</h3><br><h3>TOTAL TIME :' + total_time + ' MINUTES' + '</h3><br><h3> TOTAL PAY :' + total_time * 2 + '</h3><br><h3> PAYMENT MODE : FASTCARD</h3>';  document.getElementById('details').innerHTML = new_html; }; } setTimeout(() => { foo() }, 1000); setInterval(() => { foo() }, 2000); </script> </body>  </html> ");

}

void data_json() {

doc["slot1"] = slot_1;
doc["slot2"] = slot_2;
doc["slot3"] =  slot_3;
doc["slot4"] = slot_4;
doc["slot5"] = slot_5;
doc["slot6"] = slot_6;
doc["id"] = id;
doc["enter_time_ris"] = enter_time_ris;
doc["exit_time_ris"] = exit_time_ris;
doc["enter_time_dev"] = enter_time_dev;
doc["exit_time_dev"] = exit_time_dev;
doc["total_time_ris"] = total_time_ris;
doc["total_time_dev"] = total_time_dev;

String json_data;

serializeJson(doc, json_data);
server.send(200, "application/json", json_data);

}

void handleNotFound() {

  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);

}

void web_server(){
  server.handleClient();
  }

  void sendData(String params) {
   HTTPClient http;
   String url="https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?"+params;
   Serial.print(url);
    Serial.print("Making a request");
    http.begin(url, root_ca); //Specify the URL and certificate
    int httpCode = http.GET();  
    http.end();
    Serial.println(": done "+httpCode);
}
